#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MOD1 1000000007U
#define MOD2 1000000009U
#define BASE1 911382323U
#define BASE2 972663749U

typedef struct {
    uint64_t* keys;
    unsigned char* used;
    size_t cap;
    size_t size;
} U64Set;

static uint64_t mix64(uint64_t z) {
    z = (z ^ (z >> 33)) * 0xff51afd7ed558ccdULL;
    z = (z ^ (z >> 33)) * 0xc4ceb9fe1a85ec53ULL;
    z = z ^ (z >> 33);
    return z;
}

static U64Set* set_create(size_t initial) {
    U64Set* s = (U64Set*)malloc(sizeof(U64Set));
    s->cap = 1;
    while (s->cap < initial) s->cap <<= 1;
    s->size = 0;
    s->keys = (uint64_t*)calloc(s->cap, sizeof(uint64_t));
    s->used = (unsigned char*)calloc(s->cap, sizeof(unsigned char));
    return s;
}

static void set_free(U64Set* s) {
    if (!s) return;
    free(s->keys);
    free(s->used);
    free(s);
}

static void set_rehash(U64Set* s) {
    size_t oldcap = s->cap;
    uint64_t* oldkeys = s->keys;
    unsigned char* oldused = s->used;

    s->cap <<= 1;
    s->keys = (uint64_t*)calloc(s->cap, sizeof(uint64_t));
    s->used = (unsigned char*)calloc(s->cap, sizeof(unsigned char));
    s->size = 0;

    for (size_t i = 0; i < oldcap; ++i) {
        if (oldused[i]) {
            uint64_t k = oldkeys[i];
            uint64_t h = mix64(k);
            size_t idx = (size_t)(h & (s->cap - 1));
            while (s->used[idx]) {
                idx = (idx + 1) & (s->cap - 1);
            }
            s->keys[idx] = k;
            s->used[idx] = 1;
            s->size++;
        }
    }
    free(oldkeys);
    free(oldused);
}

static int set_insert(U64Set* s, uint64_t key) {
    if ((s->size * 10) >= (s->cap * 7)) {
        set_rehash(s);
    }
    uint64_t h = mix64(key);
    size_t idx = (size_t)(h & (s->cap - 1));
    while (s->used[idx]) {
        if (s->keys[idx] == key) return 0; // already exists
        idx = (idx + 1) & (s->cap - 1);
    }
    s->keys[idx] = key;
    s->used[idx] = 1;
    s->size++;
    return 1; // inserted
}

static inline uint32_t sub_hash(uint32_t* pref, uint32_t* pow, uint32_t mod, int l, int r) {
    uint64_t val = pref[r + 1];
    uint64_t sub = ((uint64_t)pref[l] * pow[r - l + 1]) % mod;
    uint64_t res = val + mod - sub;
    if (res >= mod) res -= mod;
    return (uint32_t)res;
}

int count_square_distinct_substrings(const char* s) {
    int n = (int)strlen(s);
    uint32_t* pow1 = (uint32_t*)malloc((n + 1) * sizeof(uint32_t));
    uint32_t* pow2 = (uint32_t*)malloc((n + 1) * sizeof(uint32_t));
    uint32_t* pref1 = (uint32_t*)malloc((n + 1) * sizeof(uint32_t));
    uint32_t* pref2 = (uint32_t*)malloc((n + 1) * sizeof(uint32_t));
    pow1[0] = 1U; pow2[0] = 1U;
    for (int i = 1; i <= n; ++i) {
        pow1[i] = (uint32_t)((uint64_t)pow1[i - 1] * BASE1 % MOD1);
        pow2[i] = (uint32_t)((uint64_t)pow2[i - 1] * BASE2 % MOD2);
    }
    pref1[0] = 0U; pref2[0] = 0U;
    for (int i = 0; i < n; ++i) {
        uint32_t v = (uint32_t)(s[i] - 'a' + 1);
        pref1[i + 1] = (uint32_t)(((uint64_t)pref1[i] * BASE1 + v) % MOD1);
        pref2[i + 1] = (uint32_t)(((uint64_t)pref2[i] * BASE2 + v) % MOD2);
    }

    U64Set* set = set_create((size_t)n * (size_t)n / 2 + 1);
    for (int l = 0; l < n; ++l) {
        for (int len = 2; l + len <= n; len += 2) {
            int mid = l + len / 2;
            int r = l + len - 1;
            uint32_t h1a = sub_hash(pref1, pow1, MOD1, l, mid - 1);
            uint32_t h1b = sub_hash(pref1, pow1, MOD1, mid, r);
            if (h1a != h1b) continue;
            uint32_t h2a = sub_hash(pref2, pow2, MOD2, l, mid - 1);
            uint32_t h2b = sub_hash(pref2, pow2, MOD2, mid, r);
            if (h2a != h2b) continue;

            uint32_t hs1 = sub_hash(pref1, pow1, MOD1, l, r);
            uint32_t hs2 = sub_hash(pref2, pow2, MOD2, l, r);
            uint64_t key = ((uint64_t)hs1 << 32) ^ (uint64_t)hs2;
            set_insert(set, key);
        }
    }
    int ans = (int)set->size;
    set_free(set);
    free(pow1); free(pow2); free(pref1); free(pref2);
    return ans;
}

int main() {
    const char* tests[5] = {
        "abcabcabc",
        "leetcodeleetcode",
        "aaaa",
        "abab",
        "xyz"
    };
    for (int i = 0; i < 5; ++i) {
        printf("%d\n", count_square_distinct_substrings(tests[i]));
    }
    return 0;
}