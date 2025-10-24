#include <stdio.h>
#include <stdlib.h>

long long sortCount(long long* sums, long long* temp, int lo, int hi, long long lower, long long upper) {
    if (hi - lo <= 1) return 0;
    int mid = (lo + hi) / 2;
    long long cnt = sortCount(sums, temp, lo, mid, lower, upper) + sortCount(sums, temp, mid, hi, lower, upper);
    int l = mid, r = mid;
    for (int i = lo; i < mid; ++i) {
        while (l < hi && sums[l] - sums[i] < lower) ++l;
        while (r < hi && sums[r] - sums[i] <= upper) ++r;
        cnt += r - l;
    }
    int i = lo, j = mid, k = lo;
    while (i < mid && j < hi) {
        if (sums[i] <= sums[j]) temp[k++] = sums[i++];
        else temp[k++] = sums[j++];
    }
    while (i < mid) temp[k++] = sums[i++];
    while (j < hi) temp[k++] = sums[j++];
    for (i = lo; i < hi; ++i) sums[i] = temp[i];
    return cnt;
}

long long countRangeSum(const int* nums, int n, long long lower, long long upper) {
    long long* sums = (long long*)malloc((n + 1) * sizeof(long long));
    long long* temp = (long long*)malloc((n + 1) * sizeof(long long));
    if (!sums || !temp) {
        if (sums) free(sums);
        if (temp) free(temp);
        return 0;
    }
    sums[0] = 0;
    for (int i = 0; i < n; ++i) sums[i + 1] = sums[i] + (long long)nums[i];
    long long ans = sortCount(sums, temp, 0, n + 1, lower, upper);
    free(sums);
    free(temp);
    return ans;
}

int main() {
    int nums1[] = {-2, 5, -1};
    int len1 = sizeof(nums1)/sizeof(nums1[0]);
    printf("%lld\n", countRangeSum(nums1, len1, -2, 2));

    int nums2[] = {0};
    int len2 = sizeof(nums2)/sizeof(nums2[0]);
    printf("%lld\n", countRangeSum(nums2, len2, 0, 0));

    int nums3[] = {1, -1, 2, -2, 3};
    int len3 = sizeof(nums3)/sizeof(nums3[0]);
    printf("%lld\n", countRangeSum(nums3, len3, 1, 3));

    int nums4[] = {0, 0, 0};
    int len4 = sizeof(nums4)/sizeof(nums4[0]);
    printf("%lld\n", countRangeSum(nums4, len4, 0, 0));

    int nums5[] = {2, -2, 2, -2};
    int len5 = sizeof(nums5)/sizeof(nums5[0]);
    printf("%lld\n", countRangeSum(nums5, len5, -1, 1));

    return 0;
}