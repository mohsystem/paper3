#include <stdio.h>
#include <limits.h>

int constrainedSubsetSum(int* nums, int n, int k) {
    int* dp = (int*)malloc(sizeof(int) * n);
    int* dq = (int*)malloc(sizeof(int) * n); // store indices
    int head = 0, tail = 0; // deque [head, tail)
    int ans = INT_MIN;

    for (int i = 0; i < n; ++i) {
        while (head < tail && i - dq[head] > k) head++;
        int bestPrev = 0;
        if (head < tail) {
            int v = dp[dq[head]];
            bestPrev = v > 0 ? v : 0;
        }
        dp[i] = nums[i] + bestPrev;
        if (dp[i] > ans) ans = dp[i];
        if (dp[i] > 0) {
            while (head < tail && dp[dq[tail - 1]] <= dp[i]) tail--;
            dq[tail++] = i;
        }
    }

    free(dp);
    free(dq);
    return ans;
}

int main() {
    int nums1[] = {10, 2, -10, 5, 20}; int k1 = 2;
    int nums2[] = {-1, -2, -3}; int k2 = 1;
    int nums3[] = {10, -2, -10, -5, 20}; int k3 = 2;
    int nums4[] = {1, -1, -1, -1, 5}; int k4 = 2;
    int nums5[] = {5, -1, -2, 10}; int k5 = 3;

    printf("%d\n", constrainedSubsetSum(nums1, 5, k1));
    printf("%d\n", constrainedSubsetSum(nums2, 3, k2));
    printf("%d\n", constrainedSubsetSum(nums3, 5, k3));
    printf("%d\n", constrainedSubsetSum(nums4, 5, k4));
    printf("%d\n", constrainedSubsetSum(nums5, 4, k5));

    return 0;
}