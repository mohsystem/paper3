#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* expanded_form(unsigned long long n) {
    if (n == 0ULL) {
        char* out = (char*)malloc(2);
        if (out == NULL) return NULL;
        out[0] = '0';
        out[1] = '\0';
        return out;
    }

    char num[32];
    int len = snprintf(num, sizeof(num), "%llu", (unsigned long long)n);
    if (len <= 0 || len >= (int)sizeof(num)) {
        return NULL;
    }

    // Worst-case length for L digits: sum_{i=1..L} i + 3*(L-1) = L*(L+1)/2 + 3*(L-1)
    // Add 1 for null terminator
    size_t L = (size_t)len;
    size_t cap = (L * (L + 1)) / 2 + (L > 0 ? 3 * (L - 1) : 0) + 1;
    char* out = (char*)malloc(cap);
    if (out == NULL) return NULL;

    size_t pos = 0;
    int wrote_any = 0;
    for (size_t i = 0; i < L; ++i) {
        char c = num[i];
        if (c == '0') continue;

        if (wrote_any) {
            if (pos + 3 >= cap) { free(out); return NULL; }
            out[pos++] = ' ';
            out[pos++] = '+';
            out[pos++] = ' ';
        }

        if (pos + 1 >= cap) { free(out); return NULL; }
        out[pos++] = c;

        size_t zeros = L - i - 1;
        if (pos + zeros >= cap) { free(out); return NULL; }
        for (size_t z = 0; z < zeros; ++z) {
            out[pos++] = '0';
        }

        wrote_any = 1;
    }

    if (!wrote_any) {
        // Should not happen for n > 0, but handle defensively
        if (pos + 2 >= cap) { free(out); return NULL; }
        out[pos++] = '0';
    }

    if (pos >= cap) { free(out); return NULL; }
    out[pos] = '\0';
    return out;
}

int main(void) {
    unsigned long long tests[5] = {
        12ULL,       // "10 + 2"
        42ULL,       // "40 + 2"
        70304ULL,    // "70000 + 300 + 4"
        9000000ULL,  // "9000000"
        105ULL       // "100 + 5"
    };

    for (int i = 0; i < 5; ++i) {
        char* out = expanded_form(tests[i]);
        if (out == NULL) {
            printf("Error processing %llu\n", tests[i]);
        } else {
            printf("%llu -> %s\n", tests[i], out);
            free(out);
        }
    }
    return 0;
}