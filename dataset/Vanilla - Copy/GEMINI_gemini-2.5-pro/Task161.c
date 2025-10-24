#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper function to find the maximum of two integers
int max(int a, int b) {
    return (a > b) ? a : b;
}

int knapsack(int capacity, int weights[], int values[], int n) {
    if (n == 0) {
        return 0;
    }

    int* dp = (int*)calloc(capacity + 1, sizeof(int));
    if (dp == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return -1; // Indicate error
    }

    for (int i = 0; i < n; i++) {
        for (int w = capacity; w >= weights[i]; w--) {
            dp[w] = max(dp[w], values[i] + dp[w - weights[i]]);
        }
    }

    int result = dp[capacity];
    free(dp);
    return result;
}

int main() {
    // Test Case 1
    int values1[] = {60, 100, 120};
    int weights1[] = {10, 20, 30};
    int n1 = sizeof(values1) / sizeof(values1[0]);
    int capacity1 = 50;
    printf("Test Case 1: Maximum value = %d\n", knapsack(capacity1, weights1, values1, n1));

    // Test Case 2
    int values2[] = {10, 20, 30};
    int weights2[] = {5, 10, 15};
    int n2 = sizeof(values2) / sizeof(values2[0]);
    int capacity2 = 50;
    printf("Test Case 2: Maximum value = %d\n", knapsack(capacity2, weights2, values2, n2));

    // Test Case 3
    int values3[] = {10, 40, 30, 50};
    int weights3[] = {5, 4, 6, 3};
    int n3 = sizeof(values3) / sizeof(values3[0]);
    int capacity3 = 10;
    printf("Test Case 3: Maximum value = %d\n", knapsack(capacity3, weights3, values3, n3));

    // Test Case 4
    int* values4 = NULL;
    int* weights4 = NULL;
    int n4 = 0;
    int capacity4 = 10;
    printf("Test Case 4: Maximum value = %d\n", knapsack(capacity4, weights4, values4, n4));
    
    // Test Case 5
    int values5[] = {60, 100, 120};
    int weights5[] = {10, 20, 30};
    int n5 = sizeof(values5) / sizeof(values5[0]);
    int capacity5 = 0;
    printf("Test Case 5: Maximum value = %d\n", knapsack(capacity5, weights5, values5, n5));

    return 0;
}