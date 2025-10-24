
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// Maximum array size to prevent memory exhaustion attacks
#define MAX_ARRAY_SIZE 1000000

// Structure to return array with its size (safer than raw pointers)
typedef struct {
    int* data;
    size_t size;
} IntArray;

// Helper function to compare integers for qsort (safe standard library function)
int compare_ints(const void* a, const void* b) {
    // Null pointer checks
    if (a == NULL || b == NULL) {
        return 0;
    }
    int arg1 = *(const int*)a;
    int arg2 = *(const int*)b;
    
    // Safe comparison avoiding integer overflow
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

// Function to sort positive numbers while keeping negative numbers in place
// Returns allocated array - caller must free result.data
IntArray posNegSort(const int* arr, size_t size) {
    IntArray result = {NULL, 0};
    
    // Input validation - check for NULL pointer
    if (arr == NULL && size > 0) {
        return result; // Return empty result for invalid input
    }
    
    // Handle empty array - return empty result (safe behavior)
    if (size == 0) {
        return result;
    }
    
    // Validate size to prevent memory exhaustion
    if (size > MAX_ARRAY_SIZE) {
        return result; // Fail closed
    }
    
    // Allocate result array with overflow check
    if (size > SIZE_MAX / sizeof(int)) {
        return result; // Prevent integer overflow in allocation
    }
    
    result.data = (int*)malloc(size * sizeof(int));
    if (result.data == NULL) {
        return result; // malloc failed, return empty result
    }
    result.size = size;
    
    // Copy input to result with bounds checking
    memcpy(result.data, arr, size * sizeof(int));
    
    // Count positive numbers with overflow protection
    size_t pos_count = 0;
    for (size_t i = 0; i < size; i++) {
        if (arr[i] > 0) {
            pos_count++;
        }
    }
    
    // Allocate array for positive numbers with overflow check
    if (pos_count > 0) {
        if (pos_count > SIZE_MAX / sizeof(int)) {
            free(result.data);
            result.data = NULL;
            result.size = 0;
            return result;
        }
        
        int* positives = (int*)malloc(pos_count * sizeof(int));
        if (positives == NULL) {
            free(result.data);
            result.data = NULL;
            result.size = 0;
            return result;
        }
        
        // Extract positive numbers with bounds checking
        size_t pos_idx = 0;
        for (size_t i = 0; i < size; i++) {
            if (arr[i] > 0) {
                if (pos_idx >= pos_count) { // Bounds check
                    free(positives);
                    free(result.data);
                    result.data = NULL;
                    result.size = 0;
                    return result;
                }
                positives[pos_idx++] = arr[i];
            }
        }
        
        // Sort positive numbers using safe standard library function
        qsort(positives, pos_count, sizeof(int), compare_ints);
        
        // Place sorted positives back with bounds checking
        pos_idx = 0;
        for (size_t i = 0; i < size; i++) {
            if (result.data[i] > 0) {
                if (pos_idx >= pos_count) { // Bounds check
                    free(positives);
                    free(result.data);
                    result.data = NULL;
                    result.size = 0;
                    return result;
                }
                result.data[i] = positives[pos_idx++];
            }
        }
        
        // Free temporary array - memory safety
        free(positives);
    }
    
    return result;
}

int main(void) {
    // Test case 1: Mixed positive and negative numbers
    int test1[] = {6, 3, -2, 5, -8, 2, -2};
    IntArray result1 = posNegSort(test1, 7);
    printf("Test 1: ");
    if (result1.data != NULL) {
        for (size_t i = 0; i < result1.size; i++) {
            printf("%d ", result1.data[i]);
        }
        free(result1.data); // Free allocated memory
    }
    printf("\\n");
    
    // Test case 2: Multiple negatives at different positions
    int test2[] = {6, 5, 4, -1, 3, 2, -1, 1};
    IntArray result2 = posNegSort(test2, 8);
    printf("Test 2: ");
    if (result2.data != NULL) {
        for (size_t i = 0; i < result2.size; i++) {
            printf("%d ", result2.data[i]);
        }
        free(result2.data);
    }
    printf("\\n");
    
    // Test case 3: Mostly negative with one positive
    int test3[] = {-5, -5, -5, -5, 7, -5};
    IntArray result3 = posNegSort(test3, 6);
    printf("Test 3: ");
    if (result3.data != NULL) {
        for (size_t i = 0; i < result3.size; i++) {
            printf("%d ", result3.data[i]);
        }
        free(result3.data);
    }
    printf("\\n");
    
    // Test case 4: Empty array
    IntArray result4 = posNegSort(NULL, 0);
    printf("Test 4: (empty)\\n");
    if (result4.data != NULL) {
        free(result4.data);
    }
    
    // Test case 5: All positive numbers
    int test5[] = {9, 3, 7, 1, 5};
    IntArray result5 = posNegSort(test5, 5);
    printf("Test 5: ");
    if (result5.data != NULL) {
        for (size_t i = 0; i < result5.size; i++) {
            printf("%d ", result5.data[i]);
        }
        free(result5.data);
    }
    printf("\\n");
    
    return 0;
}
