#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

static long long sort_count(long long* sums, size_t left, size_t right, long long lower, long long upper, long long* temp) {
    if (right - left <= 1) return 0LL;
    size_t mid = left + (right - left) / 2;
    long long cnt = 0;
    cnt += sort_count(sums, left, mid, lower, upper, temp);
    cnt += sort_count(sums, mid, right, lower, upper, temp);

    size_t lo = mid, hi = mid;
    for (size_t i = left; i < mid; ++i) {
        while (lo < right && sums[lo] - sums[i] < lower) ++lo;
        while (hi < right && sums[hi] - sums[i] <= upper) ++hi;
        cnt += (long long)(hi - lo);
    }
    // merge
    size_t i = left, j = mid, k = left;
    while (i < mid || j < right) {
        if (j >= right || (i < mid && sums[i] <= sums[j])) {
            temp[k++] = sums[i++];
        } else {
            temp[k++] = sums[j++];
        }
    }
    for (i = left; i < right; ++i) {
        sums[i] = temp[i];
    }
    return cnt;
}

int countRangeSum(const long long* nums, size_t n, long long lower, long long upper) {
    if (nums == NULL) {
        fprintf(stderr, "nums must not be NULL\n");
        return -1;
    }
    if (n < 1 || n > 100000ULL) {
        fprintf(stderr, "nums length out of allowed range\n");
        return -1;
    }
    if (lower > upper) {
        fprintf(stderr, "lower must be <= upper\n");
        return -1;
    }
    if (lower < -100000LL || upper > 100000LL) {
        fprintf(stderr, "lower/upper out of allowed range\n");
        return -1;
    }
    long long* sums = (long long*)calloc(n + 1, sizeof(long long));
    long long* temp = (long long*)calloc(n + 1, sizeof(long long));
    if (!sums || !temp) {
        fprintf(stderr, "memory allocation failed\n");
        free(sums);
        free(temp);
        return -1;
    }
    for (size_t i = 0; i < n; ++i) {
        sums[i + 1] = sums[i] + nums[i];
    }
    long long cnt = sort_count(sums, 0, n + 1, lower, upper, temp);
    free(sums);
    free(temp);
    if (cnt < INT_MIN || cnt > INT_MAX) {
        fprintf(stderr, "Result out of 32-bit integer range\n");
        return -1;
    }
    return (int)cnt;
}

static void run_test(const long long* nums, size_t n, long long lower, long long upper, int expected) {
    int res = countRangeSum(nums, n, lower, upper);
    printf("Result: %d | Expected: %d\n", res, expected);
}

int main(void) {
    // Five test cases
    long long a1[] = {-2, 5, -1};
    run_test(a1, 3, -2, 2, 3);

    long long a2[] = {0};
    run_test(a2, 1, 0, 0, 1);

    long long a3[] = {1, 2, 3};
    run_test(a3, 3, 3, 6, 4);

    long long a4[] = {-1, -1, -1};
    run_test(a4, 3, -3, -1, 6);

    long long a5[] = {1, -1, 1, -1};
    run_test(a5, 4, 0, 0, 4);

    return 0;
}