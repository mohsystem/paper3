#include <stdio.h>
#include <limits.h>
#include <stdbool.h>

static unsigned int count_digits_ull(unsigned long long n) {
    if (n == 0ULL) return 1U; /* Not expected per spec, but safe */
    unsigned int count = 0U;
    while (n > 0ULL) {
        ++count;
        n /= 10ULL;
    }
    return count;
}

static bool mul_overflow_ull(unsigned long long a, unsigned long long b) {
    if (a == 0ULL || b == 0ULL) return false;
    return a > (ULLONG_MAX / b);
}

static bool add_overflow_ull(unsigned long long a, unsigned long long b) {
    return a > (ULLONG_MAX - b);
}

static bool pow_digit_ull(unsigned int base, unsigned int exp, unsigned long long *result) {
    if (result == NULL) return false;
    *result = 1ULL;
    for (unsigned int i = 0; i < exp; ++i) {
        if (mul_overflow_ull(*result, (unsigned long long)base)) {
            return false; /* overflow */
        }
        *result *= (unsigned long long)base;
    }
    return true;
}

bool is_narcissistic(unsigned long long n) {
    if (n == 0ULL) return false; /* positive non-zero inputs expected */
    unsigned int digits = count_digits_ull(n);

    unsigned long long sum = 0ULL;
    unsigned long long temp = n;
    while (temp > 0ULL) {
        unsigned int d = (unsigned int)(temp % 10ULL);
        temp /= 10ULL;

        unsigned long long p = 0ULL;
        if (!pow_digit_ull(d, digits, &p)) {
            return false; /* overflow => cannot equal n */
        }
        if (add_overflow_ull(sum, p)) {
            return false; /* overflow => cannot equal n */
        }
        sum += p;
    }
    return sum == n;
}

int main(void) {
    unsigned long long tests[5] = {153ULL, 1652ULL, 7ULL, 9474ULL, 9926315ULL};
    for (int i = 0; i < 5; ++i) {
        bool res = is_narcissistic(tests[i]);
        printf("%llu -> %s\n", tests[i], res ? "true" : "false");
    }
    return 0;
}