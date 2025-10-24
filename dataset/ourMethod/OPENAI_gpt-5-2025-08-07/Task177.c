#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

long long constrained_subsequence_sum(const int* nums, size_t n, int k) {
    if (nums == NULL || n == 0 || k < 1 || (size_t)k > n) {
        return LLONG_MIN; // Signal invalid input
    }

    long long* dp = (long long*)malloc(n * sizeof(long long));
    size_t* deq = (size_t*)malloc(n * sizeof(size_t));
    if (dp == NULL || deq == NULL) {
        free(dp);
        free(deq);
        return LLONG_MIN; // Allocation failure
    }

    size_t head = 0, tail = 0; // deque indices represent [head, tail)
    long long ans = LLONG_MIN;

    for (size_t i = 0; i < n; ++i) {
        while (head < tail && deq[head] + (size_t)k < i + 1) {
            // deq[head] < i - k
            head++;
        }
        long long best_prev = 0;
        if (head < tail) {
            long long val = dp[deq[head]];
            if (val > 0) best_prev = val;
        }
        dp[i] = (long long)nums[i] + best_prev;
        if (dp[i] > ans) ans = dp[i];

        while (head < tail && dp[deq[tail - 1]] <= dp[i]) {
            tail--;
        }
        deq[tail++] = i;
    }

    free(dp);
    free(deq);
    return ans;
}

static void run_test(const int* nums, size_t n, int k) {
    long long res = constrained_subsequence_sum(nums, n, k);
    if (res == LLONG_MIN) {
        printf("Error: invalid input or allocation failure\n");
    } else {
        printf("Result: %lld\n", res);
    }
}

int main(void) {
    // 5 test cases
    int a1[] = {10, 2, -10, 5, 20};
    run_test(a1, sizeof(a1)/sizeof(a1[0]), 2); // Expected 37

    int a2[] = {-1, -2, -3};
    run_test(a2, sizeof(a2)/sizeof(a2[0]), 1); // Expected -1

    int a3[] = {10, -2, -10, -5, 20};
    run_test(a3, sizeof(a3)/sizeof(a3[0]), 2); // Expected 23

    int a4[] = {5};
    run_test(a4, sizeof(a4)/sizeof(a4[0]), 1); // Expected 5

    int a5[] = {1, -1, 1, -1, 1};
    run_test(a5, sizeof(a5)/sizeof(a5[0]), 4); // Expected 3

    return 0;
}