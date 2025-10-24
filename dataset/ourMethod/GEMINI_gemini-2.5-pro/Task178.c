#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Finds the length of the shortest non-empty subarray with a sum of at least k.
 * 
 * @param nums The input integer array.
 * @param numsSize The size of the input array.
 * @param k The target sum.
 * @return The length of the shortest subarray, or -1 if no such subarray exists.
 */
int shortestSubarray(int* nums, int numsSize, int k) {
    long long* prefixSums = (long long*)malloc((numsSize + 1) * sizeof(long long));
    if (prefixSums == NULL) {
        // Handle memory allocation failure
        return -1; 
    }
    prefixSums[0] = 0;
    for (int i = 0; i < numsSize; ++i) {
        prefixSums[i + 1] = prefixSums[i] + nums[i];
    }

    // Deque stores indices of the prefixSums array.
    // Implemented using an array and two pointers (head, tail).
    int* dq = (int*)malloc((numsSize + 1) * sizeof(int));
    if (dq == NULL) {
        free(prefixSums);
        // Handle memory allocation failure
        return -1;
    }
    int head = 0, tail = 0;

    int minLength = numsSize + 1;

    for (int j = 0; j <= numsSize; ++j) {
        // Condition 1: Find a valid subarray.
        while (head < tail && prefixSums[j] - prefixSums[dq[head]] >= k) {
            int currentLength = j - dq[head];
            if (currentLength < minLength) {
                minLength = currentLength;
            }
            head++;
        }

        // Condition 2: Maintain a monotonically increasing prefixSums in the deque.
        while (head < tail && prefixSums[j] <= prefixSums[dq[tail - 1]]) {
            tail--;
        }

        dq[tail++] = j;
    }

    free(prefixSums);
    free(dq);

    return minLength == numsSize + 1 ? -1 : minLength;
}

int main() {
    // Test case 1: Basic case
    int nums1[] = {1};
    int k1 = 1;
    printf("Test 1: %d\n", shortestSubarray(nums1, sizeof(nums1)/sizeof(nums1[0]), k1)); // Expected: 1

    // Test case 2: No such subarray
    int nums2[] = {1, 2};
    int k2 = 4;
    printf("Test 2: %d\n", shortestSubarray(nums2, sizeof(nums2)/sizeof(nums2[0]), k2)); // Expected: -1

    // Test case 3: With negative numbers
    int nums3[] = {2, -1, 2};
    int k3 = 3;
    printf("Test 3: %d\n", shortestSubarray(nums3, sizeof(nums3)/sizeof(nums3[0]), k3)); // Expected: 3

    // Test case 4: More complex case
    int nums4[] = {84, -37, 32, 40, 95};
    int k4 = 167;
    printf("Test 4: %d\n", shortestSubarray(nums4, sizeof(nums4)/sizeof(nums4[0]), k4)); // Expected: 3

    // Test case 5: All negative numbers
    int nums5[] = {-1, -2, -3};
    int k5 = 1;
    printf("Test 5: %d\n", shortestSubarray(nums5, sizeof(nums5)/sizeof(nums5[0]), k5)); // Expected: -1

    return 0;
}