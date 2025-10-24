/* Chain-of-Through process:
   1) Problem understanding: Verify if a number equals the sum of its digits each to the power of the number of digits.
   2) Security requirements: Avoid floating pow; use integer math.
   3) Secure coding generation: Early overflow-safe checks before addition.
   4) Code review: Ensure all arithmetic is within unsigned long long limits.
   5) Secure code output: Function returns bool; includes 5 test cases. */

#include <stdio.h>
#include <stdbool.h>

static unsigned long long ipow_ull(unsigned int base, unsigned int exp) {
    unsigned long long result = 1ULL;
    unsigned long long b = base;
    unsigned int e = exp;
    while (e > 0U) {
        if (e & 1U) {
            result *= b;
        }
        b *= b;
        e >>= 1U;
    }
    return result;
}

static unsigned int count_digits(unsigned long long n) {
    unsigned int c = 0U;
    do {
        c++;
        n /= 10ULL;
    } while (n > 0ULL);
    return c;
}

bool narcissistic(unsigned long long n) {
    if (n == 0ULL) return false; // Only positive non-zero integers
    unsigned int k = count_digits(n);
    unsigned long long sum = 0ULL;
    unsigned long long temp = n;

    while (temp > 0ULL) {
        unsigned int d = (unsigned int)(temp % 10ULL);
        unsigned long long term = ipow_ull(d, k);
        if (term > n) return false;
        if (sum > n - term) return false;
        sum += term;
        temp /= 10ULL;
    }
    return sum == n;
}

static void print_bool(bool b) {
    printf(b ? "true" : "false");
}

int main(void) {
    unsigned long long tests[5] = {153ULL, 1652ULL, 9474ULL, 7ULL, 10ULL};
    for (int i = 0; i < 5; ++i) {
        printf("n=%llu -> ", tests[i]);
        print_bool(narcissistic(tests[i]));
        printf("\n");
    }
    return 0;
}