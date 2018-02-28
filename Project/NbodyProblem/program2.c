#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>
#include <pthread.h>

#define NUMBEROFBODIES 120
#define DT 1
#define TIMESTEPS 2700
#define NUMBEROFTHREADS 1

static double G = 0.0000000000667;
void* calculateForces();
int moveBodies();
void start_clock(void);
void end_clock();
static clock_t st_time;
static clock_t en_time;
static struct tms st_cpu;
static struct tms en_cpu;
int count = 0;
int numArrived = 0;
int numWorkers = NUMBEROFTHREADS;
pthread_t thread[NUMBEROFTHREADS];
pthread_mutex_t barrier;  /* mutex lock for the barrier */
pthread_mutex_t counter;
pthread_cond_t go;        /* condition variable for leaving */

void Barrier() {
  pthread_mutex_lock(&barrier);
  numArrived++;
  if (numArrived == numWorkers) {
    numArrived = 0;
    pthread_cond_broadcast(&go);
  }
  else
    pthread_cond_wait(&go, &barrier);
  pthread_mutex_unlock(&barrier);
}

typedef struct body{
    double posX;
    double posY;
    double velX;
    double velY;
    double mass;
    double forceX;
    double forceY;
    double dirX;
    double dirY;
    double deltavX;
    double deltavY;
    double deltapX;
    double deltapY;
}body;

body bodies[NUMBEROFBODIES];

int main(){

  for(int i = 0; i < NUMBEROFBODIES; i++){
    time_t t;
    srand((unsigned) time(&t) * (i + 1));
    bodies[i].posX = rand()%1000;
    bodies[i].posY = rand()%1000;
    bodies[i].mass = 100000000000;
  }

  start_clock();
  for(long i = 0; i < NUMBEROFTHREADS; i++){
    pthread_create(&thread[i], NULL, calculateForces, (void *) i);
  }

  for(int i = 0; i < NUMBEROFTHREADS; i++){
      pthread_join(thread[i], NULL);
  }
  end_clock();
  for(long i = 0; i < NUMBEROFBODIES; i++){
    printf("Body %d velX = %lf & velY = %lf lu/tu\n", i + 1, bodies[i].velX, bodies[i].velY);
  }
}

void* calculateForces(void* arg){
  long id = (long) arg;
  //printf("Thread with id = %d\n working on forces", id );
  double distance;
  double magnitude;
  body direction;
  for(int i = id; i < NUMBEROFBODIES; i+= NUMBEROFTHREADS) {
    for(int j = i + 1; j < NUMBEROFBODIES; j++){
      distance = sqrt(((bodies[i].posX - bodies[j].posX) * (bodies[i].posX - bodies[j].posX)) +
      ((bodies[i].posY - bodies[j].posY) * (bodies[i].posY - bodies[j].posY)));
      //printf("\nDistance between body %d and %d: %lf le", i + 1, j + 1, distance);
      magnitude = G * ((bodies[i].mass * bodies[j].mass) / (distance * distance));
      //printf("\nMagnitude for these bodies are: %f Newton\n", magnitude);
      direction.dirX = bodies[j].posX - bodies[i].posX;
      direction.dirY = bodies[j].posY - bodies[i].posY;
      bodies[i].forceX = bodies[i].forceX + (magnitude * (direction.dirX / distance));
      bodies[j].forceX = bodies[j].forceX - (magnitude * (direction.dirX / distance));
      bodies[i].forceY = bodies[i].forceY + (magnitude * (direction.dirY / distance));
      bodies[j].forceY = bodies[j].forceY - (magnitude * (direction.dirY / distance));
    }
  }
  Barrier();
  moveBodies(id);
}
  int moveBodies(long arg) {
    body deltap;
    body deltav;
    for(int i = arg; i < NUMBEROFBODIES; i+=NUMBEROFTHREADS) {
      deltav.deltavX = (bodies[i].forceX / bodies[i].mass) * DT;
      deltav.deltavY = (bodies[i].forceY / bodies[i].mass) * DT;
      deltap.deltapX = (bodies[i].velX + deltav.deltavX / 2) * DT,
      deltap.deltapY = (bodies[i].velY + deltav.deltavY / 2) * DT,
      bodies[i].velX = bodies[i].velX + deltav.deltavX;
      bodies[i].velY = bodies[i].velY + deltav.deltavY;
      bodies[i].posX = bodies[i].posX + deltap.deltapX;
      bodies[i].posY = bodies[i].posY + deltap.deltapY;
      bodies[i].forceX = bodies[i].forceY = 0.0; // reset force vector
  }
  Barrier();
  pthread_mutex_lock(&counter);
  if(count < TIMESTEPS){
    count++;
    pthread_mutex_unlock(&counter);
    calculateForces((void*)arg);
  }else{
    pthread_mutex_unlock(&counter);
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
