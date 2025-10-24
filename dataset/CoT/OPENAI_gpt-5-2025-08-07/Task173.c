#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Suffix array + Kasai LCP to find longest duplicated substring in C (no external hash maps)

static const char* g_s;
static int* g_rank_arr;
static int g_n;
static int g_k;

static int cmp_sa(const void* a, const void* b) {
    int i = *(const int*)a;
    int j = *(const int*)b;
    if (g_rank_arr[i] != g_rank_arr[j]) return (g_rank_arr[i] < g_rank_arr[j]) ? -1 : 1;
    int ri = (i + g_k < g_n) ? g_rank_arr[i + g_k] : -1;
    int rj = (j + g_k < g_n) ? g_rank_arr[j + g_k] : -1;
    if (ri != rj) return (ri < rj) ? -1 : 1;
    return 0;
}

// Returns newly allocated longest duplicated substring (caller must free), or empty string if none.
char* longestDupSubstring(const char* s) {
    if (!s) {
        char* emp = (char*)calloc(1, 1);
        return emp;
    }
    int n = (int)strlen(s);
    if (n < 2) {
        char* emp = (char*)calloc(1, 1);
        return emp;
    }
    g_s = s;
    g_n = n;

    int* sa = (int*)malloc(n * sizeof(int));
    int* rank_arr = (int*)malloc(n * sizeof(int));
    int* tmp = (int*)malloc(n * sizeof(int));
    if (!sa || !rank_arr || !tmp) {
        free(sa); free(rank_arr); free(tmp);
        char* emp = (char*)calloc(1, 1);
        return emp;
    }
    for (int i = 0; i < n; ++i) {
        sa[i] = i;
        rank_arr[i] = (unsigned char)s[i];
    }

    g_rank_arr = rank_arr;
    for (g_k = 1; g_k < n; g_k <<= 1) {
        qsort(sa, n, sizeof(int), cmp_sa);
        tmp[sa[0]] = 0;
        for (int i = 1; i < n; ++i) {
            tmp[sa[i]] = tmp[sa[i-1]] + (cmp_sa(&sa[i-1], &sa[i]) < 0 ? 1 : 0);
        }
        for (int i = 0; i < n; ++i) g_rank_arr[i] = tmp[i];
        if (g_rank_arr[sa[n-1]] == n - 1) break;
    }

    int* rank_pos = (int*)malloc(n * sizeof(int));
    int* lcp = (int*)malloc(n * sizeof(int));
    if (!rank_pos || !lcp) {
        free(sa); free(rank_arr); free(tmp); free(rank_pos); free(lcp);
        char* emp = (char*)calloc(1, 1);
        return emp;
    }
    for (int i = 0; i < n; ++i) rank_pos[sa[i]] = i;

    int h = 0;
    for (int i = 0; i < n; ++i) {
        int r = rank_pos[i];
        if (r == n - 1) {
            lcp[r] = 0;
            h = 0;
            continue;
        }
        int j = sa[r + 1];
        while (i + h < n && j + h < n && s[i + h] == s[j + h]) h++;
        lcp[r] = h;
        if (h > 0) h--;
    }

    int bestLen = 0, bestStart = -1;
    for (int i = 0; i < n - 1; ++i) {
        if (lcp[i] > bestLen) {
            bestLen = lcp[i];
            bestStart = sa[i];
        }
    }

    char* result;
    if (bestLen <= 0) {
        result = (char*)calloc(1, 1); // empty string
    } else {
        result = (char*)malloc((size_t)bestLen + 1);
        if (!result) {
            result = (char*)calloc(1, 1);
        } else {
            memcpy(result, s + bestStart, (size_t)bestLen);
            result[bestLen] = '\0';
        }
    }

    free(sa);
    free(rank_arr);
    free(tmp);
    free(rank_pos);
    free(lcp);
    return result;
}

int main() {
    const char* tests[5] = {
        "banana",
        "abcd",
        "aaaaa",
        "abcdabc",
        "mississippi"
    };
    for (int i = 0; i < 5; ++i) {
        char* res = longestDupSubstring(tests[i]);
        printf("Input: %s -> Output: %s\n", tests[i], res);
        free(res);
    }
    return 0;
}