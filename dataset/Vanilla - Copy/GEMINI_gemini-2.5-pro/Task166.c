#include <stdio.h>
#include <stdlib.h>

void printArray(const int* arr, int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%d", arr[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("]");
}

// Comparison function for qsort
int compare(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

void posNegSort(int* arr, int size) {
    if (arr == NULL || size == 0) {
        return;
    }

    // 1. Count and extract positive numbers
    int pos_count = 0;
    for (int i = 0; i < size; i++) {
        if (arr[i] > 0) {
            pos_count++;
        }
    }
    
    if (pos_count == 0) {
        return; // No positives to sort
    }

    int* positives = (int*)malloc(pos_count * sizeof(int));
    if (positives == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }

    int j = 0;
    for (int i = 0; i < size; i++) {
        if (arr[i] > 0) {
            positives[j++] = arr[i];
        }
    }

    // 2. Sort the positive numbers
    qsort(positives, pos_count, sizeof(int), compare);

    // 3. Place sorted positives back into the original array
    j = 0;
    for (int i = 0; i < size; i++) {
        if (arr[i] > 0) {
            arr[i] = positives[j++];
        }
    }

    free(positives);
}

int main() {
    // Test Case 1
    int arr1[] = {6, 3, -2, 5, -8, 2, -2};
    int size1 = sizeof(arr1) / sizeof(arr1[0]);
    printf("Test 1 Input: "); printArray(arr1, size1); printf("\n");
    posNegSort(arr1, size1);
    printf("Test 1 Output: "); printArray(arr1, size1); printf("\n");

    // Test Case 2
    int arr2[] = {6, 5, 4, -1, 3, 2, -1, 1};
    int size2 = sizeof(arr2) / sizeof(arr2[0]);
    printf("Test 2 Input: "); printArray(arr2, size2); printf("\n");
    posNegSort(arr2, size2);
    printf("Test 2 Output: "); printArray(arr2, size2); printf("\n");

    // Test Case 3
    int arr3[] = {-5, -5, -5, -5, 7, -5};
    int size3 = sizeof(arr3) / sizeof(arr3[0]);
    printf("Test 3 Input: "); printArray(arr3, size3); printf("\n");
    posNegSort(arr3, size3);
    printf("Test 3 Output: "); printArray(arr3, size3); printf("\n");

    // Test Case 4
    int* arr4 = NULL;
    int size4 = 0;
    printf("Test 4 Input: "); printArray(arr4, size4); printf("\n");
    posNegSort(arr4, size4);
    printf("Test 4 Output: "); printArray(arr4, size4); printf("\n");

    // Test Case 5
    int arr5[] = {-1, -2, -3, 1, 2, 3};
    int size5 = sizeof(arr5) / sizeof(arr5[0]);
    printf("Test 5 Input: "); printArray(arr5, size5); printf("\n");
    posNegSort(arr5, size5);
    printf("Test 5 Output: "); printArray(arr5, size5); printf("\n");

    return 0;
}