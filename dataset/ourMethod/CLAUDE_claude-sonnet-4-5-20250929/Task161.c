
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// Security: Validate input ranges to prevent integer overflow and excessive memory allocation
#define MAX_CAPACITY 100000
#define MAX_ITEMS 10000
#define MAX_WEIGHT 100000
#define MAX_VALUE 1000000

// Security: Helper function to safely compare and get maximum of two integers
static int safe_max(int a, int b) {
    return (a > b) ? a : b;
}

// Knapsack solver using dynamic programming
// Returns -1 on error, otherwise returns maximum value
// Security: All parameters validated, all memory properly managed
int knapsack(int capacity, const int* weights, const int* values, int n) {
    int* dp = NULL;
    int result = -1;
    
    // Security: Validate capacity bounds to prevent excessive memory allocation
    if (capacity < 0 || capacity > MAX_CAPACITY) {
        fprintf(stderr, "Error: Capacity out of valid range [0, %d]\\n", MAX_CAPACITY);
        return -1;
    }
    
    // Security: Validate pointers are not NULL
    if (weights == NULL || values == NULL) {
        fprintf(stderr, "Error: NULL pointer passed to knapsack\\n");
        return -1;
    }
    
    // Security: Validate number of items
    if (n <= 0 || n > MAX_ITEMS) {
        fprintf(stderr, "Error: Invalid number of items (must be 1-%d)\\n", MAX_ITEMS);
        return -1;
    }
    
    // Security: Validate all weights and values are within acceptable ranges
    for (int i = 0; i < n; ++i) {
        if (weights[i] < 0 || weights[i] > MAX_WEIGHT) {
            fprintf(stderr, "Error: Weight at index %d out of range [0, %d]\\n", i, MAX_WEIGHT);
            return -1;
        }
        if (values[i] < 0 || values[i] > MAX_VALUE) {
            fprintf(stderr, "Error: Value at index %d out of range [0, %d]\\n", i, MAX_VALUE);
            return -1;
        }
    }
    
    // Security: Allocate DP array with bounds check
    // capacity + 1 cannot overflow since capacity <= MAX_CAPACITY
    size_t dpSize = (size_t)(capacity + 1);
    
    // Security: Check for allocation overflow before malloc
    if (dpSize > SIZE_MAX / sizeof(int)) {
        fprintf(stderr, "Error: DP array size would overflow\\n");
        return -1;
    }
    
    // Security: Use calloc to initialize memory to zero and check return value
    dp = (int*)calloc(dpSize, sizeof(int));
    if (dp == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return -1;
    }
    
    // Dynamic programming solution
    for (int i = 0; i < n; ++i) {
        // Iterate backwards to avoid using updated values in same iteration
        for (int w = capacity; w >= weights[i]; --w) {
            // Security: Check for potential integer overflow before addition
            if (dp[w - weights[i]] > INT_MAX - values[i]) {
                fprintf(stderr, "Error: Integer overflow detected in value calculation\\n");
                // Security: Free allocated memory before returning
                free(dp);
                return -1;
            }
            dp[w] = safe_max(dp[w], dp[w - weights[i]] + values[i]);
        }
    }
    
    // Security: Store result before freeing memory
    result = dp[capacity];
    
    // Security: Free allocated memory exactly once
    free(dp);
    dp = NULL;
    
    return result;
}

int main(void) {
    // Test case 1: Basic example
    {
        int capacity = 50;
        int weights[] = {10, 20, 30};
        int values[] = {60, 100, 120};
        int n = 3;
        int result = knapsack(capacity, weights, values, n);
        printf("Test 1 - Capacity: %d, Result: %d\\n", capacity, result);
    }
    
    // Test case 2: Single item
    {
        int capacity = 10;
        int weights[] = {5};
        int values[] = {10};
        int n = 1;
        int result = knapsack(capacity, weights, values, n);
        printf("Test 2 - Capacity: %d, Result: %d\\n", capacity, result);
    }
    
    // Test case 3: Items heavier than capacity
    {
        int capacity = 5;
        int weights[] = {10, 20, 30};
        int values[] = {60, 100, 120};
        int n = 3;
        int result = knapsack(capacity, weights, values, n);
        printf("Test 3 - Capacity: %d, Result: %d\\n", capacity, result);
    }
    
    // Test case 4: Multiple items fit
    {
        int capacity = 100;
        int weights[] = {10, 20, 30, 40};
        int values[] = {10, 20, 30, 40};
        int n = 4;
        int result = knapsack(capacity, weights, values, n);
        printf("Test 4 - Capacity: %d, Result: %d\\n", capacity, result);
    }
    
    // Test case 5: Invalid input (negative capacity)
    {
        int capacity = -10;
        int weights[] = {10, 20};
        int values[] = {60, 100};
        int n = 2;
        int result = knapsack(capacity, weights, values, n);
        printf("Test 5 - Invalid capacity: %d, Result: %d\\n", capacity, result);
    }
    
    return 0;
}
