
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Merge sort helper that counts range sums in [lower, upper]
// prefix: prefix sum array
// temp: temporary array for merge
// left, right: range [left, right) to process
// lower, upper: target range
int mergeSort(int64_t* prefix, int64_t* temp, int left, int right, 
              int64_t lower, int64_t upper) {
    // Validate pointer parameters are not NULL
    if (prefix == NULL || temp == NULL) {
        return 0;
    }
    
    // Base case: single element, no range to count
    if (right - left <= 1) {
        return 0;
    }
    
    // Validate indices to prevent overflow
    if (left < 0 || right < 0 || left >= right) {
        return 0;
    }
    
    int mid = left + (right - left) / 2;
    
    // Recursively count in left and right halves
    int count = mergeSort(prefix, temp, left, mid, lower, upper) + 
                mergeSort(prefix, temp, mid, right, lower, upper);
    
    // Count cross-partition ranges
    int j = mid;
    int k = mid;
    
    for (int i = left; i < mid; i++) {
        // Find range [j, k) with bounds checking
        while (j < right && prefix[j] - prefix[i] < lower) {
            j++;
        }
        while (k < right && prefix[k] - prefix[i] <= upper) {
            k++;
        }
        // Validate indices before counting
        if (j < right && k >= j) {
            count += k - j;
        }
    }
    
    // Merge step with bounds checking
    int i = left;
    j = mid;
    int pos = left;
    
    while (i < mid && j < right) {
        if (prefix[i] <= prefix[j]) {
            temp[pos++] = prefix[i++];
        } else {
            temp[pos++] = prefix[j++];
        }
    }
    
    // Copy remaining elements with bounds validation
    while (i < mid) {
        temp[pos++] = prefix[i++];
    }
    while (j < right) {
        temp[pos++] = prefix[j++];
    }
    
    // Copy back from temp to prefix with bounds checking
    for (int i = left; i < right; i++) {
        prefix[i] = temp[i];
    }
    
    return count;
}

int countRangeSum(int* nums, int numsSize, int lower, int upper) {
    // Input validation: check for NULL pointer
    if (nums == NULL) {
        return 0;
    }
    
    // Validate size constraints (1 <= numsSize <= 10^5)
    if (numsSize < 1 || numsSize > 100000) {
        return 0;
    }
    
    // Allocate prefix sum array with overflow protection
    // Size is numsSize + 1, check for allocation overflow
    if (numsSize >= INT32_MAX) {
        return 0;
    }
    
    int64_t* prefix = (int64_t*)calloc(numsSize + 1, sizeof(int64_t));
    if (prefix == NULL) {
        // Memory allocation failed, return 0
        return 0;
    }
    
    // Initialize first element (already 0 from calloc)
    // Build prefix sum array
    for (int i = 0; i < numsSize; i++) {
        // Check array bounds before access
        prefix[i + 1] = prefix[i] + (int64_t)nums[i];
    }
    
    // Allocate temporary array for merge sort
    int64_t* temp = (int64_t*)calloc(numsSize + 1, sizeof(int64_t));
    if (temp == NULL) {
        // Memory allocation failed, clean up and return
        free(prefix);
        return 0;
    }
    
    // Perform merge sort and count
    int result = mergeSort(prefix, temp, 0, numsSize + 1, 
                          (int64_t)lower, (int64_t)upper);
    
    // Free allocated memory to prevent leaks
    free(prefix);
    free(temp);
    
    return result;
}

int main() {
    // Test case 1: Example from problem
    int nums1[] = {-2, 5, -1};
    printf("Test 1: %d\\n", countRangeSum(nums1, 3, -2, 2));
    
    // Test case 2: Single element
    int nums2[] = {0};
    printf("Test 2: %d\\n", countRangeSum(nums2, 1, 0, 0));
    
    // Test case 3: All positive numbers
    int nums3[] = {1, 2, 3, 4};
    printf("Test 3: %d\\n", countRangeSum(nums3, 4, 3, 7));
    
    // Test case 4: Mixed values
    int nums4[] = {-1, 1, -1, 1};
    printf("Test 4: %d\\n", countRangeSum(nums4, 4, -1, 1));
    
    // Test case 5: Large range
    int nums5[] = {2, -3, 1, 4, -2};
    printf("Test 5: %d\\n", countRangeSum(nums5, 5, -5, 5));
    
    return 0;
}
