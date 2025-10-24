/* Chain-of-Through Steps:
1) Problem understanding: Count subarray sums in [lower, upper] via prefix sums and merge sort counting.
2) Security requirements: Use 64-bit integers for sums; check allocations; avoid overflow.
3) Secure coding: Reuse temp buffer; avoid unsafe memory operations; validate inputs.
4) Code review: Ensure bounds correctness and stable merge.
5) Secure code output: Robust and efficient implementation. */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

static long long sort_and_count(long long* sums, long long* temp, int lo, int hi, long long lower, long long upper) {
    if (hi - lo <= 1) return 0LL;
    int mid = (lo + hi) / 2;
    long long cnt = 0;
    cnt += sort_and_count(sums, temp, lo, mid, lower, upper);
    cnt += sort_and_count(sums, temp, mid, hi, lower, upper);

    int j = mid, k = mid, t = mid;
    int r = lo;
    for (int i = lo; i < mid; ++i) {
        while (k < hi && sums[k] - sums[i] < lower) ++k;
        while (j < hi && sums[j] - sums[i] <= upper) ++j;
        while (t < hi && sums[t] < sums[i]) temp[r++] = sums[t++];
        temp[r++] = sums[i];
        cnt += (j - k);
    }
    while (t < hi) temp[r++] = sums[t++];
    for (int p = lo; p < hi; ++p) sums[p] = temp[p];
    return cnt;
}

int countRangeSum(const int* nums, int n, long long lower, long long upper) {
    if (nums == NULL || n <= 0) return 0;
    long long* sums = (long long*)malloc((size_t)(n + 1) * sizeof(long long));
    long long* temp = (long long*)malloc((size_t)(n + 1) * sizeof(long long));
    if (sums == NULL || temp == NULL) {
        if (sums) free(sums);
        if (temp) free(temp);
        return 0; // allocation failed; safe fallback
    }
    sums[0] = 0;
    for (int i = 0; i < n; ++i) {
        sums[i + 1] = sums[i] + (long long)nums[i];
    }
    long long res = sort_and_count(sums, temp, 0, n + 1, lower, upper);
    free(sums);
    free(temp);
    if (res > INT_MAX) return INT_MAX;
    if (res < INT_MIN) return INT_MIN;
    return (int)res;
}

int main(void) {
    int nums1[] = {-2, 5, -1};
    int n1 = (int)(sizeof(nums1) / sizeof(nums1[0]));
    printf("%d\n", countRangeSum(nums1, n1, -2, 2)); // expected 3

    int nums2[] = {0};
    int n2 = (int)(sizeof(nums2) / sizeof(nums2[0]));
    printf("%d\n", countRangeSum(nums2, n2, 0, 0)); // expected 1

    int nums3[] = {1, -1, 1};
    int n3 = (int)(sizeof(nums3) / sizeof(nums3[0]));
    printf("%d\n", countRangeSum(nums3, n3, 0, 1)); // expected 5

    int nums4[] = {2147483647, -2147483648, -1, 0};
    int n4 = (int)(sizeof(nums4) / sizeof(nums4[0]));
    printf("%d\n", countRangeSum(nums4, n4, -1, 0)); // expected 4

    int nums5[] = {1, 2, 3};
    int n5 = (int)(sizeof(nums5) / sizeof(nums5[0]));
    printf("%d\n", countRangeSum(nums5, n5, 3, 6)); // expected 4

    return 0;
}