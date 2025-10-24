
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int max(int a, int b) {
    return (a > b) ? a : b;
}

int knapsack(int capacity, int weights[], int values[], int n) {
    // Input validation
    if (capacity < 0) {
        fprintf(stderr, "Error: Capacity cannot be negative\\n");
        return -1;
    }
    if (weights == NULL || values == NULL) {
        fprintf(stderr, "Error: Weights and values arrays cannot be NULL\\n");
        return -1;
    }
    if (n <= 0) {
        return 0;
    }
    
    // Validate individual weights and values
    for (int i = 0; i < n; i++) {
        if (weights[i] < 0 || values[i] < 0) {
            fprintf(stderr, "Error: Weights and values must be non-negative\\n");
            return -1;
        }
    }
    
    // Prevent integer overflow for large capacity
    if (capacity > INT_MAX / 2) {
        fprintf(stderr, "Error: Capacity is too large\\n");
        return -1;
    }
    
    // Dynamic programming approach
    int** dp = (int**)malloc((n + 1) * sizeof(int*));
    if (dp == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return -1;
    }
    
    for (int i = 0; i <= n; i++) {
        dp[i] = (int*)calloc(capacity + 1, sizeof(int));
        if (dp[i] == NULL) {
            fprintf(stderr, "Error: Memory allocation failed\\n");
            for (int j = 0; j < i; j++) {
                free(dp[j]);
            }
            free(dp);
            return -1;
        }
    }
    
    for (int i = 1; i <= n; i++) {
        for (int w = 0; w <= capacity; w++) {
            if (weights[i - 1] <= w) {
                dp[i][w] = max(
                    values[i - 1] + dp[i - 1][w - weights[i - 1]],
                    dp[i - 1][w]
                );
            } else {
                dp[i][w] = dp[i - 1][w];
            }
        }
    }
    
    int result = dp[n][capacity];
    
    // Free allocated memory
    for (int i = 0; i <= n; i++) {
        free(dp[i]);
    }
    free(dp);
    
    return result;
}

int main() {
    // Test case 1: Basic case
    int weights1[] = {2, 3, 4, 5};
    int values1[] = {3, 4, 5, 6};
    int capacity1 = 5;
    int n1 = 4;
    int result1 = knapsack(capacity1, weights1, values1, n1);
    printf("Test 1 - Capacity: %d, Max Value: %d\\n", capacity1, result1);
    
    // Test case 2: Single item
    int weights2[] = {10};
    int values2[] = {100};
    int capacity2 = 15;
    int n2 = 1;
    int result2 = knapsack(capacity2, weights2, values2, n2);
    printf("Test 2 - Capacity: %d, Max Value: %d\\n", capacity2, result2);
    
    // Test case 3: No items fit
    int weights3[] = {10, 20, 30};
    int values3[] = {60, 100, 120};
    int capacity3 = 5;
    int n3 = 3;
    int result3 = knapsack(capacity3, weights3, values3, n3);
    printf("Test 3 - Capacity: %d, Max Value: %d\\n", capacity3, result3);
    
    // Test case 4: All items fit
    int weights4[] = {1, 2, 3};
    int values4[] = {10, 20, 30};
    int capacity4 = 10;
    int n4 = 3;
    int result4 = knapsack(capacity4, weights4, values4, n4);
    printf("Test 4 - Capacity: %d, Max Value: %d\\n", capacity4, result4);
    
    // Test case 5: Zero capacity
    int weights5[] = {5, 10, 15};
    int values5[] = {50, 100, 150};
    int capacity5 = 0;
    int n5 = 3;
    int result5 = knapsack(capacity5, weights5, values5, n5);
    printf("Test 5 - Capacity: %d, Max Value: %d\\n", capacity5, result5);
    
    return 0;
}
