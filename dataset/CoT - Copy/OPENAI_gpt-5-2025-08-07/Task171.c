/* Chain-of-Through:
 * 1) Understand: count (i<j) with nums[i] > 2*nums[j]
 * 2) Security: check allocations, use long long to avoid overflow
 * 3) Secure coding: bounds checking, no UB
 * 4) Review: safe mid, free allocations
 * 5) Output: 5 tests
 */
#include <stdio.h>
#include <stdlib.h>

static long long countPairs(long long* a, int l, int m, int r) {
    long long cnt = 0;
    int j = m + 1;
    for (int i = l; i <= m; ++i) {
        while (j <= r && a[i] > 2LL * a[j]) {
            ++j;
        }
        cnt += (j - (m + 1));
    }
    return cnt;
}

static void merge(long long* a, int l, int m, int r, long long* temp) {
    int i = l, j = m + 1, k = l;
    while (i <= m && j <= r) {
        if (a[i] <= a[j]) temp[k++] = a[i++];
        else temp[k++] = a[j++];
    }
    while (i <= m) temp[k++] = a[i++];
    while (j <= r) temp[k++] = a[j++];
    for (int t = l; t <= r; ++t) a[t] = temp[t];
}

static long long sortAndCount(long long* a, int l, int r, long long* temp) {
    if (l >= r) return 0;
    int m = l + (r - l) / 2;
    long long cnt = 0;
    cnt += sortAndCount(a, l, m, temp);
    cnt += sortAndCount(a, m + 1, r, temp);
    cnt += countPairs(a, l, m, r);
    merge(a, l, m, r, temp);
    return cnt;
}

long long reversePairs(const long long* nums, int n) {
    if (nums == NULL || n < 2) return 0LL;
    long long* a = (long long*)malloc((size_t)n * sizeof(long long));
    long long* temp = (long long*)malloc((size_t)n * sizeof(long long));
    if (a == NULL || temp == NULL) {
        free(a);
        free(temp);
        return 0LL;
    }
    for (int i = 0; i < n; ++i) a[i] = nums[i];
    long long res = sortAndCount(a, 0, n - 1, temp);
    free(a);
    free(temp);
    return res;
}

int main(void) {
    long long t1[] = {1, 3, 2, 3, 1};
    long long t2[] = {2, 4, 3, 5, 1};
    long long t3[] = {5, 4, 3, 2, 1};
    long long t4[] = {1, 1, 1, 1};
    long long t5[] = {-5, -5};

    printf("%lld\n", reversePairs(t1, (int)(sizeof(t1)/sizeof(t1[0]))));
    printf("%lld\n", reversePairs(t2, (int)(sizeof(t2)/sizeof(t2[0]))));
    printf("%lld\n", reversePairs(t3, (int)(sizeof(t3)/sizeof(t3[0]))));
    printf("%lld\n", reversePairs(t4, (int)(sizeof(t4)/sizeof(t4[0]))));
    printf("%lld\n", reversePairs(t5, (int)(sizeof(t5)/sizeof(t5[0]))));

    return 0;
}