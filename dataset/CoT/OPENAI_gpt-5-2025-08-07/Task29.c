/* Task29 - C Implementation
   Chain-of-Through process:
   1) Problem understanding: return indices of two items summing to target.
   2) Security requirements: avoid buffer overflows, check inputs, manage memory safely.
   3) Secure coding generation: use a simple open-addressing hash table to achieve O(n) expected time.
   4) Code review: careful with integer types, probe termination, and memory cleanup.
   5) Secure code output: final implementation with tests.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int i1;
    int i2;
    int found; /* 1 if found, else 0 */
} PairResult;

/* Simple hash map from long long key -> int value (index) with open addressing */
typedef struct {
    long long key;
    int value;
    int used; /* 0 = empty, 1 = occupied */
} Entry;

typedef struct {
    Entry* entries;
    size_t cap;
    size_t count;
} LLHashMap;

static size_t next_pow2(size_t x) {
    if (x < 2) return 2;
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
#if ULONG_MAX > 0xFFFFFFFFu
    x |= x >> 32;
#endif
    return x + 1;
}

static size_t hash_ll(long long k) {
    /* 64-bit mix then fold to size_t */
    unsigned long long x = (unsigned long long)k;
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
#if ULONG_MAX > 0xFFFFFFFFu
    return (size_t)x;
#else
    return (size_t)((x >> 32) ^ (x & 0xFFFFFFFFULL));
#endif
}

static LLHashMap* llmap_create(size_t hint) {
    LLHashMap* m = (LLHashMap*)calloc(1, sizeof(LLHashMap));
    if (!m) return NULL;
    m->cap = next_pow2(hint ? hint : 16);
    m->entries = (Entry*)calloc(m->cap, sizeof(Entry));
    if (!m->entries) {
        free(m);
        return NULL;
    }
    m->count = 0;
    return m;
}

static void llmap_free(LLHashMap* m) {
    if (!m) return;
    free(m->entries);
    free(m);
}

static int llmap_put(LLHashMap* m, long long key, int value) {
    if (!m) return 0;
    /* Load factor threshold ~0.7 -> simple resize by reallocating not implemented for simplicity since n is small in tests */
    size_t idx = hash_ll(key) & (m->cap - 1);
    size_t start = idx;
    while (m->entries[idx].used) {
        if (m->entries[idx].key == key) {
            m->entries[idx].value = value; /* overwrite */
            return 1;
        }
        idx = (idx + 1) & (m->cap - 1);
        if (idx == start) return 0; /* table full (shouldn't happen with chosen cap) */
    }
    m->entries[idx].used = 1;
    m->entries[idx].key = key;
    m->entries[idx].value = value;
    m->count++;
    return 1;
}

static int llmap_get(const LLHashMap* m, long long key, int* out_value) {
    if (!m) return 0;
    size_t idx = hash_ll(key) & (m->cap - 1);
    size_t start = idx;
    while (m->entries[idx].used) {
        if (m->entries[idx].key == key) {
            if (out_value) *out_value = m->entries[idx].value;
            return 1;
        }
        idx = (idx + 1) & (m->cap - 1);
        if (idx == start) break;
    }
    return 0;
}

/* Function: accepts input as parameters and returns output */
PairResult two_sum(const long long* numbers, size_t n, long long target) {
    PairResult res = {0, 0, 0};
    if (numbers == NULL || n < 2) {
        return res; /* not found */
    }
    /* capacity ~ 4*n to keep load low and avoid resizing */
    LLHashMap* map = llmap_create(n * 4);
    if (!map) {
        return res; /* allocation failure -> report not found */
    }
    for (int i = 0; i < (int)n; ++i) {
        long long v = numbers[i];
        long long comp = target - v;
        int idx = -1;
        if (llmap_get(map, comp, &idx)) {
            if (idx != i) {
                res.i1 = idx;
                res.i2 = i;
                res.found = 1;
                llmap_free(map);
                return res;
            }
        }
        /* store current value -> index */
        llmap_put(map, v, i);
    }
    llmap_free(map);
    return res;
}

static void print_pair(PairResult r) {
    if (r.found) {
        printf("[%d, %d]\n", r.i1, r.i2);
    } else {
        printf("No pair found\n");
    }
}

int main(void) {
    long long a1[] = {1, 2, 3};
    long long a2[] = {3, 2, 4};
    long long a3[] = {2, 7, 11, 15};
    long long a4[] = {0, 4, 3, 0};
    long long a5[] = {-3, 4, 3, 90};

    PairResult r1 = two_sum(a1, sizeof(a1)/sizeof(a1[0]), 4);   /* e.g., [0,2] */
    PairResult r2 = two_sum(a2, sizeof(a2)/sizeof(a2[0]), 6);   /* [1,2] */
    PairResult r3 = two_sum(a3, sizeof(a3)/sizeof(a3[0]), 9);   /* [0,1] */
    PairResult r4 = two_sum(a4, sizeof(a4)/sizeof(a4[0]), 0);   /* [0,3] */
    PairResult r5 = two_sum(a5, sizeof(a5)/sizeof(a5[0]), 0);   /* [0,2] */

    print_pair(r1);
    print_pair(r2);
    print_pair(r3);
    print_pair(r4);
    print_pair(r5);

    return 0;
}