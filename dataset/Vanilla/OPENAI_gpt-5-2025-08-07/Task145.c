#include <stdio.h>

int maxSubArraySum(const int* nums, int n) {
    if (n <= 0) return 0;
    int current = nums[0], best = nums[0];
    for (int i = 1; i < n; i++) {
        current = (nums[i] > current + nums[i]) ? nums[i] : (current + nums[i]);
        best = (best > current) ? best : current;
    }
    return best;
}

int main() {
    int test1[] = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
    int test2[] = {1};
    int test3[] = {5, 4, -1, 7, 8};
    int test4[] = {-1, -2, -3, -4};
    int test5[] = {0, 0, 0, 0};

    printf("%d\n", maxSubArraySum(test1, sizeof(test1)/sizeof(test1[0])));
    printf("%d\n", maxSubArraySum(test2, sizeof(test2)/sizeof(test2[0])));
    printf("%d\n", maxSubArraySum(test3, sizeof(test3)/sizeof(test3[0])));
    printf("%d\n", maxSubArraySum(test4, sizeof(test4)/sizeof(test4[0])));
    printf("%d\n", maxSubArraySum(test5, sizeof(test5)/sizeof(test5[0])));

    return 0;
}