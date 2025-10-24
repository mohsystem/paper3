
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int* removeSmallest(const int* numbers, int size, int* resultSize) {
    if (numbers == NULL || size == 0) {
        *resultSize = 0;
        return NULL;
    }
    
    // Allocate memory for result
    int* result = (int*)malloc((size - 1) * sizeof(int));
    if (result == NULL) {
        *resultSize = 0;
        return NULL;
    }
    
    // Find the index of the smallest value
    int minIndex = 0;
    int minValue = numbers[0];
    
    for (int i = 1; i < size; i++) {
        if (numbers[i] < minValue) {
            minValue = numbers[i];
            minIndex = i;
        }
    }
    
    // Copy elements except the one at minIndex
    int j = 0;
    for (int i = 0; i < size; i++) {
        if (i != minIndex) {
            result[j++] = numbers[i];
        }
    }
    
    *resultSize = size - 1;
    return result;
}

void printArray(const int* arr, int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%d", arr[i]);
        if (i < size - 1) printf(", ");
    }
    printf("]");
}

int main() {
    // Test case 1
    int test1[] = {1, 2, 3, 4, 5};
    int resultSize1;
    int* result1 = removeSmallest(test1, 5, &resultSize1);
    printf("Input: ");
    printArray(test1, 5);
    printf(", Output: ");
    printArray(result1, resultSize1);
    printf("\\n");
    free(result1);
    
    // Test case 2
    int test2[] = {5, 3, 2, 1, 4};
    int resultSize2;
    int* result2 = removeSmallest(test2, 5, &resultSize2);
    printf("Input: ");
    printArray(test2, 5);
    printf(", Output: ");
    printArray(result2, resultSize2);
    printf("\\n");
    free(result2);
    
    // Test case 3
    int test3[] = {2, 2, 1, 2, 1};
    int resultSize3;
    int* result3 = removeSmallest(test3, 5, &resultSize3);
    printf("Input: ");
    printArray(test3, 5);
    printf(", Output: ");
    printArray(result3, resultSize3);
    printf("\\n");
    free(result3);
    
    // Test case 4
    int resultSize4;
    int* result4 = removeSmallest(NULL, 0, &resultSize4);
    printf("Input: [], Output: ");
    printArray(result4, resultSize4);
    printf("\\n");
    
    // Test case 5
    int test5[] = {10};
    int resultSize5;
    int* result5 = removeSmallest(test5, 1, &resultSize5);
    printf("Input: ");
    printArray(test5, 1);
    printf(", Output: ");
    printArray(result5, resultSize5);
    printf("\\n");
    free(result5);
    
    return 0;
}
