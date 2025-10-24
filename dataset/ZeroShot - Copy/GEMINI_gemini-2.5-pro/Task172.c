#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef long long ll;

typedef struct {
    ll h1;
    ll h2;
} HashPair;

int compare_pairs(const void* a, const void* b) {
    HashPair* p1 = (HashPair*)a;
    HashPair* p2 = (HashPair*)b;
    if (p1->h1 < p2->h1) return -1;
    if (p1->h1 > p2->h1) return 1;
    if (p1->h2 < p2->h2) return -1;
    if (p1->h2 > p2->h2) return 1;
    return 0;
}

ll get_hash(const ll* h, const ll* p_pow, ll M, int i, int j) {
    int len = j - i + 1;
    ll term = (h[i] * p_pow[len]) % M;
    ll raw_hash = h[j + 1] - term;
    return (raw_hash % M + M) % M;
}

int distinctEchoSubstrings(const char* text) {
    ll P1 = 31, M1 = 1e9 + 7;
    ll P2 = 37, M2 = 1e9 + 9;
    int n = strlen(text);

    if (n == 0) return 0;

    ll* p_pow1 = (ll*)malloc((n + 1) * sizeof(ll));
    ll* h1 = (ll*)malloc((n + 1) * sizeof(ll));
    ll* p_pow2 = (ll*)malloc((n + 1) * sizeof(ll));
    ll* h2 = (ll*)malloc((n + 1) * sizeof(ll));
    
    if (!p_pow1 || !h1 || !p_pow2 || !h2) {
        if(p_pow1) free(p_pow1); if(h1) free(h1);
        if(p_pow2) free(p_pow2); if(h2) free(h2);
        return -1; // Memory allocation error
    }

    p_pow1[0] = 1; p_pow2[0] = 1;
    h1[0] = 0; h2[0] = 0;

    for (int i = 0; i < n; ++i) {
        p_pow1[i+1] = (p_pow1[i] * P1) % M1;
        h1[i+1] = (h1[i] * P1 + (text[i] - 'a' + 1)) % M1;
        p_pow2[i+1] = (p_pow2[i] * P2) % M2;
        h2[i+1] = (h2[i] * P2 + (text[i] - 'a' + 1)) % M2;
    }
    
    size_t capacity = 16;
    size_t count = 0;
    HashPair* found_hashes = (HashPair*)malloc(capacity * sizeof(HashPair));
    if (!found_hashes) {
        free(p_pow1); free(h1); free(p_pow2); free(h2);
        return -1; // Memory allocation error
    }

    for (int l = 1; l <= n / 2; ++l) {
        for (int i = 0; i <= n - 2 * l; ++i) {
            int j = i + l;
            ll h1_a = get_hash(h1, p_pow1, M1, i, i + l - 1);
            ll h1_b = get_hash(h1, p_pow1, M1, j, j + l - 1);
            
            if (h1_a == h1_b) {
                ll h2_a = get_hash(h2, p_pow2, M2, i, i + l - 1);
                ll h2_b = get_hash(h2, p_pow2, M2, j, j + l - 1);
                
                if (h2_a == h2_b) {
                    if (count == capacity) {
                        capacity *= 2;
                        HashPair* new_ptr = (HashPair*)realloc(found_hashes, capacity * sizeof(HashPair));
                        if (!new_ptr) {
                           free(found_hashes); free(p_pow1); free(h1); free(p_pow2); free(h2);
                           return -1; // Memory allocation error
                        }
                        found_hashes = new_ptr;
                    }
                    found_hashes[count].h1 = get_hash(h1, p_pow1, M1, i, i + 2*l - 1);
                    found_hashes[count].h2 = get_hash(h2, p_pow2, M2, i, i + 2*l - 1);
                    count++;
                }
            }
        }
    }
    
    free(p_pow1); free(h1); free(p_pow2); free(h2);

    if (count == 0) {
        free(found_hashes);
        return 0;
    }
    
    qsort(found_hashes, count, sizeof(HashPair), compare_pairs);
    
    int unique_count = 1;
    for (size_t i = 1; i < count; i++) {
        if (found_hashes[i].h1 != found_hashes[i-1].h1 || found_hashes[i].h2 != found_hashes[i-1].h2) {
            unique_count++;
        }
    }
    
    free(found_hashes);
    return unique_count;
}

int main() {
    const char* test_cases[] = {
        "abcabcabc",       // Expected: 3
        "leetcodeleetcode",// Expected: 2
        "a",               // Expected: 0
        "aaaaa",           // Expected: 2
        "abacaba"          // Expected: 0
    };

    for (int i = 0; i < 5; ++i) {
        printf("Test Case %d:\n", i + 1);
        const char* text = test_cases[i];
        int result = distinctEchoSubstrings(text);
        printf("Input: text = \"%s\"\n", text);
        printf("Output: %d\n\n", result);
    }

    return 0;
}