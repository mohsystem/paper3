/* Chain-of-Through process:
 * 1) Problem: Compute GCD of two numbers.
 * 2) Security: Use unsigned magnitudes to avoid overflow on abs(LLONG_MIN).
 * 3) Secure generation: Euclidean algorithm on unsigned long long.
 * 4) Review: Handles zeros and negatives; returns non-negative magnitude.
 * 5) Output: Final code with 5 test cases.
 */

#include <stdio.h>
#include <stdint.h>

typedef unsigned long long ull;
typedef long long ll;

static ull gcd_ull(ull a, ull b) {
    while (b != 0) {
        ull t = a % b;
        a = b;
        b = t;
    }
    return a;
}

ull gcd_ll(ll x, ll y) {
    ull a = x < 0 ? (ull)(-(unsigned long long)x) : (ull)x;
    ull b = y < 0 ? (ull)(-(unsigned long long)y) : (ull)y;
    return gcd_ull(a, b);
}

int main(void) {
    ll tests[5][2] = {
        {48, 18},
        {0, 0},
        {270, 192},
        {-24, 60},
        {13, 7}
    };
    for (int i = 0; i < 5; ++i) {
        ull g = gcd_ll(tests[i][0], tests[i][1]);
        printf("gcd(%lld, %lld) = %llu\n", tests[i][0], tests[i][1], g);
    }
    return 0;
}