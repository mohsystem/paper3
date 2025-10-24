
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Comparison function for qsort
 */
int compare(const void* a, const void* b) {
    int int_a = *((int*)a);
    int int_b = *((int*)b);
    
    if (int_a < int_b) return -1;
    if (int_a > int_b) return 1;
    return 0;
}

/**
 * Sorts an array of integers in ascending order
 * @param arr The input array to be sorted
 * @param size The size of the array
 * @param result Pre-allocated array to store sorted result
 * @return 0 on success, -1 on failure
 */
int sortArray(const int* arr, int size, int* result) {
    // Input validation
    if (arr == NULL || result == NULL || size < 0) {
        return -1;
    }
    
    if (size == 0) {
        return 0;
    }
    
    // Copy array to result to avoid modifying original
    memcpy(result, arr, size * sizeof(int));
    
    // Use qsort which is the standard C library sorting function
    qsort(result, size, sizeof(int), compare);
    
    return 0;
}

/**
 * Helper function to print array
 */
void printArray(const int* arr, int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%d", arr[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("]\\n");
}

int main() {
    // Test case 1: Normal array
    int test1[] = {5, 2, 8, 1, 9};
    int size1 = sizeof(test1) / sizeof(test1[0]);
    int result1[size1];
    sortArray(test1, size1, result1);
    printf("Test 1: ");
    printArray(result1, size1);
    
    // Test case 2: Already sorted array
    int test2[] = {1, 2, 3, 4, 5};
    int size2 = sizeof(test2) / sizeof(test2[0]);
    int result2[size2];
    sortArray(test2, size2, result2);
    printf("Test 2: ");
    printArray(result2, size2);
    
    // Test case 3: Reverse sorted array
    int test3[] = {9, 7, 5, 3, 1};
    int size3 = sizeof(test3) / sizeof(test3[0]);
    int result3[size3];
    sortArray(test3, size3, result3);
    printf("Test 3: ");
    printArray(result3, size3);
    
    // Test case 4: Array with duplicates
    int test4[] = {4, 2, 7, 2, 9, 4};
    int size4 = sizeof(test4) / sizeof(test4[0]);
    int result4[size4];
    sortArray(test4, size4, result4);
    printf("Test 4: ");
    printArray(result4, size4);
    
    // Test case 5: Empty array
    int test5[] = {};
    int size5 = 0;
    int result5[1];
    sortArray(test5, size5, result5);
    printf("Test 5: ");
    printArray(result5, size5);
    
    return 0;
}
