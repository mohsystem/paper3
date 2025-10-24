#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static long long merge_and_count(int *nums, int left, int mid, int right, int *tmp) {
    if (nums == NULL || tmp == NULL) return 0;

    long long count = 0;
    int i = left;
    int j = mid + 1;

    // Count reverse pairs where i in [left..mid], j in [mid+1..right]
    for (i = left, j = mid + 1; i <= mid; ++i) {
        while (j <= right && (long long)nums[i] > 2LL * (long long)nums[j]) {
            ++j;
        }
        count += (long long)(j - (mid + 1));
    }

    // Merge step
    i = left;
    j = mid + 1;
    int k = left;
    while (i <= mid && j <= right) {
        if (nums[i] <= nums[j]) {
            tmp[k++] = nums[i++];
        } else {
            tmp[k++] = nums[j++];
        }
    }
    while (i <= mid) tmp[k++] = nums[i++];
    while (j <= right) tmp[k++] = nums[j++];
    for (int p = left; p <= right; ++p) nums[p] = tmp[p];

    return count;
}

static long long sort_and_count(int *nums, int left, int right, int *tmp) {
    if (left >= right) return 0;
    int mid = left + (right - left) / 2;
    long long count = 0;
    count += sort_and_count(nums, left, mid, tmp);
    count += sort_and_count(nums, mid + 1, right, tmp);
    count += merge_and_count(nums, left, mid, right, tmp);
    return count;
}

long long reversePairs_count(const int *arr, int n) {
    if (n <= 1 || arr == NULL) return 0;

    // Copy input to avoid mutating the caller's array
    if (n > (INT_MAX / (int)sizeof(int))) return -1; // allocation size overflow guard
    int *nums = (int *)malloc((size_t)n * sizeof(int));
    if (nums == NULL) return -1;

    memcpy(nums, arr, (size_t)n * sizeof(int));

    int *tmp = (int *)malloc((size_t)n * sizeof(int));
    if (tmp == NULL) {
        free(nums);
        return -1;
    }

    long long result = sort_and_count(nums, 0, n - 1, tmp);

    free(tmp);
    free(nums);
    return result;
}

static void print_array(const int *a, int n) {
    if (a == NULL || n < 0) {
        printf("[]");
        return;
    }
    putchar('[');
    for (int i = 0; i < n; ++i) {
        printf("%d", a[i]);
        if (i + 1 < n) printf(",");
    }
    putchar(']');
}

int main(void) {
    // Test case 1
    int nums1[] = {1, 3, 2, 3, 1};
    long long res1 = reversePairs_count(nums1, (int)(sizeof(nums1) / sizeof(nums1[0])));
    printf("Input: "); print_array(nums1, (int)(sizeof(nums1) / sizeof(nums1[0]))); printf("\n");
    printf("Reverse pairs: %lld\n\n", res1);

    // Test case 2
    int nums2[] = {2, 4, 3, 5, 1};
    long long res2 = reversePairs_count(nums2, (int)(sizeof(nums2) / sizeof(nums2[0])));
    printf("Input: "); print_array(nums2, (int)(sizeof(nums2) / sizeof(nums2[0]))); printf("\n");
    printf("Reverse pairs: %lld\n\n", res2);

    // Test case 3: strictly decreasing
    int nums3[] = {5, 4, 3, 2, 1};
    long long res3 = reversePairs_count(nums3, (int)(sizeof(nums3) / sizeof(nums3[0])));
    printf("Input: "); print_array(nums3, (int)(sizeof(nums3) / sizeof(nums3[0]))); printf("\n");
    printf("Reverse pairs: %lld\n\n", res3);

    // Test case 4: negatives
    int nums4[] = {-5, -5, -5};
    long long res4 = reversePairs_count(nums4, (int)(sizeof(nums4) / sizeof(nums4[0])));
    printf("Input: "); print_array(nums4, (int)(sizeof(nums4) / sizeof(nums4[0]))); printf("\n");
    printf("Reverse pairs: %lld\n\n", res4);

    // Test case 5: extremes to test overflow safety
    int nums5[] = {INT_MAX, INT_MIN, 0, INT_MAX - 1, INT_MIN + 1};
    long long res5 = reversePairs_count(nums5, (int)(sizeof(nums5) / sizeof(nums5[0])));
    printf("Input: "); print_array(nums5, (int)(sizeof(nums5) / sizeof(nums5[0]))); printf("\n");
    printf("Reverse pairs: %lld\n", res5);

    return 0;
}