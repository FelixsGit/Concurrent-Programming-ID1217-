#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>

#define MAXSIZE 10000  /* maximum matrix size */
#define MAXWORKERS 8   /* maximum number of workers */
double start_time;
double end_time;
int numWorkers;
int size;
int matrix[MAXSIZE][MAXSIZE];
int globalMaxValue;
int globalMaxXCord;
int globalMaxYCord;
int globalMinValue;
int globalMinXCord;
int globalMinYCord;

int main(int argc, char *argv[]) {
  int i, j, total=0;
  globalMinValue = 101;
  size = ((argc > 1)? atoi(argv[1]) : MAXSIZE);
  numWorkers = ((argc > 2)? atoi(argv[2]) : MAXWORKERS);
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;
  omp_set_num_threads(numWorkers);
  for (i = 0; i < size; i++) {
    for (j = 0; j < size; j++) {
          matrix[i][j] = rand()%99;
    }
  }
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

  start_time = omp_get_wtime();
  #pragma omp parallel for reduction (+:total) private(j)
  for (i = 0; i < size; i++){
    int localMax = matrix[i][0];
    int localMin = matrix[i][0];
    int localMaxX = 0;
    int localMaxY = i;
    int localMinX = 0;
    int localMinY = i;
    for (j = 0; j < size; j++){
      total += matrix[i][j];
      if(matrix[i][j] > localMax){
        localMax = matrix[i][j];
        localMaxX = j;
        localMaxY = i;
      }
      if(matrix[i][j] < localMin){
        localMin = matrix[i][j];
        localMinX = j;
        localMinY = i;
      }
    }
    if(localMax > globalMaxValue){
      #pragma omp critical
      {
        if(localMax > globalMaxValue){
          globalMaxValue = localMax;
          globalMaxXCord = localMaxX;
          globalMaxYCord = localMaxY;
        }
      }
    }
    if(localMin < globalMinValue){
      #pragma omp critical
      {
        if(localMin < globalMinValue){
          globalMinValue = localMin;
          globalMinXCord = localMinX;
          globalMinYCord = localMinY;
        }
      }
    }
  }
  end_time = omp_get_wtime();
  printf("the total is %d\n", total);
  printf("max value is %d on cords[%d , %d]", globalMaxValue, globalMaxYCord, globalMaxXCord);
  printf("\nmin value is %d on cords[%d , %d]", globalMinValue, globalMinYCord, globalMinXCord);
  printf("\nit took %g seconds\n", end_time - start_time);
}
