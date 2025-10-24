#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long long max(long long a, long long b) {
    return (a > b) ? a : b;
}

/**
 * Solves the 0/1 knapsack problem.
 *
 * @param capacity The maximum weight capacity of the knapsack.
 * @param weights  An array of weights of the items.
 * @param values   An array of values of the items.
 * @param n        The number of items.
 * @return The maximum value that can be put into the knapsack, or -1 if input is invalid.
 */
long long knapsack(int capacity, const int* weights, const int* values, int n) {
    // Input validation
    if (capacity < 0 || n < 0 || (n > 0 && (weights == NULL || values == NULL))) {
        return -1;
    }
    for (int i = 0; i < n; ++i) {
        if (weights[i] < 0 || values[i] < 0) return -1;
    }

    if (n == 0 || capacity == 0) {
        return 0;
    }

    // DP table to store results. Using long long to prevent overflow.
    long long** dp = (long long**)malloc((n + 1) * sizeof(long long*));
    if (dp == NULL) {
        perror("Failed to allocate memory for DP table rows");
        return -1;
    }

    for (int i = 0; i <= n; i++) {
        dp[i] = (long long*)malloc((capacity + 1) * sizeof(long long));
        if (dp[i] == NULL) {
            perror("Failed to allocate memory for DP table columns");
            // Cleanup previously allocated memory
            for (int j = 0; j < i; j++) {
                free(dp[j]);
            }
            free(dp);
            return -1;
        }
    }

    // Build table dp[][] in bottom up manner
    for (int i = 0; i <= n; i++) {
        for (int w = 0; w <= capacity; w++) {
            if (i == 0 || w == 0) {
                dp[i][w] = 0;
            } else {
                int currentWeight = weights[i - 1];
                int currentValue = values[i - 1];
                if (currentWeight <= w) {
                    dp[i][w] = max((long long)currentValue + dp[i - 1][w - currentWeight], dp[i - 1][w]);
                } else {
                    dp[i][w] = dp[i - 1][w];
                }
            }
        }
    }

    long long result = dp[n][capacity];

    // Free allocated memory
    for (int i = 0; i <= n; i++) {
        free(dp[i]);
    }
    free(dp);

    return result;
}

int main() {
    // Test Case 1: Standard case
    int weights1[] = {10, 20, 30};
    int values1[] = {60, 100, 120};
    int capacity1 = 50;
    int n1 = sizeof(weights1) / sizeof(weights1[0]);
    printf("Test Case 1: Max value = %lld\n", knapsack(capacity1, weights1, values1, n1)); // Expected: 220

    // Test Case 2: No items
    int weights2[] = {};
    int values2[] = {};
    int capacity2 = 10;
    int n2 = 0;
    printf("Test Case 2: Max value = %lld\n", knapsack(capacity2, weights2, values2, n2)); // Expected: 0

    // Test Case 3: Capacity too small for any item
    int weights3[] = {10, 20};
    int values3[] = {100, 200};
    int capacity3 = 5;
    int n3 = sizeof(weights3) / sizeof(weights3[0]);
    printf("Test Case 3: Max value = %lld\n", knapsack(capacity3, weights3, values3, n3)); // Expected: 0

    // Test Case 4: All items fit
    int weights4[] = {10, 20, 30};
    int values4[] = {60, 100, 120};
    int capacity4 = 100;
    int n4 = sizeof(weights4) / sizeof(weights4[0]);
    printf("Test Case 4: Max value = %lld\n", knapsack(capacity4, weights4, values4, n4)); // Expected: 280

    // Test Case 5: Complex case
    int weights5[] = {5, 4, 6, 3};
    int values5[] = {10, 40, 30, 50};
    int capacity5 = 10;
    int n5 = sizeof(weights5) / sizeof(weights5[0]);
    printf("Test Case 5: Max value = %lld\n", knapsack(capacity5, weights5, values5, n5)); // Expected: 90

    return 0;
}