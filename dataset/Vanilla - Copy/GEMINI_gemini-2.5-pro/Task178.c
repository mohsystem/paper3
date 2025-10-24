#include <stdio.h>
#include <stdlib.h>

// Helper function for min
int min(int a, int b) {
    return a < b ? a : b;
}

int shortestSubarray(int* nums, int numsSize, int k) {
    long long* prefixSums = (long long*)malloc((numsSize + 1) * sizeof(long long));
    if (prefixSums == NULL) return -1; // Memory allocation failed
    prefixSums[0] = 0;
    for (int i = 0; i < numsSize; i++) {
        prefixSums[i + 1] = prefixSums[i] + nums[i];
    }

    int* dq = (int*)malloc((numsSize + 1) * sizeof(int));
    if (dq == NULL) {
        free(prefixSums);
        return -1; // Memory allocation failed
    }
    int head = 0, tail = 0; // Deque indices for a simple array-based deque

    int minLength = numsSize + 1;

    for (int i = 0; i <= numsSize; i++) {
        // Check for valid subarrays ending at i-1
        while (head < tail && prefixSums[i] - prefixSums[dq[head]] >= k) {
            minLength = min(minLength, i - dq[head]);
            head++;
        }

        // Maintain monotonic property of the deque (increasing prefix sums)
        while (head < tail && prefixSums[i] <= prefixSums[dq[tail - 1]]) {
            tail--;
        }

        dq[tail++] = i;
    }

    free(prefixSums);
    free(dq);

    return minLength == numsSize + 1 ? -1 : minLength;
}

void run_test_case(const char* case_name, int* nums, int numsSize, int k) {
    int result = shortestSubarray(nums, numsSize, k);
    printf("%s: %d\n", case_name, result);
}

int main() {
    // Test Case 1
    int nums1[] = {1};
    run_test_case("Test Case 1", nums1, 1, 1);

    // Test Case 2
    int nums2[] = {1, 2};
    run_test_case("Test Case 2", nums2, 2, 4);

    // Test Case 3
    int nums3[] = {2, -1, 2};
    run_test_case("Test Case 3", nums3, 3, 3);

    // Test Case 4
    int nums4[] = {84, -37, 32, 40, 95};
    run_test_case("Test Case 4", nums4, 5, 167);
    
    // Test Case 5
    int nums5[] = {-28, 81, -20, 28, -29};
    run_test_case("Test Case 5", nums5, 5, 89);

    return 0;
}