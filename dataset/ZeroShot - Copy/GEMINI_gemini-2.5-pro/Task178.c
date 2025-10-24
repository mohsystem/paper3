#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// Helper for min function
int min(int a, int b) {
    return a < b ? a : b;
}

int shortestSubarray(int* nums, int numsSize, int k) {
    if (nums == NULL || numsSize == 0) {
        return -1;
    }

    // Use long long for prefix sums to avoid integer overflow
    long long* prefixSums = (long long*)malloc((numsSize + 1) * sizeof(long long));
    if (prefixSums == NULL) {
        // Handle memory allocation failure
        return -1; 
    }
    prefixSums[0] = 0;
    for (int i = 0; i < numsSize; i++) {
        prefixSums[i + 1] = prefixSums[i] + nums[i];
    }

    int minLength = INT_MAX;
    
    // Use an array to simulate a deque for storing indices
    int* dq = (int*)malloc((numsSize + 1) * sizeof(int));
    if (dq == NULL) {
        // Handle memory allocation failure
        free(prefixSums);
        return -1;
    }
    int head = 0, tail = 0;

    for (int i = 0; i <= numsSize; i++) {
        // Condition 1: Find a valid subarray
        while (head < tail && prefixSums[i] - prefixSums[dq[head]] >= k) {
            minLength = min(minLength, i - dq[head]);
            head++;
        }

        // Condition 2: Maintain monotonic property (increasing prefix sums)
        while (head < tail && prefixSums[i] <= prefixSums[dq[tail - 1]]) {
            tail--;
        }

        dq[tail++] = i;
    }

    free(prefixSums);
    free(dq);

    return minLength == INT_MAX ? -1 : minLength;
}

int main() {
    // Test Case 1
    int nums1[] = {1};
    printf("Test Case 1: %d\n", shortestSubarray(nums1, sizeof(nums1)/sizeof(nums1[0]), 1));

    // Test Case 2
    int nums2[] = {1, 2};
    printf("Test Case 2: %d\n", shortestSubarray(nums2, sizeof(nums2)/sizeof(nums2[0]), 4));

    // Test Case 3
    int nums3[] = {2, -1, 2};
    printf("Test Case 3: %d\n", shortestSubarray(nums3, sizeof(nums3)/sizeof(nums3[0]), 3));

    // Test Case 4
    int nums4[] = {84, -37, 32, 40, 95};
    printf("Test Case 4: %d\n", shortestSubarray(nums4, sizeof(nums4)/sizeof(nums4[0]), 167));
    
    // Test Case 5
    int nums5[] = {-28, 81, -20, 28, -29};
    printf("Test Case 5: %d\n", shortestSubarray(nums5, sizeof(nums5)/sizeof(nums5[0]), 89));

    return 0;
}