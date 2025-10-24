#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* str_dup(const char* s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char* r = (char*)malloc(n + 1);
    if (!r) return NULL;
    memcpy(r, s, n + 1);
    return r;
}

char* shortestPalindrome(const char* s) {
    if (s == NULL) return str_dup("");
    size_t n = strlen(s);
    if (n <= 1) return str_dup(s);

    char* rev = (char*)malloc(n + 1);
    if (!rev) return NULL;
    for (size_t i = 0; i < n; ++i) rev[i] = s[n - 1 - i];
    rev[n] = '\0';

    size_t m = 2 * n + 1;
    char* pat = (char*)malloc(m + 1);
    if (!pat) { free(rev); return NULL; }
    memcpy(pat, s, n);
    pat[n] = '#';
    memcpy(pat + n + 1, rev, n);
    pat[m] = '\0';

    int* lps = (int*)malloc(sizeof(int) * m);
    if (!lps) { free(rev); free(pat); return NULL; }
    lps[0] = 0;
    int len = 0;
    size_t i = 1;
    while (i < m) {
        if (pat[i] == pat[len]) {
            len++;
            lps[i] = len;
            i++;
        } else if (len != 0) {
            len = lps[len - 1];
        } else {
            lps[i] = 0;
            i++;
        }
    }

    int palLen = lps[m - 1];
    size_t resLen = (n - palLen) + n;
    char* res = (char*)malloc(resLen + 1);
    if (!res) { free(rev); free(pat); free(lps); return NULL; }

    size_t idx = 0;
    for (int k = (int)n - 1; k >= palLen; --k) {
        res[idx++] = s[k];
    }
    memcpy(res + idx, s, n);
    res[resLen] = '\0';

    free(rev);
    free(pat);
    free(lps);
    return res;
}

int main() {
    const char* tests[5] = {
        "aacecaaa",
        "abcd",
        "",
        "a",
        "abbacd"
    };
    for (int t = 0; t < 5; ++t) {
        char* ans = shortestPalindrome(tests[t]);
        if (ans) {
            printf("%s\n", ans);
            free(ans);
        } else {
            printf("\n");
        }
    }
    return 0;
}