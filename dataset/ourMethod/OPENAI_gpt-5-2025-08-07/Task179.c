#include <stdio.h>
#include <stdlib.h>

int* maxSlidingWindow(const int* nums, int n, int k, int* returnSize) {
    if (returnSize == NULL) {
        return NULL;
    }
    *returnSize = 0;
    if (nums == NULL || n <= 0 || k <= 0 || k > n) {
        return NULL;
    }

    int outLen = n - k + 1;
    int* result = (int*)malloc((size_t)outLen * sizeof(int));
    if (result == NULL) {
        return NULL;
    }

    int* dq = (int*)malloc((size_t)n * sizeof(int)); // store indices
    if (dq == NULL) {
        free(result);
        return NULL;
    }
    int head = 0, tail = 0; // deque is [head, tail)

    for (int i = 0; i < n; ++i) {
        while (head < tail && dq[head] <= i - k) {
            head++;
        }
        while (head < tail && nums[dq[tail - 1]] <= nums[i]) {
            tail--;
        }
        dq[tail++] = i;
        if (i >= k - 1) {
            result[i - k + 1] = nums[dq[head]];
        }
    }

    free(dq);
    *returnSize = outLen;
    return result;
}

void printArray(const int* arr, int n) {
    printf("[");
    for (int i = 0; i < n; ++i) {
        if (i) printf(",");
        printf("%d", arr[i]);
    }
    printf("]");
}

void runTest(const int* nums, int n, int k, const int* expected, int expectedSize) {
    int outSize = 0;
    int* out = maxSlidingWindow(nums, n, k, &outSize);
    printf("nums=");
    printArray(nums, n);
    printf(", k=%d\n", k);
    printf("out =");
    if (out != NULL) {
        printArray(out, outSize);
    } else {
        printf("[]");
    }
    printf("\nexp =");
    printArray(expected, expectedSize);
    printf("\n---\n");
    free(out);
}

int main(void) {
    int nums1[] = {1,3,-1,-3,5,3,6,7};
    int exp1[]  = {3,3,5,5,6,7};
    runTest(nums1, 8, 3, exp1, 6);

    int nums2[] = {1};
    int exp2[]  = {1};
    runTest(nums2, 1, 1, exp2, 1);

    int nums3[] = {9,8,7,6};
    int exp3[]  = {9};
    runTest(nums3, 4, 4, exp3, 1);

    int nums4[] = {1,2,3,4,5};
    int exp4[]  = {2,3,4,5};
    runTest(nums4, 5, 2, exp4, 4);

    int nums5[] = {5,4,3,2,1};
    int exp5[]  = {5,4,3,2};
    runTest(nums5, 5, 2, exp5, 4);

    return 0;
}