#include <stdio.h>
#include <stdlib.h>

static long long sort_and_count(long long* sums, long long* temp, int left, int right, long long lower, long long upper) {
    if (right - left <= 1) return 0LL;
    int mid = left + (right - left) / 2;
    long long count = 0LL;
    count += sort_and_count(sums, temp, left, mid, lower, upper);
    count += sort_and_count(sums, temp, mid, right, lower, upper);

    int j = mid, k = mid, r = mid, p = left;
    for (int i = left; i < mid; ++i) {
        while (j < right && sums[j] - sums[i] < lower) ++j;
        while (k < right && sums[k] - sums[i] <= upper) ++k;
        count += (k - j);
        while (r < right && sums[r] < sums[i]) temp[p++] = sums[r++];
        temp[p++] = sums[i];
    }
    while (r < right) temp[p++] = sums[r++];

    for (int i = left; i < right; ++i) sums[i] = temp[i];
    return count;
}

int countRangeSum(const int* nums, int n, long long lower, long long upper) {
    if (n < 0) return 0;
    long long* prefix = (long long*)malloc((size_t)(n + 1) * sizeof(long long));
    long long* temp = (long long*)malloc((size_t)(n + 1) * sizeof(long long));
    if (prefix == NULL || temp == NULL) {
        free(prefix);
        free(temp);
        return 0;
    }
    prefix[0] = 0LL;
    for (int i = 0; i < n; ++i) {
        prefix[i + 1] = prefix[i] + (long long)nums[i];
    }
    long long cnt = sort_and_count(prefix, temp, 0, n + 1, lower, upper);
    free(prefix);
    free(temp);
    if (cnt > 2147483647LL) return 2147483647;
    if (cnt < -2147483648LL) return -2147483648;
    return (int)cnt;
}

int main(void) {
    int a1[] = {-2, 5, -1};
    int a2[] = {0};
    int a3[] = {1, 2, 3, 4};
    int a4[] = {-1, -1, -1};
    int a5[] = {0, 0, 0, 0};

    printf("%d\n", countRangeSum(a1, 3, -2, 2));
    printf("%d\n", countRangeSum(a2, 1, 0, 0));
    printf("%d\n", countRangeSum(a3, 4, 3, 6));
    printf("%d\n", countRangeSum(a4, 3, -2, -1));
    printf("%d\n", countRangeSum(a5, 4, 0, 0));
    return 0;
}