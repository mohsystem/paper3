#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#ifdef __linux__
#include <sys/types.h>
#include <sys/random.h>
#endif

struct Result {
    int first;
    int second;
    int difference;
};

static int get_u32(uint32_t *out) {
    if (out == NULL) {
        return -1;
    }
#ifdef __linux__
    ssize_t n = getrandom(out, sizeof(*out), 0);
    if (n == (ssize_t)sizeof(*out)) {
        return 0;
    }
#endif
    {
        FILE *f = fopen("/dev/urandom", "rb");
        if (f != NULL) {
            size_t r = fread(out, 1, sizeof(*out), f);
            fclose(f);
            if (r == sizeof(*out)) {
                return 0;
            }
        }
    }
    {
        static int seeded = 0;
        if (!seeded) {
            unsigned seed = (unsigned)time(NULL) ^ (unsigned)(uintptr_t)&seeded;
            srand(seed);
            seeded = 1;
        }
        uint32_t r1 = (uint32_t)rand();
        uint32_t r2 = (uint32_t)rand();
        *out = (r1 << 16) ^ r2;
        return 0;
    }
}

static int random_int_inclusive(int low, int high, int *out) {
    if (out == NULL || low > high) {
        return -1;
    }
    uint32_t range = (uint32_t)((high - low) + 1);
    uint32_t limit = 0xFFFFFFFFu - (0xFFFFFFFFu % range);
    uint32_t r = 0;

    do {
        if (get_u32(&r) != 0) {
            return -1;
        }
    } while (r >= limit);

    *out = low + (int)(r % range);
    return 0;
}

int generate_and_diff(struct Result *out) {
    if (out == NULL) {
        return -1;
    }
    int a = 0, b = 0;
    if (random_int_inclusive(10, 1000, &a) != 0) {
        return -1;
    }
    if (random_int_inclusive(10, 1000, &b) != 0) {
        return -1;
    }
    out->first = a;
    out->second = b;
    out->difference = a - b;
    return 0;
}

int main(void) {
    for (int i = 1; i <= 5; ++i) {
        struct Result r;
        if (generate_and_diff(&r) != 0) {
            puts("Error generating random numbers.");
            continue;
        }
        printf("Test %d: First: %d, Second: %d, Difference (first - second): %d\n",
               i, r.first, r.second, r.difference);
    }
    return 0;
}