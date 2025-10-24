#include <stdio.h>
#include <stdlib.h>

int knapsack(int capacity, int weights[], int values[], int n) {
    if (capacity <= 0 || n <= 0 || weights == NULL || values == NULL) return 0;
    int *dp = (int*)calloc((size_t)capacity + 1, sizeof(int));
    if (!dp) return 0;
    for (int i = 0; i < n; ++i) {
        int w = weights[i];
        int v = values[i];
        for (int c = capacity; c >= w; --c) {
            int candidate = dp[c - w] + v;
            if (candidate > dp[c]) dp[c] = candidate;
        }
    }
    int result = dp[capacity];
    free(dp);
    return result;
}

int main() {
    // Test case 1
    int cap1 = 5;
    int w1[] = {1, 2, 3};
    int v1[] = {6, 10, 12};
    printf("%d\n", knapsack(cap1, w1, v1, 3)); // Expected 22

    // Test case 2
    int cap2 = 3;
    int w2[] = {4, 5};
    int v2[] = {7, 8};
    printf("%d\n", knapsack(cap2, w2, v2, 2)); // Expected 0

    // Test case 3
    int cap3 = 0;
    int w3[] = {1, 2};
    int v3[] = {10, 20};
    printf("%d\n", knapsack(cap3, w3, v3, 2)); // Expected 0

    // Test case 4
    int cap4 = 4;
    int w4[] = {4};
    int v4[] = {10};
    printf("%d\n", knapsack(cap4, w4, v4, 1)); // Expected 10

    // Test case 5
    int cap5 = 10;
    int w5[] = {2, 5, 1, 3, 4};
    int v5[] = {3, 4, 7, 8, 9};
    printf("%d\n", knapsack(cap5, w5, v5, 5)); // Expected 27

    return 0;
}