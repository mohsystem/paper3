
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

// Security: Define maximum array size to prevent excessive memory allocation
#define MAX_ARRAY_SIZE 1000000

// Security: Heapify function with strict bounds checking
// All array accesses validated against size parameter
void heapify(int* arr, size_t n, size_t i) {
    // Security: Validate index to prevent out-of-bounds access
    if (arr == NULL || i >= n) return;
    
    size_t largest = i;
    size_t left = 2 * i + 1;
    size_t right = 2 * i + 2;
    
    // Security: Check bounds before array access
    if (left < n && arr[left] > arr[largest]) {
        largest = left;
    }
    
    // Security: Check bounds before array access
    if (right < n && arr[right] > arr[largest]) {
        largest = right;
    }
    
    if (largest != i) {
        int temp = arr[i];
        arr[i] = arr[largest];
        arr[largest] = temp;
        heapify(arr, n, largest);
    }
}

// Security: Heap sort with NULL pointer checks and bounds validation
int heapSort(int* arr, size_t n) {
    // Security: Validate input pointer
    if (arr == NULL) {
        fprintf(stderr, "Error: NULL pointer provided\\n");
        return -1;
    }
    
    // Security: Validate array size
    if (n > MAX_ARRAY_SIZE) {
        fprintf(stderr, "Error: Array size exceeds maximum\\n");
        return -1;
    }
    
    // Security: Handle edge cases safely
    if (n <= 1) return 0;
    
    // Build max heap
    // Security: Careful with size_t underflow - handle i=0 case
    if (n >= 2) {
        for (size_t i = n / 2; i > 0; i--) {
            heapify(arr, n, i - 1);
        }
        heapify(arr, n, 0);
    }
    
    // Extract elements from heap
    for (size_t i = n - 1; i > 0; i--) {
        int temp = arr[0];
        arr[0] = arr[i];
        arr[i] = temp;
        heapify(arr, i, 0);
    }
    
    return 0;
}

// Security: Safe array printing with bounds checking
void printArray(const int* arr, size_t n) {
    // Security: NULL pointer check
    if (arr == NULL) {
        printf("(null)\\n");
        return;
    }
    
    // Security: Bounds-checked loop
    for (size_t i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\\n");
}

int main(void) {
    // Test case 1: Regular unsorted array
    {
        int arr[] = {12, 11, 13, 5, 6, 7};
        size_t n = sizeof(arr) / sizeof(arr[0]);
        printf("Test 1: ");
        if (heapSort(arr, n) == 0) {
            printArray(arr, n);
        }
    }
    
    // Test case 2: Already sorted array
    {
        int arr[] = {1, 2, 3, 4, 5};
        size_t n = sizeof(arr) / sizeof(arr[0]);
        printf("Test 2: ");
        if (heapSort(arr, n) == 0) {
            printArray(arr, n);
        }
    }
    
    // Test case 3: Reverse sorted array
    {
        int arr[] = {9, 8, 7, 6, 5, 4, 3, 2, 1};
        size_t n = sizeof(arr) / sizeof(arr[0]);
        printf("Test 3: ");
        if (heapSort(arr, n) == 0) {
            printArray(arr, n);
        }
    }
    
    // Test case 4: Array with duplicates
    {
        int arr[] = {5, 2, 8, 2, 9, 1, 5, 5};
        size_t n = sizeof(arr) / sizeof(arr[0]);
        printf("Test 4: ");
        if (heapSort(arr, n) == 0) {
            printArray(arr, n);
        }
    }
    
    // Test case 5: Edge cases - single element and empty
    {
        int arr1[] = {42};
        size_t n1 = sizeof(arr1) / sizeof(arr1[0]);
        printf("Test 5a: ");
        if (heapSort(arr1, n1) == 0) {
            printArray(arr1, n1);
        }
        
        // Security: Handle zero-size array safely
        int* arr2 = NULL;
        size_t n2 = 0;
        printf("Test 5b: ");
        // Security: Allocate minimal valid memory for empty array test
        arr2 = (int*)malloc(1);
        if (arr2 != NULL) {
            if (heapSort(arr2, n2) == 0) {
                printf("(empty)\\n");
            }
            // Security: Free allocated memory
            free(arr2);
            arr2 = NULL;
        }
    }
    
    return 0;
}
