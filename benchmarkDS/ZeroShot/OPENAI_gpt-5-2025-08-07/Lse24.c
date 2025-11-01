#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>

struct Result {
    int first;
    int second;
    int difference;
};

static bool get_u32_from_urandom(uint32_t *out) {
    if (out == NULL) return false;
    FILE *f = fopen("/dev/urandom", "rb");
    if (!f) return false;
    uint32_t val = 0;
    size_t n = fread(&val, 1, sizeof(val), f);
    fclose(f);
    if (n != sizeof(val)) return false;
    *out = val;
    return true;
}

static bool secure_random_in_range(int min, int max, int *out) {
    if (out == NULL) return false;
    if (min > max) {
        int t = min; min = max; max = t;
    }
    uint32_t range = (uint32_t)((uint64_t)(max - min) + 1u);
    if (range == 0) return false; // overflow safeguard, though unlikely for int
    uint32_t r;
    uint32_t limit = UINT32_MAX - (UINT32_MAX % range);
    do {
        if (!get_u32_from_urandom(&r)) {
            return false;
        }
    } while (r > limit);
    *out = (int)(min + (int)(r % range));
    return true;
}

static bool generate_and_diff(int min, int max, struct Result *out) {
    if (out == NULL) return false;
    int a, b;
    if (!secure_random_in_range(min, max, &a)) return false;
    if (!secure_random_in_range(min, max, &b)) return false;
    int diff = (a > b) ? (a - b) : (b - a);
    out->first = a;
    out->second = b;
    out->difference = diff;
    return true;
}

int main(void) {
    // 5 test cases
    for (int i = 0; i < 5; i++) {
        struct Result r;
        if (generate_and_diff(10, 1000, &r)) {
            printf("Test %d: first=%d, second=%d, difference=%d\n", i + 1, r.first, r.second, r.difference);
        } else {
            fprintf(stderr, "Secure random generation failed on test %d\n", i + 1);
            return 1;
        }
    }
    return 0;
}