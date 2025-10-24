#include <stdio.h>
#include <stdint.h>
#include <limits.h>

static inline unsigned long long umag_ll(long long x) {
    if (x >= 0) return (unsigned long long)x;
    if (x == LLONG_MIN) {
        return (unsigned long long)LLONG_MAX + 1ULL;
    }
    return (unsigned long long)(-x);
}

long long gcd_ll(long long a, long long b) {
    unsigned long long x = umag_ll(a);
    unsigned long long y = umag_ll(b);
    if (x == 0ULL && y == 0ULL) return 0LL;
    while (y != 0ULL) {
        unsigned long long r = x % y;
        x = y;
        y = r;
    }
    if (x > (unsigned long long)LLONG_MAX) {
        return LLONG_MIN; // Sentinel for 2^63
    }
    return (long long)x;
}

int main(void) {
    long long tests[5][2] = {
        {48, 18},
        {0, 0},
        {-48, 18},
        {270, 192},
        {1234567890LL, 9876543210LL}
    };
    for (int i = 0; i < 5; ++i) {
        long long a = tests[i][0];
        long long b = tests[i][1];
        long long res = gcd_ll(a, b);
        printf("gcd(%lld, %lld) = %lld\n", a, b, res);
    }
    return 0;
}