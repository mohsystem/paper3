#include <stdio.h>
#include <stdlib.h>

int mergeSortAndCount(long long* sums, int start, int end, int lower, int upper, long long* temp) {
    if (end - start <= 1) {
        return 0;
    }
    
    int mid = start + (end - start) / 2;
    int count = mergeSortAndCount(sums, start, mid, lower, upper, temp)
              + mergeSortAndCount(sums, mid, end, lower, upper, temp);
              
    int l = mid, r = mid;
    for (int i = start; i < mid; i++) {
        while (l < end && sums[l] - sums[i] < lower) {
            l++;
        }
        while (r < end && sums[r] - sums[i] <= upper) {
            r++;
        }
        count += r - l;
    }
    
    int i = start, j = mid, t = 0;
    while (i < mid && j < end) {
        if (sums[i] <= sums[j]) {
            temp[t++] = sums[i++];
        } else {
            temp[t++] = sums[j++];
        }
    }
    while (i < mid) {
        temp[t++] = sums[i++];
    }
    while (j < end) {
        temp[t++] = sums[j++];
    }
    
    for (int k = 0; k < end - start; k++) {
        sums[start + k] = temp[k];
    }
    
    return count;
}

int countRangeSum(int* nums, int numsSize, int lower, int upper) {
    if (nums == NULL || numsSize == 0) {
        return 0;
    }
    
    long long* prefixSums = (long long*)malloc((numsSize + 1) * sizeof(long long));
    if (prefixSums == NULL) return -1;
    
    prefixSums[0] = 0;
    for (int i = 0; i < numsSize; i++) {
        prefixSums[i + 1] = prefixSums[i] + nums[i];
    }
    
    long long* temp = (long long*)malloc((numsSize + 1) * sizeof(long long));
    if (temp == NULL) {
        free(prefixSums);
        return -1;
    }
    
    int count = mergeSortAndCount(prefixSums, 0, numsSize + 1, lower, upper, temp);
    
    free(prefixSums);
    free(temp);
    
    return count;
}

int main() {
    // Test case 1
    int nums1[] = {-2, 5, -1};
    int lower1 = -2, upper1 = 2;
    printf("Test 1: %d\n", countRangeSum(nums1, 3, lower1, upper1));

    // Test case 2
    int nums2[] = {0};
    int lower2 = 0, upper2 = 0;
    printf("Test 2: %d\n", countRangeSum(nums2, 1, lower2, upper2));

    // Test case 3
    int nums3[] = {2147483647, -2147483647, -1, 0};
    int lower3 = -1, upper3 = 0;
    printf("Test 3: %d\n", countRangeSum(nums3, 4, lower3, upper3));

    // Test case 4
    int nums4[] = {0, 0, 0};
    int lower4 = 0, upper4 = 0;
    printf("Test 4: %d\n", countRangeSum(nums4, 3, lower4, upper4));

    // Test case 5
    int nums5[] = {-1, 1};
    int lower5 = 0, upper5 = 0;
    printf("Test 5: %d\n", countRangeSum(nums5, 2, lower5, upper5));
    
    return 0;
}