
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

// Structure to hold array and its size for safe passing
typedef struct {
    int* data;
    size_t size;
} IntArray;

// Function to remove the smallest value from array (first occurrence)
// Returns a new IntArray without mutating the original
// Caller is responsible for freeing the returned array's data
// Security: validates all inputs, checks bounds, prevents buffer overflows
IntArray removeSmallest(const int* numbers, size_t size) {
    IntArray result = {NULL, 0};
    
    // Validate input pointer - NULL pointer check
    if (numbers == NULL && size > 0) {
        return result; // Return empty array for invalid input
    }
    
    // Empty array returns empty array as per spec
    if (size == 0) {
        return result;
    }
    
    // Security: validate size to prevent excessive memory allocation
    if (size > 1000000) {
        return result; // Return empty for unreasonably large input
    }
    
    // Find the minimum value and its index
    int minValue = INT_MAX;
    size_t minIndex = 0;
    
    // Security: explicit bounds checking in loop
    for (size_t i = 0; i < size; i++) {
        if (numbers[i] < minValue) {
            minValue = numbers[i];
            minIndex = i;
        }
    }
    
    // Calculate result size (one element removed)
    size_t resultSize = size - 1;
    
    // Security: check for potential overflow before allocation
    if (resultSize > 0) {
        // Allocate memory for result - use calloc for zero-initialization
        result.data = (int*)calloc(resultSize, sizeof(int));
        
        // Security: check if allocation succeeded
        if (result.data == NULL) {
            result.size = 0;
            return result;
        }
        
        // Copy elements excluding the minimum at minIndex
        size_t destIndex = 0;
        for (size_t i = 0; i < size; i++) {
            if (i != minIndex) {
                // Security: verify destIndex is within bounds before write
                if (destIndex < resultSize) {
                    result.data[destIndex] = numbers[i];
                    destIndex++;
                } else {
                    // Should never happen, but safety check
                    free(result.data);
                    result.data = NULL;
                    result.size = 0;
                    return result;
                }
            }
        }
        
        result.size = resultSize;
    }
    
    return result;
}

// Helper function to print array safely
void printArray(const int* arr, size_t size) {
    printf("[");
    for (size_t i = 0; i < size; i++) {
        printf("%d", arr[i]);
        if (i < size - 1) {
            printf(",");
        }
    }
    printf("]\\n");
}

int main(void) {
    // Test case 1: [1,2,3,4,5] -> [2,3,4,5]
    int test1[] = {1, 2, 3, 4, 5};
    IntArray result1 = removeSmallest(test1, 5);
    printf("Test 1: ");
    printArray(result1.data, result1.size);
    free(result1.data); // Free allocated memory
    
    // Test case 2: [5,3,2,1,4] -> [5,3,2,4]
    int test2[] = {5, 3, 2, 1, 4};
    IntArray result2 = removeSmallest(test2, 5);
    printf("Test 2: ");
    printArray(result2.data, result2.size);
    free(result2.data);
    
    // Test case 3: [2,2,1,2,1] -> [2,2,2,1]
    int test3[] = {2, 2, 1, 2, 1};
    IntArray result3 = removeSmallest(test3, 5);
    printf("Test 3: ");
    printArray(result3.data, result3.size);
    free(result3.data);
    
    // Test case 4: [] -> []
    IntArray result4 = removeSmallest(NULL, 0);
    printf("Test 4: ");
    printArray(result4.data, result4.size);
    // No free needed for empty array
    
    // Test case 5: [7] -> []
    int test5[] = {7};
    IntArray result5 = removeSmallest(test5, 1);
    printf("Test 5: ");
    printArray(result5.data, result5.size);
    // No free needed for empty result
    
    return 0;
}
