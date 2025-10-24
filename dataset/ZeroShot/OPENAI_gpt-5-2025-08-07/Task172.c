#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MOD1 1000000007U
#define MOD2 1000000009U
#define BASE1 911382323U
#define BASE2 972663749U

typedef struct {
    uint32_t h1;
    uint32_t h2;
    uint32_t len;
} Key;

static int key_cmp(const Key* a, const Key* b) {
    if (a->h1 != b->h1) return (a->h1 < b->h1) ? -1 : 1;
    if (a->h2 != b->h2) return (a->h2 < b->h2) ? -1 : 1;
    if (a->len != b->len) return (a->len < b->len) ? -1 : 1;
    return 0;
}

static int key_bsearch(const Key* arr, int n, const Key* target, int* insert_pos) {
    int lo = 0, hi = n - 1, mid, cmp;
    while (lo <= hi) {
        mid = lo + (hi - lo) / 2;
        cmp = key_cmp(&arr[mid], target);
        if (cmp == 0) {
            if (insert_pos) *insert_pos = mid;
            return 1;
        } else if (cmp < 0) {
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }
    if (insert_pos) *insert_pos = lo;
    return 0;
}

static inline uint32_t sub_hash(const uint64_t* pref, const uint64_t* powv, uint32_t mod, int l, int r) {
    uint64_t left = pref[r + 1];
    uint64_t right = (pref[l] * powv[r - l + 1]) % mod;
    uint64_t res = (left + mod - right) % mod;
    return (uint32_t)res;
}

int count_even_square_distinct(const char* text) {
    if (text == NULL) return 0;
    size_t ns = strlen(text);
    if (ns <= 1) return 0;
    if (ns > 10000000U) return 0; /* sanity check to avoid excessive allocations */

    int n = (int)ns;

    uint64_t* pref1 = (uint64_t*)calloc((size_t)n + 1, sizeof(uint64_t));
    uint64_t* pref2 = (uint64_t*)calloc((size_t)n + 1, sizeof(uint64_t));
    uint64_t* pow1  = (uint64_t*)calloc((size_t)n + 1, sizeof(uint64_t));
    uint64_t* pow2  = (uint64_t*)calloc((size_t)n + 1, sizeof(uint64_t));
    if (!pref1 || !pref2 || !pow1 || !pow2) {
        free(pref1); free(pref2); free(pow1); free(pow2);
        return 0;
    }

    pow1[0] = 1;
    pow2[0] = 1;
    for (int i = 0; i < n; ++i) {
        uint32_t v = (uint32_t)(text[i] - 'a' + 1);
        pref1[i + 1] = (pref1[i] * BASE1 + v) % MOD1;
        pref2[i + 1] = (pref2[i] * BASE2 + v) % MOD2;
        pow1[i + 1]  = (pow1[i]  * BASE1) % MOD1;
        pow2[i + 1]  = (pow2[i]  * BASE2) % MOD2;
    }

    Key* arr = NULL;
    int size = 0;
    int cap = 0;

    for (int len = 2; len <= n; len += 2) {
        int half = len / 2;
        for (int i = 0; i + len <= n; ++i) {
            int mid = i + half;
            int j = i + len - 1;

            uint32_t a1 = sub_hash(pref1, pow1, MOD1, i, mid - 1);
            uint32_t b1 = sub_hash(pref1, pow1, MOD1, mid, j);
            if (a1 != b1) continue;

            uint32_t a2 = sub_hash(pref2, pow2, MOD2, i, mid - 1);
            uint32_t b2 = sub_hash(pref2, pow2, MOD2, mid, j);
            if (a2 != b2) continue;

            Key k;
            k.h1 = sub_hash(pref1, pow1, MOD1, i, j);
            k.h2 = sub_hash(pref2, pow2, MOD2, i, j);
            k.len = (uint32_t)len;

            int pos = 0;
            if (key_bsearch(arr, size, &k, &pos)) {
                continue;
            }
            if (size == cap) {
                int new_cap = cap ? cap * 2 : 16;
                Key* new_arr = (Key*)realloc(arr, (size_t)new_cap * sizeof(Key));
                if (!new_arr) {
                    free(arr); arr = NULL;
                    free(pref1); free(pref2); free(pow1); free(pow2);
                    return 0;
                }
                arr = new_arr;
                cap = new_cap;
            }
            if (pos < size) {
                memmove(&arr[pos + 1], &arr[pos], (size_t)(size - pos) * sizeof(Key));
            }
            arr[pos] = k;
            size++;
        }
    }

    free(pref1); free(pref2); free(pow1); free(pow2);
    free(arr);
    return size;
}

int main(void) {
    const char* tests[5] = {
        "abcabcabc",
        "leetcodeleetcode",
        "aaaa",
        "ababa",
        "z"
    };
    for (int i = 0; i < 5; ++i) {
        int res = count_even_square_distinct(tests[i]);
        printf("%d\n", res);
    }
    return 0;
}