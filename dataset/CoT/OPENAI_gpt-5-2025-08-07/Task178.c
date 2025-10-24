#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int shortestSubarray(int* nums, int n, long long k) {
    long long* prefix = (long long*)malloc(sizeof(long long) * (n + 1));
    if (!prefix) return -1;
    prefix[0] = 0;
    for (int i = 0; i < n; ++i) prefix[i + 1] = prefix[i] + nums[i];

    int* dq = (int*)malloc(sizeof(int) * (n + 1));
    if (!dq) {
        free(prefix);
        return -1;
    }
    int head = 0, tail = 0;
    int ans = n + 1;

    for (int i = 0; i <= n; ++i) {
        while (head < tail && prefix[i] - prefix[dq[head]] >= k) {
            int len = i - dq[head];
            if (len < ans) ans = len;
            head++;
        }
        while (head < tail && prefix[i] <= prefix[dq[tail - 1]]) {
            tail--;
        }
        dq[tail++] = i;
    }

    free(prefix);
    free(dq);
    return (ans <= n) ? ans : -1;
}

int main() {
    int nums1[] = {1};
    int nums2[] = {1, 2};
    int nums3[] = {2, -1, 2};
    int nums4[] = {84, -37, 32, 40, 95};
    int nums5[] = {17, 85, 93, -45, -21};

    printf("%d\n", shortestSubarray(nums1, 1, 1));
    printf("%d\n", shortestSubarray(nums2, 2, 4));
    printf("%d\n", shortestSubarray(nums3, 3, 3));
    printf("%d\n", shortestSubarray(nums4, 5, 167));
    printf("%d\n", shortestSubarray(nums5, 5, 150));

    return 0;
}