#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Park-Miller LCG parameters */
static const uint64_t MOD = 2147483647ULL; /* 2^31 - 1 */
static const uint64_t MUL = 48271ULL;
static uint64_t seed = 123456789ULL;

static uint32_t rngNext(void) {
    seed = (seed * MUL) % MOD; /* 1..MOD-1 */
    return (uint32_t)seed;
}

static int rngUniform(int k) {
    if (k <= 0) {
        return -1;
    }
    uint64_t m1 = MOD - 1ULL; /* 2147483646 */
    uint64_t limit = (m1 / (uint64_t)k) * (uint64_t)k;
    for (;;) {
        uint64_t r = (uint64_t)rngNext() - 1ULL; /* 0..m1-1 */
        if (r < limit) {
            return (int)(r % (uint64_t)k);
        }
    }
}

/* Given API */
int rand7(void) {
    return rngUniform(7) + 1; /* 1..7 */
}

/* Required function */
int rand10(void) {
    for (;;) {
        int a = rand7();
        int b = rand7();
        int num = (a - 1) * 7 + b; /* 1..49 */
        if (num <= 40) {
            return 1 + (num - 1) % 10;
        }
    }
}

int* runRand10NTimes(int n, int* outSize) {
    if (outSize == NULL) return NULL;
    *outSize = 0;
    if (n < 1 || n > 100000) return NULL;
    int* arr = (int*)malloc((size_t)n * sizeof(int));
    if (!arr) return NULL;
    for (int i = 0; i < n; ++i) {
        arr[i] = rand10();
    }
    *outSize = n;
    return arr;
}

static void printArray(const int* arr, int len) {
    if (!arr || len < 0) {
        puts("[]");
        return;
    }
    putchar('[');
    for (int i = 0; i < len; ++i) {
        if (i > 0) putchar(',');
        printf("%d", arr[i]);
    }
    putchar(']');
    putchar('\n');
}

int main(void) {
    int tests[5] = {1, 2, 3, 5, 10};
    for (int i = 0; i < 5; ++i) {
        int n = tests[i];
        int outSize = 0;
        int* res = runRand10NTimes(n, &outSize);
        if (res && outSize == n) {
            printArray(res, outSize);
        } else {
            puts("[]");
        }
        free(res);
    }
    return 0;
}