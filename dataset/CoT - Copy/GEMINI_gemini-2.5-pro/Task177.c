#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// Helper for max
int max(int a, int b) {
    return a > b ? a : b;
}

int constrainedSubsetSum(int* nums, int numsSize, int k) {
    if (numsSize == 0) {
        return 0;
    }
    
    int* dp = (int*)malloc(numsSize * sizeof(int));
    if (dp == NULL) return -1; // Error handling
    
    // Deque implementation with a dynamic array
    int* dq = (int*)malloc(numsSize * sizeof(int));
    if (dq == NULL) {
        free(dp);
        return -1; // Error handling
    }
    
    int head = 0, tail = 0; // Deque pointers
    int maxSum = INT_MIN;

    for (int i = 0; i < numsSize; ++i) {
        // Deque: pop_front if index is out of window
        if (head < tail && dq[head] < i - k) {
            head++;
        }

        // Calculate dp[i]
        int prevMax = (head == tail) ? 0 : dp[dq[head]];
        dp[i] = nums[i] + max(0, prevMax);

        // Deque: maintain decreasing order by popping from back
        while (head < tail && dp[i] >= dp[dq[tail - 1]]) {
            tail--;
        }
        // Deque: push_back current index
        dq[tail++] = i;

        maxSum = max(maxSum, dp[i]);
    }

    free(dp);
    free(dq);
    return maxSum;
}

void run_test_case(char* test_name, int* nums, int numsSize, int k) {
    int result = constrainedSubsetSum(nums, numsSize, k);
    printf("%s: %d\n", test_name, result);
}

int main() {
    // Test Case 1
    int nums1[] = {10, 2, -10, 5, 20};
    run_test_case("Test Case 1", nums1, 5, 2);

    // Test Case 2
    int nums2[] = {-1, -2, -3};
    run_test_case("Test Case 2", nums2, 3, 1);

    // Test Case 3
    int nums3[] = {10, -2, -10, -5, 20};
    run_test_case("Test Case 3", nums3, 5, 2);

    // Test Case 4
    int nums4[] = {1, -2, 3, 4, -5, 6};
    run_test_case("Test Case 4", nums4, 6, 3);

    // Test Case 5
    int nums5[] = {-5, -4, -3, -2, -1};
    run_test_case("Test Case 5", nums5, 5, 5);

    return 0;
}