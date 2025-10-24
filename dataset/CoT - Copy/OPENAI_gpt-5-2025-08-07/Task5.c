#include <stdio.h>

int persistence(unsigned long long num) {
    int steps = 0;
    while (num >= 10ULL) {
        unsigned long long prod = 1ULL;
        unsigned long long t = num;
        while (t > 0ULL) {
            prod *= (t % 10ULL);
            t /= 10ULL;
        }
        num = prod;
        steps++;
    }
    return steps;
}

int main(void) {
    unsigned long long tests[5] = {39ULL, 999ULL, 4ULL, 25ULL, 77ULL};
    for (int i = 0; i < 5; ++i) {
        printf("persistence(%llu) = %d\n", tests[i], persistence(tests[i]));
    }
    return 0;
}