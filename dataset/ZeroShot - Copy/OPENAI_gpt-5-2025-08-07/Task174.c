#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void build_lps(const char* str, size_t len, size_t* lps) {
    size_t length = 0;
    lps[0] = 0;
    for (size_t i = 1; i < len; ++i) {
        while (length > 0 && str[i] != str[length]) {
            length = lps[length - 1];
        }
        if (str[i] == str[length]) {
            length++;
        }
        lps[i] = length;
    }
}

char* Task174_shortest_palindrome(const char* s) {
    if (s == NULL) {
        char* empty = (char*)calloc(1, 1);
        return empty;
    }
    size_t n = strlen(s);
    if (n <= 1) {
        char* out = (char*)malloc(n + 1);
        if (!out) return NULL;
        if (n == 1) out[0] = s[0];
        out[n] = '\0';
        return out;
    }
    char* rev = (char*)malloc(n + 1);
    if (!rev) return NULL;
    for (size_t i = 0; i < n; ++i) {
        rev[i] = s[n - 1 - i];
    }
    rev[n] = '\0';

    size_t combined_len = n + 1 + n;
    char* combined = (char*)malloc(combined_len);
    if (!combined) {
        free(rev);
        return NULL;
    }
    memcpy(combined, s, n);
    combined[n] = '#';
    memcpy(combined + n + 1, rev, n);

    size_t* lps = (size_t*)malloc(sizeof(size_t) * combined_len);
    if (!lps) {
        free(rev);
        free(combined);
        return NULL;
    }
    build_lps(combined, combined_len, lps);
    size_t prefix_len = lps[combined_len - 1];
    size_t suffix_len = n - prefix_len;

    size_t res_len = n + suffix_len;
    char* res = (char*)malloc(res_len + 1);
    if (!res) {
        free(rev);
        free(combined);
        free(lps);
        return NULL;
    }
    // reversed suffix is the first suffix_len characters of rev
    if (suffix_len > 0) {
        memcpy(res, rev, suffix_len);
    }
    memcpy(res + suffix_len, s, n);
    res[res_len] = '\0';

    free(rev);
    free(combined);
    free(lps);
    return res;
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
        char* ans = Task174_shortest_palindrome(tests[i]);
        if (ans) {
            printf("%s\n", ans);
            free(ans);
        } else {
            printf("\n");
        }
    }
    return 0;
}