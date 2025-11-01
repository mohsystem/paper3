#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

static char* a(float f) {
    // Allocate a fixed-size buffer sufficient for formatted float
    // Example format: "-1.234567" plus null terminator
    size_t cap = 32U;
    char *buf = (char*)malloc(cap);
    if (buf == NULL) {
        return NULL;
    }
    int written = snprintf(buf, cap, "%.6f", (double)f);
    if (written < 0 || (size_t)written >= cap) {
        free(buf);
        return NULL;
    }
    return buf;
}

// Xorshift32 PRNG (non-cryptographic). Seed must not be zero.
typedef struct {
    uint32_t s;
} prng32_t;

static void prng32_init(prng32_t* st, uint32_t seed) {
    if (st == NULL) return;
    if (seed == 0U) {
        seed = 0x6D2B79F5U; // Non-zero default
    }
    st->s = seed;
}

static uint32_t prng32_next(prng32_t* st) {
    uint32_t x = st->s;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    st->s = x;
    return x;
}

static float prng32_next_float01(prng32_t* st) {
    // Map 32-bit int to [0,1)
    uint32_t x = prng32_next(st);
    const float denom = 4294967296.0f; // 2^32
    return (float)((double)x / (double)denom);
}

static char** generate_n_random_float_strings(size_t n, uint32_t seed) {
    if (n == 0U || n > 1000U) {
        return NULL;
    }

    char** out = (char**)calloc(n, sizeof(char*));
    if (out == NULL) {
        return NULL;
    }

    prng32_t rng;
    prng32_init(&rng, seed);

    for (size_t i = 0; i < n; ++i) {
        float v = prng32_next_float01(&rng);
        char* s = a(v);
        if (s == NULL) {
            // Cleanup allocated strings on failure
            for (size_t j = 0; j < i; ++j) {
                free(out[j]);
                out[j] = NULL;
            }
            free(out);
            return NULL;
        }
        out[i] = s;
    }
    return out;
}

int main(void) {
    // Seed based on current time to vary outputs
    uint32_t base_seed = (uint32_t)time(NULL);
    if (base_seed == 0U) {
        base_seed = 123456789U;
    }

    // Core requirement demonstration
    char** trio = generate_n_random_float_strings(3U, base_seed ^ 0xA5A5A5A5U);
    if (trio == NULL) {
        fprintf(stderr, "error\n");
        return 1;
    }
    char* str_a = trio[0];
    char* str_b = trio[1];
    char* str_c = trio[2];
    printf("Test 1: [%s, %s, %s]\n", str_a, str_b, str_c);

    // Additional 4 test cases
    for (int t = 2; t <= 5; ++t) {
        char** r = generate_n_random_float_strings(3U, base_seed ^ (uint32_t)(t * 0x9E3779B9U));
        if (r == NULL) {
            fprintf(stderr, "error\n");
            // Free previous trio before exiting
            free(str_a); free(str_b); free(str_c);
            free(trio);
            return 1;
        }
        printf("Test %d: [%s, %s, %s]\n", t, r[0], r[1], r[2]);
        free(r[0]); free(r[1]); free(r[2]);
        free(r);
    }

    // Cleanup
    free(str_a); free(str_b); free(str_c);
    free(trio);

    return 0;
}