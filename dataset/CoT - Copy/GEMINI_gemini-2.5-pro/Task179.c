#include <stdio.h>
#include <stdlib.h>

/**
 * Note: The returned array must be malloced, assume caller calls free().
 * This function finds the maximum value in a sliding window of size k.
 *
 * @param nums The input array of integers.
 * @param numsSize The size of the input array.
 * @param k The size of the sliding window.
 * @param returnSize A pointer to an integer that will store the size of the returned array.
 * @return A dynamically allocated array containing the maximum of each sliding window.
 */
int* maxSlidingWindow(int* nums, int numsSize, int k, int* returnSize) {
    if (nums == NULL || numsSize == 0 || k <= 0 || k > numsSize) {
        *returnSize = 0;
        return NULL;
    }

    *returnSize = numsSize - k + 1;
    int* result = (int*)malloc(*returnSize * sizeof(int));
    if (result == NULL) {
        *returnSize = 0;
        return NULL;
    }
    
    // Deque simulation using an array to store indices.
    int* dq = (int*)malloc(numsSize * sizeof(int));
    if (dq == NULL) {
        free(result);
        *returnSize = 0;
        return NULL;
    }
    int head = 0, tail = 0;
    int resultIndex = 0;

    for (int i = 0; i < numsSize; i++) {
        // Remove indices from the front of the deque that are out of the current window.
        if (head < tail && dq[head] <= i - k) {
            head++;
        }

        // Maintain the decreasing order property of the deque.
        // Remove indices from the back whose corresponding elements are smaller than or equal to the current element.
        while (head < tail && nums[dq[tail - 1]] <= nums[i]) {
            tail--;
        }

        // Add the current element's index to the back of the deque.
        dq[tail++] = i;

        // Once the window is fully formed, the maximum element is at the front of the deque.
        if (i >= k - 1) {
            result[resultIndex++] = nums[dq[head]];
        }
    }

    free(dq);
    return result;
}

void printArray(int* arr, int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%d%s", arr[i], (i == size - 1) ? "" : ", ");
    }
    printf("]\n");
}

int main() {
    // Test Case 1
    int nums1[] = {1, 3, -1, -3, 5, 3, 6, 7};
    int k1 = 3;
    int returnSize1;
    printf("Test Case 1: nums = [1, 3, -1, -3, 5, 3, 6, 7], k = 3\n");
    int* result1 = maxSlidingWindow(nums1, sizeof(nums1)/sizeof(int), k1, &returnSize1);
    printf("Output: ");
    printArray(result1, returnSize1);
    free(result1);

    // Test Case 2
    int nums2[] = {1};
    int k2 = 1;
    int returnSize2;
    printf("\nTest Case 2: nums = [1], k = 1\n");
    int* result2 = maxSlidingWindow(nums2, sizeof(nums2)/sizeof(int), k2, &returnSize2);
    printf("Output: ");
    printArray(result2, returnSize2);
    free(result2);

    // Test Case 3
    int nums3[] = {1, -1};
    int k3 = 1;
    int returnSize3;
    printf("\nTest Case 3: nums = [1, -1], k = 1\n");
    int* result3 = maxSlidingWindow(nums3, sizeof(nums3)/sizeof(int), k3, &returnSize3);
    printf("Output: ");
    printArray(result3, returnSize3);
    free(result3);

    // Test Case 4
    int nums4[] = {9, 9, 9, 9};
    int k4 = 2;
    int returnSize4;
    printf("\nTest Case 4: nums = [9, 9, 9, 9], k = 2\n");
    int* result4 = maxSlidingWindow(nums4, sizeof(nums4)/sizeof(int), k4, &returnSize4);
    printf("Output: ");
    printArray(result4, returnSize4);
    free(result4);

    // Test Case 5
    int nums5[] = {7, 6, 5, 4, 3, 2, 1};
    int k5 = 4;
    int returnSize5;
    printf("\nTest Case 5: nums = [7, 6, 5, 4, 3, 2, 1], k = 4\n");
    int* result5 = maxSlidingWindow(nums5, sizeof(nums5)/sizeof(int), k5, &returnSize5);
    printf("Output: ");
    printArray(result5, returnSize5);
    free(result5);

    return 0;
}