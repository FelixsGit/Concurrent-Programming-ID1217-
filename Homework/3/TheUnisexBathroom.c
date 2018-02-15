#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#define DEFAULTMEN 100
#define DEFAULTWOMEN 100
#define SHARED 1
void* menEnter(void*);
void* womenEnter(void*);

int menInBathroom = 0;
int womenInBathroom = 0;
int menInQueue = 0;
int womenInQueue = 0;

sem_t bathroomLock;
sem_t queueMen;
sem_t queueWomen;

int main(int argc, char *argv[]) {
  int numberOfMen = ((argc > 1)? atoi(argv[1]) : DEFAULTMEN);
  int numberOfWomen = ((argc > 2)? atoi(argv[2]) : DEFAULTWOMEN);
  pthread_t men[numberOfMen];
  pthread_t women[numberOfWomen];
  sem_init(&bathroomLock, SHARED, 1);
  sem_init(&queueMen, SHARED, 0);
  sem_init(&queueWomen, SHARED, 0);

  for(int i = 0; i < numberOfMen; i ++){
    pthread_create(&men[i], NULL, menEnter, (void *) i);
  }
  for(int z = 0; z < numberOfWomen; z++){
    pthread_create(&women[z], NULL, womenEnter, (void *) z);
  }
  pthread_exit(&numberOfMen);
  pthread_exit(&numberOfWomen);
}

void* menEnter(void* arg){
  int id = (int) arg;
  time_t t;
  srand((unsigned) time(&t));
  while(1){
    usleep(1000000 + rand() % 5000000);
    sem_wait(&bathroomLock);
    if(womenInBathroom > 0){
      menInQueue ++;
      printf("\nMen in queue = %d", menInQueue );
      sem_post(&bathroomLock);
      sem_wait(&queueMen);
    }
    menInBathroom ++;
    if(menInQueue > 0){
      menInQueue --;
      sem_post(&queueMen);
    }else{
      sem_post(&bathroomLock);
    }
    ////////////////inside bathroom/////////////////////////
    printf("\n-Man with id %d ENTERS bathroom", id);
    usleep(rand() % 1500000);
    ////////////////leaving bathroom///////////////////////
    sem_wait(&bathroomLock);
    menInBathroom --;
    printf("\n-Man with id %d leaves bathroom", id);
    if((menInBathroom == 0) && (womenInQueue > 0)){
      printf("\n\nWomans turn\n");
      womenInQueue --;
      sem_post(&queueWomen);
    }else{
      sem_post(&bathroomLock);
    }
  }
}

void* womenEnter(void* arg){
  int id = (int) arg;
  time_t t;
  srand((unsigned) time(&t));
  while(1){
    usleep(1000000 +rand() % 5000000);
    sem_wait(&bathroomLock);
    if(menInBathroom > 0){
      womenInQueue ++;
      printf("\nWomen in queue = %d", womenInQueue );
      sem_post(&bathroomLock);
      sem_wait(&queueWomen);
    }
    womenInBathroom ++;
    if(womenInQueue > 0){
      womenInQueue --;
      sem_post(&queueWomen);
    }else{
      sem_post(&bathroomLock);
    }
    ////////////////inside bathroom/////////////////////////
    printf("\n-Woman with id %d ENTERS bathroom", id);
    usleep(rand() % 1500000);
    ////////////////leaving bathroom///////////////////////
    sem_wait(&bathroomLock);
    womenInBathroom --;
    printf("\n-Woman with id %d leaves bathroom", id);
    if((womenInBathroom == 0) && (menInQueue > 0)){
      printf("\n\nMans turn\n");
      menInQueue --;
      sem_post(&queueMen);
    }else{
      sem_post(&bathroomLock);
    }
  }
}
