#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int mergeSortAndCount(long long* sums, int start, int end, int lower, int upper) {
    if (start >= end) {
        return 0;
    }

    int mid = start + (end - start) / 2;
    int count = mergeSortAndCount(sums, start, mid, lower, upper) + 
                mergeSortAndCount(sums, mid + 1, end, lower, upper);

    int l = mid + 1;
    int r = mid + 1;
    for (int i = start; i <= mid; ++i) {
        while (l <= end && sums[l] - sums[i] < lower) {
            l++;
        }
        while (r <= end && sums[r] - sums[i] <= upper) {
            r++;
        }
        count += (r - l);
    }
    
    // Merge step
    int tempSize = end - start + 1;
    long long* temp = (long long*)malloc(tempSize * sizeof(long long));
    if (temp == NULL) {
        exit(1); 
    }
    
    int p1 = start, p2 = mid + 1, p = 0;
    while (p1 <= mid && p2 <= end) {
        if (sums[p1] <= sums[p2]) {
            temp[p++] = sums[p1++];
        } else {
            temp[p++] = sums[p2++];
        }
    }
    while (p1 <= mid) {
        temp[p++] = sums[p1++];
    }
    while (p2 <= end) {
        temp[p++] = sums[p2++];
    }

    for (int i = 0; i < tempSize; ++i) {
        sums[start + i] = temp[i];
    }
    
    free(temp);

    return count;
}

int countRangeSum(int* nums, int numsSize, int lower, int upper) {
    if (nums == NULL || numsSize == 0) {
        return 0;
    }
    
    long long* prefixSums = (long long*)malloc((numsSize + 1) * sizeof(long long));
    if (prefixSums == NULL) {
        exit(1);
    }
    
    prefixSums[0] = 0;
    for (int i = 0; i < numsSize; ++i) {
        prefixSums[i + 1] = prefixSums[i] + nums[i];
    }
    
    int result = mergeSortAndCount(prefixSums, 0, numsSize, lower, upper);
    
    free(prefixSums);
    
    return result;
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
    int nums3[] = {1, -1, 1, -1};
    int lower3 = 0, upper3 = 0;
    printf("Test Case 3: %d\n", countRangeSum(nums3, sizeof(nums3)/sizeof(int), lower3, upper3));

    // Test Case 4
    int nums4[] = {INT_MAX, INT_MIN, 0, -1, 1};
    int lower4 = -1, upper4 = 1;
    printf("Test Case 4: %d\n", countRangeSum(nums4, sizeof(nums4)/sizeof(int), lower4, upper4));
    
    // Test Case 5
    int nums5[] = {0, 0, 0, 0, 0};
    int lower5 = 0, upper5 = 0;
    printf("Test Case 5: %d\n", countRangeSum(nums5, sizeof(nums5)/sizeof(int), lower5, upper5));
    
    return 0;
}