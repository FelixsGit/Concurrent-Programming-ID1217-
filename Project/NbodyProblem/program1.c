#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>

#define NUMBEROFBODIES 10
double DT = 100;

static double G = 0.0000000000667;
int calculateForces();
int moveBodies();
void start_clock(void);
void end_clock();

static clock_t st_time;
static clock_t en_time;
static struct tms st_cpu;
static struct tms en_cpu;

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
    bodies[i].mass = 1000 + rand()%100000000000000000;
  }
  start_clock();

  calculateForces();
  moveBodies();

  end_clock();

  for(int i = 0; i < NUMBEROFBODIES; i++){
    printf("Body %d velX = %lf & velY = %lf le/s\n", i + 1, bodies[i].velX, bodies[i].velY);
  }

}

int calculateForces(){
  double distance;
  double magnitude;
  body direction;
  for(int i = 0; i < NUMBEROFBODIES - 1; i++) {
    int j = i + 1;
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
  int moveBodies() {
    body deltap;
    body deltav;
    for(int i = 0; i < NUMBEROFBODIES; i++) {
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
