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
void inserIntoTree();
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

struct body *bodies;

typedef struct Node{
  struct body bodyInNode;
  bool isLeaf;
  bool hasParticle;
  struct vector pos;
  int size;
  double totalMass;
  struct Node *nw;
  struct Node *ne;
  struct Node *sw;
  struct Node *se;
  struct vector centerOfMass;
}Node;

Node *root;

Node *findQuadrant(body bod, struct Node *node){
  struct Node *nodeToGoTo;
  //going to nw
  if((bod.pos.x < node->size/2) && (bod.pos.y >= node->size/2)){
      node->nw->size = node->size/2;
      node->nw->pos.x = node->pos.x;
      node->nw->pos.y = node->pos.y + node->size/2;
      nodeToGoTo = node->nw;

  //going to ne
}else if((bod.pos.x >= node->size/2) && (bod.pos.y >= node->size/2)){
      node->ne->size = node->size/2;
      node->ne->pos.x = node->pos.x + node->size/2;
      node->ne->pos.y = node->pos.y + node->size/2;
      nodeToGoTo = node->nw;

  //going to sw
}else if((bod.pos.x < node->size/2) && (bod.pos.y < node->size/2)){
      node->sw->size = node->size/2;
      node->sw->pos.x = node->pos.x;
      node->sw->pos.y = node->pos.y;
      nodeToGoTo = node->nw;

  //going to se
}else if((bod.pos.x >= node->size/2) && (bod.pos.y < node->size/2)){
      node->se->size = node->size/2;
      node->se->pos.x = node->pos.x + node->size/2;
      node->se->pos.y = node->pos.y;
      nodeToGoTo = node->nw;
  }
  return nodeToGoTo;
}

void inserIntoTree(body bodies, struct Node *node){
  if(!node->isLeaf && node->hasParticle){
    Node *newNodeToGoTo = findQuadrant(bodies, node);
    inserIntoTree(bodies, newNodeToGoTo);
  }
  if(node->isLeaf && !node->hasParticle){
    node->bodyInNode = bodies;
    node->hasParticle = true;
  }
  if(node->isLeaf && node->hasParticle){
    Node *newNodeToGoToOldBody = findQuadrant(node->bodyInNode, node);
    Node *newNodeToGoToNewBody= findQuadrant(bodies, node);
    inserIntoTree(node->bodyInNode, newNodeToGoToOldBody);
    inserIntoTree(bodies, newNodeToGoToNewBody);
  }
}

int main(int argc, char *argv[]) {
  numberOfBodies = ((argc > 1)? atoi(argv[1]) : DEFAULTBODIES);
  numberOfTimesteps = ((argc > 2)? atoi(argv[2]) : DEFAULTTIMESTEPS);

  initBodies();

  root->pos.x = 0;
  root->pos.y = 0;
  root->size = 100;
  root->isLeaf = true;
  root->hasParticle = false;

  for(int i = 0; i < numberOfBodies; i++){
    inserIntoTree(bodies[i], root);
  }

  start_clock();
  for(int i = 0; i < numberOfTimesteps; i++){
    calculateForces();
    moveBodies();
  }
  end_clock();

}

void initBodies(){
  bodies = (body*)malloc(sizeof(body) * numberOfBodies);
  for(int i = 0; i < numberOfBodies; i++){
    time_t t;
    srand((unsigned) time(&t) * (i + 1));
    bodies[i].pos.x = rand()%100;
    bodies[i].pos.y = rand()%100;
    bodies[i].mass = 1 + rand()%100000000000;
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
