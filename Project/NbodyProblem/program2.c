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

static double G = 0.0000000000667;
void calculateForces();
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
pthread_mutex_t counter;
pthread_cond_t go;        /* condition variable for leaving */

int numberOfThreads;
int numberOfBodies;
int numberOfTimesteps;

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


int main(int argc, char *argv[]) {
  numberOfBodies = ((argc > 1)? atoi(argv[1]) : DEFAULTBODIES);
  numberOfTimesteps = ((argc > 2)? atoi(argv[2]) : DEFAULTTIMESTEPS);
  numberOfThreads = ((argc > 3)? atoi(argv[3]) : DEFAULTHREADS);
  pthread_t thread[numberOfThreads];
  body *bodies = (body*) malloc(sizeof(body) * numberOfBodies);
  pthread_mutex_init(&barrier, NULL);
  pthread_mutex_init(&counter, NULL);
  pthread_cond_init(&go, NULL);

  for(int i = 0; i < numberOfBodies; i++){
    time_t t;
    srand((unsigned) time(&t) * (i + 1));
    bodies[i].posX = rand()%1000;
    bodies[i].posY = rand()%1000;
    bodies[i].mass = rand()%100000000000;
  }

  start_clock();
  for(long i = 0; i < numberOfThreads; i++){
    pthread_create(&thread[i], NULL, work, (void *) i);
  }

  for(int i = 0; i < numberOfThreads; i++){
      pthread_join(thread[i], NULL);
  }
  end_clock();
  for(long i = 0; i < numberOfBodies; i++){
    printf("Body %d velX = %lf & velY = %lf lu/tu\n", i + 1, bodies[i].velX, bodies[i].velY);
  }
}

void* work(void* arg){
  long id = (long) arg;
  for(int i = 0; i < numberOfTimesteps; i++){
    //printf("\nITERATION %d starting", i );
    calculateForces(id);
    Barrier();
    moveBodies(id);
    Barrier();
  }
}
void calculateForces(void* arg){
  long id = (long) arg;
  //printf("\nthread with id %d calculateForces");
  //printf("Thread with id = %d\n working on forces", id );
  double distance;
  double magnitude;
  body direction;
  for(int i = id; i < numberOfBodies; i+= numberOfThreads) {
    for(int j = i + 1; j < numberOfBodies; j++){
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
}
  void moveBodies(long arg) {
    long id = (long) arg;
    //printf("\nthread with id %d moving bodies", id);
    body deltap;
    body deltav;
    for(int i = id; i < numberOfBodies; i+= numberOfThreads) {
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
