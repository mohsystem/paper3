#include <stdio.h>
#include <stdlib.h>

long long knapsack(int capacity, const int* weights, const int* values, size_t n) {
    if (capacity <= 0 || weights == NULL || values == NULL || n == 0) {
        return 0LL;
    }

    size_t dp_size = (size_t)capacity + 1;
    long long* dp = (long long*)calloc(dp_size, sizeof(long long));
    if (dp == NULL) {
        return 0LL; // Allocation failed, safe fallback
    }

    for (size_t i = 0; i < n; ++i) {
        int w = weights[i];
        int v = values[i];
        if (w <= 0 || v < 0) {
            continue; // Skip invalid items
        }
        for (int c = capacity; c >= w; --c) {
            long long cand = dp[(size_t)(c - w)] + (long long)v;
            if (cand > dp[(size_t)c]) {
                dp[(size_t)c] = cand;
            }
        }
    }

    long long result = dp[(size_t)capacity];
    free(dp);
    return result;
}

int main(void) {
    // Test case 1
    int cap1 = 50;
    int w1[] = {10, 20, 30};
    int v1[] = {60, 100, 120};
    printf("%lld\n", knapsack(cap1, w1, v1, sizeof(w1)/sizeof(w1[0]))); // Expected: 220

    // Test case 2
    int cap2 = 0;
    int w2[] = {5, 10};
    int v2[] = {10, 20};
    printf("%lld\n", knapsack(cap2, w2, v2, sizeof(w2)/sizeof(w2[0]))); // Expected: 0

    // Test case 3
    int cap3 = 7;
    int w3[] = {1, 3, 4, 5};
    int v3[] = {1, 4, 5, 7};
    printf("%lld\n", knapsack(cap3, w3, v3, sizeof(w3)/sizeof(w3[0]))); // Expected: 9

    // Test case 4
    int cap4 = 10;
    int w4[] = {6, 3, 4, 2};
    int v4[] = {30, 14, 16, 9};
    printf("%lld\n", knapsack(cap4, w4, v4, sizeof(w4)/sizeof(w4[0]))); // Expected: 46

    // Test case 5
    int cap5 = 5;
    int w5[] = {4, 2, 3};
    int v5[] = {10, 4, 7};
    printf("%lld\n", knapsack(cap5, w5, v5, sizeof(w5)/sizeof(w5[0]))); // Expected: 11

    return 0;
}