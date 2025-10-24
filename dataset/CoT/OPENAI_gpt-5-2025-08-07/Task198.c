#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/*
Chain-of-Through Steps 1-5:
- Implement rand10 using only rand7.
- Use internal PRNG (xorshift64) to define rand7 without built-in randomness.
- Use rejection sampling to ensure uniformity and avoid modulo bias.
*/

static uint64_t rngState = 0x123456789ABCDEFULL; // non-zero seed

static uint64_t next64(void) {
    uint64_t x = rngState;
    x ^= (x << 13);
    x ^= (x >> 7);
    x ^= (x << 17);
    rngState = x;
    return x & 0x7fffffffffffffffULL; // 63-bit non-negative
}

// Uniform in [1..7]
int rand7(void) {
    const uint32_t limit = 2147483646u; // 7 * floor(2^31/7)
    for (;;) {
        uint32_t v = (uint32_t)(next64() & 0x7fffffffULL); // 31-bit non-negative
        if (v <= limit) return (int)(v % 7u) + 1;
    }
}

// Uniform in [1..10] using only rand7()
int rand10(void) {
    for (;;) {
        int a = rand7();
        int b = rand7();
        int idx = (a - 1) * 7 + b; // 1..49
        if (idx <= 40) return 1 + (idx - 1) % 10;
    }
}

// Run n trials and return an allocated array of results
int* run_test(int n) {
    if (n < 0) return NULL;
    int* arr = (int*)malloc((size_t)n * sizeof(int));
    if (!arr) return NULL;
    for (int i = 0; i < n; ++i) arr[i] = rand10();
    return arr;
}

static void print_array(const int* arr, int n) {
    putchar('[');
    for (int i = 0; i < n; ++i) {
        if (i) putchar(',');
        printf("%d", arr[i]);
    }
    putchar(']');
    putchar('\n');
}

int main(void) {
    int tests[5] = {1, 2, 3, 5, 20}; // 5 test cases
    for (int i = 0; i < 5; ++i) {
        int n = tests[i];
        int* res = run_test(n);
        if (res) {
            print_array(res, n);
            free(res);
        } else {
            // If allocation fails, print empty result to maintain output format
            printf("[]\n");
        }
    }
    return 0;
}