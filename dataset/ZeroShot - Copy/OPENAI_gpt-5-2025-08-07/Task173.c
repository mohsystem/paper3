#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void counting_sort(const int* sa_in, const int* key, int n, int max_key, int* sa_out) {
    int* cnt = (int*)calloc((size_t)max_key + 1, sizeof(int));
    if (!cnt) exit(1);
    for (int i = 0; i < n; i++) cnt[key[sa_in[i]]]++;
    int sum = 0;
    for (int i = 0; i <= max_key; i++) {
        int t = cnt[i];
        cnt[i] = sum;
        sum += t;
    }
    for (int i = 0; i < n; i++) {
        int idx = sa_in[i];
        int k = key[idx];
        sa_out[cnt[k]++] = idx;
    }
    free(cnt);
}

static int* build_suffix_array(const char* s, int n) {
    int* sa = (int*)malloc(sizeof(int) * n);
    int* rnk = (int*)malloc(sizeof(int) * n);
    int* tmp = (int*)malloc(sizeof(int) * n);
    int* key1 = (int*)malloc(sizeof(int) * n);
    int* key2 = (int*)malloc(sizeof(int) * n);
    int* out = (int*)malloc(sizeof(int) * n);
    if (!sa || !rnk || !tmp || !key1 || !key2 || !out) exit(1);

    for (int i = 0; i < n; i++) {
        sa[i] = i;
        rnk[i] = s[i] - 'a';
    }

    for (int k = 1; k < n; k <<= 1) {
        int maxRank = 0;
        for (int i = 0; i < n; i++) if (rnk[i] > maxRank) maxRank = rnk[i];

        for (int i = 0; i < n; i++) key2[i] = (i + k < n ? rnk[i + k] + 1 : 0);
        counting_sort(sa, key2, n, maxRank + 2, out);

        for (int i = 0; i < n; i++) key1[i] = rnk[i] + 1;
        counting_sort(out, key1, n, maxRank + 2, sa);

        tmp[sa[0]] = 0;
        int classes = 1;
        for (int i = 1; i < n; i++) {
            int cur = sa[i], prev = sa[i - 1];
            if (rnk[cur] != rnk[prev] || key2[cur] != key2[prev]) classes++;
            tmp[cur] = classes - 1;
        }
        int* swap = rnk; rnk = tmp; tmp = swap;
        if (classes == n) break;
    }

    free(rnk);
    free(tmp);
    free(key1);
    free(key2);
    free(out);
    return sa;
}

static int* build_lcp(const char* s, const int* sa, int n) {
    int* rank = (int*)malloc(sizeof(int) * n);
    int* lcp = (int*)malloc(sizeof(int) * (n > 0 ? n - 1 : 0));
    if (!rank || (!lcp && n > 0)) exit(1);

    for (int i = 0; i < n; i++) rank[sa[i]] = i;
    int k = 0;
    for (int i = 0; i < n; i++) {
        int r = rank[i];
        if (r == n - 1) {
            k = 0;
            continue;
        }
        int j = sa[r + 1];
        while (i + k < n && j + k < n && s[i + k] == s[j + k]) k++;
        lcp[r] = k;
        if (k > 0) k--;
    }
    free(rank);
    return lcp;
}

char* longestDupSubstring(const char* s) {
    if (s == NULL) {
        char* empty = (char*)malloc(1);
        if (!empty) exit(1);
        empty[0] = '\0';
        return empty;
    }
    int n = (int)strlen(s);
    if (n < 2) {
        char* empty = (char*)malloc(1);
        if (!empty) exit(1);
        empty[0] = '\0';
        return empty;
    }
    int* sa = build_suffix_array(s, n);
    int* lcp = build_lcp(s, sa, n);

    int maxL = 0, pos = -1;
    for (int i = 0; i < n - 1; i++) {
        if (lcp[i] > maxL) {
            maxL = lcp[i];
            pos = sa[i];
        }
    }
    free(sa);
    free(lcp);

    if (maxL == 0) {
        char* empty = (char*)malloc(1);
        if (!empty) exit(1);
        empty[0] = '\0';
        return empty;
    }
    char* res = (char*)malloc((size_t)maxL + 1);
    if (!res) exit(1);
    memcpy(res, s + pos, (size_t)maxL);
    res[maxL] = '\0';
    return res;
}

int main(void) {
    const char* tests[5] = {
        "banana",
        "abcd",
        "aaaaa",
        "abcabca",
        "mississippi"
    };
    for (int i = 0; i < 5; i++) {
        char* out = longestDupSubstring(tests[i]);
        printf("Input: %s -> Output: %s\n", tests[i], out);
        free(out);
    }
    return 0;
}