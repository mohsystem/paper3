/* Chain-of-Through process in code generation:
 * 1) Problem understanding: Generate a random number larger than 1000 and print it.
 * 2) Security requirements: Prefer strong randomness, validate bounds, avoid bias.
 * 3) Secure coding generation: Use /dev/urandom with rejection sampling; safe bounds and overflow checks.
 * 4) Code review: Ensure no overflow, handle edge cases, fallback safely if urandom unavailable.
 * 5) Secure code output: Finalized after review.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>

/* Fallback PRNG (non-cryptographic) if /dev/urandom not available */
static int seeded = 0;
static uint32_t fallback_random_u32(void) {
    if (!seeded) {
        seeded = 1;
        unsigned int seed = (unsigned int) time(NULL) ^ (unsigned int) getpid();
        srand(seed);
    }
    /* Combine two rand() calls to get more bits */
    uint32_t r1 = (uint32_t) rand();
    uint32_t r2 = (uint32_t) rand();
    return (r1 << 1) ^ r2;
}

/* Try to get 32-bit random value from /dev/urandom */
static int get_random_u32(uint32_t *out) {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return -1;
    uint32_t val;
    ssize_t n = read(fd, &val, sizeof(val));
    close(fd);
    if (n != (ssize_t) sizeof(val)) return -1;
    *out = val;
    return 0;
}

/* Return uniform number in [0, bound-1] using rejection sampling to avoid modulo bias */
static uint32_t bounded_random(uint32_t bound) {
    if (bound == 0) return 0;
    uint32_t threshold = (uint32_t)(-bound) % bound;
    for (;;) {
        uint32_t x;
        if (get_random_u32(&x) != 0) {
            x = fallback_random_u32();
        }
        if (x >= threshold) {
            return x % bound;
        }
    }
}

/* Generates a random integer strictly greater than min_exclusive and at least > 1000. */
int generate_random_above(int min_exclusive) {
    long long lower = (long long)((min_exclusive > 1000 ? min_exclusive : 1000)) + 1LL;
    if (lower > INT_MAX) lower = INT_MAX;
    long long upper = lower + 1000000LL;
    if (upper > INT_MAX) upper = INT_MAX;
    if (upper < lower) upper = lower;

    uint32_t bound = (uint32_t)((upper - lower) + 1LL);
    uint32_t offset = bounded_random(bound);
    long long result = lower + (long long) offset;
    if (result > INT_MAX) result = INT_MAX;
    return (int) result;
}

int main(void) {
    int tests[5] = {-1, 0, 1000, 5000, 1000000};
    for (int i = 0; i < 5; ++i) {
        int val = generate_random_above(tests[i]);
        printf("%d\n", val);
    }
    return 0;
}