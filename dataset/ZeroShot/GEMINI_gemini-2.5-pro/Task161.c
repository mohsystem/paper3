#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper function to find the maximum of two integers
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
 * @return The maximum value that can be put into the knapsack, or -1 on memory allocation error.
 */
int knapsack(int capacity, const int weights[], const int values[], int n) {
    // --- Secure Code: Input Validation ---
    if (capacity < 0 || n < 0 || (n > 0 && (weights == NULL || values == NULL))) {
        // Invalid arguments, returning 0 as a safe default.
        return 0;
    }
    
    if (n == 0 || capacity == 0) {
        return 0;
    }

    // dp[w] will be the maximum value that can be obtained for a knapsack of capacity w
    // --- Secure Code: Use calloc for zero-initialized memory and check for allocation failure. ---
    int* dp = (int*)calloc(capacity + 1, sizeof(int));
    if (dp == NULL) {
        perror("Failed to allocate memory for DP table");
        return -1; // Indicate error
    }

    // Build table dp[] in a bottom-up manner
    for (int i = 0; i < n; i++) {
        // --- Secure Code: Check for invalid item properties ---
        if (weights[i] < 0 || values[i] < 0) {
            // Skip items with negative weight or value.
            continue;
        }
        // Traverse backwards to ensure each item is considered only once (0/1 property)
        for (int w = capacity; w >= weights[i]; w--) {
            dp[w] = max(dp[w], values[i] + dp[w - weights[i]]);
        }
    }

    int result = dp[capacity];
    free(dp); // --- Secure Code: Free allocated memory ---
    return result;
}

void run_test_cases() {
    // Test Case 1: General case
    int capacity1 = 50;
    int values1[] = {60, 100, 120};
    int weights1[] = {10, 20, 30};
    int n1 = sizeof(values1) / sizeof(values1[0]);
    printf("Test Case 1: Max value = %d\n", knapsack(capacity1, weights1, values1, n1));

    // Test Case 2: Empty items
    int capacity2 = 10;
    int* values2 = NULL;
    int* weights2 = NULL;
    int n2 = 0;
    printf("Test Case 2: Max value = %d\n", knapsack(capacity2, weights2, values2, n2));

    // Test Case 3: Zero capacity
    int capacity3 = 0;
    int values3[] = {60, 100};
    int weights3[] = {10, 20};
    int n3 = sizeof(values3) / sizeof(values3[0]);
    printf("Test Case 3: Max value = %d\n", knapsack(capacity3, weights3, values3, n3));

    // Test Case 4: Items that partially fit
    int capacity4 = 5;
    int values4[] = {3, 4, 5, 6};
    int weights4[] = {2, 3, 4, 5};
    int n4 = sizeof(values4) / sizeof(values4[0]);
    printf("Test Case 4: Max value = %d\n", knapsack(capacity4, weights4, values4, n4));

    // Test Case 5: More complex case
    int capacity5 = 10;
    int values5[] = {10, 40, 30, 50};
    int weights5[] = {5, 4, 6, 3};
    int n5 = sizeof(values5) / sizeof(values5[0]);
    printf("Test Case 5: Max value = %d\n", knapsack(capacity5, weights5, values5, n5));
}

int main() {
    run_test_cases();
    return 0;
}