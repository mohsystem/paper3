
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int compare(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

void sortArray(int* arr, int size, int* result) {
    if (arr == NULL || size == 0) {
        return;
    }
    memcpy(result, arr, size * sizeof(int));
    qsort(result, size, sizeof(int), compare);
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
    // Test case 1: Regular unsorted array
    int test1[] = {5, 2, 8, 1, 9};
    int size1 = 5;
    int result1[5];
    sortArray(test1, size1, result1);
    printf("Test 1: ");
    printArray(result1, size1);
    
    // Test case 2: Already sorted array
    int test2[] = {1, 2, 3, 4, 5};
    int size2 = 5;
    int result2[5];
    sortArray(test2, size2, result2);
    printf("Test 2: ");
    printArray(result2, size2);
    
    // Test case 3: Reverse sorted array
    int test3[] = {9, 7, 5, 3, 1};
    int size3 = 5;
    int result3[5];
    sortArray(test3, size3, result3);
    printf("Test 3: ");
    printArray(result3, size3);
    
    // Test case 4: Array with duplicates
    int test4[] = {4, 2, 7, 2, 9, 4};
    int size4 = 6;
    int result4[6];
    sortArray(test4, size4, result4);
    printf("Test 4: ");
    printArray(result4, size4);
    
    // Test case 5: Single element array
    int test5[] = {42};
    int size5 = 1;
    int result5[1];
    sortArray(test5, size5, result5);
    printf("Test 5: ");
    printArray(result5, size5);
    
    return 0;
}
