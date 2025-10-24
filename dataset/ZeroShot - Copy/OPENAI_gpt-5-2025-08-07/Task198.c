#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    uint32_t state;
} RNG;

static uint64_t splitmix64(uint64_t x) {
    x += 0x9E3779B97F4A7C15ULL;
    uint64_t z = x;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    z ^= (z >> 31);
    return z;
}

void rng_init(RNG* rng, uint64_t seed) {
    uint64_t mixed = splitmix64(seed);
    rng->state = (uint32_t)(mixed & 0xFFFFFFFFu);
    if (rng->state == 0u) {
        rng->state = 0x6D2B79F5u; // Non-zero default
    }
}

static uint32_t rng_next_uint32(RNG* rng) {
    uint32_t x = rng->state;
    x ^= (x << 13);
    x ^= (x >> 17);
    x ^= (x << 5);
    rng->state = x;
    return x;
}

// Returns uniform in [1,7]
int rand7(RNG* rng) {
    const uint32_t LIMIT = 4294967291u; // floor((2^32)/7)*7 - 1
    for (;;) {
        uint32_t u = rng_next_uint32(rng);
        if (u <= LIMIT) {
            return 1 + (int)(u % 7u);
        }
    }
}

// Returns uniform in [1,10] using only rand7()
int rand10(RNG* rng) {
    for (;;) {
        int a = rand7(rng);
        int b = rand7(rng);
        int idx = (a - 1) * 7 + b; // 1..49
        if (idx <= 40) {
            return 1 + (idx - 1) % 10;
        }
    }
}

// Runs n times rand10 and returns a heap-allocated array of size n
int* runTest(int n, uint64_t seed) {
    if (n < 0) return NULL;
    RNG rng;
    rng_init(&rng, seed);
    int* arr = (int*)malloc((size_t)n * sizeof(int));
    if (!arr) return NULL;
    for (int i = 0; i < n; ++i) {
        arr[i] = rand10(&rng);
    }
    return arr;
}

static void printArray(const int* arr, int n) {
    putchar('[');
    for (int i = 0; i < n; ++i) {
        if (i) putchar(',');
        printf("%d", arr[i]);
    }
    putchar(']');
    putchar('\n');
}

int main(void) {
    int tests[5] = {1, 2, 3, 5, 10};
    uint64_t seeds[5] = {12345ULL, 67890ULL, 13579ULL, 24680ULL, 424242ULL};
    for (int i = 0; i < 5; ++i) {
        int n = tests[i];
        int* out = runTest(n, seeds[i]);
        if (!out) {
            fprintf(stderr, "Allocation failed\n");
            return 1;
        }
        printArray(out, n);
        free(out);
    }
    return 0;
}