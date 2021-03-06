#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#define MAXSIZE 10000  /* maximum matrix size */
#define MAXWORKERS 10   /* maximum number of workers */

/*Global Variables*/
int globalTotalSum = 0;
int globalMaxValue;
int globalMaxX;
int globalMaxY;
int globalMinValue;
int globalMinX;
int globalMinY;
int numWorkers;           /* number of workers */
bool bagEmpty = false;
int nextRow;              /*Bag of tast variable*/

pthread_mutex_t mutexOne;
pthread_mutex_t mutexTwo;

/* timer */
double read_timer() {
  static bool initialized = false;
  static struct timeval start;
  struct timeval end;
  if( !initialized ){
      gettimeofday( &start, NULL );
      initialized = true;
  }
  gettimeofday( &end, NULL );
  return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

double start_time, end_time; /* start and end times */
int size, stripSize;  /* assume size is multiple of numWorkers */
int sums[MAXWORKERS]; /* partial sums */
int matrix[MAXSIZE][MAXSIZE]; /* matrix */
void *Worker(void *);

/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
  int i, j;
  long l; /* use long in case of a 64-bit system */
  pthread_attr_t attr; /* Thread creation attribute */
  pthread_t workerid[MAXWORKERS]; /* P thread handle */

  /* set and init global thread attributes */
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
  /* initialize mutex */
  pthread_mutex_init(&mutexOne, NULL);
  pthread_mutex_init(&mutexTwo, NULL);

  /* read command line args if any */
  size = ((argc > 1)? atoi(argv[1]) : MAXSIZE);
  numWorkers = ((argc > 2)? atoi(argv[2]) : MAXWORKERS);
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;
  stripSize = size/numWorkers;

  /* initialize the matrix */
  for (i = 0; i < size; i++) {
	  for (j = 0; j < size; j++) {
        matrix[i][j] = rand()%99;
	  }
  }
  //sets the minimul variable to equal the first element in the matrix array.
  globalMinValue = matrix[0][0];
  //sets the first row to be worked on
  nextRow = size; /*bag of task variable*/

  /* print the matrix */
#ifdef DEBUG
  for (i = 0; i < size; i++) {
	  printf("[ ");
	  for (j = 0; j < size; j++) {
	    printf(" %d", matrix[i][j]);
	  }
	  printf(" ]\n");
  }
#endif

  /* do the parallel work: create the workers */
  start_time = read_timer();
  for (l = 0; l < numWorkers; l++)
    pthread_create(&workerid[l], &attr, Worker, (void *) l);

  //All threads have to be finnished before continueing.
  for(int thread = 0; thread < numWorkers; thread++){
    pthread_join(workerid[thread], NULL);
    #ifdef DEBUG
    printf("Thread: %d , joined \n", thread);
    #endif
  }

  /* get end time */
  end_time = read_timer();

  /* print results */
  printf("maxvalue: %d\n", globalMaxValue );
  printf("maxvalueCords:{%d, %d}", globalMaxY, globalMaxX );
  printf("\n");
  printf("minvalue %d\n", globalMinValue );
  printf("minvalueCords:{%d, %d}", globalMinY, globalMinX );
  printf("\n");
  printf("The total is %d\n", globalTotalSum);
  printf("The execution time is %g sec\n", end_time - start_time);

  pthread_exit(NULL); /* Kill with no return value */
}

/* Each worker sums the values in one strip of the matrix.
   After a barrier, worker(0) computes and prints the total */
void *Worker(void *arg) {
  long myid = (long) arg;
  int total, i, j, first, last;

#ifdef DEBUG
  printf("worker %d (pthread id %d) has started\n", myid, pthread_self());
#endif

  /* determine first and last rows of my strip */
  first = myid*stripSize;
  last = (myid == numWorkers - 1) ? (size - 1) : (first + stripSize - 1);

  int currentxCordMax = 0;
  int currentyCordMax = first;
  int currentMax = matrix[first][0]; /* The values can only take the form of 1-99 */

  int currentxCordMin = 0;
  int currentyCordMin = first;
  int currentMin = matrix[first][0]; /* The values can only take the form of 1-99 */

  total = 0;
  int row;

  while(!bagEmpty){
    pthread_mutex_lock(&mutexOne);  /*Here we handel shared variables so we have to lock*/
      row = nextRow - 1;
      nextRow --;
    pthread_mutex_unlock(&mutexOne);
    if(row <= 0){
      bagEmpty = true;
    }else{
      for (j = 0; j < size; j++){
        total += matrix[row][j];
        if(matrix[row][j] > currentMax){
          currentMax = matrix[row][j];
          currentxCordMax = j;
          currentyCordMax = row;
        }
        if(matrix[row][j] < currentMin){
          currentMin = matrix[row][j];
          currentxCordMin = j;
          currentyCordMin = row;
        }
      }
    }
  }
  if(currentMax > globalMaxValue){
      pthread_mutex_lock(&mutexTwo);  /*Here we handel shared variables so we have to lock*/
      globalMaxValue = currentMax;
      globalMaxX = currentxCordMax;
      globalMaxY = currentyCordMax;
      pthread_mutex_unlock(&mutexTwo);  /*Here we handel shared variables so we have to lock*/
    }
    if(currentMin < globalMinValue){
      pthread_mutex_lock(&mutexTwo);  /*Here we handel shared variables so we have to lock*/
      globalMinValue = currentMin;
      globalMinX = currentxCordMin;
      globalMinY = currentyCordMin;
      pthread_mutex_unlock(&mutexTwo);
    }
  pthread_mutex_lock(&mutexTwo);  /*Here we handel shared variables so we have to lock*/
  globalTotalSum += total;
  pthread_mutex_unlock(&mutexTwo);

}
