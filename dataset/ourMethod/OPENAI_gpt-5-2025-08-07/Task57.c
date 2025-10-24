#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

enum FactError {
    FACT_ERR_NONE = 0,
    FACT_ERR_NEGATIVE = 1,
    FACT_ERR_OVERFLOW = 2
};

struct FactorialResult {
    int ok;                 /* 1 = success, 0 = failure */
    uint64_t value;         /* valid only if ok == 1 */
    enum FactError error;   /* error code if ok == 0 */
};

struct FactorialResult factorial_ll(long long n) {
    struct FactorialResult res;
    if (n < 0) {
        res.ok = 0;
        res.value = 0;
        res.error = FACT_ERR_NEGATIVE;
        return res;
    }

    uint64_t result = 1;
    for (long long i = 2; i <= n; ++i) {
        uint64_t ui = (uint64_t)i;
        if (result > UINT64_MAX / ui) {
            res.ok = 0;
            res.value = 0;
            res.error = FACT_ERR_OVERFLOW;
            return res;
        }
        result *= ui;
    }

    res.ok = 1;
    res.value = result;
    res.error = FACT_ERR_NONE;
    return res;
}

int main(void) {
    long long tests[5] = {0LL, 1LL, 5LL, 20LL, 21LL};

    for (size_t i = 0; i < 5; ++i) {
        long long n = tests[i];
        struct FactorialResult r = factorial_ll(n);
        if (r.ok) {
            printf("factorial(%lld) = %" PRIu64 "\n", n, r.value);
        } else {
            if (r.error == FACT_ERR_NEGATIVE) {
                printf("Error for n=%lld: input must be a non-negative integer.\n", n);
            } else if (r.error == FACT_ERR_OVERFLOW) {
                printf("Error for n=%lld: overflow (result does not fit in 64-bit unsigned integer).\n", n);
            } else {
                printf("Error for n=%lld: unknown error.\n", n);
            }
        }
    }

    return 0;
}