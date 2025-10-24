#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/* Secure mix function for hashing */
static inline uint64_t mix64(uint64_t x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    x ^= x >> 31;
    return x;
}

/* HashSet for triples (h1, h2, len) with open addressing */
typedef struct {
    uint64_t h1;
    uint64_t h2;
    int len;
    int occupied; /* 0 empty, 1 occupied */
} SetEntry;

typedef struct {
    SetEntry* entries;
    size_t cap;
    size_t size;
} HashSet;

static uint64_t key_hash(uint64_t h1, uint64_t h2, int len) {
    uint64_t lmix = mix64((uint64_t)len ^ ((uint64_t)len << 32));
    uint64_t x = h1 ^ (h2 + 0x9e3779b97f4a7c15ULL) ^ (lmix << 1);
    return mix64(x);
}

static size_t next_pow2(size_t x) {
    if (x < 8) return 8;
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
#if SIZE_MAX > UINT32_MAX
    x |= x >> 32;
#endif
    x++;
    return x;
}

static int set_init(HashSet* set, size_t init_cap) {
    if (!set) return -1;
    set->cap = next_pow2(init_cap);
    size_t bytes;
    if (__builtin_mul_overflow(set->cap, sizeof(SetEntry), &bytes)) {
        return -1;
    }
    set->entries = (SetEntry*)calloc(set->cap, sizeof(SetEntry));
    if (!set->entries) return -1;
    set->size = 0;
    return 0;
}

static void set_free(HashSet* set) {
    if (!set) return;
    free(set->entries);
    set->entries = NULL;
    set->cap = 0;
    set->size = 0;
}

static int set_rehash(HashSet* set, size_t new_cap) {
    HashSet tmp;
    if (set_init(&tmp, new_cap) != 0) return -1;
    for (size_t i = 0; i < set->cap; ++i) {
        if (set->entries[i].occupied) {
            SetEntry e = set->entries[i];
            uint64_t h = key_hash(e.h1, e.h2, e.len);
            size_t idx = (size_t)(h & (tmp.cap - 1));
            while (tmp.entries[idx].occupied) {
                idx = (idx + 1) & (tmp.cap - 1);
            }
            tmp.entries[idx] = e;
            tmp.entries[idx].occupied = 1;
            tmp.size++;
        }
    }
    free(set->entries);
    set->entries = tmp.entries;
    set->cap = tmp.cap;
    set->size = tmp.size;
    return 0;
}

/* returns 1 if inserted new, 0 if already existed, -1 on error */
static int set_insert(HashSet* set, uint64_t h1, uint64_t h2, int len) {
    if (!set || !set->entries) return -1;
    if ((set->size + 1) * 10 > set->cap * 7) { /* load factor > 0.7 */
        if (set_rehash(set, set->cap << 1) != 0) return -1;
    }
    uint64_t h = key_hash(h1, h2, len);
    size_t idx = (size_t)(h & (set->cap - 1));
    while (set->entries[idx].occupied) {
        if (set->entries[idx].h1 == h1 &&
            set->entries[idx].h2 == h2 &&
            set->entries[idx].len == len) {
            return 0; /* already present */
        }
        idx = (idx + 1) & (set->cap - 1);
    }
    set->entries[idx].h1 = h1;
    set->entries[idx].h2 = h2;
    set->entries[idx].len = len;
    set->entries[idx].occupied = 1;
    set->size++;
    return 1;
}

/* Rolling hash utilities */
typedef struct {
    int n;
    uint64_t mod;
    uint64_t base;
    uint64_t* powv;
    uint64_t* pref;
} RH;

static int rh_init(RH* rh, const char* s, int n, uint64_t mod, uint64_t base) {
    if (!rh || !s || n < 0) return -1;
    rh->n = n;
    rh->mod = mod;
    rh->base = base;
    rh->powv = (uint64_t*)malloc((size_t)(n + 1) * sizeof(uint64_t));
    rh->pref = (uint64_t*)malloc((size_t)(n + 1) * sizeof(uint64_t));
    if (!rh->powv || !rh->pref) {
        free(rh->powv); rh->powv = NULL;
        free(rh->pref); rh->pref = NULL;
        return -1;
    }
    rh->powv[0] = 1ULL % mod;
    for (int i = 1; i <= n; ++i) {
        __int128 t = (__int128)rh->powv[i - 1] * (__int128)base;
        rh->powv[i] = (uint64_t)(t % mod);
    }
    rh->pref[0] = 0;
    for (int i = 0; i < n; ++i) {
        uint64_t v = (uint64_t)(unsigned char)(s[i] - 'a' + 1);
        __int128 t = ((__int128)rh->pref[i] * (__int128)base + v);
        rh->pref[i + 1] = (uint64_t)(t % mod);
    }
    return 0;
}

