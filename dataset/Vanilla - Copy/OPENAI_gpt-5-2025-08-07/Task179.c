#include <stdio.h>
#include <stdlib.h>

int* maxSlidingWindow(const int* nums, int n, int k, int* returnSize) {
    if (n == 0 || k == 0) {
        *returnSize = 0;
        return NULL;
    }
    int outLen = n - k + 1;
    *returnSize = outLen;
    int* res = (int*)malloc(sizeof(int) * outLen);
    int* dq = (int*)malloc(sizeof(int) * n); // store indices
    int head = 0, tail = -1; // deque range [head, tail]

    int idx = 0;
    for (int i = 0; i < n; ++i) {
        while (head <= tail && nums[dq[tail]] <= nums[i]) tail--;
        dq[++tail] = i;
        if (dq[head] <= i - k) head++;
        if (i >= k - 1) res[idx++] = nums[dq[head]];
    }
    free(dq);
    return res;
}

static void printArray(const int* arr, int n) {
    printf("[");
    for (int i = 0; i < n; ++i) {
        if (i) printf(",");
        printf("%d", arr[i]);
    }
    printf("]\n");
}

int main() {
    int returnSize;

    int nums1[] = {1,3,-1,-3,5,3,6,7}; int n1 = 8; int k1 = 3;
    int* res1 = maxSlidingWindow(nums1, n1, k1, &returnSize);
    printArray(res1, returnSize); free(res1);

    int nums2[] = {1}; int n2 = 1; int k2 = 1;
    int* res2 = maxSlidingWindow(nums2, n2, k2, &returnSize);
    printArray(res2, returnSize); free(res2);

    int nums3[] = {9,8,7,6,5}; int n3 = 5; int k3 = 2;
    int* res3 = maxSlidingWindow(nums3, n3, k3, &returnSize);
    printArray(res3, returnSize); free(res3);

    int nums4[] = {-1,-3,-5,-2,-1}; int n4 = 5; int k4 = 3;
    int* res4 = maxSlidingWindow(nums4, n4, k4, &returnSize);
    printArray(res4, returnSize); free(res4);

    int nums5[] = {4,2,12,11,-5,6,2}; int n5 = 7; int k5 = 4;
    int* res5 = maxSlidingWindow(nums5, n5, k5, &returnSize);
    printArray(res5, returnSize); free(res5);

    return 0;
}