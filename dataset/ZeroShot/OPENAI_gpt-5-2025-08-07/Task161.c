#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define MAX_CAPACITY 2000000

static long long max_ll(long long a, long long b) { return a > b ? a : b; }

long long maxKnapsackValue(int capacity, const int* weights, const int* values, int n) {
    if (capacity < 0) return 0LL;
    if (weights == NULL || values == NULL || n < 0) return -1LL;
    if (capacity > MAX_CAPACITY) return -1LL;

    for (int i = 0; i < n; ++i) {
        if (weights[i] < 0 || values[i] < 0) return -1LL;
    }

    long long* dp = (long long*)calloc((size_t)capacity + 1, sizeof(long long));
    if (!dp) return -1LL;

    for (int i = 0; i < n; ++i) {
        int wt = weights[i];
        long long val = values[i];
        if (wt <= capacity) {
            for (int c = capacity; c >= wt; --c) {
                long long base = dp[c - wt];
                long long candidate;
                if (base > LLONG_MAX - val) candidate = LLONG_MAX;
                else candidate = base + val;
                if (candidate > dp[c]) dp[c] = candidate;
            }
        }
    }

    long long result = dp[capacity];
    free(dp);
    return result;
}

int main(void) {
    // Test case 1
    int cap1 = 50;
    int w1[] = {10, 20, 30};
    int v1[] = {60, 100, 120};
    printf("%lld\n", maxKnapsackValue(cap1, w1, v1, 3)); // Expected 220

    // Test case 2
    int cap2 = 7;
    int w2[] = {1, 3, 4, 5};
    int v2[] = {1, 4, 5, 7};
    printf("%lld\n", maxKnapsackValue(cap2, w2, v2, 4)); // Expected 9

    // Test case 3
    int cap3 = 0;
    int w3[] = {1, 2, 3};
    int v3[] = {10, 15, 40};
    printf("%lld\n", maxKnapsackValue(cap3, w3, v3, 3)); // Expected 0

    // Test case 4
    int cap4 = 10;
    int w4[] = {3, 4, 5, 9, 4};
    int v4[] = {4, 5, 7, 10, 6};
    printf("%lld\n", maxKnapsackValue(cap4, w4, v4, 5)); // Expected 13

    // Test case 5
    int cap5 = 15;
    int w5[] = {12, 2, 1, 1, 4};
    int v5[] = {4, 2, 2, 1, 10};
    printf("%lld\n", maxKnapsackValue(cap5, w5, v5, 5)); // Expected 15

    return 0;
}