static void rh_free(RH* rh) {
    if (!rh) return;
    free(rh->powv); rh->powv = NULL;
    free(rh->pref); rh->pref = NULL;
    rh->n = 0;
}

static inline uint64_t rh_sub(const RH* rh, int l, int r) {
    /* returns hash of s[l..r], inclusive */
    uint64_t mod = rh->mod;
    __int128 mul = (__int128)rh->pref[l] * (__int128)rh->powv[r - l + 1];
    uint64_t sub = (uint64_t)(mul % mod);
    uint64_t val = rh->pref[r + 1];
    uint64_t res = (val + mod - sub);
    if (res >= mod) res -= mod;
    return res;
}

/* Main function: count distinct substrings that are a + a */
int countDistinctSquareSubstrings(const char* text) {
    if (text == NULL) return -1;
    size_t slen = strlen(text);
    if (slen == 0 || slen > 2000) return -1;
    int n = (int)slen;

    /* Validate lowercase letters */
    for (int i = 0; i < n; ++i) {
        char c = text[i];
        if (c < 'a' || c > 'z') {
            return -1;
        }
    }

    const uint64_t MOD1 = 1000000007ULL;
    const uint64_t MOD2 = 1000000009ULL;
    const uint64_t BASE1 = 911382323ULL % MOD1;
    const uint64_t BASE2 = 972663749ULL % MOD2;

    RH rh1, rh2;
    if (rh_init(&rh1, text, n, MOD1, BASE1) != 0) {
        return -1;
    }
    if (rh_init(&rh2, text, n, MOD2, BASE2) != 0) {
        rh_free(&rh1);
        return -1;
    }

    HashSet set;
    if (set_init(&set, 1024) != 0) {
        rh_free(&rh1);
        rh_free(&rh2);
        return -1;
    }

    for (int i = 0; i < n; ++i) {
        int maxEvenLen = n - i;
        if ((maxEvenLen & 1) == 1) maxEvenLen--; /* ensure even */
        for (int len = 2; len <= maxEvenLen; len += 2) {
            int k = len >> 1;
            int l1 = i, r1 = i + k - 1;
            int l2 = i + k, r2 = i + len - 1;

            uint64_t a1 = rh_sub(&rh1, l1, r1);
            uint64_t b1 = rh_sub(&rh1, l2, r2);
            if (a1 != b1) continue;
            uint64_t a2 = rh_sub(&rh2, l1, r1);
            uint64_t b2 = rh_sub(&rh2, l2, r2);
            if (a2 != b2) continue;

            /* Full substring hash */
            uint64_t hfull1 = rh_sub(&rh1, i, i + len - 1);
            uint64_t hfull2 = rh_sub(&rh2, i, i + len - 1);
            int ins = set_insert(&set, hfull1, hfull2, len);
            if (ins < 0) {
                set_free(&set);
                rh_free(&rh1);
                rh_free(&rh2);
                return -1;
            }
        }
    }

    int result = (int)set.size;

    set_free(&set);
    rh_free(&rh1);
    rh_free(&rh2);
    return result;
}

int main(void) {
    const char* tests[5] = {
        "abcabcabc",          /* expected 3: "abcabc","bcabca","cabcab" */
        "leetcodeleetcode",   /* expected 2: "ee","leetcodeleetcode" */
        "aaaaa",              /* expected 2: "aa","aaaa" */
        "ababab",             /* expected 2: "abab","baba" */
        "ee"                  /* expected 1: "ee" */
    };

    for (int i = 0; i < 5; ++i) {
        int res = countDistinctSquareSubstrings(tests[i]);
        if (res < 0) {
            printf("Test %d: Invalid input or error\n", i + 1);
        } else {
            printf("Test %d: \"%s\" -> %d\n", i + 1, tests[i], res);
        }
    }
    return 0;
}