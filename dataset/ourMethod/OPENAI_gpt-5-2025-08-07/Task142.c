#include <stdio.h>
#include <limits.h>

long long gcd(long long a, long long b) {
    if (a == 0) {
        if (b == LLONG_MIN) return LLONG_MIN;
        return (b < 0) ? -b : b;
    }
    if (b == 0) {
        if (a == LLONG_MIN) return LLONG_MIN;
        return (a < 0) ? -a : a;
    }
    while (b != 0) {
        long long r = a % b;
        a = b;
        b = r;
    }
    if (a < 0) {
        if (a == LLONG_MIN) return LLONG_MIN;
        return -a;
    }
    return a;
}

int main(void) {
    long long tests[5][2] = {
        {48, 18},
        {0, 5},
        {0, 0},
        {-24, 60},
        {13, 17}
    };
    for (int i = 0; i < 5; i++) {
        long long x = tests[i][0];
        long long y = tests[i][1];
        long long res = gcd(x, y);
        printf("gcd(%lld, %lld) = %lld\n", x, y, res);
    }
    return 0;
}