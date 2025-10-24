#include <stdio.h>
#include <stdlib.h>

int shortestSubarray(const int* nums, int n, long long k) {
    if (nums == NULL || n <= 0) {
        return -1;
    }
    long long* prefix = (long long*)malloc((size_t)(n + 1) * sizeof(long long));
    if (prefix == NULL) {
        return -1;
    }
    prefix[0] = 0;
    for (int i = 0; i < n; ++i) {
        prefix[i + 1] = prefix[i] + (long long)nums[i];
    }

    int* dq = (int*)malloc((size_t)(n + 1) * sizeof(int));
    if (dq == NULL) {
        free(prefix);
        return -1;
    }
    int head = 0, tail = 0;
    int ans = n + 1;

    for (int i = 0; i <= n; ++i) {
        while (head < tail && prefix[i] - prefix[dq[head]] >= k) {
            int cand = i - dq[head];
            if (cand < ans) ans = cand;
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

static void run_test(const int* nums, int n, long long k) {
    int res = shortestSubarray(nums, n, k);
    printf("%d\n", res);
}

int main(void) {
    int t1[] = {1};
    run_test(t1, 1, 1); // 1

    int t2[] = {1, 2};
    run_test(t2, 2, 4); // -1

    int t3[] = {2, -1, 2};
    run_test(t3, 3, 3); // 3

    int t4[] = {84, -37, 32, 40, 95};
    run_test(t4, 5, 167); // 3

    int t5[] = {17, 85, 93, -45, -21};
    run_test(t5, 5, 150); // 2

    return 0;
}