
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define BASE 26
#define MOD 1000000007
#define MAX_LEN 30000

// Hash table entry for storing hash values
typedef struct HashNode {
    uint64_t hash;
    struct HashNode* next;
} HashNode;

// Simple hash table for checking duplicate hashes
typedef struct {
    HashNode** buckets;
    size_t size;
} HashTable;

// Initialize hash table with given size
// Security: Check malloc return, initialize all pointers to NULL
HashTable* createHashTable(size_t size) {
    if (size == 0 || size > MAX_LEN) {
        return NULL;
    }
    
    HashTable* table = (HashTable*)malloc(sizeof(HashTable));
    if (table == NULL) {
        return NULL;
    }
    
    table->buckets = (HashNode**)calloc(size, sizeof(HashNode*));
    if (table->buckets == NULL) {
        free(table);
        return NULL;
    }
    
    table->size = size;
    return table;
}

// Free hash table memory
// Security: Check for NULL, free all nodes, clear pointers
void freeHashTable(HashTable* table) {
    if (table == NULL) {
        return;
    }
    
    if (table->buckets != NULL) {
        for (size_t i = 0; i < table->size; i++) {
            HashNode* node = table->buckets[i];
            while (node != NULL) {
                HashNode* temp = node;
                node = node->next;
                free(temp);
            }
        }
        free(table->buckets);
    }
    free(table);
}

// Insert hash into table
// Security: Bounds check on bucket index, validate malloc
bool insertHash(HashTable* table, uint64_t hash) {
    if (table == NULL || table->buckets == NULL) {
        return false;
    }
    
    size_t bucket = (size_t)(hash % table->size);
    if (bucket >= table->size) {
        return false;
    }
    
    HashNode* node = (HashNode*)malloc(sizeof(HashNode));
    if (node == NULL) {
        return false;
    }
    
    node->hash = hash;
    node->next = table->buckets[bucket];
    table->buckets[bucket] = node;
    return true;
}

// Check if hash exists in table
// Security: NULL checks, bounds validation
bool containsHash(HashTable* table, uint64_t hash) {
    if (table == NULL || table->buckets == NULL) {
        return false;
    }
    
    size_t bucket = (size_t)(hash % table->size);
    if (bucket >= table->size) {
        return false;
    }
    
    HashNode* node = table->buckets[bucket];
    while (node != NULL) {
        if (node->hash == hash) {
            return true;
        }
        node = node->next;
    }
    return false;
}

// Check if duplicate substring of given length exists
// Security: Validate all inputs, bounds check all array accesses
char* hasDuplicate(const char* s, int len, size_t slen) {
    if (s == NULL || len <= 0 || len > (int)slen) {
        return NULL;
    }
    
    // Compute BASE^(len-1) % MOD
    uint64_t basePower = 1;
    for (int i = 0; i < len - 1; i++) {
        basePower = (basePower * BASE) % MOD;
    }
    
    // Compute initial hash
    uint64_t hash = 0;
    for (int i = 0; i < len; i++) {
        if (s[i] < 'a' || s[i] > 'z') {
            return NULL;
        }
        hash = (hash * BASE + (uint64_t)(s[i] - 'a')) % MOD;
    }
    
    HashTable* table = createHashTable(slen);
    if (table == NULL) {
        return NULL;
    }
    
    if (!insertHash(table, hash)) {
        freeHashTable(table);
        return NULL;
    }
    
    // Rolling hash with bounds checking
    for (size_t i = 1; i + len <= slen; i++) {
        if (s[i - 1] < 'a' || s[i - 1] > 'z' || 
            s[i + len - 1] < 'a' || s[i + len - 1] > 'z') {
            freeHashTable(table);
            return NULL;
        }
        
        hash = (hash + MOD - (basePower * (uint64_t)(s[i - 1] - 'a')) % MOD) % MOD;
        hash = (hash * BASE + (uint64_t)(s[i + len - 1] - 'a')) % MOD;
        
        if (containsHash(table, hash)) {
            // Verify actual substring match
            for (size_t j = 0; j < i; j++) {
                if (j + len <= slen && memcmp(s + j, s + i, len) == 0) {
                    char* result = (char*)malloc(len + 1);
                    if (result == NULL) {
                        freeHashTable(table);
                        return NULL;
                    }
                    memcpy(result, s + i, len);
                    result[len] = '\\0';
                    freeHashTable(table);
                    return result;
                }
            }
        }
        
        if (!insertHash(table, hash)) {
            freeHashTable(table);
            return NULL;
        }
    }
    
    freeHashTable(table);
    return NULL;
}

// Find longest duplicate substring
// Security: Validate inputs, check string length, bounds check
char* longestDupSubstring(const char* s) {
    if (s == NULL) {
        return NULL;
    }
    
    size_t slen = strlen(s);
    if (slen < 2 || slen > MAX_LEN) {
        char* empty = (char*)malloc(1);
        if (empty != NULL) {
            empty[0] = '\\0';
        }
        return empty;
    }
    
    // Validate all characters
    for (size_t i = 0; i < slen; i++) {
        if (s[i] < 'a' || s[i] > 'z') {
            char* empty = (char*)malloc(1);
            if (empty != NULL) {
                empty[0] = '\\0';
            }
            return empty;
        }
    }
    
    int left = 1;
    int right = (int)slen - 1;
    char* result = NULL;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        char* dup = hasDuplicate(s, mid, slen);
        
        if (dup != NULL) {
            if (result != NULL) {
                free(result);
            }
            result = dup;
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    if (result == NULL) {
        result = (char*)malloc(1);
        if (result != NULL) {
            result[0] = '\\0';
        }
    }
    
    return result;
}

int main() {
    // Test case 1
    char* result1 = longestDupSubstring("banana");
    printf("Test 1: %s (expected: ana)\\n", result1 ? result1 : "NULL");
    free(result1);
    
    // Test case 2
    char* result2 = longestDupSubstring("abcd");
    printf("Test 2: %s (expected: empty)\\n", result2 ? result2 : "NULL");
    free(result2);
    
    // Test case 3
    char* result3 = longestDupSubstring("aa");
    printf("Test 3: %s (expected: a)\\n", result3 ? result3 : "NULL");
    free(result3);
    
    // Test case 4
    char* result4 = longestDupSubstring("aaaaa");
    printf("Test 4: %s (expected: aaaa)\\n", result4 ? result4 : "NULL");
    free(result4);
    
    // Test case 5
    char* result5 = longestDupSubstring("abcabcabc");
    printf("Test 5: %s (expected: abcabc)\\n", result5 ? result5 : "NULL");
    free(result5);
    
    printf("All tests completed!\\n");
    return 0;
}
