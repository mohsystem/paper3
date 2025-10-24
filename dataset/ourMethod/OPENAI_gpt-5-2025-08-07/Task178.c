#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int shortestSubarray(const long long* nums, int n, long long k) {
    if (nums == NULL || n <= 0 || n > 1000000) {
        return -1;
    }

    long long* prefix = (long long*)malloc(sizeof(long long) * (n + 1));
    if (!prefix) {
        return -1;
    }
    prefix[0] = 0;
    for (int i = 0; i < n; i++) {
        prefix[i + 1] = prefix[i] + nums[i];
    }

    int* dq = (int*)malloc(sizeof(int) * (n + 1));
    if (!dq) {
        free(prefix);
        return -1;
    }
    int head = 0, tail = 0;

    int ans = n + 1;

    for (int i = 0; i <= n; i++) {
        while (head < tail && prefix[i] - prefix[dq[head]] >= k) {
            int length = i - dq[head];
            if (length < ans) ans = length;
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

static void print_array(const long long* arr, int n) {
    printf("[");
    for (int i = 0; i < n; i++) {
        if (i) printf(",");
        printf("%lld", arr[i]);
    }
    printf("]");
}

int main(void) {
    long long a1[] = {1};
    long long a2[] = {1, 2};
    long long a3[] = {2, -1, 2};
    long long a4[] = {84, -37, 32, 40, 95};
    long long a5[] = {17, 85, 93, -45, -21};

    struct {
        long long* arr;
        int n;
        long long k;
    } tests[5] = {
        {a1, 1, 1},
        {a2, 2, 4},
        {a3, 3, 3},
        {a4, 5, 167},
        {a5, 5, 150}
    };

    for (int i = 0; i < 5; i++) {
        printf("Test %d nums=", i + 1);
        print_array(tests[i].arr, tests[i].n);
        printf(" k=%lld -> %d\n", tests[i].k, shortestSubarray(tests[i].arr, tests[i].n, tests[i].k));
    }

    return 0;
}