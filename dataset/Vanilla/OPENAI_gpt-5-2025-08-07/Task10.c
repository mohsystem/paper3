#include <stdio.h>

long long getSum(long long a, long long b) {
    long long lo = (a < b) ? a : b;
    long long hi = (a > b) ? a : b;
    long long n = hi - lo + 1;
    if ((n & 1LL) == 0LL) {
        return (n / 2) * (lo + hi);
    } else {
        return n * ((lo + hi) / 2);
    }
}

int main() {
    long long tests[5][2] = {
        {1, 0},
        {1, 2},
        {0, 1},
        {1, 1},
        {-1, 2}
    };
    for (int i = 0; i < 5; ++i) {
        printf("%lld\n", getSum(tests[i][0], tests[i][1]));
    }
    return 0;
}