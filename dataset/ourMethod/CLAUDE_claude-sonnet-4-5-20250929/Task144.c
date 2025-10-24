
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

// Structure to hold array and its size
typedef struct {
    int* data;
    size_t size;
} IntArray;

// Merges two sorted arrays into a single sorted array
// Security: Validates all inputs, checks for overflow, and manages memory safely
// Returns NULL on error (caller must check)
IntArray* mergeSortedArrays(const int* arr1, size_t size1, const int* arr2, size_t size2) {
    // Input validation: NULL pointer checks
    if ((arr1 == NULL && size1 > 0) || (arr2 == NULL && size2 > 0)) {
        return NULL;
    }
    
    // Handle empty arrays
    if (size1 == 0 && size2 == 0) {
        IntArray* result = (IntArray*)malloc(sizeof(IntArray));
        if (result == NULL) {
            return NULL;
        }
        result->data = NULL;
        result->size = 0;
        return result;
    }
    
    // Security: Maximum safe size to prevent excessive memory allocation (100MB limit)
    const size_t MAX_SAFE_SIZE = 100000000 / sizeof(int);
    
    if (size1 > MAX_SAFE_SIZE || size2 > MAX_SAFE_SIZE) {
        return NULL;
    }
    
    // Security: Check for integer overflow in size addition
    if (size1 > SIZE_MAX - size2) {
        return NULL;
    }
    
    size_t totalSize = size1 + size2;
    if (totalSize > MAX_SAFE_SIZE) {
        return NULL;
    }
    
    // Allocate result structure
    IntArray* result = (IntArray*)malloc(sizeof(IntArray));
    if (result == NULL) {
        return NULL;
    }
    
    // Initialize to NULL for safe cleanup on error
    result->data = NULL;
    result->size = 0;
    
    // Security: Check for integer overflow in memory allocation size calculation
    if (totalSize > SIZE_MAX / sizeof(int)) {
        free(result);
        return NULL;
    }
    
    // Allocate memory for merged array
    result->data = (int*)malloc(totalSize * sizeof(int));
    if (result->data == NULL) {
        free(result);
        return NULL;
    }
    
    result->size = totalSize;
    
    // Two-pointer merge algorithm with strict bounds checking
    size_t i = 0;  // Index for arr1
    size_t j = 0;  // Index for arr2
    size_t k = 0;  // Index for result
    
    // Merge while both arrays have elements
    // Bounds checking: ensure i < size1 and j < size2
    while (i < size1 && j < size2) {
        // Additional bounds check for result array
        if (k >= totalSize) {
            free(result->data);
            free(result);
            return NULL;
        }
        
        if (arr1[i] <= arr2[j]) {
            result->data[k] = arr1[i];
            i++;
        } else {
            result->data[k] = arr2[j];
            j++;
        }
        k++;
    }
    
    // Copy remaining elements from arr1 (if any)
    while (i < size1) {
        if (k >= totalSize) {
            free(result->data);
            free(result);
            return NULL;
        }
        result->data[k] = arr1[i];
        i++;
        k++;
    }
    
    // Copy remaining elements from arr2 (if any)
    while (j < size2) {
        if (k >= totalSize) {
            free(result->data);
            free(result);
            return NULL;
        }
        result->data[k] = arr2[j];
        j++;
        k++;
    }
    
    return result;
}

// Helper function to free IntArray structure
void freeIntArray(IntArray* arr) {
    if (arr != NULL) {
        if (arr->data != NULL) {
            free(arr->data);
            arr->data = NULL;
        }
        free(arr);
    }
}

// Helper function to print array
void printArray(const int* arr, size_t size) {
    printf("[");
    for (size_t i = 0; i < size; i++) {
        printf("%d", arr[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("]\\n");
}

int main(void) {
    // Test case 1: Normal case with positive integers
    int arr1_1[] = {1, 3, 5, 7};
    int arr2_1[] = {2, 4, 6, 8};
    IntArray* result1 = mergeSortedArrays(arr1_1, 4, arr2_1, 4);
    if (result1 != NULL) {
        printf("Test 1: ");
        printArray(result1->data, result1->size);
        freeIntArray(result1);
    } else {
        printf("Test 1: Error\\n");
    }
    
    // Test case 2: One empty array
    int arr1_2[] = {1, 2, 3};
    IntArray* result2 = mergeSortedArrays(arr1_2, 3, NULL, 0);
    if (result2 != NULL) {
        printf("Test 2: ");
        printArray(result2->data, result2->size);
        freeIntArray(result2);
    } else {
        printf("Test 2: Error\\n");
    }
    
    // Test case 3: Arrays with negative and positive integers
    int arr1_3[] = {-5, -1, 3, 10};
    int arr2_3[] = {-3, 0, 5, 12};
    IntArray* result3 = mergeSortedArrays(arr1_3, 4, arr2_3, 4);
    if (result3 != NULL) {
        printf("Test 3: ");
        printArray(result3->data, result3->size);
        freeIntArray(result3);
    } else {
        printf("Test 3: Error\\n");
    }
    
    // Test case 4: Arrays with duplicate values
    int arr1_4[] = {1, 3, 3, 7};
    int arr2_4[] = {3, 5, 7, 9};
    IntArray* result4 = mergeSortedArrays(arr1_4, 4, arr2_4, 4);
    if (result4 != NULL) {
        printf("Test 4: ");
        printArray(result4->data, result4->size);
        freeIntArray(result4);
    } else {
        printf("Test 4: Error\\n");
    }
    
    // Test case 5: Both empty arrays
    IntArray* result5 = mergeSortedArrays(NULL, 0, NULL, 0);
    if (result5 != NULL) {
        printf("Test 5: ");
        printArray(result5->data, result5->size);
        freeIntArray(result5);
    } else {
        printf("Test 5: Error\\n");
    }
    
    return 0;
}
