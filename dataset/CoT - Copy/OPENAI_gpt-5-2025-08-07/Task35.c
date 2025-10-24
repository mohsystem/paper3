/* Chain-of-Through process in code generation:
 * 1) Problem understanding: compute remaining people on a bus given pairs [on, off].
 * 2) Security requirements: validate inputs, avoid negative totals, check for overflow.
 * 3) Secure coding generation: safe add/sub helpers, validate values.
 * 4) Code review: ensure checks on inputs and computations.
 * 5) Secure code output: final code with 5 tests.
 */

#include <stdio.h>
#include <limits.h>
#include <stddef.h>

struct Pair {
    long long on;
    long long off;
};

static int safe_add_ll(long long a, long long b, long long* out) {
    if ((b > 0 && a > LLONG_MAX - b) || (b < 0 && a < LLONG_MIN - b)) {
        return 0;
    }
    *out = a + b;
    return 1;
}

static int safe_sub_ll(long long a, long long b, long long* out) {
    if ((b > 0 && a < LLONG_MIN + b) || (b < 0 && a > LLONG_MAX + b)) {
        return 0;
    }
    *out = a - b;
    return 1;
}

/* Computes remaining people on the bus after processing all stops.
 * Each stop is a pair {on, off}. Returns total remaining, or -1 on error.
 */
long long peopleOnBus(const struct Pair* stops, size_t n) {
    long long total = 0;
    if (stops == NULL || n == 0) {
        return 0;
    }
    for (size_t i = 0; i < n; ++i) {
        long long on = stops[i].on;
        long long off = stops[i].off;
        if (on < 0 || off < 0) {
            return -1; /* negative values not allowed */
        }
        long long temp = 0, next = 0;
        if (!safe_add_ll(total, on, &temp)) {
            return -1; /* overflow */
        }
        if (!safe_sub_ll(temp, off, &next)) {
            return -1; /* overflow */
        }
        if (next < 0) {
            return -1; /* negative population */
        }
        total = next;
    }
    return total;
}

int main(void) {
    struct Pair t1[] = { {10,0}, {3,5}, {5,8} }; /* 5 */
    struct Pair t2[] = { {3,0}, {9,1}, {4,10}, {12,2}, {6,1}, {7,10} }; /* 17 */
    struct Pair t3[] = { }; /* 0 */
    struct Pair t4[] = { {0,0} }; /* 0 */
    struct Pair t5[] = {
        {1000000000000LL, 0LL},
        {2000000000000LL, 500000000000LL},
        {3000000000000LL, 1000000000000LL}
    }; /* 4500000000000 */

    long long res1 = peopleOnBus(t1, sizeof(t1)/sizeof(t1[0]));
    long long res2 = peopleOnBus(t2, sizeof(t2)/sizeof(t2[0]));
    long long res3 = peopleOnBus(t3, sizeof(t3)/sizeof(t3[0]));
    long long res4 = peopleOnBus(t4, sizeof(t4)/sizeof(t4[0]));
    long long res5 = peopleOnBus(t5, sizeof(t5)/sizeof(t5[0]));

    printf("Test 1: %lld\n", res1);
    printf("Test 2: %lld\n", res2);
    printf("Test 3: %lld\n", res3);
    printf("Test 4: %lld\n", res4);
    printf("Test 5: %lld\n", res5);

    return 0;
}