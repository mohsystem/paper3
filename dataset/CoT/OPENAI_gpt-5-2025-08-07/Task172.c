#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static const long long MOD1 = 1000000007LL;
static const long long MOD2 = 1000000009LL;
static const long long BASE1 = 911382323LL;
static const long long BASE2 = 972663749LL;

typedef struct {
    uint64_t h1;
    uint64_t h2;
    uint32_t len;
} Key;

typedef struct {
    long long *h1, *h2, *p1, *p2;
    int n;
} RollingHash;

static RollingHash build_hash(const char *s) {
    int n = (int)strlen(s);
    RollingHash rh;
    rh.n = n;
    rh.h1 = (long long*)calloc(n + 1, sizeof(long long));
    rh.h2 = (long long*)calloc(n + 1, sizeof(long long));
    rh.p1 = (long long*)calloc(n + 1, sizeof(long long));
    rh.p2 = (long long*)calloc(n + 1, sizeof(long long));
    if (!rh.h1 || !rh.h2 || !rh.p1 || !rh.p2) {
        // Allocation failure fallback to zeros
        free(rh.h1); free(rh.h2); free(rh.p1); free(rh.p2);
        rh.h1 = rh.h2 = rh.p1 = rh.p2 = NULL;
        rh.n = 0;
        return rh;
    }
    rh.p1[0] = 1;
    rh.p2[0] = 1;
    for (int i = 0; i < n; ++i) {
        int v = (int)(s[i] - 'a' + 1);
        rh.h1[i + 1] = ( (rh.h1[i] * BASE1) % MOD1 + v ) % MOD1;
        rh.h2[i + 1] = ( (rh.h2[i] * BASE2) % MOD2 + v ) % MOD2;
        rh.p1[i + 1] = (rh.p1[i] * BASE1) % MOD1;
        rh.p2[i + 1] = (rh.p2[i] * BASE2) % MOD2;
    }
    return rh;
}

static void get_hash(const RollingHash *rh, int l, int r, long long *out1, long long *out2) {
    long long x1 = (rh->h1[r] - (rh->h1[l] * rh->p1[r - l]) % MOD1);
    if (x1 < 0) x1 += MOD1;
    long long x2 = (rh->h2[r] - (rh->h2[l] * rh->p2[r - l]) % MOD2);
    if (x2 < 0) x2 += MOD2;
    *out1 = x1;
    *out2 = x2;
}

static int key_cmp(const void *a, const void *b) {
    const Key *ka = (const Key*)a;
    const Key *kb = (const Key*)b;
    if (ka->len != kb->len) return (ka->len < kb->len) ? -1 : 1;
    if (ka->h1 != kb->h1) return (ka->h1 < kb->h1) ? -1 : 1;
    if (ka->h2 != kb->h2) return (ka->h2 < kb->h2) ? -1 : 1;
    return 0;
}

int countSquareSubstrings(const char *text) {
    if (text == NULL) return 0;
    int n = (int)strlen(text);
    if (n <= 1) return 0;

    RollingHash rh = build_hash(text);
    if (rh.n == 0) return 0;

    // Dynamic array to collect keys
    size_t cap = 64;
    size_t sz = 0;
    Key *arr = (Key*)malloc(cap * sizeof(Key));
    if (!arr) {
        free(rh.h1); free(rh.h2); free(rh.p1); free(rh.p2);
        return 0;
    }

    for (int len = 2; len <= n; len += 2) {
        for (int i = 0; i + len <= n; ++i) {
            int mid = i + len / 2;
            long long a1, a2, b1, b2;
            get_hash(&rh, i, mid, &a1, &a2);
            get_hash(&rh, mid, i + len, &b1, &b2);
            if (a1 == b1 && a2 == b2) {
                long long f1, f2;
                get_hash(&rh, i, i + len, &f1, &f2);
                if (sz == cap) {
                    cap <<= 1;
                    Key *tmp = (Key*)realloc(arr, cap * sizeof(Key));
                    if (!tmp) { // allocation failure, cleanup
                        free(arr);
                        free(rh.h1); free(rh.h2); free(rh.p1); free(rh.p2);
                        return 0;
                    }
                    arr = tmp;
                }
                arr[sz].h1 = (uint64_t)f1;
                arr[sz].h2 = (uint64_t)f2;
                arr[sz].len = (uint32_t)len;
                sz++;
            }
        }
    }

    // Sort and unique
    qsort(arr, sz, sizeof(Key), key_cmp);
    int unique_count = 0;
    if (sz > 0) {
        unique_count = 1;
        for (size_t i = 1; i < sz; ++i) {
            if (key_cmp(&arr[i], &arr[i - 1]) != 0) unique_count++;
        }
    }

    free(arr);
    free(rh.h1); free(rh.h2); free(rh.p1); free(rh.p2);
    return unique_count;
}

int main() {
    const char *tests[5] = {
        "abcabcabc",
        "leetcodeleetcode",
        "aaaaaa",
        "abcd",
        "abbaabba"
    };
    for (int i = 0; i < 5; ++i) {
        printf("%d\n", countSquareSubstrings(tests[i]));
    }
    return 0;
}