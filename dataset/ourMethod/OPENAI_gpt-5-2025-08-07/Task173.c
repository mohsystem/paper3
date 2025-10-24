#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int is_valid_input(const char *s) {
    if (s == NULL) return 0;
    size_t n = strlen(s);
    if (n < 2 || n > 30000) return 0;
    for (size_t i = 0; i < n; ++i) {
        if (s[i] < 'a' || s[i] > 'z') return 0;
    }
    return 1;
}

typedef struct {
    int idx;
    int r0;
    int r1;
} Suffix;

static int cmp_suffix(const void *a, const void *b) {
    const Suffix *sa = (const Suffix*)a;
    const Suffix *sb = (const Suffix*)b;
    if (sa->r0 != sb->r0) return (sa->r0 < sb->r0) ? -1 : 1;
    if (sa->r1 != sb->r1) return (sa->r1 < sb->r1) ? -1 : 1;
    return 0;
}

static int* build_suffix_array(const char *s, int n) {
    int *rank = (int*)malloc(sizeof(int) * n);
    int *newRank = (int*)malloc(sizeof(int) * n);
    Suffix *suffixes = (Suffix*)malloc(sizeof(Suffix) * n);
    if (!rank || !newRank || !suffixes) {
        free(rank); free(newRank); free(suffixes);
        return NULL;
    }

    for (int i = 0; i < n; ++i) {
        rank[i] = s[i] - 'a';
    }

    for (int k = 1; k < n; k <<= 1) {
        for (int i = 0; i < n; ++i) {
            suffixes[i].idx = i;
            suffixes[i].r0 = rank[i];
            suffixes[i].r1 = (i + k < n) ? rank[i + k] : -1;
        }
        qsort(suffixes, n, sizeof(Suffix), cmp_suffix);

        newRank[suffixes[0].idx] = 0;
        for (int i = 1; i < n; ++i) {
            int diff = (suffixes[i].r0 != suffixes[i-1].r0) || (suffixes[i].r1 != suffixes[i-1].r1);
            newRank[suffixes[i].idx] = newRank[suffixes[i-1].idx] + (diff ? 1 : 0);
        }
        for (int i = 0; i < n; ++i) rank[i] = newRank[i];
        if (newRank[suffixes[n-1].idx] == n - 1) break;
    }

    int *sa = (int*)malloc(sizeof(int) * n);
    if (!sa) {
        free(rank); free(newRank); free(suffixes);
        return NULL;
    }
    for (int i = 0; i < n; ++i) sa[i] = suffixes[i].idx;

    free(rank);
    free(newRank);
    free(suffixes);
    return sa;
}

static int* build_lcp(const char *s, int n, const int *sa) {
    int *rank = (int*)malloc(sizeof(int) * n);
    if (!rank) return NULL;
    for (int i = 0; i < n; ++i) rank[sa[i]] = i;
    int *lcp = (int*)malloc(sizeof(int) * (n > 0 ? n - 1 : 0));
    if (n > 0 && !lcp) { free(rank); return NULL; }

    int h = 0;
    for (int i = 0; i < n; ++i) {
        int r = rank[i];
        if (r == n - 1) { h = 0; continue; }
        int j = sa[r + 1];
        while (i + h < n && j + h < n && s[i + h] == s[j + h]) h++;
        lcp[r] = h;
        if (h > 0) h--;
    }
    free(rank);
    return lcp;
}

char* longestDupSubstring(const char *s) {
    if (!is_valid_input(s)) {
        char *empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    int n = (int)strlen(s);
    int *sa = build_suffix_array(s, n);
    if (!sa) {
        char *empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    int *lcp = build_lcp(s, n, sa);
    if (n == 0) {
        free(sa);
        char *empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    int bestLen = 0, bestPos = -1;
    if (n > 1) {
        for (int i = 0; i < n - 1; ++i) {
            if (lcp[i] > bestLen) {
                bestLen = lcp[i];
                bestPos = sa[i];
            }
        }
    }
    char *res;
    if (bestLen <= 0) {
        res = (char*)malloc(1);
        if (res) res[0] = '\0';
    } else {
        res = (char*)malloc((size_t)bestLen + 1);
        if (res) {
            memcpy(res, s + bestPos, (size_t)bestLen);
            res[bestLen] = '\0';
        }
    }

    free(sa);
    if (lcp) free(lcp);
    return res;
}

int main(void) {
    const char *tests[] = {
        "banana",
        "abcd",
        "aaaaa",
        "abcabcabc",
        "mississippi"
    };
    size_t T = sizeof(tests) / sizeof(tests[0]);
    for (size_t i = 0; i < T; ++i) {
        char *res = longestDupSubstring(tests[i]);
        if (res) {
            printf("Input: %s -> Output: %s\n", tests[i], res);
            free(res);
        } else {
            printf("Input: %s -> Output: \n", tests[i]);
        }
    }
    return 0;
}