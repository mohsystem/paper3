/* Chain-of-Through process:
 * 1) Problem: Sum all integers inclusively between a and b (unordered). If equal, return a.
 * 2) Security: Use long long to reduce overflow risk and divide the even factor first.
 * 3) Secure coding: Constant-time arithmetic formula, no iteration across large ranges.
 * 4) Review: Ensure one factor is even before division to preserve exactness.
 * 5) Secure output: Function with 5 test cases in main.
 */

#include <stdio.h>

long long get_sum(long long a, long long b) {
    if (a == b) return a;
    long long m = (a < b) ? a : b;
    long long n = (a < b) ? b : a;
    long long count = n - m + 1LL;
    long long s = m + n;
    if ((count & 1LL) == 0LL) {
        return (count / 2LL) * s;
    } else {
        return count * (s / 2LL);
    }
}

int main(void) {
    long long tests[5][2] = {
        {1, 0},
        {1, 2},
        {0, 1},
        {1, 1},
        {-1, 2}
    };
    for (int i = 0; i < 5; ++i) {
        printf("%lld\n", get_sum(tests[i][0], tests[i][1]));
    }
    return 0;
}