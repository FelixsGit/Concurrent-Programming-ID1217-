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
int maxValues[MAXSIZE];
int maxXCords[MAXSIZE];
int maxYCords[MAXSIZE];
int minValues[MAXSIZE];
int minXCords[MAXSIZE];
int minYCords[MAXSIZE];
int finalMax;
int finalMaxX;
int finalMaxY;
int finalMin;
int finalMinX;
int finalMinY;

int main(int argc, char *argv[]) {
  int i, j, total=0;
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
    int maxValue = matrix[i][0];
    int maxX = 0;
    int maxY = i;
    int minValue = matrix[i][0];
    int minX = 0;
    int minY = i;
    for (j = 0; j < size; j++){
      total += matrix[i][j];
      if(matrix[i][j] > maxValue){
        maxValue = matrix[i][j];
        maxX = j;
        maxY = i;
      }
      if(matrix[i][j] < minValue){
        minValue = matrix[i][j];
        minX = j;
        minY = i;
      }
    }
    maxValues[i] = maxValue;
    maxXCords[i] = maxX;
    maxYCords[i] = maxY;
    minValues[i] = minValue;
    minXCords[i] = minX;
    minYCords[i] = minY;
  }
  finalMin = matrix[0][0];
  //implicit barrier
  for(int z = 0; z < size; z++){
    if(maxValues[z] > finalMax){
      finalMax = maxValues[z];
      finalMaxX = maxXCords[z];
      finalMaxY = maxYCords[z];
    }
    if(minValues[z] < finalMin){
      finalMin = minValues[z];
      finalMinX = minXCords[z];
      finalMinY = minYCords[z];
    }
  }
  end_time = omp_get_wtime();
  printf("the total is %d\n", total);
  printf("max value is %d on cords[%d , %d]", finalMax, finalMaxY, finalMaxX);
  printf("\nmin value is %d on cords[%d , %d]", finalMin, finalMinY, finalMinX);
  printf("\nit took %g seconds\n", end_time - start_time);
}
