#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int maxSlidingWindow(const int *nums, int n, int k, int *out) {
    if (nums == NULL || out == NULL || n <= 0 || k <= 0 || k > n) {
        return 0;
    }
    int *dq = (int *)malloc(sizeof(int) * (size_t)n);
    if (dq == NULL) {
        return 0;
    }
    int front = 0, back = 0;
    int outIdx = 0;
    for (int i = 0; i < n; ++i) {
        while (front < back && dq[front] <= i - k) front++;
        while (front < back && nums[dq[back - 1]] <= nums[i]) back--;
        dq[back++] = i;
        if (i >= k - 1) {
            out[outIdx++] = nums[dq[front]];
        }
    }
    free(dq);
    return outIdx;
}

static void print_array(const int *arr, int len) {
    putchar('[');
    for (int i = 0; i < len; ++i) {
        if (i) putchar(',');
        printf("%d", arr[i]);
    }
    putchar(']');
    putchar('\n');
}

int main(void) {
    int a1[] = {1,3,-1,-3,5,3,6,7}; int k1 = 3; int out1[sizeof(a1)/sizeof(a1[0])];
    int a2[] = {1}; int k2 = 1; int out2[sizeof(a2)/sizeof(a2[0])];
    int a3[] = {9,11}; int k3 = 2; int out3[sizeof(a3)/sizeof(a3[0])];
    int a4[] = {4,-2}; int k4 = 1; int out4[sizeof(a4)/sizeof(a4[0])];
    int a5[] = {7,2,4}; int k5 = 2; int out5[sizeof(a5)/sizeof(a5[0])];

    int len1 = maxSlidingWindow(a1, (int)(sizeof(a1)/sizeof(a1[0])), k1, out1);
    int len2 = maxSlidingWindow(a2, (int)(sizeof(a2)/sizeof(a2[0])), k2, out2);
    int len3 = maxSlidingWindow(a3, (int)(sizeof(a3)/sizeof(a3[0])), k3, out3);
    int len4 = maxSlidingWindow(a4, (int)(sizeof(a4)/sizeof(a4[0])), k4, out4);
    int len5 = maxSlidingWindow(a5, (int)(sizeof(a5)/sizeof(a5[0])), k5, out5);

    print_array(out1, len1);
    print_array(out2, len2);
    print_array(out3, len3);
    print_array(out4, len4);
    print_array(out5, len5);

    return 0;
}