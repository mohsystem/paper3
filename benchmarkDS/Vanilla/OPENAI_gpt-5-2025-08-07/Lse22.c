#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

long long random_greater_than(long long minExclusive) {
    long long lower = minExclusive + 1;
    long long spanMax = 1000000LL;
    long long upper;
    if (minExclusive <= LLONG_MAX - 1 - spanMax) {
        upper = minExclusive + 1 + spanMax;
    } else {
        upper = LLONG_MAX;
    }
    if (upper < lower) upper = lower;

    unsigned long long range = (unsigned long long)(upper - lower + 1);
    unsigned long long r = 0;
    for (int i = 0; i < 5; ++i) {
        r = (r << 15) ^ (unsigned long long)rand();
    }
    if (range == 0) return lower;
    unsigned long long result = (unsigned long long)lower + (r % range);
    return (long long)result;
}

int main() {
    srand((unsigned int)time(NULL));
    long long tests[5] = {1000, 1001, 50000, 0, 123456789};
    for (int i = 0; i < 5; ++i) {
        long long val = random_greater_than(tests[i]);
        printf("Random > %lld: %lld\n", tests[i], val);
    }
    return 0;
}