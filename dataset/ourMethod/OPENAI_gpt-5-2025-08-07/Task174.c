#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static bool is_lowercase_string(const char* s) {
    if (s == NULL) return false;
    for (const char* p = s; *p; ++p) {
        if (*p < 'a' || *p > 'z') return false;
    }
    return true;
}

static int* build_lps(const char* str, size_t m) {
    int* lps = (int*)malloc(sizeof(int) * m);
    if (!lps) return NULL;
    lps[0] = 0;
    int len = 0;
    for (size_t i = 1; i < m; ++i) {
        while (len > 0 && str[i] != str[(size_t)len]) {
            len = lps[(size_t)len - 1];
        }
        if (str[i] == str[(size_t)len]) {
            len++;
        }
        lps[i] = len;
    }
    return lps;
}

char* shortest_palindrome(const char* s) {
    if (s == NULL) {
        char* out = (char*)malloc(1);
        if (out) out[0] = '\0';
        return out;
    }
    size_t n = strlen(s);
    if (n > 50000 || !is_lowercase_string(s)) {
        char* out = (char*)malloc(1);
        if (out) out[0] = '\0';
        return out;
    }
    if (n <= 1) {
        char* out = (char*)malloc(n + 1);
        if (!out) return NULL;
        memcpy(out, s, n + 1);
        return out;
    }
    size_t M = n + 1 + n;
    char* combined = (char*)malloc(M + 1);
    if (!combined) return NULL;
    memcpy(combined, s, n);
    combined[n] = '#';
    for (size_t i = 0; i < n; ++i) {
        combined[n + 1 + i] = s[n - 1 - i];
    }
    combined[M] = '\0';

    int* lps = build_lps(combined, M);
    if (!lps) {
        free(combined);
        return NULL;
    }
    int l = lps[M - 1];
    free(lps);
    size_t suffix_len = n - (size_t)l;

    char* result = (char*)malloc(n + suffix_len + 1);
    if (!result) {
        free(combined);
        return NULL;
    }
    // Add reverse of suffix s[l..n-1] to the front
    for (size_t i = 0; i < suffix_len; ++i) {
        result[i] = s[n - 1 - i];
    }
    memcpy(result + suffix_len, s, n);
    result[n + suffix_len] = '\0';

    free(combined);
    return result;
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
        char* res = shortest_palindrome(tests[i]);
        if (!res) {
            printf("Allocation error\n");
            continue;
        }
        printf("Input: \"%s\" -> Output: \"%s\"\n", tests[i], res);
        free(res);
    }
    return 0;
}