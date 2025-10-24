#include <stdio.h>
#include <limits.h>

typedef unsigned long long u64;
typedef long long i64;

static unsigned __int128 u128_max_value(void) {
    return (unsigned __int128)(~(unsigned long long)0) | (((unsigned __int128)(~(unsigned long long)0)) << 64);
}

static unsigned __int128 safe_mul_u128(unsigned __int128 a, unsigned __int128 b, int *overflow) {
    unsigned __int128 maxv = u128_max_value();
    if (a != 0 && b > maxv / a) {
        *overflow = 1;
        return maxv;
    }
    return a * b;
}

static unsigned __int128 ipow_u128(unsigned int base, unsigned int exp, int *overflow) {
    if (base == 0u) return 0;
    unsigned __int128 result = 1;
    unsigned __int128 b = base;
    unsigned int e = exp;
    while (e > 0 && !*overflow) {
        if (e & 1u) {
            result = safe_mul_u128(result, b, overflow);
        }
        e >>= 1u;
        if (e > 0) {
            b = safe_mul_u128(b, b, overflow);
        }
    }
    return *overflow ? u128_max_value() : result;
}

i64 dig_pow(u64 n, unsigned int p) {
    if (n == 0ull || p == 0u) return -1;
    // Convert n to string to process digits left-to-right
    char buf[32];
    int written = snprintf(buf, sizeof(buf), "%llu", (unsigned long long)n);
    if (written <= 0 || written >= (int)sizeof(buf)) return -1;

    int overflow = 0;
    unsigned __int128 sum = 0;
    unsigned __int128 maxv = u128_max_value();

    for (int i = 0; i < written; ++i) {
        unsigned int digit = (unsigned int)(buf[i] - '0');
        if (digit > 9u) return -1; // defensive
        unsigned int exp = p + (unsigned int)i;
        unsigned __int128 term = ipow_u128(digit, exp, &overflow);
        if (overflow) return -1;
        if (sum > maxv - term) {
            return -1;
        }
        sum += term;
    }

    if (n == 0ull) return -1; // defensive
    u64 rem = (u64)(sum % n);
    if (rem != 0ull) return -1;

    unsigned __int128 k128 = sum / n;
    if (k128 == 0) return -1;

    unsigned __int128 llmax = (unsigned __int128)LLONG_MAX;
    if (k128 > llmax) return -1;

    return (i64)k128;
}

int main(void) {
    struct Test { u64 n; unsigned int p; } tests[5] = {
        {89ull, 1u},
        {92ull, 1u},
        {695ull, 2u},
        {46288ull, 3u},
        {1ull, 1u}
    };

    for (int i = 0; i < 5; ++i) {
        i64 k = dig_pow(tests[i].n, tests[i].p);
        printf("n=%llu, p=%u => k=%lld\n",
               (unsigned long long)tests[i].n,
               tests[i].p,
               (long long)k);
    }
    return 0;
}