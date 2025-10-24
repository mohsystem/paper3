#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A utility function that returns maximum of two integers
int max(int a, int b) {
    return (a > b) ? a : b;
}

/**
 * Solves the 0/1 knapsack problem using dynamic programming.
 *
 * @param capacity The maximum weight capacity of the knapsack.
 * @param weights  An array of weights of the items.
 * @param values   An array of values of the items.
 * @param n        The number of items.
 * @return The maximum value that can be put into the knapsack.
 */
int knapSack(int capacity, int weights[], int values[], int n) {
    // Basic input validation
    if (n <= 0 || capacity <= 0) {
        return 0;
    }

    // dp[w] will be storing the maximum value that can be obtained with knapsack capacity w.
    // Use calloc to allocate and initialize memory to zero.
    int* dp = (int*)calloc(capacity + 1, sizeof(int));
    if (dp == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return -1; // Indicate error
    }

    // Traverse through all items
    for (int i = 0; i < n; i++) {
        // Traverse dp array from right to left to prevent using the same item multiple times
        for (int w = capacity; w >= weights[i]; w--) {
            dp[w] = max(dp[w], values[i] + dp[w - weights[i]]);
        }
    }
    
    int result = dp[capacity];
    free(dp); // Free the allocated memory
    return result;
}

int main() {
    // Test Case 1: Standard case
    int values1[] = {60, 100, 120};
    int weights1[] = {10, 20, 30};
    int capacity1 = 50;
    int n1 = sizeof(values1) / sizeof(values1[0]);
    printf("Test Case 1: Maximum value is %d\n", knapSack(capacity1, weights1, values1, n1));

    // Test Case 2: Small capacity with more items
    int values2[] = {10, 40, 30, 50};
    int weights2[] = {5, 4, 6, 3};
    int capacity2 = 10;
    int n2 = sizeof(values2) / sizeof(values2[0]);
    printf("Test Case 2: Maximum value is %d\n", knapSack(capacity2, weights2, values2, n2));

    // Test Case 3: All items fit
    int values3[] = {60, 100, 120};
    int weights3[] = {10, 20, 30};
    int capacity3 = 100;
    int n3 = sizeof(values3) / sizeof(values3[0]);
    printf("Test Case 3: Maximum value is %d\n", knapSack(capacity3, weights3, values3, n3));

    // Test Case 4: No items
    int values4[] = {};
    int weights4[] = {};
    int capacity4 = 50;
    int n4 = 0;
    printf("Test Case 4: Maximum value is %d\n", knapSack(capacity4, weights4, values4, n4));

    // Test Case 5: Zero capacity
    int values5[] = {60, 100, 120};
    int weights5[] = {10, 20, 30};
    int capacity5 = 0;
    int n5 = sizeof(values5) / sizeof(values5[0]);
    printf("Test Case 5: Maximum value is %d\n", knapSack(capacity5, weights5, values5, n5));

    return 0;
}