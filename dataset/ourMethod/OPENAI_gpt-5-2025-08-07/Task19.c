#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define MAX_INPUT_LEN (1024 * 1024u)

static size_t safe_strnlen_s(const char *s, size_t max) {
    if (s == NULL) return 0u;
    for (size_t i = 0; i < max; ++i) {
        if (s[i] == '\0') return i;
    }
    return SIZE_MAX; // indicates the string exceeds the max bound
}

static int is_letters_and_spaces(const char *s) {
    if (s == NULL) return 0;
    for (const unsigned char *p = (const unsigned char *)s; *p != '\0'; ++p) {
        if (*p != ' ' && !isalpha(*p)) {
            return 0;
        }
    }
    return 1;
}

char* spin_words(const char* input) {
    if (input == NULL) {
        return NULL;
    }

    // Validate input length
    size_t len = safe_strnlen_s(input, MAX_INPUT_LEN);
    if (len == SIZE_MAX) {
        return NULL; // input too long
    }

    // Validate characters: only letters and spaces
    if (!is_letters_and_spaces(input)) {
        return NULL; // fail closed on invalid characters
    }

    char *out = (char*)malloc(len + 1u);
    if (out == NULL) {
        return NULL;
    }

    size_t i = 0u, pos = 0u;
    while (i < len) {
        if (input[i] == ' ') {
            out[pos++] = ' ';
            ++i;
            continue;
        }

        size_t j = i;
        while (j < len && input[j] != ' ') {
            ++j;
        }
        size_t wlen = j - i;

        if (wlen >= 5u) {
            for (size_t k = 0u; k < wlen; ++k) {
                out[pos++] = input[j - 1u - k];
            }
        } else {
            // Safe memcpy because we allocated len+1 and pos+wlen <= len
            memcpy(out + pos, input + i, wlen);
            pos += wlen;
        }
        i = j;
    }

    out[pos] = '\0';
    return out;
}

int main(void) {
    const char *tests[5] = {
        "Hey fellow warriors",
        "This is a test",
        "This is another test",
        "Welcome",
        "abcde fghij klmno pqrst uvwxyz"
    };

    for (int idx = 0; idx < 5; ++idx) {
        const char *in = tests[idx];
        char *out = spin_words(in);
        if (out == NULL) {
            printf("In:  %s\nOut: [error]\n---\n", in);
        } else {
            printf("In:  %s\nOut: %s\n---\n", in, out);
            free(out);
        }
    }
    return 0;
}