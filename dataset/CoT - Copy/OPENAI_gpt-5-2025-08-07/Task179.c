#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Steps 1-4: Robust C implementation with boundary checks and safe allocation
int* maxSlidingWindow(const int* nums, int n, int k, int* returnSize) {
    if (!returnSize) return NULL;
    *returnSize = 0;
    if (!nums || n <= 0 || k <= 0 || k > n) {
        return NULL;
    }
    int outSize = n - k + 1;
    int* result = (int*)malloc(sizeof(int) * outSize);
    if (!result) {
        return NULL;
    }
    int* dq = (int*)malloc(sizeof(int) * n); // store indices
    if (!dq) {
        free(result);
        return NULL;
    }
    int head = 0, tail = 0;

    for (int i = 0, rIdx = 0; i < n; ++i) {
        while (tail > head && dq[head] <= i - k) head++;
        while (tail > head && nums[dq[tail - 1]] <= nums[i]) tail--;
        dq[tail++] = i;
        if (i >= k - 1) {
            result[rIdx++] = nums[dq[head]];
        }
    }
    free(dq);
    *returnSize = outSize;
    return result;
}

static void printArray(const int* arr, int sz) {
    printf("[");
    for (int i = 0; i < sz; ++i) {
        if (i) printf(",");
        printf("%d", arr[i]);
    }
    printf("]\n");
}

// Step 5: tests
int main(void) {
    int returnSize = 0;

    int nums1[] = {1,3,-1,-3,5,3,6,7}; int k1 = 3;
    int* res1 = maxSlidingWindow(nums1, (int)(sizeof(nums1)/sizeof(nums1[0])), k1, &returnSize);
    if (res1) { printArray(res1, returnSize); free(res1); }

    int nums2[] = {1}; int k2 = 1;
    int* res2 = maxSlidingWindow(nums2, (int)(sizeof(nums2)/sizeof(nums2[0])), k2, &returnSize);
    if (res2) { printArray(res2, returnSize); free(res2); }

    int nums3[] = {9,10,9,-7,-4,-8,2,-6}; int k3 = 5;
    int* res3 = maxSlidingWindow(nums3, (int)(sizeof(nums3)/sizeof(nums3[0])), k3, &returnSize);
    if (res3) { printArray(res3, returnSize); free(res3); }

    int nums4[] = {1,-1}; int k4 = 1;
    int* res4 = maxSlidingWindow(nums4, (int)(sizeof(nums4)/sizeof(nums4[0])), k4, &returnSize);
    if (res4) { printArray(res4, returnSize); free(res4); }

    int nums5[] = {7,2,4}; int k5 = 2;
    int* res5 = maxSlidingWindow(nums5, (int)(sizeof(nums5)/sizeof(nums5[0])), k5, &returnSize);
    if (res5) { printArray(res5, returnSize); free(res5); }

    return 0;
}