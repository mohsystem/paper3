#include <stdio.h>
#include <stdlib.h>

long long gcd(long long a, long long b) {
    a = llabs(a);
    b = llabs(b);
    if (a == 0) return b;
    if (b == 0) return a;
    while (b != 0) {
        long long t = a % b;
        a = b;
        b = t;
    }
    return a;
}

int main() {
    long long tests[5][2] = {
        {54, 24},
        {0, 5},
        {17, 13},
        {-48, 18},
        {0, 0}
    };
    for (int i = 0; i < 5; ++i) {
        long long x = tests[i][0], y = tests[i][1];
        printf("gcd(%lld, %lld) = %lld\n", x, y, gcd(x, y));
    }
    return 0;
}