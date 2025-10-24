#include <stdio.h>
#include <stddef.h>

long long maxSubarraySum(const int* nums, size_t n) {
    if (nums == NULL || n == 0) {
        return 0LL;
    }
    long long current = nums[0];
    long long best = nums[0];
    for (size_t i = 1; i < n; ++i) {
        long long v = nums[i];
        long long sum = current + v;
        current = (v > sum) ? v : sum;
        best = (best > current) ? best : current;
    }
    return best;
}

int main(void) {
    int tests0[] = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
    size_t len0 = sizeof(tests0) / sizeof(tests0[0]);

    int tests1[] = {1, 2, 3, 4};
    size_t len1 = sizeof(tests1) / sizeof(tests1[0]);

    int tests2[] = {-1, -2, -3};
    size_t len2 = sizeof(tests2) / sizeof(tests2[0]);

    int tests3[] = {5, -2, 3, -1, 2};
    size_t len3 = sizeof(tests3) / sizeof(tests3[0]);

    int* tests4 = NULL; // empty array
    size_t len4 = 0;

    printf("Test 1 -> Max Sum: %lld\n", maxSubarraySum(tests0, len0));
    printf("Test 2 -> Max Sum: %lld\n", maxSubarraySum(tests1, len1));
    printf("Test 3 -> Max Sum: %lld\n", maxSubarraySum(tests2, len2));
    printf("Test 4 -> Max Sum: %lld\n", maxSubarraySum(tests3, len3));
    printf("Test 5 -> Max Sum: %lld\n", maxSubarraySum(tests4, len4));

    return 0;
}