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

struct tree {
  struct Node* root;
} quadTree;

void newTree(int worldSize) {
  struct Node *root;
  quadTree.root = root;
  quadTree.root->size = worldSize;
  quadTree.root->pos.x = 0;
  quadTree.root->pos.y = 0;
}

Node *findQuadrant(struct vector pos, struct Node parent){
  if(pos.x > parent.size/2){
   if(pos.y > parent.size/2){
     return parent.ne;
   }
   else{
     return parent.se;
   }
  }
  else{
   if(pos.y > parent.size/2){
     return parent.nw;
   }
   else{
     return parent.sw;
   }
 }
}

void insertIntoTree(struct body particle, struct Node *node){
  if(node->isLeaf && !node->hasParticle){
    node->bodyInNode = particle;
    node->hasParticle = true;
  }
  else if(!node->isLeaf && node->hasParticle){
    struct Node *newNodeToGoTo = findQuadrant(particle.pos, *node);
    insertIntoTree(particle, newNodeToGoTo);
  }
  else if(node->isLeaf && node->hasParticle){
    initChildren(node);
    struct Node *newNodeToGoToOldBody = findQuadrant(node->bodyInNode.pos, *node);
    newNodeToGoToOldBody->hasParticle = true;
    insertIntoTree(node->bodyInNode, newNodeToGoToOldBody);
    struct Node *newNodeToGoToNewBody= findQuadrant(particle.pos, *node);
    newNodeToGoToNewBody->hasParticle = true;
    insertIntoTree(particle, newNodeToGoToNewBody);
  }
}

void initChildren(struct Node *parent){
  int sizeOfChildren = parent->size/2;
  parent->nw->size = parent->ne->size = parent->sw->size = parent->se->size = sizeOfChildren;
  parent->sw->pos.x = parent->pos.x;
  parent->sw->pos.y = parent->size + sizeOfChildren;

  parent->ne->pos.x = parent->pos.x + sizeOfChildren;
  parent->ne->pos.y = parent->pos.y + sizeOfChildren;

  parent->sw->pos.x = parent->pos.x;
  parent->sw->pos.y = parent->pos.y;

  parent->se->pos.x = parent->pos.x + sizeOfChildren;
  parent->se->pos.y = parent->pos.y;
}

int main(int argc, char *argv[]) {
  numberOfBodies = ((argc > 1)? atoi(argv[1]) : DEFAULTBODIES);
  numberOfTimesteps = ((argc > 2)? atoi(argv[2]) : DEFAULTTIMESTEPS);

  initBodies();
  newTree(100);

  for(int i = 0; i < numberOfBodies; i++){
    insertIntoTree(bodies[i], quadTree.root);
  }

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
