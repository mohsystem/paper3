
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int binarySearch(int arr[], int size, int target) {
    int left = 0;
    int right = size - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        if (arr[mid] == target) {
            return mid;
        } else if (arr[mid] < target) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    return -1;
}

void writeExecutionTime(long long executionTime) {
    char tempFileName[256];
    sprintf(tempFileName, "/tmp/binary_search_execution_%ld.txt", time(NULL));
    
    FILE *file = fopen(tempFileName, "w");
    if (file != NULL) {
        fprintf(file, "Binary Search Execution Time: %lld nanoseconds\\n", executionTime);
        fprintf(file, "Execution Time: %.6f milliseconds\\n", executionTime / 1000000.0);
        fclose(file);
        printf("Execution time written to: %s\\n", tempFileName);
    } else {
        printf("Error writing to file\\n");
    }
}

void printArray(int arr[], int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%d", arr[i]);
        if (i < size - 1) printf(", ");
    }
    printf("]");
}

long long getTimeNano() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)(ts.tv_sec) * 1000000000LL + (long long)(ts.tv_nsec);
}

int main() {
    // Test Case 1
    int arr1[] = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19};
    int size1 = sizeof(arr1) / sizeof(arr1[0]);
    int target1 = 7;
    long long start1 = getTimeNano();
    int result1 = binarySearch(arr1, size1, target1);
    long long end1 = getTimeNano();
    long long executionTime1 = end1 - start1;
    printf("Test 1: Array: ");
    printArray(arr1, size1);
    printf(", Target: %d, Result: %d\\n", target1, result1);
    writeExecutionTime(executionTime1);
    
    // Test Case 2
    int arr2[] = {2, 4, 6, 8, 10, 12, 14, 16, 18, 20};
    int size2 = sizeof(arr2) / sizeof(arr2[0]);
    int target2 = 20;
    long long start2 = getTimeNano();
    int result2 = binarySearch(arr2, size2, target2);
    long long end2 = getTimeNano();
    long long executionTime2 = end2 - start2;
    printf("Test 2: Array: ");
    printArray(arr2, size2);
    printf(", Target: %d, Result: %d\\n", target2, result2);
    writeExecutionTime(executionTime2);
    
    // Test Case 3
    int arr3[] = {5, 10, 15, 20, 25, 30, 35, 40};
    int size3 = sizeof(arr3) / sizeof(arr3[0]);
    int target3 = 100;
    long long start3 = getTimeNano();
    int result3 = binarySearch(arr3, size3, target3);
    long long end3 = getTimeNano();
    long long executionTime3 = end3 - start3;
    printf("Test 3: Array: ");
    printArray(arr3, size3);
    printf(", Target: %d, Result: %d\\n", target3, result3);
    writeExecutionTime(executionTime3);
    
    // Test Case 4
    int arr4[] = {1, 2, 3, 4, 5};
    int size4 = sizeof(arr4) / sizeof(arr4[0]);
    int target4 = 1;
    long long start4 = getTimeNano();
    int result4 = binarySearch(arr4, size4, target4);
    long long end4 = getTimeNano();
    long long executionTime4 = end4 - start4;
    printf("Test 4: Array: ");
    printArray(arr4, size4);
    printf(", Target: %d, Result: %d\\n", target4, result4);
    writeExecutionTime(executionTime4);
    
    // Test Case 5
    int arr5[] = {11, 22, 33, 44, 55, 66, 77, 88, 99};
    int size5 = sizeof(arr5) / sizeof(arr5[0]);
    int target5 = 55;
    long long start5 = getTimeNano();
    int result5 = binarySearch(arr5, size5, target5);
    long long end5 = getTimeNano();
    long long executionTime5 = end5 - start5;
    printf("Test 5: Array: ");
    printArray(arr5, size5);
    printf(", Target: %d, Result: %d\\n", target5, result5);
    writeExecutionTime(executionTime5);
    
    return 0;
}
