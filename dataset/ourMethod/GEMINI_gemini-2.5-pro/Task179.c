#include <stdio.h>
#include <stdlib.h>

/**
 * Note: The returned array must be malloced, assume caller calls free().
 */
int* maxSlidingWindow(const int* nums, int numsSize, int k, int* returnSize) {
    if (nums == NULL || numsSize == 0 || k <= 0 || k > numsSize) {
        *returnSize = 0;
        return NULL;
    }

    int resultSize = numsSize - k + 1;
    int* result = (int*)malloc(resultSize * sizeof(int));
    if (result == NULL) {
        *returnSize = 0;
        return NULL;
    }

    // Deque implemented with a dynamic array, storing indices of elements from nums.
    // The max size of the deque is k, but using numsSize is safe and simple.
    int* dq = (int*)malloc(numsSize * sizeof(int));
    if (dq == NULL) {
        free(result);
        *returnSize = 0;
        return NULL;
    }

    int head = 0; // Front of the deque
    int tail = 0; // Back of the deque (points to the next empty spot)
    int resultIndex = 0;

    for (int i = 0; i < numsSize; ++i) {
        // 1. Remove indices from the front that are out of the current window.
        if (head < tail && dq[head] <= i - k) {
            head++;
        }

        // 2. Remove indices from the back whose corresponding elements are smaller
        //    than the current element.
        while (head < tail && nums[dq[tail - 1]] < nums[i]) {
            tail--;
        }

        // 3. Add the current index to the back.
        dq[tail++] = i;

        // 4. Once the window is full, the maximum is at the front of the deque.
        if (i >= k - 1) {
            result[resultIndex++] = nums[dq[head]];
        }
    }

    free(dq);
    *returnSize = resultSize;
    return result;
}

void print_array(const char* prefix, const int* arr, int size) {
    printf("%s", prefix);
    printf("[");
    for (int i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i == size - 1) ? "" : ", ");
    }
    printf("]\n");
}

int main() {
    // Test Case 1
    int nums1[] = {1, 3, -1, -3, 5, 3, 6, 7};
    int k1 = 3;
    int returnSize1;
    int* output1 = maxSlidingWindow(nums1, sizeof(nums1) / sizeof(int), k1, &returnSize1);
    print_array("Test Case 1: ", output1, returnSize1);
    free(output1);

    // Test Case 2
    int nums2[] = {1};
    int k2 = 1;
    int returnSize2;
    int* output2 = maxSlidingWindow(nums2, sizeof(nums2) / sizeof(int), k2, &returnSize2);
    print_array("Test Case 2: ", output2, returnSize2);
    free(output2);

    // Test Case 3
    int nums3[] = {1, -1};
    int k3 = 1;
    int returnSize3;
    int* output3 = maxSlidingWindow(nums3, sizeof(nums3) / sizeof(int), k3, &returnSize3);
    print_array("Test Case 3: ", output3, returnSize3);
    free(output3);

    // Test Case 4
    int nums4[] = {9, 11};
    int k4 = 2;
    int returnSize4;
    int* output4 = maxSlidingWindow(nums4, sizeof(nums4) / sizeof(int), k4, &returnSize4);
    print_array("Test Case 4: ", output4, returnSize4);
    free(output4);

    // Test Case 5
    int nums5[] = {7, 2, 4};
    int k5 = 2;
    int returnSize5;
    int* output5 = maxSlidingWindow(nums5, sizeof(nums5) / sizeof(int), k5, &returnSize5);
    print_array("Test Case 5: ", output5, returnSize5);
    free(output5);

    return 0;
}