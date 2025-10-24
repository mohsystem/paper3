#include <stdio.h>
#include <limits.h>

static int safe_mul_ull(unsigned long long a, unsigned long long b, unsigned long long* res) {
    if (a == 0 || b == 0) { *res = 0ULL; return 1; }
    if (a > ULLONG_MAX / b) return 0;
    *res = a * b;
    return 1;
}

static int safe_add_ull(unsigned long long a, unsigned long long b, unsigned long long* res) {
    if (ULLONG_MAX - a < b) return 0;
    *res = a + b; return 1;
}

static int pow_ull(unsigned long long base, int exp, unsigned long long* out) {
    *out = 1ULL;
    for (int i = 0; i < exp; ++i) {
        unsigned long long tmp;
        if (!safe_mul_ull(*out, base, &tmp)) return 0;
        *out = tmp;
    }
    return 1;
}

long long digPow(long long n, int p) {
    if (n <= 0 || p <= 0) return -1;
    long long tn = n;
    int digits[32];
    int len = 0;
    while (tn > 0) {
        tn /= 10;
        len++;
    }
    tn = n;
    for (int i = len - 1; i >= 0; --i) {
        digits[i] = (int)(tn % 10);
        tn /= 10;
    }

    unsigned long long sum = 0ULL;
    int exp = p;
    for (int i = 0; i < len; ++i) {
        unsigned long long term;
        if (!pow_ull((unsigned long long)digits[i], exp, &term)) return -1;
        unsigned long long tmp;
        if (!safe_add_ull(sum, term, &tmp)) return -1;
        sum = tmp;
        exp++;
    }
    unsigned long long un = (unsigned long long)n;
    if (un == 0ULL) return -1;
    if (sum % un == 0ULL) {
        unsigned long long k = sum / un;
        if (k <= (unsigned long long)LLONG_MAX) {
            return (long long)k;
        }
    }
    return -1;
}

int main(void) {
    struct { long long n; int p; } tests[5] = {
        {89, 1},
        {92, 1},
        {695, 2},
        {46288, 3},
        {135, 1}
    };
    for (int i = 0; i < 5; ++i) {
        long long res = digPow(tests[i].n, tests[i].p);
        printf("digPow(%lld, %d) = %lld\n", tests[i].n, tests[i].p, res);
    }
    return 0;
}