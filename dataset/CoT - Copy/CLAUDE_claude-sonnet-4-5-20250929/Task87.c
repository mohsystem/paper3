
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int binarySearch(int* arr, int size, int target) {
    if (arr == NULL || size == 0) {
        return -1;
    }
    
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
    snprintf(tempFileName, sizeof(tempFileName), "/tmp/binary_search_execution_%ld.txt", (long)time(NULL));
    
    FILE* tempFile = fopen(tempFileName, "w");
    if (tempFile != NULL) {
        fprintf(tempFile, "Binary Search Execution Time: %lld nanoseconds\\n", executionTime);
        fclose(tempFile);
        printf("Execution time written to: %s\\n", tempFileName);
    } else {
        fprintf(stderr, "Error: Unable to open temporary file\\n");
    }
}

void printArray(int* arr, int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%d", arr[i]);
        if (i < size - 1) printf(", ");
    }
    printf("]");
}

long long getCurrentTimeNanos() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

int main() {
    // Test Case 1: Target found in middle
    int arr1[] = {1, 3, 5, 7, 9, 11, 13, 15};
    int size1 = sizeof(arr1) / sizeof(arr1[0]);
    int target1 = 7;
    long long start1 = getCurrentTimeNanos();
    int result1 = binarySearch(arr1, size1, target1);
    long long end1 = getCurrentTimeNanos();
    long long executionTime1 = end1 - start1;
    printf("Test 1: Array: ");
    printArray(arr1, size1);
    printf(", Target: %d, Result: %d\\n", target1, result1);
    writeExecutionTime(executionTime1);
    
    // Test Case 2: Target found at beginning
    int arr2[] = {2, 4, 6, 8, 10, 12, 14};
    int size2 = sizeof(arr2) / sizeof(arr2[0]);
    int target2 = 2;
    long long start2 = getCurrentTimeNanos();
    int result2 = binarySearch(arr2, size2, target2);
    long long end2 = getCurrentTimeNanos();
    long long executionTime2 = end2 - start2;
    printf("Test 2: Array: ");
    printArray(arr2, size2);
    printf(", Target: %d, Result: %d\\n", target2, result2);
    writeExecutionTime(executionTime2);
    
    // Test Case 3: Target found at end
    int arr3[] = {5, 10, 15, 20, 25, 30};
    int size3 = sizeof(arr3) / sizeof(arr3[0]);
    int target3 = 30;
    long long start3 = getCurrentTimeNanos();
    int result3 = binarySearch(arr3, size3, target3);
    long long end3 = getCurrentTimeNanos();
    long long executionTime3 = end3 - start3;
    printf("Test 3: Array: ");
    printArray(arr3, size3);
    printf(", Target: %d, Result: %d\\n", target3, result3);
    writeExecutionTime(executionTime3);
    
    // Test Case 4: Target not found
    int arr4[] = {1, 2, 3, 4, 5};
    int size4 = sizeof(arr4) / sizeof(arr4[0]);
    int target4 = 10;
    long long start4 = getCurrentTimeNanos();
    int result4 = binarySearch(arr4, size4, target4);
    long long end4 = getCurrentTimeNanos();
    long long executionTime4 = end4 - start4;
    printf("Test 4: Array: ");
    printArray(arr4, size4);
    printf(", Target: %d, Result: %d\\n", target4, result4);
    writeExecutionTime(executionTime4);
    
    // Test Case 5: Empty array
    int* arr5 = NULL;
    int size5 = 0;
    int target5 = 5;
    long long start5 = getCurrentTimeNanos();
    int result5 = binarySearch(arr5, size5, target5);
    long long end5 = getCurrentTimeNanos();
    long long executionTime5 = end5 - start5;
    printf("Test 5: Array: []");
    printf(", Target: %d, Result: %d\\n", target5, result5);
    writeExecutionTime(executionTime5);
    
    return 0;
}
