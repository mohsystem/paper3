#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int knapsack(int capacity, const int* weights, const int* values, int n) {
    if (capacity < 0) {
        return -1; // invalid capacity
    }
    if (n < 0) {
        return -1; // invalid n
    }
    if ((weights == NULL && n != 0) || (values == NULL && n != 0)) {
        return -1; // invalid pointers
    }
    for (int i = 0; i < n; i++) {
        if (weights[i] < 0) {
            return -1; // negative weight invalid
        }
    }

    size_t size = (size_t)capacity + 1U;
    int* dp = (int*)calloc(size, sizeof(int));
    if (dp == NULL) {
        return -1; // allocation failure
    }

    for (int i = 0; i < n; i++) {
        int wt = weights[i];
        int val = values[i];
        if (wt > capacity) {
            continue;
        }
        for (int w = capacity; w >= wt; w--) {
            int cand = dp[(size_t)(w - wt)] + val;
            if (cand > dp[(size_t)w]) {
                dp[(size_t)w] = cand;
            }
        }
    }

    int result = dp[(size_t)capacity];
    free(dp);
    return result;
}

static void run_test(int test_num, int capacity, const int* weights, const int* values, int n, int expected) {
    int result = knapsack(capacity, weights, values, n);
    printf("Test %d -> result=%d expected=%d\n", test_num, result, expected);
}

int main(void) {
    // Test 1: Classic example
    int w1[] = {10, 20, 30};
    int v1[] = {60, 100, 120};
    run_test(1, 50, w1, v1, 3, 220);

    // Test 2: Empty items
    run_test(2, 10, NULL, NULL, 0, 0);

    // Test 3: Zero capacity
    int w3[] = {1, 2, 3};
    int v3[] = {10, 20, 30};
    run_test(3, 0, w3, v3, 3, 0);

    // Test 4: Items heavier than capacity
    int w4[] = {6, 7, 8};
    int v4[] = {10, 20, 30};
    run_test(4, 5, w4, v4, 3, 0);

    // Test 5: Mixed items
    int w5[] = {1, 3, 4, 5};
    int v5[] = {1, 4, 5, 7};
    run_test(5, 7, w5, v5, 4, 9);

    return 0;
}