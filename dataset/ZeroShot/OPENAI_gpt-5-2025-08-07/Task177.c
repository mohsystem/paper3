#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

long long constrainedSubsetSum(const int* nums, int n, int k) {
    if (nums == NULL || n <= 0 || k <= 0) return 0LL;
    long long* dp = (long long*)malloc((size_t)n * sizeof(long long));
    if (dp == NULL) {
        return 0LL;
    }
    int* dq = (int*)malloc((size_t)n * sizeof(int)); // deque of indices
    if (dq == NULL) {
        free(dp);
        return 0LL;
    }
    int head = 0, tail = 0; // [head, tail)
    long long ans = LLONG_MIN;

    for (int i = 0; i < n; ++i) {
        while (head < tail && dq[head] < i - k) head++;
        long long best = (head < tail) ? dp[dq[head]] : 0LL;
        dp[i] = (long long)nums[i] + (best > 0 ? best : 0LL);
        if (dp[i] > ans) ans = dp[i];
        while (head < tail && dp[dq[tail - 1]] <= dp[i]) tail--;
        dq[tail++] = i;
    }

    free(dp);
    free(dq);
    return ans;
}

static void runTest(const int* nums, int n, int k) {
    long long res = constrainedSubsetSum(nums, n, k);
    printf("%lld\n", res);
}

int main(void) {
    int a1[] = {10, 2, -10, 5, 20};
    int a2[] = {-1, -2, -3};
    int a3[] = {10, -2, -10, -5, 20};
    int a4[] = {1, -1, -2, 4, -7, 3};
    int a5[] = {5, -1, 5};

    runTest(a1, (int)(sizeof(a1) / sizeof(a1[0])), 2);
    runTest(a2, (int)(sizeof(a2) / sizeof(a2[0])), 1);
    runTest(a3, (int)(sizeof(a3) / sizeof(a3[0])), 2);
    runTest(a4, (int)(sizeof(a4) / sizeof(a4[0])), 2);
    runTest(a5, (int)(sizeof(a5) / sizeof(a5[0])), 1);
    return 0;
}