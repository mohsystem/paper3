
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int countWhileMergeSort(long long* sums, int start, int end, int lower, int upper) {
    if (end - start <= 1) return 0;
    
    int mid = (start + end) / 2;
    int count = countWhileMergeSort(sums, start, mid, lower, upper) 
              + countWhileMergeSort(sums, mid, end, lower, upper);
    
    int j = mid, k = mid, t = mid;
    long long* cache = (long long*)malloc((end - start) * sizeof(long long));
    int r = 0;
    
    for (int i = start; i < mid; i++) {
        while (k < end && sums[k] - sums[i] < lower) k++;
        while (j < end && sums[j] - sums[i] <= upper) j++;
        count += j - k;
        
        while (t < end && sums[t] < sums[i]) cache[r++] = sums[t++];
        cache[r++] = sums[i];
    }
    
    memcpy(&sums[start], cache, r * sizeof(long long));
    free(cache);
    
    return count;
}

int countRangeSum(int* nums, int numsSize, int lower, int upper) {
    long long* prefixSum = (long long*)calloc(numsSize + 1, sizeof(long long));
    
    for (int i = 0; i < numsSize; i++) {
        prefixSum[i + 1] = prefixSum[i] + nums[i];
    }
    
    int result = countWhileMergeSort(prefixSum, 0, numsSize + 1, lower, upper);
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
    int nums3[] = {-2, 5, -1, 3, -2};
    printf("Test 3: %d\\n", countRangeSum(nums3, 5, -2, 3)); // Expected: 8
    
    // Test case 4
    int nums4[] = {1, 2, 3, 4, 5};
    printf("Test 4: %d\\n", countRangeSum(nums4, 5, 3, 7)); // Expected: 6
    
    // Test case 5
    int nums5[] = {-1, -2, -3};
    printf("Test 5: %d\\n", countRangeSum(nums5, 3, -5, -1)); // Expected: 5
    
    return 0;
}
