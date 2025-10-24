#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// A simple hash set for storing long long values, implemented with linear probing.
#define HASH_SET_CAPACITY 4000007 // A large prime number, > max possible substrings (2000^2/4)
typedef struct {
    long long* keys;
    char* states; // 0: empty, 1: occupied
    int size;
} HashSet;

// A simple hash function to map a long long key to an index.
unsigned int hash_code(long long key) {
    key = (~key) + (key << 21);
    key = key ^ (key >> 24);
    key = (key + (key << 3)) + (key << 8);
    key = key ^ (key >> 14);
    key = (key + (key << 2)) + (key << 4);
    key = key ^ (key >> 28);
    key = key + (key << 31);
    return (unsigned int)key;
}

// Inserts a key into the hash set. If the key already exists, does nothing.
void hash_set_insert(HashSet* set, long long key) {
    unsigned int index = hash_code(key) % HASH_SET_CAPACITY;
    while (set->states[index] == 1) {
        if (set->keys[index] == key) {
            return; // Key already exists
        }
        index = (index + 1) % HASH_SET_CAPACITY; // Linear probing
    }
    set->keys[index] = key;
    set->states[index] = 1;
    set->size++;
}


int countDistinct(const char* text) {
    int n = strlen(text);
    if (n <= 1) {
        return 0;
    }

    long long p1 = 31, m1 = 1000000007;
    long long p2 = 37, m2 = 1000000009;

    long long* p1_powers = (long long*)malloc(sizeof(long long) * (n + 1));
    long long* p2_powers = (long long*)malloc(sizeof(long long) * (n + 1));
    long long* h1 = (long long*)calloc(n + 1, sizeof(long long));
    long long* h2 = (long long*)calloc(n + 1, sizeof(long long));

    p1_powers[0] = 1;
    p2_powers[0] = 1;
    for (int i = 1; i <= n; i++) {
        p1_powers[i] = (p1_powers[i - 1] * p1) % m1;
        p2_powers[i] = (p2_powers[i - 1] * p2) % m2;
    }

    for (int i = 0; i < n; i++) {
        h1[i + 1] = (h1[i] * p1 + (text[i] - 'a' + 1)) % m1;
        h2[i + 1] = (h2[i] * p2 + (text[i] - 'a' + 1)) % m2;
    }

    HashSet foundHashes;
    foundHashes.keys = (long long*)malloc(sizeof(long long) * HASH_SET_CAPACITY);
    foundHashes.states = (char*)calloc(HASH_SET_CAPACITY, sizeof(char));
    foundHashes.size = 0;

    for (int len = 1; len * 2 <= n; len++) {
        for (int i = 0; i <= n - 2 * len; i++) {
            int mid_idx = i + len;

            long long hash1_a = (h1[mid_idx] - (h1[i] * p1_powers[len]) % m1 + m1) % m1;
            long long hash2_a = (h2[mid_idx] - (h2[i] * p2_powers[len]) % m2 + m2) % m2;

            long long hash1_b = (h1[i + 2 * len] - (h1[mid_idx] * p1_powers[len]) % m1 + m1) % m1;
            long long hash2_b = (h2[i + 2 * len] - (h2[mid_idx] * p2_powers[len]) % m2 + m2) % m2;

            if (hash1_a == hash1_b && hash2_a == hash2_b) {
                long long total_hash1 = (h1[i + 2 * len] - (h1[i] * p1_powers[2 * len]) % m1 + m1) % m1;
                long long total_hash2 = (h2[i + 2 * len] - (h2[i] * p2_powers[2 * len]) % m2 + m2) % m2;
                hash_set_insert(&foundHashes, total_hash1 * m2 + total_hash2);
            }
        }
    }
    
    int result = foundHashes.size;

    free(p1_powers);
    free(p2_powers);
    free(h1);
    free(h2);
    free(foundHashes.keys);
    free(foundHashes.states);

    return result;
}

int main() {
    const char* text1 = "abcabcabc";
    printf("Input: %s, Output: %d\n", text1, countDistinct(text1));

    const char* text2 = "leetcodeleetcode";
    printf("Input: %s, Output: %d\n", text2, countDistinct(text2));
    
    const char* text3 = "aaaaa";
    printf("Input: %s, Output: %d\n", text3, countDistinct(text3));

    const char* text4 = "ababa";
    printf("Input: %s, Output: %d\n", text4, countDistinct(text4));

    const char* text5 = "zzzzzzzzzz";
    printf("Input: %s, Output: %d\n", text5, countDistinct(text5));

    return 0;
}