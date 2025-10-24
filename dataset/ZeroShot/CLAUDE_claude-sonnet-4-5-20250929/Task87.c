
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
    char tempFileName[L_tmpnam];
    tmpnam(tempFileName);
    char fileName[256];
    snprintf(fileName, sizeof(fileName), "%s_execution_time.txt", tempFileName);
    
    FILE* tempFile = fopen(fileName, "w");
    if (tempFile != NULL) {
        fprintf(tempFile, "Binary Search Execution Time: %lld nanoseconds\\n", executionTime);
        fclose(tempFile);
        printf("Execution time written to: %s\\n", fileName);
    } else {
        fprintf(stderr, "Error writing to file\\n");
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

int main() {
    struct timespec start, end;
    
    // Test case 1
    int arr1[] = {1, 3, 5, 7, 9, 11, 13, 15};
    int size1 = sizeof(arr1) / sizeof(arr1[0]);
    int target1 = 7;
    clock_gettime(CLOCK_MONOTONIC, &start);
    int result1 = binarySearch(arr1, size1, target1);
    clock_gettime(CLOCK_MONOTONIC, &end);
    long long duration1 = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
    printf("Test 1: Array: ");
    printArray(arr1, size1);
    printf(", Target: %d, Result: %d\\n", target1, result1);
    writeExecutionTime(duration1);
    
    // Test case 2
    int arr2[] = {2, 4, 6, 8, 10, 12, 14, 16, 18, 20};
    int size2 = sizeof(arr2) / sizeof(arr2[0]);
    int target2 = 12;
    clock_gettime(CLOCK_MONOTONIC, &start);
    int result2 = binarySearch(arr2, size2, target2);
    clock_gettime(CLOCK_MONOTONIC, &end);
    long long duration2 = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
    printf("Test 2: Array: ");
    printArray(arr2, size2);
    printf(", Target: %d, Result: %d\\n", target2, result2);
    writeExecutionTime(duration2);
    
    // Test case 3
    int arr3[] = {1, 2, 3, 4, 5};
    int size3 = sizeof(arr3) / sizeof(arr3[0]);
    int target3 = 6;
    clock_gettime(CLOCK_MONOTONIC, &start);
    int result3 = binarySearch(arr3, size3, target3);
    clock_gettime(CLOCK_MONOTONIC, &end);
    long long duration3 = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
    printf("Test 3: Array: ");
    printArray(arr3, size3);
    printf(", Target: %d, Result: %d\\n", target3, result3);
    writeExecutionTime(duration3);
    
    // Test case 4
    int arr4[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    int size4 = sizeof(arr4) / sizeof(arr4[0]);
    int target4 = 10;
    clock_gettime(CLOCK_MONOTONIC, &start);
    int result4 = binarySearch(arr4, size4, target4);
    clock_gettime(CLOCK_MONOTONIC, &end);
    long long duration4 = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
    printf("Test 4: Array: ");
    printArray(arr4, size4);
    printf(", Target: %d, Result: %d\\n", target4, result4);
    writeExecutionTime(duration4);
    
    // Test case 5
    int arr5[] = {5, 15, 25, 35, 45, 55};
    int size5 = sizeof(arr5) / sizeof(arr5[0]);
    int target5 = 55;
    clock_gettime(CLOCK_MONOTONIC, &start);
    int result5 = binarySearch(arr5, size5, target5);
    clock_gettime(CLOCK_MONOTONIC, &end);
    long long duration5 = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
    printf("Test 5: Array: ");
    printArray(arr5, size5);
    printf(", Target: %d, Result: %d\\n", target5, result5);
    writeExecutionTime(duration5);
    
    return 0;
}
