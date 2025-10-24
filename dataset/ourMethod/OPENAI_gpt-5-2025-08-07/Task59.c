#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int is_ascii_letter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static char to_upper_ascii(char c) {
    if (c >= 'a' && c <= 'z') return (char)(c - 'a' + 'A');
    return c;
}

static char to_lower_ascii(char c) {
    if (c >= 'A' && c <= 'Z') return (char)(c - 'A' + 'a');
    return c;
}

static char* alloc_empty_string(void) {
    char* s = (char*)malloc(1u);
    if (s != NULL) {
        s[0] = '\0';
    }
    return s;
}

char* accum(const char* s) {
    if (s == NULL) {
        return alloc_empty_string();
    }

    const size_t MAX_LEN = 50000u; // conservative bound to avoid overflow and huge allocations
    size_t n = 0u;
    // Validate and get length safely
    for (n = 0u; s[n] != '\0'; ++n) {
        if (n > MAX_LEN) {
            return alloc_empty_string(); // too large, fail closed
        }
        if (!is_ascii_letter(s[n])) {
            return alloc_empty_string(); // invalid character
        }
    }

    // letters_total = n(n+1)/2
    // Use 64-bit math to avoid overflow in intermediate calculations
    unsigned long long nn = (unsigned long long)n;
    unsigned long long letters_total = (nn * (nn + 1ull)) / 2ull;
    unsigned long long dashes = (n > 0u) ? (nn - 1ull) : 0ull;
    unsigned long long total_ull = letters_total + dashes;

    // Check size_t overflow before allocation
    if (total_ull > (unsigned long long)~(size_t)0) {
        return alloc_empty_string();
    }

    size_t total = (size_t)total_ull;
    // +1 for null terminator
    if (total + 1u < total) { // overflow check
        return alloc_empty_string();
    }

    char* out = (char*)malloc(total + 1u);
    if (out == NULL) {
        return alloc_empty_string(); // allocation failure, return empty string to fail closed
    }

    size_t pos = 0u;
    for (size_t i = 0u; i < n; ++i) {
        char c = s[i];
        if (i != 0u) {
            out[pos++] = '-';
        }
        out[pos++] = to_upper_ascii(c);
        for (size_t j = 0u; j < i; ++j) {
            out[pos++] = to_lower_ascii(c);
        }
    }
    if (pos > total) { // sanity check
        // Should not happen, but fail closed
        free(out);
        return alloc_empty_string();
    }
    out[pos] = '\0';
    return out;
}

int main(void) {
    const char* tests[5] = {
        "abcd",
        "RqaEzty",
        "cwAt",
        "",
        "ZpG1" // invalid input (contains '1'), should produce empty string
    };

    for (int i = 0; i < 5; ++i) {
        char* r = accum(tests[i]);
        if (r == NULL) {
            // As per our function, NULL should not be returned; handle defensively.
            printf("accum(\"%s\") -> <null>\n", tests[i] ? tests[i] : "(null)");
        } else {
            printf("accum(\"%s\") -> \"%s\"\n", tests[i] ? tests[i] : "(null)", r);
            free(r);
        }
    }
    return 0;
}