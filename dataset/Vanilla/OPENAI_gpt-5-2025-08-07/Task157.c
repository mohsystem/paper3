#include <stdio.h>
#include <stdlib.h>

long long allocate_and_sum(int n) {
    if (n <= 0) return 0LL;
    long long* arr = (long long*)malloc((size_t)n * sizeof(long long));
    if (!arr) {
        return -1; // indicate failure
    }
    for (int i = 0; i < n; ++i) {
        arr[i] = (long long)(i + 1);
    }
    long long sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += arr[i];
    }
    free(arr);
    return sum;
}

int main(void) {
    int tests[5] = {0, 1, 5, 10, 100000};
    for (int i = 0; i < 5; ++i) {
        int n = tests[i];
        long long result = allocate_and_sum(n);
        printf("n=%d sum=%lld\n", n, result);
    }
    return 0;
}