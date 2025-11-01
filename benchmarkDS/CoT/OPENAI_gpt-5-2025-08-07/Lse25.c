/* Chain-of-Through process:
1) Problem understanding: Produce 12 monthly random sales (0..100000), sum by quarter, print.
2) Security requirements: No global RNG state; deterministic per-seed; avoid modulo bias using rejection sampling.
3) Secure coding generation: Use xorshift64* with per-call state; validate and safe printing.
4) Code review: No dynamic allocation, no buffer overflow risks, no external input.
5) Secure code output: Deterministic, unbiased random range, and safe operations.
*/

#include <stdio.h>
#include <stdint.h>

typedef struct {
    int q[4];
} QuarterSums;

/* xorshift64* PRNG; state must not be zero */
static inline uint64_t xs64star(uint64_t* state) {
    uint64_t x = *state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    *state = x;
    return x * 2685821657736338717ULL;
}

/* Return uniform in [0, max_inclusive] using rejection sampling */
static uint32_t rand_range_inclusive(uint64_t* state, uint32_t max_inclusive) {
    uint64_t range = (uint64_t)max_inclusive + 1ULL;
    uint64_t threshold = UINT64_MAX - (UINT64_MAX % range);
    uint64_t r;
    do {
        r = xs64star(state);
    } while (r >= threshold);
    return (uint32_t)(r % range);
}

/* Returns a random monthly sales value in [0, 100000] */
int getMonthlySales(uint64_t* state) {
    return (int)rand_range_inclusive(state, 100000u);
}

/* Computes quarterly sales sums for a single year using a provided seed. */
QuarterSums computeQuarterlySales(uint64_t seed) {
    QuarterSums qs = {{0, 0, 0, 0}};
    /* Ensure non-zero seed for xorshift; use a fixed constant if zero provided */
    uint64_t st = (seed == 0ULL) ? 0x2545F4914F6CDD1DULL : seed;
    for (int month = 0; month < 12; ++month) {
        int sale = getMonthlySales(&st);
        qs.q[month / 3] += sale;
    }
    return qs;
}

void printQuarterResults(uint64_t seed, QuarterSums qs) {
    printf("Seed %llu quarterly totals:\n", (unsigned long long)seed);
    for (int i = 0; i < 4; ++i) {
        printf("  Q%d: %d\n", i + 1, qs.q[i]);
    }
}

int main(void) {
    uint64_t seeds[5] = {0ULL, 1ULL, 42ULL, 2025ULL, 999999ULL};
    for (int i = 0; i < 5; ++i) {
        QuarterSums q = computeQuarterlySales(seeds[i]);
        printQuarterResults(seeds[i], q);
    }
    return 0;
}