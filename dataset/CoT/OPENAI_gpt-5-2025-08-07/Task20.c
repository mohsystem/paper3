#include <stdio.h>
#include <limits.h>

typedef unsigned __int128 u128;

static inline u128 u128_max(void) {
    return ~((u128)0);
}

typedef struct {
    u128 val;
    int overflow;
} PowRes;

static PowRes u128_pow(unsigned int base, unsigned int exp) {
    PowRes r;
    r.val = 1;
    r.overflow = 0;
    if (base == 0) {
        r.val = (exp == 0) ? 1 : 0;
        return r;
    }
    u128 maxv = u128_max();
    for (unsigned int i = 0; i < exp; ++i) {
        if (r.val != 0 && r.val > maxv / base) {
            r.overflow = 1;
            return r;
        }
        r.val *= base;
    }
    return r;
}

long long digPow(long long n, int p) {
    if (n <= 0 || p <= 0) return -1;
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "%lld", n);
    if (len <= 0 || len >= (int)sizeof(buf)) return -1;

    u128 sum = 0;
    u128 maxv = u128_max();

    for (int i = 0; i < len; ++i) {
        int digit = buf[i] - '0';
        if (digit < 0 || digit > 9) return -1;
        PowRes pr = u128_pow((unsigned int)digit, (unsigned int)(p + i));
        if (pr.overflow) return -1;
        if (sum > maxv - pr.val) return -1; // addition overflow
        sum += pr.val;
    }

    u128 un = (u128)(unsigned long long)n;
    if (un == 0) return -1;

    u128 rem = sum % un;
    if (rem != 0) return -1;

    u128 k = sum / un;
    if (k > (u128)LLONG_MAX) return -1;
    long long kres = (long long)k;
    return (kres > 0) ? kres : -1;
}

int main(void) {
    struct { long long n; int p; } tests[5] = {
        {89, 1},
        {92, 1},
        {695, 2},
        {46288, 3},
        {1, 1}
    };
    for (int i = 0; i < 5; ++i) {
        long long res = digPow(tests[i].n, tests[i].p);
        printf("digPow(%lld, %d) = %lld\n", tests[i].n, tests[i].p, res);
    }
    return 0;
}