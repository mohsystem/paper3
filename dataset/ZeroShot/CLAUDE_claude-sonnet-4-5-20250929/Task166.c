
#include <stdio.h>
#include <stdlib.h>

int compare(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

int* posNegSort(int* arr, int size, int* resultSize) {
    *resultSize = size;
    
    if (arr == NULL || size == 0) {
        return NULL;
    }
    
    // Count positive numbers
    int posCount = 0;
    for (int i = 0; i < size; i++) {
        if (arr[i] > 0) {
            posCount++;
        }
    }
    
    // Extract and sort positive numbers
    int* positives = (int*)malloc(posCount * sizeof(int));
    int posIndex = 0;
    for (int i = 0; i < size; i++) {
        if (arr[i] > 0) {
            positives[posIndex++] = arr[i];
        }
    }
    qsort(positives, posCount, sizeof(int), compare);
    
    // Create result array
    int* result = (int*)malloc(size * sizeof(int));
    posIndex = 0;
    
    for (int i = 0; i < size; i++) {
        if (arr[i] < 0) {
            result[i] = arr[i];
        } else {
            result[i] = positives[posIndex++];
        }
    }
    
    free(positives);
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
    
    // Test case 1
    int test1[] = {6, 3, -2, 5, -8, 2, -2};
    int* result1 = posNegSort(test1, 7, &resultSize);
    printf("Test 1: ");
    printArray(result1, resultSize);
    free(result1);
    
    // Test case 2
    int test2[] = {6, 5, 4, -1, 3, 2, -1, 1};
    int* result2 = posNegSort(test2, 8, &resultSize);
    printf("Test 2: ");
    printArray(result2, resultSize);
    free(result2);
    
    // Test case 3
    int test3[] = {-5, -5, -5, -5, 7, -5};
    int* result3 = posNegSort(test3, 6, &resultSize);
    printf("Test 3: ");
    printArray(result3, resultSize);
    free(result3);
    
    // Test case 4
    int* result4 = posNegSort(NULL, 0, &resultSize);
    printf("Test 4: ");
    if (result4 == NULL) printf("[]\\n");
    
    // Test case 5
    int test5[] = {10, -3, 8, -1, 5, -7, 3};
    int* result5 = posNegSort(test5, 7, &resultSize);
    printf("Test 5: ");
    printArray(result5, resultSize);
    free(result5);
    
    return 0;
}
