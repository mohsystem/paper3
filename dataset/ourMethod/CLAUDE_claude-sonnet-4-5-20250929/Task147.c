
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

// Comparison function for qsort (descending order)
// Used for sorting array to find kth largest element
int compareDescending(const void* a, const void* b) {
    int arg1 = *(const int*)a;
    int arg2 = *(const int*)b;
    
    // Return positive if arg1 < arg2 for descending order
    if (arg1 < arg2) return 1;
    if (arg1 > arg2) return -1;
    return 0;
}

// Function to find kth largest element in array
// Returns 1 on success, 0 on failure
// result: pointer to store the kth largest element
// nums: input array
// size: size of the array (must be validated by caller)
// k: position of largest element to find (1-indexed)
int findKthLargest(int* result, const int* nums, size_t size, int k) {
    // Input validation: check for NULL pointers
    if (result == NULL || nums == NULL) {
        fprintf(stderr, "Error: NULL pointer passed to findKthLargest\\n");
        return 0;
    }
    
    // Input validation: check if array is empty
    if (size == 0) {
        fprintf(stderr, "Error: Array cannot be empty\\n");
        return 0;
    }
    
    // Input validation: check if k is in valid range [1, size]
    // Also check for integer overflow when comparing k with size
    if (k < 1 || (size_t)k > size) {
        fprintf(stderr, "Error: k must be between 1 and array size\\n");
        return 0;
    }
    
    // Check for potential integer overflow in allocation
    // SIZE_MAX / sizeof(int) gives max number of ints we can allocate
    if (size > SIZE_MAX / sizeof(int)) {
        fprintf(stderr, "Error: Array size too large for allocation\\n");
        return 0;
    }
    
    // Allocate memory for sorted copy to avoid modifying original array
    // This prevents side effects and maintains const correctness
    int* sortedNums = (int*)malloc(size * sizeof(int));
    if (sortedNums == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return 0;
    }
    
    // Copy array data - memcpy is safe here as we've validated sizes\n    // source and destination cannot overlap as sortedNums is newly allocated\n    memcpy(sortedNums, nums, size * sizeof(int));\n    \n    // Sort array in descending order using standard library qsort\n    // qsort is a well-tested and safe sorting function\n    qsort(sortedNums, size, sizeof(int), compareDescending);\n    \n    // Return kth largest element (k-1 for 0-based indexing)\n    // This access is safe as we've validated k <= size above
    *result = sortedNums[k - 1];
    
    // Free allocated memory to prevent memory leak
    free(sortedNums);
    sortedNums = NULL;
    
    return 1;
}

int main(void) {
    int result = 0;
    int success = 0;
    
    // Test case 1: Normal case with distinct elements
    int test1[] = {3, 2, 1, 5, 6, 4};
    size_t size1 = sizeof(test1) / sizeof(test1[0]);
    int k1 = 2;
    printf("Test 1: Array [3,2,1,5,6,4], k=2\\n");
    success = findKthLargest(&result, test1, size1, k1);
    if (success) {
        printf("Result: %d\\n", result);
        printf("Expected: 5\\n\\n");
    }
    
    // Test case 2: Array with duplicates
    int test2[] = {3, 2, 3, 1, 2, 4, 5, 5, 6};
    size_t size2 = sizeof(test2) / sizeof(test2[0]);
    int k2 = 4;
    printf("Test 2: Array [3,2,3,1,2,4,5,5,6], k=4\\n");
    success = findKthLargest(&result, test2, size2, k2);
    if (success) {
        printf("Result: %d\\n", result);
        printf("Expected: 4\\n\\n");
    }
    
    // Test case 3: Single element array
    int test3[] = {1};
    size_t size3 = sizeof(test3) / sizeof(test3[0]);
    int k3 = 1;
    printf("Test 3: Array [1], k=1\\n");
    success = findKthLargest(&result, test3, size3, k3);
    if (success) {
        printf("Result: %d\\n", result);
        printf("Expected: 1\\n\\n");
    }
    
    // Test case 4: Array with negative numbers
    int test4[] = {-1, -2, -3, -4, -5};
    size_t size4 = sizeof(test4) / sizeof(test4[0]);
    int k4 = 3;
    printf("Test 4: Array [-1,-2,-3,-4,-5], k=3\\n");
    success = findKthLargest(&result, test4, size4, k4);
    if (success) {
        printf("Result: %d\\n", result);
        printf("Expected: -3\\n\\n");
    }
    
    // Test case 5: Array with extreme values
    int test5[] = {INT_MAX, 0, INT_MIN, 100, -100};
    size_t size5 = sizeof(test5) / sizeof(test5[0]);
    int k5 = 2;
    printf("Test 5: Array [INT_MAX,0,INT_MIN,100,-100], k=2\\n");
    success = findKthLargest(&result, test5, size5, k5);
    if (success) {
        printf("Result: %d\\n", result);
        printf("Expected: 100\\n\\n");
    }
    
    return 0;
}
