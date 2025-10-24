#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

static long long merge_sort_count(long long* arr, long long* temp, int left, int right) {
    if (left >= right) return 0LL;
    int mid = left + (right - left) / 2;
    long long count = 0LL;
    count += merge_sort_count(arr, temp, left, mid);
    count += merge_sort_count(arr, temp, mid + 1, right);

    int j = mid + 1;
    for (int i = left; i <= mid; ++i) {
        while (j <= right && arr[i] > 2LL * arr[j]) ++j;
        count += (long long)(j - (mid + 1));
    }

    int i = left; j = mid + 1; int k = left;
    while (i <= mid && j <= right) {
        if (arr[i] <= arr[j]) temp[k++] = arr[i++];
        else temp[k++] = arr[j++];
    }
    while (i <= mid) temp[k++] = arr[i++];
    while (j <= right) temp[k++] = arr[j++];
    for (int idx = left; idx <= right; ++idx) arr[idx] = temp[idx];

    return count;
}

long long count_reverse_pairs(const long long* nums, int n) {
    if (nums == NULL || n <= 1) return 0LL;
    long long* arr = (long long*)malloc((size_t)n * sizeof(long long));
    long long* temp = (long long*)malloc((size_t)n * sizeof(long long));
    if (arr == NULL || temp == NULL) {
        free(arr);
        free(temp);
        return 0LL;
    }
    for (int i = 0; i < n; ++i) arr[i] = nums[i];
    long long result = merge_sort_count(arr, temp, 0, n - 1);
    free(temp);
    free(arr);
    return result;
}

int main(void) {
    long long t1[] = {1, 3, 2, 3, 1};
    long long t2[] = {2, 4, 3, 5, 1};
    long long t3[] = {1};
    long long t4[] = {2147483647LL, -2147483648LL, 0LL};
    long long t5[] = {-5LL, -5LL};

    printf("Test 1 result: %lld\n", count_reverse_pairs(t1, (int)(sizeof(t1)/sizeof(t1[0]))));
    printf("Test 2 result: %lld\n", count_reverse_pairs(t2, (int)(sizeof(t2)/sizeof(t2[0]))));
    printf("Test 3 result: %lld\n", count_reverse_pairs(t3, (int)(sizeof(t3)/sizeof(t3[0]))));
    printf("Test 4 result: %lld\n", count_reverse_pairs(t4, (int)(sizeof(t4)/sizeof(t4[0]))));
    printf("Test 5 result: %lld\n", count_reverse_pairs(t5, (int)(sizeof(t5)/sizeof(t5[0]))));

    return 0;
}