/* Chain-of-Through process in code generation:
1. Problem understanding: Generate 3 random integers > 1000, print them, compute sum and print sum and sum2.
2. Security requirements: Prefer secure randomness (/dev/urandom), validate ranges, avoid modulo bias.
3. Secure coding generation: Implement rejection sampling for uniform distribution, handle fallback safely.
4. Code review: Verified range math, integer types, and formatting correctness.
5. Secure code output: Final code with 5 test cases and functions returning results.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>

struct Result {
    int a, b, c, sum, sum2;
};

static uint32_t get_secure_uint32(void) {
    uint32_t val;
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd >= 0) {
        ssize_t n = read(fd, &val, sizeof(val));
        close(fd);
        if (n == (ssize_t)sizeof(val)) {
            return val;
        }
    }
    static int seeded = 0;
    if (!seeded) {
        unsigned seed = (unsigned)time(NULL) ^ (unsigned)(uintptr_t)&seeded ^ (unsigned)clock();
        srand(seed);
        seeded = 1;
    }
    uint32_t v = ((uint32_t)rand() << 16) ^ (uint32_t)rand();
    v ^= (uint32_t)clock();
    return v;
}

static int random_in_range(int min_inclusive, int max_inclusive) {
    if (max_inclusive < min_inclusive) {
        return min_inclusive;
    }
    uint32_t span = (uint32_t)((uint64_t)max_inclusive - (uint64_t)min_inclusive + 1u);
    uint32_t limit = UINT32_MAX - (UINT32_MAX % span);
    uint32_t r;
    do {
        r = get_secure_uint32();
    } while (r >= limit);
    return (int)(min_inclusive + (int)(r % span));
}

struct Result generate_and_sum(int minExclusive, int maxInclusive) {
    struct Result res = {0, 0, 0, 0, 0};
    if (maxInclusive <= minExclusive) {
        return res;
    }
    int min_incl = minExclusive + 1;
    res.a = random_in_range(min_incl, maxInclusive);
    res.b = random_in_range(min_incl, maxInclusive);
    res.c = random_in_range(min_incl, maxInclusive);
    res.sum = res.a + res.b + res.c;
    res.sum2 = res.sum;
    return res;
}

int main(void) {
    int minExclusive = 1000;
    int maxInclusive = 9999;
    for (int i = 0; i < 5; ++i) {
        struct Result r = generate_and_sum(minExclusive, maxInclusive);
        printf("a : %d b : %d c : %d sum: %d sum2: %d\n", r.a, r.b, r.c, r.sum, r.sum2);
    }
    return 0;
}