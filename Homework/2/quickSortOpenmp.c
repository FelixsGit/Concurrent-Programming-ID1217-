#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#define ARRAYSIZE 100000000
double start_time, end_time;
void quickSort(int[], int);
void quickSortSerial(int[], int);
void swap(int[], int, int);
int listToBeSorted[ARRAYSIZE];

void quickSort(int *inputArray, int size){
	int pivot, leftIndex, rightIndex;
	if (size <= 1) {
    return;
  }
	pivot = inputArray[size/2];
	for(leftIndex = 0, rightIndex = size -1;; leftIndex++, rightIndex--) {
		while(inputArray[leftIndex] < pivot){
			leftIndex++;
		}
		while(pivot < inputArray[rightIndex]){
			rightIndex--;
		}
		if(rightIndex <= leftIndex){
			break;
		}
		swap(inputArray, leftIndex, rightIndex);
	}

  if(leftIndex > 10000){
    #pragma omp task
    {
      //printf("\nThread with Id = %d doing work on - LOWER" ,  omp_get_thread_num());
      quickSort(inputArray, leftIndex); /* Sort lower */
    }
  }else{
    quickSortSerial(inputArray, leftIndex); /* Sort lower */
  }

  if(rightIndex > 10000){
    #pragma omp task
    {
      //printf("\nThread with Id = %d doing work on - UPPER" ,  omp_get_thread_num());
      quickSort(inputArray + rightIndex + 1, size - rightIndex -1); /* Sort upper */
    }
  }else{
    quickSortSerial(inputArray + rightIndex + 1, size - rightIndex -1); /* Sort upper */
  }
}

void quickSortSerial(int *inputArray, int size){
	int pivot, leftIndex, rightIndex;
	if (size <= 1) {
    return;
  }
	pivot = inputArray[size/2];
	for(leftIndex = 0, rightIndex = size -1;; leftIndex++, rightIndex--) {
		while(inputArray[leftIndex] < pivot){
			leftIndex++;
		}
		while(pivot < inputArray[rightIndex]){
			rightIndex--;
		}
		if(rightIndex <= leftIndex){
			break;
		}
		swap(inputArray, leftIndex, rightIndex);
	}
  quickSortSerial(inputArray, leftIndex); /* Sort lower */
  quickSortSerial(inputArray + rightIndex + 1, size - rightIndex -1); /* Sort upper */
}

void swap(int *inputArray, int leftIndex, int rightIndex){
	int temp;
	temp = inputArray[leftIndex];
	inputArray[leftIndex] = inputArray[rightIndex];
	inputArray[rightIndex] = temp;
}
void printArray(int arr[], int size){
  int i;
  for (i=0; i < size; i++){
    printf("%d ", arr[i]);
  }
}

int main(){
  for(int i = 0; i < ARRAYSIZE; i++){
    listToBeSorted[i] = (rand()%99) + 1;
  }
  int n = sizeof(listToBeSorted)/sizeof(listToBeSorted[0]);
  listToBeSorted[n/2] = 50;
  printf("\n");
  printf("Sorting an array of size: %d\n", n);
  printf("Unsorted numbers: \n");
  omp_set_num_threads(4);
  start_time = omp_get_wtime();
  #pragma omp parallel
  {
    #pragma omp single nowait
    {
      quickSort(listToBeSorted, n);
    }
  }
  end_time = omp_get_wtime();
  printf("\n");
  printf("\n");
  printf("\nSorted numbers: \n");
  //printArray(listToBeSorted, n);
  printf("\n");
  printf("\nThe execution time is %g sec", end_time - start_time);
  return 0;
}
