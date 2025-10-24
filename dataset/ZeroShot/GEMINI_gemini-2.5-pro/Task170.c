#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int mergeSortAndCount(long long* sums, int start, int end, int lower, int upper);

/**
 * Given an integer array nums and two integers lower and upper, return the number of range sums
 * that lie in [lower, upper] inclusive.
 */
int countRangeSum(int* nums, int numsSize, int lower, int upper) {
    if (nums == NULL || numsSize == 0) {
        return 0;
    }
    // Prefix sums can exceed int range, so use long long.
    long long* prefixSums = (long long*)malloc((numsSize + 1) * sizeof(long long));
    if (prefixSums == NULL) {
        // Handle memory allocation failure
        exit(1);
    }
    
    prefixSums[0] = 0;
    for (int i = 0; i < numsSize; i++) {
        prefixSums[i + 1] = prefixSums[i] + nums[i];
    }

    int count = mergeSortAndCount(prefixSums, 0, numsSize, lower, upper);
    
    free(prefixSums);
    return count;
}

/**
 * A recursive helper function using a divide and conquer approach (Merge Sort).
 * It counts the valid range sums and sorts the prefix sum subarray.
 */
int mergeSortAndCount(long long* sums, int start, int end, int lower, int upper) {
    if (start >= end) {
        return 0;
    }
    int mid = start + (end - start) / 2;
    int count = mergeSortAndCount(sums, start, mid, lower, upper) +
                mergeSortAndCount(sums, mid + 1, end, lower, upper);

    // Count range sums S(i, j) where i is in the left half and j in the right half.
    // For each sums[j] in the right half, we need to find how many sums[i] in the left half
    // satisfy: sums[j] - upper <= sums[i] <= sums[j] - lower
    int k = start, l = start;
    for (int j = mid + 1; j <= end; j++) {
        while (k <= mid && sums[k] < sums[j] - upper) {
            k++;
        }
        while (l <= mid && sums[l] <= sums[j] - lower) {
            l++;
        }
        count += l - k;
    }

    // Standard merge step for merge sort
    long long* temp = (long long*)malloc((end - start + 1) * sizeof(long long));
    if (temp == NULL) {
        // Handle memory allocation failure
        exit(1); 
    }
    
    int i = start, j = mid + 1, t = 0;
    while (i <= mid && j <= end) {
        if (sums[i] <= sums[j]) {
            temp[t++] = sums[i++];
        } else {
            temp[t++] = sums[j++];
        }
    }
    while (i <= mid) {
        temp[t++] = sums[i++];
    }
    while (j <= end) {
        temp[t++] = sums[j++];
    }
    
    memcpy(sums + start, temp, (end - start + 1) * sizeof(long long));
    
    free(temp);
    return count;
}

int main() {
    // Test Case 1
    int nums1[] = {-2, 5, -1};
    int lower1 = -2, upper1 = 2;
    printf("Test Case 1: %d\n", countRangeSum(nums1, sizeof(nums1)/sizeof(int), lower1, upper1));

    // Test Case 2
    int nums2[] = {0};
    int lower2 = 0, upper2 = 0;
    printf("Test Case 2: %d\n", countRangeSum(nums2, sizeof(nums2)/sizeof(int), lower2, upper2));

    // Test Case 3
    int nums3[] = {1, 1, 1};
    int lower3 = 1, upper3 = 2;
    printf("Test Case 3: %d\n", countRangeSum(nums3, sizeof(nums3)/sizeof(int), lower3, upper3));

    // Test Case 4
    int nums4[] = {-1, -1, -1};
    int lower4 = -2, upper4 = -1;
    printf("Test Case 4: %d\n", countRangeSum(nums4, sizeof(nums4)/sizeof(int), lower4, upper4));

    // Test Case 5 (uses INT_MAX and INT_MIN)
    int nums5[] = {2147483647, -2147483647 - 1, -1, 0};
    int lower5 = -1, upper5 = 0;
    printf("Test Case 5: %d\n", countRangeSum(nums5, sizeof(nums5)/sizeof(int), lower5, upper5));

    return 0;
}