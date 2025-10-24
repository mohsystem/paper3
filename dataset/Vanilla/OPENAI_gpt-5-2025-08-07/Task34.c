#include <stdio.h>
#include <stdbool.h>

unsigned long long ipow(unsigned long long base, int exp) {
    unsigned long long result = 1;
    unsigned long long b = base;
    int e = exp;
    while (e > 0) {
        if (e & 1) result *= b;
        b *= b;
        e >>= 1;
    }
    return result;
}

bool narcissistic(unsigned long long n) {
    if (n == 0) return false;
    int digits = 0;
    unsigned long long temp = n;
    while (temp > 0) {
        digits++;
        temp /= 10;
    }
    unsigned long long sum = 0;
    temp = n;
    while (temp > 0) {
        unsigned int d = (unsigned int)(temp % 10ULL);
        sum += ipow(d, digits);
        temp /= 10ULL;
    }
    return sum == n;
}

int main(void) {
    unsigned long long tests[] = {153ULL, 1652ULL, 7ULL, 370ULL, 9474ULL};
    size_t count = sizeof(tests) / sizeof(tests[0]);
    for (size_t i = 0; i < count; ++i) {
        printf("%llu -> %s\n", tests[i], narcissistic(tests[i]) ? "true" : "false");
    }
    return 0;
}