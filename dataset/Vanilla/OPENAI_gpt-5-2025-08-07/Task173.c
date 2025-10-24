#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static const long long MOD1_C = 1000000007LL;
static const long long MOD2_C = 1000000009LL;
static const long long BASE1_C = 911382323LL % 1000000007LL;
static const long long BASE2_C = 972663749LL % 1000000009LL;

typedef struct {
    unsigned long long* keys;
    int* vals;
    unsigned char* used;
    int cap;
    int mask;
} HashMap64;

static unsigned int next_pow2(unsigned int x) {
    if (x <= 1) return 1;
    --x;
    x |= x >> 1; x |= x >> 2; x |= x >> 4; x |= x >> 8; x |= x >> 16;
    return x + 1;
}

static HashMap64* hm_create(int need) {
    HashMap64* hm = (HashMap64*)malloc(sizeof(HashMap64));
    hm->cap = (int)next_pow2((unsigned int)need);
    hm->mask = hm->cap - 1;
    hm->keys = (unsigned long long*)calloc(hm->cap, sizeof(unsigned long long));
    hm->vals = (int*)malloc(hm->cap * sizeof(int));
    hm->used = (unsigned char*)calloc(hm->cap, 1);
    return hm;
}

static void hm_free(HashMap64* hm) {
    if (!hm) return;
    free(hm->keys);
    free(hm->vals);
    free(hm->used);
    free(hm);
}

static int hm_find_prev(HashMap64* hm, unsigned long long key, int* out_idx) {
    unsigned long long k = key;
    // simple mixing
    unsigned int idx = (unsigned int)((k ^ (k >> 33) ^ (k >> 17)) & (unsigned long long)hm->mask);
    while (hm->used[idx]) {
        if (hm->keys[idx] == key) {
            if (out_idx) *out_idx = idx;
            return 1;
        }
        idx = (idx + 1) & hm->mask;
    }
    if (out_idx) *out_idx = idx;
    return 0;
}

static void hm_insert(HashMap64* hm, unsigned long long key, int val) {
    int idx;
    if (!hm_find_prev(hm, key, &idx)) {
        hm->used[idx] = 1;
        hm->keys[idx] = key;
        hm->vals[idx] = val;
    } else {
        // already exists, leave existing value
    }
}

static long long get_hash_ll(const long long* pref, const long long* powv, long long mod, int i, int L) {
    long long res = (pref[i + L] - (pref[i] * powv[L]) % mod + mod) % mod;
    return res;
}

static int check_c(int L, int n, const long long* pref1, const long long* pref2, const long long* pow1, const long long* pow2) {
    if (L == 0) return 0;
    int count = n - L + 1;
    int cap_need = count * 2 + 1;
    HashMap64* hm = hm_create(cap_need);
    int idx_prev;
    for (int i = 0; i + L <= n; ++i) {
        long long h1 = get_hash_ll(pref1, pow1, MOD1_C, i, L);
        long long h2 = get_hash_ll(pref2, pow2, MOD2_C, i, L);
        unsigned long long key = (((unsigned long long)h1) << 32) ^ (unsigned long long)h2;
        if (hm_find_prev(hm, key, &idx_prev)) {
            hm_free(hm);
            return i;
        } else {
            hm_insert(hm, key, i);
        }
    }
    hm_free(hm);
    return -1;
}

char* longestDupSubstring(const char* s) {
    int n = (int)strlen(s);
    if (n <= 1) {
        char* out = (char*)malloc(1);
        out[0] = '\0';
        return out;
    }
    int* vals = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; ++i) vals[i] = (s[i] - 'a' + 1);

    long long* pow1 = (long long*)malloc((n + 1) * sizeof(long long));
    long long* pow2 = (long long*)malloc((n + 1) * sizeof(long long));
    long long* pref1 = (long long*)malloc((n + 1) * sizeof(long long));
    long long* pref2 = (long long*)malloc((n + 1) * sizeof(long long));
    pow1[0] = 1; pow2[0] = 1;
    pref1[0] = 0; pref2[0] = 0;

    for (int i = 0; i < n; ++i) {
        pow1[i + 1] = (pow1[i] * BASE1_C) % MOD1_C;
        pow2[i + 1] = (pow2[i] * BASE2_C) % MOD2_C;
        pref1[i + 1] = (pref1[i] * BASE1_C + vals[i]) % MOD1_C;
        pref2[i + 1] = (pref2[i] * BASE2_C + vals[i]) % MOD2_C;
    }

    int lo = 1, hi = n - 1;
    int bestLen = 0, bestIdx = -1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        int idx = check_c(mid, n, pref1, pref2, pow1, pow2);
        if (idx != -1) {
            bestLen = mid;
            bestIdx = idx;
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }

    char* out;
    if (bestIdx == -1) {
        out = (char*)malloc(1);
        out[0] = '\0';
    } else {
        out = (char*)malloc((size_t)bestLen + 1);
        memcpy(out, s + bestIdx, (size_t)bestLen);
        out[bestLen] = '\0';
    }

    free(vals);
    free(pow1); free(pow2);
    free(pref1); free(pref2);
    return out;
}

int main() {
    const char* tests[5] = {"banana", "abcd", "aaaaa", "abcabc", "mississippi"};
    for (int i = 0; i < 5; ++i) {
        char* res = longestDupSubstring(tests[i]);
        printf("Input: %s\n", tests[i]);
        printf("Output: %s\n\n", res);
        free(res);
    }
    return 0;
}