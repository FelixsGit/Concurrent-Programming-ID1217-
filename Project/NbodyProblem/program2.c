#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>
#include <pthread.h>

#define DEFAULTBODIES 100
#define DT 1
#define DEFAULTTIMESTEPS 1000
#define DEFAULTHREADS 1
const double G = 0.0000000000667;

void Barrier();
void calculateForces();
void initBodies();
void* work();
void moveBodies();
void start_clock(void);
void end_clock();
static clock_t st_time;
static clock_t en_time;
static struct tms st_cpu;
static struct tms en_cpu;
int numArrived = 0;
pthread_mutex_t barrier;  /* mutex lock for the barrier */
pthread_cond_t go;        /* condition variable for leaving */
int numberOfThreads;
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
struct vector **force;

int main(int argc, char *argv[]) {
  numberOfBodies = ((argc > 1)? atoi(argv[1]) : DEFAULTBODIES);
  numberOfTimesteps = ((argc > 2)? atoi(argv[2]) : DEFAULTTIMESTEPS);
  numberOfThreads = ((argc > 3)? atoi(argv[3]) : DEFAULTHREADS);
  pthread_t thread[numberOfThreads];
  pthread_mutex_init(&barrier, NULL);
  pthread_cond_init(&go, NULL);

  initBodies();

  start_clock();
  for(long i = 0; i < numberOfThreads; i++){
    pthread_create(&thread[i], NULL, work, (void *) i);
  }
  for(int i = 0; i < numberOfThreads; i++){
      pthread_join(thread[i], NULL);
  }
  end_clock();

  /*
  for(int i = 0; i < numberOfBodies; i++){
    printf("\nBody %d has Mass %lf & velX = %lf & velY = %lf", i, bodies[i].mass, bodies[i].vel.x, bodies[i].vel.y);
  }
  */

}

void* work(void* arg){
  long id = (long) arg;
  for(int i = 0; i < numberOfTimesteps; i++){
    calculateForces(id);
    Barrier();
    moveBodies(id);
    Barrier();
  }
}

void initBodies(){
  bodies = (body*)malloc(sizeof(body) * numberOfBodies);
  force = malloc(sizeof *force * numberOfThreads);
  if(force){
    for(int i = 0; i < numberOfThreads; i++){
      force[i] = malloc(sizeof *force[i] * numberOfBodies);
    }
  }
  for(int i = 0; i < numberOfBodies; i++){
    time_t t;
    srand((unsigned) time(&t) * (i + 1));
    bodies[i].pos.x = rand()%1000;
    bodies[i].pos.y = rand()%1000;
    bodies[i].mass = 1 + rand()%100000000000;
  }
}

void calculateForces(long id){
  double distance;
  double magnitude;
  vector directions;
  for(int i = id; i < numberOfBodies; i+= numberOfThreads) {
    for(int j = i + 1; j < numberOfBodies; j++){
      distance = sqrt(((bodies[i].pos.x - bodies[j].pos.x) * (bodies[i].pos.x - bodies[j].pos.x)) +
      ((bodies[i].pos.y - bodies[j].pos.y) * (bodies[i].pos.y - bodies[j].pos.y)));
      magnitude = G * ((bodies[i].mass * bodies[j].mass) / (distance * distance));
      directions.x = bodies[j].pos.x - bodies[i].pos.x;
      directions.y = bodies[j].pos.y - bodies[i].pos.y;
      force[id][i].x = force[id][i].x + (magnitude * (directions.x / distance));
      force[id][j].x = force[id][j].x - (magnitude * (directions.x / distance));
      force[id][i].y = force[id][i].y + (magnitude * (directions.y / distance));
      force[id][j].y = force[id][j].y - (magnitude * (directions.y / distance));
    }
  }
}

void moveBodies(long id) {
  vector deltap;
  vector deltav;
  for(int i = id; i < numberOfBodies; i+= numberOfThreads) {
    for(int k = 0; k < numberOfThreads; k++){
      bodies[i].force.x += force[k][i].x;
      bodies[i].force.y += force[k][i].y;
      force[k][i].x = force[k][i].y = 0.0;
    }
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

void Barrier() {
  pthread_mutex_lock(&barrier);
  numArrived++;
  if (numArrived == numberOfThreads) {
    numArrived = 0;
    pthread_cond_broadcast(&go);
  }
  else
    pthread_cond_wait(&go, &barrier);
  pthread_mutex_unlock(&barrier);
}
