#include <stdio.h>
#include <stdlib.h>

int shortestSubarray(int* nums, int n, long long k) {
    long long* ps = (long long*)malloc((n + 1) * sizeof(long long));
    if (!ps) return -1;
    ps[0] = 0;
    for (int i = 0; i < n; ++i) ps[i + 1] = ps[i] + nums[i];

    int* dq = (int*)malloc((n + 1) * sizeof(int));
    if (!dq) { free(ps); return -1; }
    int front = 0, back = 0;
    int ans = n + 1;

    for (int i = 0; i <= n; ++i) {
        while (front < back && ps[i] - ps[dq[front]] >= k) {
            int idx = dq[front++];
            int len = i - idx;
            if (len < ans) ans = len;
        }
        while (front < back && ps[i] <= ps[dq[back - 1]]) back--;
        dq[back++] = i;
    }

    free(ps);
    free(dq);
    return ans <= n ? ans : -1;
}

int main() {
    int a1[] = {1}; int n1 = sizeof(a1)/sizeof(a1[0]); printf("%d\n", shortestSubarray(a1, n1, 1));
    int a2[] = {1,2}; int n2 = sizeof(a2)/sizeof(a2[0]); printf("%d\n", shortestSubarray(a2, n2, 4));
    int a3[] = {2,-1,2}; int n3 = sizeof(a3)/sizeof(a3[0]); printf("%d\n", shortestSubarray(a3, n3, 3));
    int a4[] = {17,85,93,-45,-21}; int n4 = sizeof(a4)/sizeof(a4[0]); printf("%d\n", shortestSubarray(a4, n4, 150));
    int a5[] = {48,99,37,4,-31}; int n5 = sizeof(a5)/sizeof(a5[0]); printf("%d\n", shortestSubarray(a5, n5, 140));
    return 0;
}