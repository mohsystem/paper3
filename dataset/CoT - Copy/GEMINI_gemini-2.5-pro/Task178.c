#include <stdio.h>
#include <stdlib.h>

int min(int a, int b) {
    return a < b ? a : b;
}

int shortestSubarray(int* nums, int numsSize, int k) {
    long long* prefixSums = (long long*)calloc(numsSize + 1, sizeof(long long));
    if (!prefixSums) {
        return -1; // Memory allocation failed
    }
    
    for (int i = 0; i < numsSize; ++i) {
        prefixSums[i + 1] = prefixSums[i] + nums[i];
    }

    int minLength = numsSize + 1;
    
    // Using an array as a deque for indices
    int* dq = (int*)malloc((numsSize + 1) * sizeof(int));
    if (!dq) {
        free(prefixSums);
        return -1; // Memory allocation failed
    }
    int head = 0, tail = 0;

    for (int i = 0; i <= numsSize; ++i) {
        // Check if any subarray ending at i-1 satisfies the condition
        while (head < tail && prefixSums[i] - prefixSums[dq[head]] >= k) {
            minLength = min(minLength, i - dq[head]);
            head++;
        }
        // Maintain the monotonic (increasing) property of the deque
        while (head < tail && prefixSums[i] <= prefixSums[dq[tail - 1]]) {
            tail--;
        }
        dq[tail++] = i;
    }
    
    free(prefixSums);
    free(dq);

    return minLength > numsSize ? -1 : minLength;
}

int main() {
    // Test Case 1
    int nums1[] = {1};
    int k1 = 1;
    printf("Test Case 1: %d\n", shortestSubarray(nums1, sizeof(nums1)/sizeof(int), k1));

    // Test Case 2
    int nums2[] = {1, 2};
    int k2 = 4;
    printf("Test Case 2: %d\n", shortestSubarray(nums2, sizeof(nums2)/sizeof(int), k2));

    // Test Case 3
    int nums3[] = {2, -1, 2};
    int k3 = 3;
    printf("Test Case 3: %d\n", shortestSubarray(nums3, sizeof(nums3)/sizeof(int), k3));

    // Test Case 4
    int nums4[] = {84, -37, 32, 40, 95};
    int k4 = 167;
    printf("Test Case 4: %d\n", shortestSubarray(nums4, sizeof(nums4)/sizeof(int), k4));
    
    // Test Case 5
    int nums5[] = {-28, 81, -20, 28, -29};
    int k5 = 89;
    printf("Test Case 5: %d\n", shortestSubarray(nums5, sizeof(nums5)/sizeof(int), k5));

    return 0;
}