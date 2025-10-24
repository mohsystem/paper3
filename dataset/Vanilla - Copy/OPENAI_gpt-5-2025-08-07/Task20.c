#include <stdio.h>

static unsigned long long pow_ull(int base, int exp) {
    unsigned long long res = 1;
    for (int i = 0; i < exp; ++i) res *= (unsigned long long)base;
    return res;
}

long long digPow(long long n, int p) {
    char buf[32];
#if defined(_MSC_VER)
    _snprintf(buf, sizeof(buf), "%lld", n);
#else
    snprintf(buf, sizeof(buf), "%lld", n);
#endif
    unsigned long long sum = 0ULL;
    for (int i = 0; buf[i] != '\0'; ++i) {
        int d = buf[i] - '0';
        sum += pow_ull(d, p + i);
    }
    if (sum % (unsigned long long)n == 0ULL) {
        return (long long)(sum / (unsigned long long)n);
    }
    return -1;
}

int main(void) {
    printf("%lld\n", digPow(89, 1));      // 1
    printf("%lld\n", digPow(92, 1));      // -1
    printf("%lld\n", digPow(695, 2));     // 2
    printf("%lld\n", digPow(46288, 3));   // 51
    printf("%lld\n", digPow(135, 1));     // 1
    return 0;
}