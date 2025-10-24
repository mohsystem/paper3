#include <stdio.h>
#include <limits.h>
#include <stdbool.h>

static unsigned int countDigits(unsigned long long n) {
    unsigned int cnt = 0;
    do {
        ++cnt;
        n /= 10ULL;
    } while (n > 0ULL);
    return cnt;
}

static bool safePowULL(unsigned long long base, unsigned int exp, unsigned long long *out) {
    unsigned long long result = 1ULL;
    for (unsigned int i = 0; i < exp; ++i) {
        if (base != 0ULL && result > ULLONG_MAX / base) {
            return false; // overflow
        }
        result *= base;
    }
    *out = result;
    return true;
}

bool isNarcissistic(unsigned long long n) {
    if (n == 0ULL) {
        // Typically 0 is considered Armstrong (0^1 = 0). But per prompt, positive non-zero inputs are expected.
        return true;
    }
    unsigned int digits = countDigits(n);
    unsigned long long sum = 0ULL;
    unsigned long long t = n;
    while (t > 0ULL) {
        unsigned long long d = t % 10ULL;
        unsigned long long term = 0ULL;
        if (!safePowULL(d, digits, &term)) {
            return false; // overflow implies cannot equal n
        }
        if (sum > ULLONG_MAX - term) {
            return false; // overflow implies cannot equal n
        }
        sum += term;
        t /= 10ULL;
    }
    return sum == n;
}

int main(void) {
    unsigned long long tests[5] = {153ULL, 1652ULL, 9474ULL, 370ULL, 9926315ULL};
    for (int i = 0; i < 5; ++i) {
        printf("%s\n", isNarcissistic(tests[i]) ? "true" : "false");
    }
    return 0;
}