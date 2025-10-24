#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

uint64_t factorial_u64(unsigned int n, int* overflow) {
    if (overflow == NULL) {
        return 0; /* Invalid parameter */
    }
    *overflow = 0;
    uint64_t result = 1;
    for (unsigned int i = 2; i <= n; ++i) {
        if (i != 0 && result > UINT64_MAX / i) {
            *overflow = 1;
            return 0;
        }
        result *= i;
    }
    return result;
}

int main(void) {
    unsigned int tests[5] = {0u, 1u, 5u, 10u, 20u};
    for (size_t idx = 0; idx < 5; ++idx) {
        unsigned int n = tests[idx];
        int overflow = 0;
        uint64_t res = factorial_u64(n, &overflow);
        if (overflow) {
            printf("factorial(%u) overflow\n", n);
        } else {
            printf("factorial(%u) = %" PRIu64 "\n", n, res);
        }
    }
    return 0;
}