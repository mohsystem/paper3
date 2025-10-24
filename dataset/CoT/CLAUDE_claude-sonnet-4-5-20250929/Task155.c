
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Heapify a subtree rooted at index i
void heapify(int* arr, int n, int i) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    // Check if left child exists and is greater than root
    if (left < n && arr[left] > arr[largest]) {
        largest = left;
    }

    // Check if right child exists and is greater than largest so far
    if (right < n && arr[right] > arr[largest]) {
        largest = right;
    }

    // If largest is not root, swap and continue heapifying
    if (largest != i) {
        int temp = arr[i];
        arr[i] = arr[largest];
        arr[largest] = temp;

        heapify(arr, n, largest);
    }
}

// Main heap sort function
int* heapSort(const int* arr, int size, int* resultSize) {
    if (arr == NULL || size <= 0) {
        *resultSize = 0;
        return NULL;
    }

    // Create a copy to avoid modifying original array
    int* result = (int*)malloc(size * sizeof(int));
    if (result == NULL) {
        *resultSize = 0;
        return NULL;
    }

    memcpy(result, arr, size * sizeof(int));
    *resultSize = size;

    // Build max heap
    for (int i = size / 2 - 1; i >= 0; i--) {
        heapify(result, size, i);
    }

    // Extract elements from heap one by one
    for (int i = size - 1; i > 0; i--) {
        // Move current root to end
        int temp = result[0];
        result[0] = result[i];
        result[i] = temp;

        // Heapify the reduced heap
        heapify(result, i, 0);
    }

    return result;
}

void printArray(const int* arr, int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%d", arr[i]);
        if (i < size - 1) printf(", ");
    }
    printf("]\\n");
}

int main() {
    // Test case 1: Random integers
    int test1[] = {64, 34, 25, 12, 22, 11, 90};
    int size1 = sizeof(test1) / sizeof(test1[0]);
    printf("Test 1 - Input: ");
    printArray(test1, size1);
    int resultSize1;
    int* result1 = heapSort(test1, size1, &resultSize1);
    printf("Test 1 - Output: ");
    printArray(result1, resultSize1);
    free(result1);

    // Test case 2: Already sorted array
    int test2[] = {1, 2, 3, 4, 5};
    int size2 = sizeof(test2) / sizeof(test2[0]);
    printf("\\nTest 2 - Input: ");
    printArray(test2, size2);
    int resultSize2;
    int* result2 = heapSort(test2, size2, &resultSize2);
    printf("Test 2 - Output: ");
    printArray(result2, resultSize2);
    free(result2);

    // Test case 3: Reverse sorted array
    int test3[] = {9, 7, 5, 3, 1};
    int size3 = sizeof(test3) / sizeof(test3[0]);
    printf("\\nTest 3 - Input: ");
    printArray(test3, size3);
    int resultSize3;
    int* result3 = heapSort(test3, size3, &resultSize3);
    printf("Test 3 - Output: ");
    printArray(result3, resultSize3);
    free(result3);

    // Test case 4: Array with duplicates
    int test4[] = {5, 2, 8, 2, 9, 1, 5, 5};
    int size4 = sizeof(test4) / sizeof(test4[0]);
    printf("\\nTest 4 - Input: ");
    printArray(test4, size4);
    int resultSize4;
    int* result4 = heapSort(test4, size4, &resultSize4);
    printf("Test 4 - Output: ");
    printArray(result4, resultSize4);
    free(result4);

    // Test case 5: Single element array
    int test5[] = {42};
    int size5 = sizeof(test5) / sizeof(test5[0]);
    printf("\\nTest 5 - Input: ");
    printArray(test5, size5);
    int resultSize5;
    int* result5 = heapSort(test5, size5, &resultSize5);
    printf("Test 5 - Output: ");
    printArray(result5, resultSize5);
    free(result5);

    return 0;
}
