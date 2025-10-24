#include <stdio.h>

static unsigned long long isqrt_ull(unsigned long long x) {
    if (x < 2ULL) return x;
    unsigned long long low = 1ULL, high = (x < 3037000499ULL ? x : 3037000499ULL);
    while (low <= high) {
        unsigned long long mid = low + (high - low) / 2ULL;
        if (mid <= x / mid) {
            low = mid + 1ULL;
        } else {
            if (mid == 0ULL) break;
            high = mid - 1ULL;
        }
    }
    return high;
}

long long findNb(long long m) {
    if (m <= 0) return -1;
    unsigned long long um = (unsigned long long)m;

    unsigned long long k = isqrt_ull(um);
    if (k * k != um) return -1;

    unsigned long long d = 1ULL + 8ULL * k;
    unsigned long long sd = isqrt_ull(d);
    if (sd * sd != d) return -1;

    unsigned long long n = (sd - 1ULL) / 2ULL;

    unsigned long long t = (n % 2ULL == 0ULL) ? (n / 2ULL) * (n + 1ULL)
                                              : n * ((n + 1ULL) / 2ULL);
    if (t == k) return (long long)n;
    return -1;
}

int main(void) {
    long long tests[5] = {
        1071225LL,          // -> 45
        91716553919377LL,   // -> -1
        4183059834009LL,    // -> 2022
        1LL,                // -> 1
        0LL                 // -> -1
    };
    for (int i = 0; i < 5; ++i) {
        printf("findNb(%lld) = %lld\n", tests[i], findNb(tests[i]));
    }
    return 0;
}