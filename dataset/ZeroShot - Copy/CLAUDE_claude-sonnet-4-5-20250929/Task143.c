
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int compare(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

int* sortArray(int* arr, int size, int* resultSize) {
    if (arr == NULL || size == 0) {
        *resultSize = size;
        return arr;
    }
    
    int* result = (int*)malloc(size * sizeof(int));
    if (result == NULL) {
        *resultSize = 0;
        return NULL;
    }
    
    memcpy(result, arr, size * sizeof(int));
    qsort(result, size, sizeof(int), compare);
    *resultSize = size;
    return result;
}

void printArray(int* arr, int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%d", arr[i]);
        if (i < size - 1) printf(", ");
    }
    printf("]\\n");
}

int main() {
    int resultSize;
    
    // Test case 1: Regular array
    int test1[] = {5, 2, 8, 1, 9};
    int* result1 = sortArray(test1, 5, &resultSize);
    printf("Test 1: ");
    printArray(result1, resultSize);
    free(result1);
    
    // Test case 2: Already sorted array
    int test2[] = {1, 2, 3, 4, 5};
    int* result2 = sortArray(test2, 5, &resultSize);
    printf("Test 2: ");
    printArray(result2, resultSize);
    free(result2);
    
    // Test case 3: Reverse sorted array
    int test3[] = {9, 7, 5, 3, 1};
    int* result3 = sortArray(test3, 5, &resultSize);
    printf("Test 3: ");
    printArray(result3, resultSize);
    free(result3);
    
    // Test case 4: Array with duplicates
    int test4[] = {4, 2, 7, 2, 9, 4};
    int* result4 = sortArray(test4, 6, &resultSize);
    printf("Test 4: ");
    printArray(result4, resultSize);
    free(result4);
    
    // Test case 5: Single element array
    int test5[] = {42};
    int* result5 = sortArray(test5, 1, &resultSize);
    printf("Test 5: ");
    printArray(result5, resultSize);
    free(result5);
    
    return 0;
}
