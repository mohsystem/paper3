#include <openssl/rand.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BYTES 1048576 /* 1 MiB safety cap */
static const char* ALPHANUM = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

unsigned char* random_bytes(size_t length) {
    if (length > MAX_BYTES) {
        return NULL;
    }
    unsigned char* buf = (unsigned char*)malloc(length ? length : 1);
    if (!buf) {
        return NULL;
    }
    if (length > 0) {
        if (RAND_bytes(buf, (int)length) != 1) {
            free(buf);
            return NULL;
        }
    }
    return buf;
}

char* to_hex(const unsigned char* data, size_t len) {
    if (!data && len != 0) return NULL;
    size_t out_len = len * 2;
    char* out = (char*)malloc(out_len + 1);
    if (!out) return NULL;
    static const char* hex = "0123456789abcdef";
    for (size_t i = 0; i < len; i++) {
        out[2 * i] = hex[(data[i] >> 4) & 0xF];
        out[2 * i + 1] = hex[data[i] & 0xF];
    }
    out[out_len] = '\0';
    return out;
}

char* random_hex(size_t n_bytes) {
    unsigned char* buf = random_bytes(n_bytes);
    if (!buf && n_bytes != 0) return NULL;
    char* hex = to_hex(buf ? buf : (unsigned char*)"", n_bytes);
    free(buf);
    return hex;
}

static int rand_u32(uint32_t* out) {
    if (!out) return 0;
    return RAND_bytes((unsigned char*)out, sizeof(uint32_t)) == 1;
}

static size_t secure_index(size_t bound) {
    if (bound == 0) return 0;
    const uint64_t space = (uint64_t)1 << 32;
    const uint64_t limit = (space / bound) * bound; /* largest multiple <= 2^32 */
    while (1) {
        uint32_t v = 0;
        if (!rand_u32(&v)) exit(EXIT_FAILURE);
        if ((uint64_t)v < limit) {
            return (size_t)((uint64_t)v % bound);
        }
    }
}

char* random_alphanum(size_t length) {
    if (length > MAX_BYTES) return NULL;
    char* out = (char*)malloc(length + 1);
    if (!out) return NULL;
    size_t alphabet_len = strlen(ALPHANUM);
    for (size_t i = 0; i < length; i++) {
        size_t idx = secure_index(alphabet_len);
        out[i] = ALPHANUM[idx];
    }
    out[length] = '\0';
    return out;
}

int random_int_in_range(int min, int max) {
    if (min == INT32_MIN && max == INT32_MAX) {
        int32_t r = 0;
        if (RAND_bytes((unsigned char*)&r, sizeof(r)) != 1) {
            fprintf(stderr, "RAND_bytes failed\n");
            exit(EXIT_FAILURE);
        }
        return r;
    }
    if (min > max) {
        fprintf(stderr, "min must be <= max\n");
        exit(EXIT_FAILURE);
    }
    uint64_t range = (uint64_t)((int64_t)max - (int64_t)min) + 1u; /* 1..2^32 */
    const uint64_t space = (uint64_t)1 << 32;
    const uint64_t limit = (space / range) * range;
    while (1) {
        uint32_t v = 0;
        if (RAND_bytes((unsigned char*)&v, sizeof(v)) != 1) {
            fprintf(stderr, "RAND_bytes failed\n");
            exit(EXIT_FAILURE);
        }
        if ((uint64_t)v < limit) {
            return (int)((int64_t)min + (int64_t)((uint64_t)v % range));
        }
    }
}

int main(void) {
    /* 5 test cases */
    char* h1 = random_hex(16);
    char* h2 = random_hex(32);
    char* a1 = random_alphanum(20);
    int n1 = random_int_in_range(0, 10);
    int n2 = random_int_in_range(INT32_MIN, INT32_MAX);

    if (!h1 || !h2 || !a1) {
        fprintf(stderr, "Allocation or RNG failure\n");
        free(h1); free(h2); free(a1);
        return EXIT_FAILURE;
    }

    printf("Random hex (16 bytes): %s\n", h1);
    printf("Random hex (32 bytes): %s\n", h2);
    printf("Random alphanum (20 chars): %s\n", a1);
    printf("Random int [0,10]: %d\n", n1);
    printf("Random int full range: %d\n", n2);

    free(h1);
    free(h2);
    free(a1);
    return EXIT_SUCCESS;
}