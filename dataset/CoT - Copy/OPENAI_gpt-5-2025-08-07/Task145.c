/* Task145 - C implementation
   Chain-of-Through process:
   1) Problem: Max sum of a contiguous subarray.
   2) Security: Use 64-bit accumulation; validate inputs; no unsafe IO.
   3) Secure coding: Pure function; no dynamic allocation from untrusted input.
   4) Code review: Handles empty and all-negative arrays properly.
   5) Final secure output.
*/

#include <stdio.h>
#include <stddef.h>

long long maxSubArray(const int* nums, size_t n) {
    if (nums == NULL || n == 0U) {
        return 0LL;
    }
    long long best = -(1LL<<62); /* sufficiently small sentinel */
    long long curr = 0LL;
    for (size_t i = 0U; i < n; ++i) {
        long long x = (long long)nums[i];
        long long sum = curr + x;
        curr = (x > sum) ? x : sum;
        if (curr > best) {
            best = curr;
        }
    }
    return best;
}

static void run_test(const int* arr, size_t n) {
    long long res = maxSubArray(arr, n);
    printf("%lld\n", res);
}

int main(void) {
    // 5 test cases
    int a1[] = {1, -3, 2, 1, -1};
    run_test(a1, sizeof(a1)/sizeof(a1[0])); // 3

    int a2[] = {-2, -3, -1, -4};
    run_test(a2, sizeof(a2)/sizeof(a2[0])); // -1

    int a3[] = {5};
    run_test(a3, sizeof(a3)/sizeof(a3[0])); // 5

    int a4[] = {};
    run_test(a4, 0U); // 0

    int a5[] = {4, -1, 2, 1, -5, 4};
    run_test(a5, sizeof(a5)/sizeof(a5[0])); // 6

    return 0;
}