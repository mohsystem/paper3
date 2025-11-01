#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>

/* Simple xorshift64* PRNG (not cryptographic). */
static uint64_t xorshift64star_next(uint64_t *state) {
    uint64_t x = *state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    *state = x;
    return x * UINT64_C(2685821657736338717);
}

/* SplitMix64 for seeding xorshift state. */
static uint64_t splitmix64(uint64_t *x) {
    uint64_t z = (*x += UINT64_C(0x9E3779B97F4A7C15));
    z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
    return z ^ (z >> 31);
}

/* Returns a random monthly sales figure between 0 and 100000 (inclusive). */
static long long getMonthlySales(uint64_t *state) {
    uint64_t r = xorshift64star_next(state);
    return (long long)(r % UINT64_C(100001));
}

typedef struct {
    long long q[4];
} QuarterTotals;

/* Computes quarterly totals by summing 12 monthly sales into 4 quarters. */
static QuarterTotals computeQuarterlySales(uint64_t *state) {
    QuarterTotals totals = {{0, 0, 0, 0}};
    for (int month = 0; month < 12; ++month) {
        long long sale = getMonthlySales(state);
        totals.q[month / 3] += sale;
    }
    return totals;
}

int main(void) {
    for (int t = 1; t <= 5; ++t) {
        uint64_t seed = (uint64_t)time(NULL) ^ (uint64_t)(uintptr_t)&seed ^ ((uint64_t)t << 32);
        if (seed == 0) {
            seed = UINT64_C(0x9E3779B97F4A7C15);
        }
        uint64_t state = splitmix64(&seed);

        QuarterTotals totals = computeQuarterlySales(&state);
        printf("Test %d - Quarterly sales: Q1=%lld, Q2=%lld, Q3=%lld, Q4=%lld\n",
               t,
               totals.q[0], totals.q[1], totals.q[2], totals.q[3]);
    }
    return 0;
}