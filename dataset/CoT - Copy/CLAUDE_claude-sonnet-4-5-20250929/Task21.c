
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int* removeSmallest(const int* numbers, int length, int* resultLength) {
    // Handle NULL pointer or empty array
    if (numbers == NULL || length == 0) {
        *resultLength = 0;
        return NULL;
    }
    
    // Handle single element array
    if (length == 1) {
        *resultLength = 0;
        return NULL;
    }
    
    // Find the minimum value and its first index
    int minValue = numbers[0];
    int minIndex = 0;
    
    for (int i = 1; i < length; i++) {
        if (numbers[i] < minValue) {
            minValue = numbers[i];
            minIndex = i;
        }
    }
    
    // Allocate memory for result array
    int* result = (int*)malloc((length - 1) * sizeof(int));
    if (result == NULL) {
        *resultLength = 0;
        return NULL;
    }
    
    // Copy elements except the minimum
    int resultIndex = 0;
    for (int i = 0; i < length; i++) {
        if (i != minIndex) {
            result[resultIndex++] = numbers[i];
        }
    }
    
    *resultLength = length - 1;
    return result;
}

void printArray(const int* arr, int length) {
    printf("[");
    for (int i = 0; i < length; i++) {
        printf("%d", arr[i]);
        if (i < length - 1) printf(", ");
    }
    printf("]\\n");
}

int main() {
    int resultLength;
    int* result;
    
    // Test case 1
    int test1[] = {1, 2, 3, 4, 5};
    result = removeSmallest(test1, 5, &resultLength);
    printf("Test 1: ");
    printArray(result, resultLength);
    free(result);
    
    // Test case 2
    int test2[] = {5, 3, 2, 1, 4};
    result = removeSmallest(test2, 5, &resultLength);
    printf("Test 2: ");
    printArray(result, resultLength);
    free(result);
    
    // Test case 3
    int test3[] = {2, 2, 1, 2, 1};
    result = removeSmallest(test3, 5, &resultLength);
    printf("Test 3: ");
    printArray(result, resultLength);
    free(result);
    
    // Test case 4
    int test4[] = {};
    result = removeSmallest(test4, 0, &resultLength);
    printf("Test 4: ");
    printArray(result, resultLength);
    
    // Test case 5
    int test5[] = {10};
    result = removeSmallest(test5, 1, &resultLength);
    printf("Test 5: ");
    printArray(result, resultLength);
    
    return 0;
}
