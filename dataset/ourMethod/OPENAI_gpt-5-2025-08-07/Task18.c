#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>

typedef struct {
    int ok;             // 1 on success, 0 on failure
    uint64_t value;     // valid if ok == 1
    const char* error;  // points to a constant string literal
} BinToIntResult;

// Converts a big-endian bit array (MSB first) to an unsigned 64-bit integer.
// bits: pointer to an array of ints where each element must be 0 or 1.
// n: number of elements in bits. Empty input is treated as 0.
// Validates inputs and checks for overflow.
BinToIntResult binary_array_to_int(const int* bits, size_t n) {
    BinToIntResult res;
    res.ok = 1;
    res.value = 0u;
    res.error = NULL;

    if (n > 0 && bits == NULL) {
        res.ok = 0;
        res.error = "Null pointer for non-empty input";
        return res;
    }

    uint64_t acc = 0u;
    for (size_t i = 0; i < n; ++i) {
        int b = bits[i];
        if (!(b == 0 || b == 1)) {
            res.ok = 0;
            res.error = "Invalid bit value (must be 0 or 1)";
            return res;
        }
        // Check for overflow of acc*2 + b
        if (acc > (UINT64_MAX - (uint64_t)b) / 2u) {
            res.ok = 0;
            res.error = "Overflow: value exceeds uint64_t range";
            return res;
        }
        acc = (acc << 1) | (uint64_t)b;
    }

    res.value = acc;
    return res;
}

static void print_array(const int* bits, size_t n) {
    putchar('[');
    for (size_t i = 0; i < n; ++i) {
        if (i) { fputs(", ", stdout); }
        printf("%d", bits[i]);
    }
    putchar(']');
}

int main(void) {
    int t1[] = {0, 0, 0, 1};
    int t2[] = {0, 0, 1, 0};
    int t3[] = {0, 1, 0, 1};
    int t4[] = {1, 0, 0, 1};
    int t5[] = {1, 1, 1, 1};

    const int* tests[] = { t1, t2, t3, t4, t5 };
    size_t sizes[] = { sizeof(t1)/sizeof(t1[0]),
                       sizeof(t2)/sizeof(t2[0]),
                       sizeof(t3)/sizeof(t3[0]),
                       sizeof(t4)/sizeof(t4[0]),
                       sizeof(t5)/sizeof(t5[0]) };

    for (size_t i = 0; i < 5; ++i) {
        BinToIntResult r = binary_array_to_int(tests[i], sizes[i]);
        fputs("Testing: ", stdout);
        print_array(tests[i], sizes[i]);
        if (r.ok) {
            printf(" ==> %" PRIu64 "\n", r.value);
        } else {
            printf(" ==> error: %s\n", r.error);
        }
    }

    return 0;
}