#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>

#define DEFAULTBODIES 100
#define DT 1
#define DEFAULTTIMESTEPS 1000
const double G = 0.0000000000667;

void calculateForces();
void initBodies();
void* work();
void moveBodies();
void start_clock(void);
void end_clock();
void insertIntoTree();
void initChildren();
static clock_t st_time;
static clock_t en_time;
static struct tms st_cpu;
static struct tms en_cpu;
int numberOfBodies;
int numberOfTimesteps;

typedef struct vector{
  double x;
  double y;
}vector;

typedef struct body{
    struct vector pos, vel, force;
    double mass;
}body;

body* bodies;

typedef struct Node{
  struct body bodyInNode;
  bool isLeaf;
  bool hasParticle;
  struct vector pos;
  double size;
  double totalMass;
  struct Node* nw;
  struct Node* ne;
  struct Node* sw;
  struct Node* se;
  struct vector centerOfMass;
}Node;


Node* findQuadrant(vector pos, Node* parent){
  if(pos.x >= parent->pos.x + parent->size/2){
   if(pos.y >= parent->pos.y + parent->size/2){
     //printf("\nParticle with pos {%lf, %lf} was found in NE",pos.x,pos.y);
     return parent->ne;
   }
   else{
     //printf("\nParticle with pos {%lf, %lf} was found in SE",pos.x,pos.y);
     return parent->se;
   }
  }
  else{
   if(pos.y >= parent->pos.y + parent->size/2){
     //printf("\nParticle with pos {%lf, %lf} was found in NW",pos.x,pos.y);
     return parent->nw;
   }
   else{
     //printf("\nParticle with pos {%lf, %lf} was found in SW",pos.x,pos.y);
     return parent->sw;
   }
 }
}

void insertIntoTree(body particle, Node* node){
  node->totalMass += particle.mass;
  if(node->isLeaf && !node->hasParticle){
    node->bodyInNode = particle;
    node->hasParticle = true;
  }
  else if(!node->isLeaf && node->hasParticle){
    Node* newNodeToGoTo = findQuadrant(particle.pos, node);
    insertIntoTree(particle, newNodeToGoTo);
  }
  else if(node->isLeaf && node->hasParticle){
    printf("\nNodes in same tree");
    if((node->bodyInNode.pos.x == particle.pos.x) && (node->bodyInNode.pos.y == particle.pos.y)){
      printf("\nCRASH");
      exit(1);
    }
    node->isLeaf = false;
    initChildren(node);
    Node* newNodeToGoToOldBody = findQuadrant(node->bodyInNode.pos, node);
    insertIntoTree(node->bodyInNode, newNodeToGoToOldBody);
    Node* newNodeToGoToNewBody = findQuadrant(particle.pos, node);
    insertIntoTree(particle, newNodeToGoToNewBody);
  }
}

void initChildren(Node* parent){
  parent->nw = (Node*)malloc(sizeof(Node));
  parent->ne = (Node*)malloc(sizeof(Node));
  parent->sw = (Node*)malloc(sizeof(Node));
  parent->se = (Node*)malloc(sizeof(Node));

  double sizeOfChildren = parent->size/2;
  parent->nw->size = parent->ne->size = parent->sw->size = parent->se->size = sizeOfChildren;

  parent->nw->pos.x = parent->pos.x;
  parent->nw->pos.y = parent->pos.y + sizeOfChildren;
  parent->nw->isLeaf = true;
  parent->nw->hasParticle = false;
  parent->nw->totalMass = 0.0;

  parent->ne->pos.x = parent->pos.x + sizeOfChildren;
  parent->ne->pos.y = parent->pos.y + sizeOfChildren;
  parent->ne->isLeaf = true;
  parent->ne->hasParticle = false;
  parent->ne->totalMass = 0.0;

  parent->sw->pos.x = parent->pos.x;
  parent->sw->pos.y = parent->pos.y;
  parent->sw->isLeaf = true;
  parent->sw->hasParticle = false;
  parent->sw->totalMass = 0.0;

  parent->se->pos.x = parent->pos.x + sizeOfChildren;
  parent->se->pos.y = parent->pos.y;
  parent->se->isLeaf = true;
  parent->se->hasParticle = false;
  parent->se->totalMass = 0.0;
}

Node *root;

void summarizeTree() {
  setCenterOfMasses(root);
}

