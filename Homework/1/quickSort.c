#ifndef _REENTRANT
#define _REENTRANT
#endif
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#define ARRAYSIZE 100
double start_time, end_time; /* start and end times */
void *quickSort(void *);
int partition(int[], int, int);
void swap(int*, int*);
int listToBeSorted[ARRAYSIZE];

typedef struct partArray{
  int first;
  int last;
  int* inputArray;
}partArray;

partArray aData;

void *quickSort(void* list){
  partArray leftArray;
  partArray rightArray;
  pthread_t leftThreadId;
  int pivot, first, last, *array;

  first = ((partArray *) list)->first;
  last = ((partArray *) list)->last;
  array = ((partArray *) list)->inputArray;
  if(first >= last){
    return 0;
  }
  pivot = partition(array, first, last);
  listToBeSorted[pivot] = *(array + pivot);
  leftArray.first = first;
  leftArray.last = pivot - 1;
  leftArray.inputArray = array;
  rightArray.first = pivot + 1;
  rightArray.last = last;
  rightArray.inputArray = array;
  pthread_create(&leftThreadId, NULL, quickSort, &leftArray);
  quickSort((void *) &rightArray);
  pthread_join(leftThreadId, NULL);
}

int partition(int arr[], int low, int high){
  int pivot = arr[high];
  int i = (low - 1);
  for (int j = low; j <= high- 1; j++){
    if (arr[j] <= pivot){
      i++;
      swap(&arr[i], &arr[j]);
    }
  }
  swap(&arr[i + 1], &arr[high]);
  return (i + 1);
}
/* Function to swap to elements */
void swap(int* a, int* b){
  int temp = *a;
  *a = *b;
  *b = temp;
}
/* Function to print an array */
void printArray(int arr[], int size){
  int i;
  for (i=0; i < size; i++){
    printf("%d ", arr[i]);
  }
}
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
// Driver program to test above functions
int main(){
  for(int i = 0; i < ARRAYSIZE; i++){
    listToBeSorted[i] = (rand()%99) + 1;
  }
  int n = sizeof(listToBeSorted)/sizeof(listToBeSorted[0]);
  printf("\n");
  printf("Unsorted numbers: \n");
  printArray(listToBeSorted, n);
  aData.inputArray = &listToBeSorted[0];
  aData.first = 0;
  aData.last = ARRAYSIZE - 1;

  start_time = read_timer();
  quickSort((void*) &aData);
  end_time = read_timer();

  printf("\n");
  printf("\n");
  printf("\nSorted numbers: \n");
  printArray(listToBeSorted, n);
  printf("\n");
  printf("\nThe execution time is %g sec", end_time - start_time);
  return 0;
}
