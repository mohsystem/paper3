#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// Helper for max
int max(int a, int b) {
    return a > b ? a : b;
}

/**
 * Calculates the maximum sum of a non-empty subsequence of the array
 * such that for every two consecutive integers in the subsequence,
 * nums[i] and nums[j] with i < j, the condition j - i <= k is satisfied.
 *
 * @param nums The input integer array.
 * @param numsSize The size of the input array.
 * @param k The constraint for the distance between consecutive indices.
 * @return The maximum constrained subsequence sum.
 */
int constrainedSubsetSum(int* nums, int numsSize, int k) {
    if (nums == NULL || numsSize <= 0) {
        return 0;
    }

    int* dp = (int*)malloc(numsSize * sizeof(int));
    if (dp == NULL) {
        return 0; // Allocation failure
    }

    // Deque implementation using a simple array
    int* dq = (int*)malloc(numsSize * sizeof(int));
    if (dq == NULL) {
        free(dp);
        return 0; // Allocation failure
    }
    int head = 0, tail = 0; // Deque is empty when head == tail

    int maxSum = INT_MIN;

    for (int i = 0; i < numsSize; ++i) {
        // Remove from front: indices out of the window [i-k, i-1]
        if (head < tail && dq[head] < i - k) {
            head++;
        }
        
        // Calculate dp[i]. The max sum from the previous valid window is at the front of the deque.
        int prevMax = 0;
        if (head < tail) {
            prevMax = dp[dq[head]];
        }
        
        // If prevMax is positive, extend the subsequence. Otherwise, start a new one.
        dp[i] = nums[i] + max(0, prevMax);

        // Maintain the decreasing property of the deque (in terms of dp values)
        while (head < tail && dp[dq[tail - 1]] <= dp[i]) {
            tail--;
        }
        dq[tail++] = i;

        // Update the overall maximum sum found so far
        maxSum = max(maxSum, dp[i]);
    }
    
    free(dp);
    free(dq);
    
    return maxSum;
}

void run_test_case(const char* name, int* nums, int size, int k) {
    int result = constrainedSubsetSum(nums, size, k);
    printf("%s: %d\n", name, result);
}

int main() {
    // Test Case 1
    int nums1[] = {10, 2, -10, 5, 20};
    run_test_case("Test Case 1", nums1, sizeof(nums1)/sizeof(int), 2);

    // Test Case 2
    int nums2[] = {-1, -2, -3};
    run_test_case("Test Case 2", nums2, sizeof(nums2)/sizeof(int), 1);
    
    // Test Case 3
    int nums3[] = {10, -2, -10, -5, 20};
    run_test_case("Test Case 3", nums3, sizeof(nums3)/sizeof(int), 2);

    // Test Case 4
    int nums4[] = {-8269, 3217, -4023, -4138, -683, 6455, -3621, 9242, 4015, -3790};
    run_test_case("Test Case 4", nums4, sizeof(nums4)/sizeof(int), 1);
    
    // Test Case 5
    int nums5[] = {1, -1, -2, -3, 5};
    run_test_case("Test Case 5", nums5, sizeof(nums5)/sizeof(int), 2);

    return 0;
}