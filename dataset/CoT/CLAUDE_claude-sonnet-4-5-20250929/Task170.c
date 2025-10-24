
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int mergeSortCount(long long* sums, int start, int end, int lower, int upper) {
    if (end - start <= 1) {
        return 0;
    }
    
    int mid = start + (end - start) / 2;
    int count = mergeSortCount(sums, start, mid, lower, upper) + 
                mergeSortCount(sums, mid, end, lower, upper);
    
    int j = mid, k = mid, t = mid;
    long long* cache = (long long*)malloc((end - start) * sizeof(long long));
    if (cache == NULL) {
        return 0;
    }
    int r = 0;
    
    for (int i = start; i < mid; i++) {
        // Count valid ranges
        while (k < end && sums[k] - sums[i] < lower) k++;
        while (j < end && sums[j] - sums[i] <= upper) j++;
        count += j - k;
        
        // Merge for sorting
        while (t < end && sums[t] < sums[i]) {
            cache[r++] = sums[t++];
        }
        cache[r++] = sums[i];
    }
    
    memcpy(sums + start, cache, r * sizeof(long long));
    free(cache);
    
    return count;
}

int countRangeSum(int* nums, int numsSize, int lower, int upper) {
    if (nums == NULL || numsSize == 0) {
        return 0;
    }
    
    long long* prefixSum = (long long*)calloc(numsSize + 1, sizeof(long long));
    if (prefixSum == NULL) {
        return 0;
    }
    
    // Calculate prefix sums with overflow protection
    for (int i = 0; i < numsSize; i++) {
        prefixSum[i + 1] = prefixSum[i] + nums[i];
    }
    
    int result = mergeSortCount(prefixSum, 0, numsSize + 1, lower, upper);
    free(prefixSum);
    
    return result;
}

int main() {
    // Test case 1
    int nums1[] = {-2, 5, -1};
    printf("Test 1: %d\\n", countRangeSum(nums1, 3, -2, 2)); // Expected: 3
    
    // Test case 2
    int nums2[] = {0};
    printf("Test 2: %d\\n", countRangeSum(nums2, 1, 0, 0)); // Expected: 1
    
    // Test case 3
    int nums3[] = {-2147483647, 0, -2147483647, 2147483647};
    printf("Test 3: %d\\n", countRangeSum(nums3, 4, -564, 3864)); // Expected: 3
    
    // Test case 4
    int nums4[] = {1, 2, 3, 4, 5};
    printf("Test 4: %d\\n", countRangeSum(nums4, 5, 3, 8)); // Expected: 6
    
    // Test case 5
    int nums5[] = {-1, -1, -1};
    printf("Test 5: %d\\n", countRangeSum(nums5, 3, -2, -1)); // Expected: 6
    
    return 0;
}
