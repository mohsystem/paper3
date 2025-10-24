#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// --- Hash Pair Set Implementation ---
typedef struct {
    long long h1;
    long long h2;
} HashPair;

typedef struct Node {
    HashPair pair;
    struct Node* next;
} Node;

// A prime number is a good choice for capacity to improve hash distribution.
#define HASH_SET_CAPACITY 4099

typedef struct {
    Node** buckets;
    int capacity;
    int size;
} HashSet;

unsigned int hash_pair(HashPair pair, int capacity) {
    // Combine the two 64-bit hashes into a single hash value for the bucket index.
    // A simple combination using a prime multiplier.
    unsigned long long combined_hash = (unsigned long long)pair.h1 * 31 + (unsigned long long)pair.h2;
    return combined_hash % capacity;
}

HashSet* create_hash_set(int capacity) {
    HashSet* set = (HashSet*)malloc(sizeof(HashSet));
    if (!set) {
        return NULL;
    }
    set->capacity = capacity;
    set->size = 0;
    set->buckets = (Node**)calloc(capacity, sizeof(Node*));
    if (!set->buckets) {
        free(set);
        return NULL;
    }
    return set;
}

void free_hash_set(HashSet* set) {
    if (!set) {
        return;
    }
    for (int i = 0; i < set->capacity; i++) {
        Node* current = set->buckets[i];
        while (current) {
            Node* temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(set->buckets);
    free(set);
}

// Returns true if the pair was newly inserted, false if it already existed.
bool hash_set_insert(HashSet* set, HashPair pair) {
    if (!set) {
        return false;
    }
    unsigned int index = hash_pair(pair, set->capacity);
    Node* current = set->buckets[index];

    // Check if the pair already exists in the chain
    while (current) {
        if (current->pair.h1 == pair.h1 && current->pair.h2 == pair.h2) {
            return false; // Already exists
        }
        current = current->next;
    }

    // If not found, create a new node and insert it at the head of the chain
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        // In a real-world application, this failure should be propagated
        return false; 
    }
    newNode->pair = pair;
    newNode->next = set->buckets[index];
    set->buckets[index] = newNode;
    set->size++;
    return true;
}

int distinctEchoSubstrings(const char* text) {
    if (!text) {
        return 0;
    }
    int n = strlen(text);
    if (n <= 1) {
        return 0;
    }

    // --- Rolling Hash Setup ---
    long long p1 = 31, m1 = 1e9 + 7;
    long long p2 = 37, m2 = 1e9 + 9;

    long long* p1_pow = (long long*)malloc((n + 1) * sizeof(long long));
    long long* h1 = (long long*)malloc((n + 1) * sizeof(long long));
    long long* p2_pow = (long long*)malloc((n + 1) * sizeof(long long));
    long long* h2 = (long long*)malloc((n + 1) * sizeof(long long));
    
    // Security: Check all malloc results
    if (!p1_pow || !h1 || !p2_pow || !h2) {
        free(p1_pow);
        free(h1);
        free(p2_pow);
        free(h2);
        return -1; // Indicate error
    }

    p1_pow[0] = 1;
    h1[0] = 0;
    p2_pow[0] = 1;
    h2[0] = 0;

    for (int i = 0; i < n; i++) {
        p1_pow[i + 1] = (p1_pow[i] * p1) % m1;
        h1[i + 1] = (h1[i] * p1 + (text[i] - 'a' + 1)) % m1;
        p2_pow[i + 1] = (p2_pow[i] * p2) % m2;
        h2[i + 1] = (h2[i] * p2 + (text[i] - 'a' + 1)) % m2;
    }

    // --- HashSet Initialization ---
    HashSet* set = create_hash_set(HASH_SET_CAPACITY);
    if (!set) {
        free(p1_pow);
        free(h1);
        free(p2_pow);
        free(h2);
        return -1; // Indicate error
    }

    // --- Main Loop ---
    // Iterate through all possible lengths of 'a'
    for (int k = 1; k <= n / 2; k++) {
        int len = 2 * k;
        // Iterate through all starting positions for a substring of length `len`
        for (int i = 0; i <= n - len; i++) {
            // First half: text[i ... i+k-1]
            // Second half: text[i+k ... i+len-1]
            
            // Calculate hash of the first half
            long long h1_a = (h1[i+k] - (h1[i] * p1_pow[k]) % m1 + m1) % m1;
            long long h2_a = (h2[i+k] - (h2[i] * p2_pow[k]) % m2 + m2) % m2;

            // Calculate hash of the second half
            long long h1_b = (h1[i+len] - (h1[i+k] * p1_pow[k]) % m1 + m1) % m1;
            long long h2_b = (h2[i+len] - (h2[i+k] * p2_pow[k]) % m2 + m2) % m2;
            
            if (h1_a == h1_b && h2_a == h2_b) {
                // With double hashing, the probability of collision is negligible.
                // We assume the substrings are identical if their hashes match.
                
                // Get hash of the full substring text[i...i+len-1]
                long long full_h1 = (h1[i+len] - (h1[i] * p1_pow[len]) % m1 + m1) % m1;
                long long full_h2 = (h2[i+len] - (h2[i] * p2_pow[len]) % m2 + m2) % m2;
                HashPair pair = {full_h1, full_h2};
                hash_set_insert(set, pair);
            }
        }
    }

    int count = set->size;
    
    // Cleanup
    free_hash_set(set);
    free(p1_pow);
    free(h1);
    free(p2_pow);
    free(h2);

    return count;
}


void run_test_case(const char* name, const char* text, int expected) {
    int result = distinctEchoSubstrings(text);
    printf("Test Case: %s\n", name);
    printf("Input: \"%s\"\n", text);
    printf("Expected: %d\n", expected);
    printf("Actual: %d\n", result);
    printf(result == expected ? "Result: PASS\n" : "Result: FAIL\n");
    printf("\n");
}


int main() {
    run_test_case("Example 1", "abcabcabc", 3);
    run_test_case("Example 2", "leetcodeleetcode", 2);
    run_test_case("Custom 1: All same chars", "aaaaa", 2);
    run_test_case("Custom 2: Overlapping", "ababa", 2);
    run_test_case("Custom 3: No echo", "abacaba", 0);
    
    return 0;
}