vector ZERO_VECTOR(){
  struct vector v;
  v.x = 0;
  v.y = 0;
  return v;
}
vector calcNumeratorCOM(struct Node* n) {
  struct vector v;
  if(n->isLeaf && !n->hasParticle) {
    v = ZERO_VECTOR();
  }
  else if(n->isLeaf && n->hasParticle) {
    v.x = n->bodyInNode.mass * n->bodyInNode.pos.x;
    v.y = n->bodyInNode.mass * n->bodyInNode.pos.y;
  }
  else{
    struct vector v1, v2, v3, v4;
    v1 = calcNumeratorCOM(n->nw);
    v2 = calcNumeratorCOM(n->ne);
    v3 = calcNumeratorCOM(n->sw);
    v4 = calcNumeratorCOM(n->se);
    v.x = v1.x + v2.x + v3.x + v4.x;
    v.y = v1.y + v2.y + v3.y + v4.y;
  }
  return v;
}

void setCenterOfMasses(struct Node* n) {
  // Only set center of mass on internal nodes
  if(n->isLeaf){
    return;
  }
  struct vector v = calcNumeratorCOM(n);
  struct vector res = ZERO_VECTOR();
  res.x = v.x / n->totalMass;
  res.y = v.y / n->totalMass;
  n->centerOfMass = res;

  setCenterOfMasses(n->nw);
  setCenterOfMasses(n->ne);
  setCenterOfMasses(n->sw);
  setCenterOfMasses(n->se);
}

int main(int argc, char *argv[]) {
  numberOfBodies = ((argc > 1)? atoi(argv[1]) : DEFAULTBODIES);
  numberOfTimesteps = ((argc > 2)? atoi(argv[2]) : DEFAULTTIMESTEPS);

  initBodies();
  root = (Node*)malloc(sizeof(Node));

  root->size = 100;
  root->pos.x = 0;
  root->pos.y = 0;
  root->isLeaf = true;
  root->totalMass = 0.0;
  root->hasParticle = false;

  for(int i = 0; i < numberOfBodies; i++){
    insertIntoTree(bodies[i], root);
  }
  summarizeTree();
  printf("\ncenter of mass of root {%lf, %lf}", root->centerOfMass.x, root->centerOfMass.y);

  /*
  start_clock();
  for(int i = 0; i < numberOfTimesteps; i++){
    calculateForces();
    moveBodies();
  }
  end_clock();
  */

}

void initBodies(){
  bodies = (body*)malloc(sizeof(body) * numberOfBodies);
  for(int i = 0; i < numberOfBodies; i++){
    time_t t;
    srand((unsigned) time(&t) * (i + 1));
    bodies[i].pos.x = rand()%98 + 1;
    bodies[i].pos.y = rand()%98 + 1;
    bodies[i].mass = 1000;
  }
}

void calculateForces(){
  double distance;
  double magnitude;
  vector directions;
  for(int i = 0; i < numberOfBodies - 1; i++) {
    for(int j = i + 1; j < numberOfBodies; j++){
      distance = sqrt(((bodies[i].pos.x - bodies[j].pos.x) * (bodies[i].pos.x - bodies[j].pos.x)) +
      ((bodies[i].pos.y - bodies[j].pos.y) * (bodies[i].pos.y - bodies[j].pos.y)));
      magnitude = G * ((bodies[i].mass * bodies[j].mass) / (distance * distance));
      directions.x = bodies[j].pos.x - bodies[i].pos.x;
      directions.y = bodies[j].pos.y - bodies[i].pos.y;
      bodies[i].force.x = bodies[i].force.x + (magnitude * (directions.x / distance));
      bodies[j].force.x = bodies[j].force.x - (magnitude * (directions.x / distance));
      bodies[i].force.y = bodies[i].force.y + (magnitude * (directions.y / distance));
      bodies[j].force.y = bodies[j].force.y - (magnitude * (directions.y / distance));
    }
  }
}

void moveBodies() {
  vector deltap;
  vector deltav;
  for(int i = 0; i < numberOfBodies; i++) {
    deltav.x = (bodies[i].force.x / bodies[i].mass) * DT;
    deltav.y = (bodies[i].force.y / bodies[i].mass) * DT;
    deltap.x = (bodies[i].vel.x + deltav.x / 2) * DT,
    deltap.y = (bodies[i].vel.y + deltav.y / 2) * DT,
    bodies[i].vel.x = bodies[i].vel.x + deltav.x;
    bodies[i].vel.y = bodies[i].vel.y + deltav.y;
    bodies[i].pos.x = bodies[i].pos.x + deltap.x;
    bodies[i].pos.y = bodies[i].pos.y + deltap.y;
    bodies[i].force.x = bodies[i].force.y = 0.0;
  }
}

void start_clock(){
    st_time = times(&st_cpu);
}

void end_clock(){
    en_time = times(&en_cpu);
    printf("\nReal Time: %jd, User Time %jd, System Time %jd\n",
        (intmax_t)(en_time - st_time),
        (intmax_t)(en_cpu.tms_utime - st_cpu.tms_utime),
        (intmax_t)(en_cpu.tms_stime - st_cpu.tms_stime));
}
