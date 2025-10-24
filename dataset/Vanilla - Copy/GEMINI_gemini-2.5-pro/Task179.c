#include <stdio.h>
#include <stdlib.h>

/**
 * Finds the maximum value in a sliding window of size k.
 *
 * @param nums The input array of integers.
 * @param numsSize The size of the input array.
 * @param k The size of the sliding window.
 * @param returnSize A pointer to an integer that will store the size of the returned array.
 * @return A dynamically allocated array containing the maximum of each sliding window.
 *         The caller is responsible for freeing this memory.
 */
int* maxSlidingWindow(int* nums, int numsSize, int k, int* returnSize) {
    if (nums == NULL || numsSize == 0 || k <= 0 || k > numsSize) {
        *returnSize = 0;
        return NULL;
    }

    *returnSize = numsSize - k + 1;
    int* result = (int*)malloc(sizeof(int) * (*returnSize));
    if (result == NULL) {
        *returnSize = 0;
        return NULL;
    }

    // Use a simple array to simulate a deque of indices.
    int* dq = (int*)malloc(sizeof(int) * numsSize);
    if (dq == NULL) {
        free(result);
        *returnSize = 0;
        return NULL;
    }
    int head = 0, tail = 0; // The deque is empty when head == tail

    for (int i = 0; i < numsSize; i++) {
        // Remove indices from the front that are out of the current window.
        if (head < tail && dq[head] <= i - k) {
            head++;
        }

        // Maintain the decreasing order of values. Remove indices from the back
        // whose corresponding values are smaller than the current element.
        while (head < tail && nums[dq[tail - 1]] < nums[i]) {
            tail--;
        }

        // Add the current index to the back.
        dq[tail++] = i;

        // Once the window is full, the max is at the front of the deque.
        if (i >= k - 1) {
            result[i - k + 1] = nums[dq[head]];
        }
    }

    free(dq);
    return result;
}

void printArray(const int* arr, int size) {
    printf("[");
    for (int i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i == size - 1) ? "" : ", ");
    }
    printf("]");
}

int main() {
    // Test Case 1
    int nums1[] = {1, 3, -1, -3, 5, 3, 6, 7};
    int k1 = 3;
    int returnSize1;
    printf("Test Case 1:\n");
    printf("Input: nums = "); printArray(nums1, sizeof(nums1)/sizeof(int)); printf(", k = %d\n", k1);
    int* res1 = maxSlidingWindow(nums1, sizeof(nums1)/sizeof(int), k1, &returnSize1);
    printf("Output: "); printArray(res1, returnSize1); printf("\n\n");
    free(res1);

    // Test Case 2
    int nums2[] = {1};
    int k2 = 1;
    int returnSize2;
    printf("Test Case 2:\n");
    printf("Input: nums = "); printArray(nums2, sizeof(nums2)/sizeof(int)); printf(", k = %d\n", k2);
    int* res2 = maxSlidingWindow(nums2, sizeof(nums2)/sizeof(int), k2, &returnSize2);
    printf("Output: "); printArray(res2, returnSize2); printf("\n\n");
    free(res2);

    // Test Case 3
    int nums3[] = {1, -1};
    int k3 = 1;
    int returnSize3;
    printf("Test Case 3:\n");
    printf("Input: nums = "); printArray(nums3, sizeof(nums3)/sizeof(int)); printf(", k = %d\n", k3);
    int* res3 = maxSlidingWindow(nums3, sizeof(nums3)/sizeof(int), k3, &returnSize3);
    printf("Output: "); printArray(res3, returnSize3); printf("\n\n");
    free(res3);

    // Test Case 4
    int nums4[] = {7, 2, 4};
    int k4 = 2;
    int returnSize4;
    printf("Test Case 4:\n");
    printf("Input: nums = "); printArray(nums4, sizeof(nums4)/sizeof(int)); printf(", k = %d\n", k4);
    int* res4 = maxSlidingWindow(nums4, sizeof(nums4)/sizeof(int), k4, &returnSize4);
    printf("Output: "); printArray(res4, returnSize4); printf("\n\n");
    free(res4);

    // Test Case 5
    int nums5[] = {1, 3, 1, 2, 0, 5};
    int k5 = 3;
    int returnSize5;
    printf("Test Case 5:\n");
    printf("Input: nums = "); printArray(nums5, sizeof(nums5)/sizeof(int)); printf(", k = %d\n", k5);
    int* res5 = maxSlidingWindow(nums5, sizeof(nums5)/sizeof(int), k5, &returnSize5);
    printf("Output: "); printArray(res5, returnSize5); printf("\n\n");
    free(res5);
    
    return 0;
}