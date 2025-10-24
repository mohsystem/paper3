#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

long long constrained_subsequence_sum(const int* nums, int n, int k) {
    if (nums == NULL || n <= 0 || k <= 0) {
        fprintf(stderr, "Invalid input\n");
        exit(EXIT_FAILURE);
    }
    long long* dp = (long long*)malloc((size_t)n * sizeof(long long));
    if (!dp) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    int* dq = (int*)malloc((size_t)n * sizeof(int)); // deque of indices
    if (!dq) {
        free(dp);
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    int front = 0, back = -1; // empty deque when front > back
    long long ans = LLONG_MIN;

    for (int i = 0; i < n; ++i) {
        while (front <= back && dq[front] < i - k) {
            front++;
        }
        long long bestPrev = 0;
        if (front <= back && dp[dq[front]] > 0) {
            bestPrev = dp[dq[front]];
        }
        dp[i] = (long long)nums[i] + bestPrev;
        if (dp[i] > ans) ans = dp[i];

        while (front <= back && dp[dq[back]] <= dp[i]) {
            back--;
        }
        dq[++back] = i;
    }

    free(dp);
    free(dq);
    return ans;
}

int main(void) {
    int nums1[] = {10, 2, -10, 5, 20}; int k1 = 2; int n1 = (int)(sizeof(nums1)/sizeof(nums1[0]));
    int nums2[] = {-1, -2, -3}; int k2 = 1; int n2 = (int)(sizeof(nums2)/sizeof(nums2[0]));
    int nums3[] = {10, -2, -10, -5, 20}; int k3 = 2; int n3 = (int)(sizeof(nums3)/sizeof(nums3[0]));
    int nums4[] = {1, -1, -2, 4, -7, 3}; int k4 = 2; int n4 = (int)(sizeof(nums4)/sizeof(nums4[0]));
    int nums5[] = {-5}; int k5 = 1; int n5 = (int)(sizeof(nums5)/sizeof(nums5[0]));

    long long res1 = constrained_subsequence_sum(nums1, n1, k1);
    long long res2 = constrained_subsequence_sum(nums2, n2, k2);
    long long res3 = constrained_subsequence_sum(nums3, n3, k3);
    long long res4 = constrained_subsequence_sum(nums4, n4, k4);
    long long res5 = constrained_subsequence_sum(nums5, n5, k5);

    printf("Result 1: %lld (Expected: 37)\n", res1);
    printf("Result 2: %lld (Expected: -1)\n", res2);
    printf("Result 3: %lld (Expected: 23)\n", res3);
    printf("Result 4: %lld (Expected: 7)\n", res4);
    printf("Result 5: %lld (Expected: -5)\n", res5);

    return 0;
}