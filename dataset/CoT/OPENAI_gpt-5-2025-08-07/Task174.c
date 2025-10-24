/* Step 1-5: Secure, efficient implementation using KMP to find longest palindromic prefix. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t safe_strlen(const char* s) {
    return s ? strlen(s) : 0u;
}

// Computes the shortest palindrome by adding characters in front of the input string.
// Returns a newly allocated string that the caller must free.
char* shortestPalindrome(const char* s) {
    if (s == NULL) {
        char* out = (char*)malloc(1);
        if (out) out[0] = '\0';
        return out;
    }
    size_t n = strlen(s);
    if (n == 0) {
        char* out = (char*)malloc(1);
        if (out) out[0] = '\0';
        return out;
    }

    // Reverse s
    char* rev = (char*)malloc(n + 1);
    if (!rev) return NULL;
    for (size_t i = 0; i < n; ++i) rev[i] = s[n - 1 - i];
    rev[n] = '\0';

    // Build combined = s + '#' + rev
    size_t m = n + 1 + n;
    char* comb = (char*)malloc(m + 1);
    if (!comb) { free(rev); return NULL; }
    memcpy(comb, s, n);
    comb[n] = '#';
    memcpy(comb + n + 1, rev, n);
    comb[m] = '\0';

    // KMP prefix function
    int* pi = (int*)malloc(m * sizeof(int));
    if (!pi) { free(rev); free(comb); return NULL; }
    pi[0] = 0;
    int j = 0;
    for (size_t i = 1; i < m; ++i) {
        while (j > 0 && comb[i] != comb[j]) j = pi[j - 1];
        if (comb[i] == comb[j]) j++;
        pi[i] = j;
    }

    int L = pi[m - 1];
    size_t rem = n - (size_t)L;

    // Result: reverse of s[L..n-1] + s
    char* out = (char*)malloc(rem + n + 1);
    if (!out) { free(rev); free(comb); free(pi); return NULL; }
    size_t idx = 0;
    for (size_t k = 0; k < rem; ++k) {
        out[idx++] = s[n - 1 - k];
    }
    memcpy(out + idx, s, n);
    out[rem + n] = '\0';

    free(rev);
    free(comb);
    free(pi);
    return out;
}

int main(void) {
    const char* tests[5] = {
        "aacecaaa",
        "abcd",
        "",
        "a",
        "aaab"
    };
    for (int i = 0; i < 5; ++i) {
        char* res = shortestPalindrome(tests[i]);
        if (res) {
            printf("Input: \"%s\" -> Output: \"%s\"\n", tests[i], res);
            free(res);
        } else {
            printf("Input: \"%s\" -> Output: (null)\n", tests[i]);
        }
    }
    return 0;
}