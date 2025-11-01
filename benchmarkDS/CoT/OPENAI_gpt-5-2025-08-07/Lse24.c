/* 
Chain-of-Through Step 1: Problem understanding
- Generate two random integers in [10, 1000], compute their difference, print results.
- Provide a function that accepts parameters and returns outputs.

Chain-of-Through Step 2: Security requirements
- Prefer entropy from /dev/urandom; avoid predictable seeds.
- Validate bounds and use rejection sampling to avoid modulo bias.

Chain-of-Through Step 3: Secure coding generation
- Implement secure random 32-bit generator with fallback.
- Provide a function returning the two numbers and their absolute difference.

Chain-of-Through Step 4: Code review
- Handle inclusive ranges safely, avoid overflow in range calculations, robust I/O.

Chain-of-Through Step 5: Secure code output
- Final implementation below.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>

typedef struct {
    int first;
    int second;
    int difference;
} Result;

static int secure_random_u32(uint32_t *out) {
    if (!out) return -1;
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        return -1;
    }
    ssize_t need = (ssize_t)sizeof(uint32_t);
    uint8_t *p = (uint8_t*)out;
    ssize_t got = 0;
    while (got < need) {
        ssize_t r = read(fd, p + got, (size_t)(need - got));
        if (r < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return -1;
        }
        if (r == 0) {
            close(fd);
            return -1;
        }
        got += r;
    }
    close(fd);
    return 0;
}

static uint32_t rand_range_inclusive_u32(uint32_t min, uint32_t max) {
    if (min > max) {
        uint32_t t = min; min = max; max = t;
    }
    uint64_t range = (uint64_t)max - (uint64_t)min + 1u;
    uint32_t x;
    if (secure_random_u32(&x) == 0) {
        uint64_t limit = (UINT32_MAX / range) * range;
        do {
            if (secure_random_u32(&x) != 0) break;
        } while ((uint64_t)x >= limit);
        return (uint32_t)(min + (x % range));
    } else {
        static int seeded = 0;
        if (!seeded) {
            unsigned seed = (unsigned)time(NULL) ^ (unsigned)getpid();
            srand(seed);
            seeded = 1;
        }
        uint64_t limit = (RAND_MAX / range) * range;
        int r;
        do {
            r = rand();
        } while ((uint64_t)r >= limit);
        return (uint32_t)(min + ((uint32_t)r % range));
    }
}

Result generate_and_diff(int min, int max) {
    if (min > max) {
        int t = min; min = max; max = t;
    }
    uint32_t a = rand_range_inclusive_u32((uint32_t)min, (uint32_t)max);
    uint32_t b = rand_range_inclusive_u32((uint32_t)min, (uint32_t)max);
    int diff = (a > b) ? (int)(a - b) : (int)(b - a);
    Result r = { (int)a, (int)b, diff };
    return r;
}

int main(void) {
    // 5 test cases
    for (int i = 1; i <= 5; ++i) {
        Result r = generate_and_diff(10, 1000);
        printf("Test %d: First=%d, Second=%d, Difference=%d\n", i, r.first, r.second, r.difference);
    }
    return 0;
}