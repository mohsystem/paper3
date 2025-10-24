#include <stdio.h>
#include <limits.h>

long long get_sum(long long a, long long b) {
    if (a == b) return a;
    long long lo = (a < b) ? a : b;
    long long hi = (a < b) ? b : a;

    __int128 n = (__int128)hi - (__int128)lo + 1;
    __int128 s = (__int128)lo + (__int128)hi;

    if ((n & 1) == 0) {
        n /= 2;
    } else {
        s /= 2;
    }

    __int128 res = n * s;
    if (res > (__int128)LLONG_MAX) return LLONG_MAX;
    if (res < (__int128)LLONG_MIN) return LLONG_MIN;
    return (long long)res;
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