#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct HashNode {
    int key;
    int value;
    struct HashNode* next;
} HashNode;

typedef struct {
    size_t capacity;
    HashNode** buckets;
} HashTable;

typedef struct {
    int found; // 1 if found, 0 otherwise
    int value;
} SearchResult;

static size_t ht_index(const HashTable* ht, int key) {
    uint32_t h = (uint32_t)key * 0x9E3779B1u;
    h ^= (h >> 16);
    return (size_t)(h % (uint32_t)ht->capacity);
}

HashTable* ht_create(size_t capacity) {
    if (capacity == 0) capacity = 17;
    HashTable* ht = (HashTable*)calloc(1, sizeof(HashTable));
    if (!ht) return NULL;
    ht->capacity = capacity;
    ht->buckets = (HashNode**)calloc(capacity, sizeof(HashNode*));
    if (!ht->buckets) {
        free(ht);
        return NULL;
    }
    return ht;
}

void ht_destroy(HashTable* ht) {
    if (!ht) return;
    for (size_t i = 0; i < ht->capacity; ++i) {
        HashNode* cur = ht->buckets[i];
        while (cur) {
            HashNode* nxt = cur->next;
            free(cur);
            cur = nxt;
        }
    }
    free(ht->buckets);
    free(ht);
}

// Inserts key->value. Returns 1 if inserted new, 0 if updated existing, -1 on error.
int ht_insert(HashTable* ht, int key, int value) {
    if (!ht || !ht->buckets) return -1;
    size_t idx = ht_index(ht, key);
    HashNode* cur = ht->buckets[idx];
    while (cur) {
        if (cur->key == key) {
            cur->value = value;
            return 0;
        }
        cur = cur->next;
    }
    HashNode* node = (HashNode*)calloc(1, sizeof(HashNode));
    if (!node) return -1;
    node->key = key;
    node->value = value;
    node->next = ht->buckets[idx];
    ht->buckets[idx] = node;
    return 1;
}

// Deletes key. Returns 1 if existed and removed, 0 if not found.
int ht_delete(HashTable* ht, int key) {
    if (!ht || !ht->buckets) return 0;
    size_t idx = ht_index(ht, key);
    HashNode* cur = ht->buckets[idx];
    HashNode* prev = NULL;
    while (cur) {
        if (cur->key == key) {
            if (prev) prev->next = cur->next;
            else ht->buckets[idx] = cur->next;
            free(cur);
            return 1;
        }
        prev = cur;
        cur = cur->next;
    }
    return 0;
}

// Searches key. Returns SearchResult {found, value}
SearchResult ht_search(const HashTable* ht, int key) {
    SearchResult res;
    res.found = 0;
    res.value = 0;
    if (!ht || !ht->buckets) return res;
    size_t idx = ht_index(ht, key);
    HashNode* cur = ht->buckets[idx];
    while (cur) {
        if (cur->key == key) {
            res.found = 1;
            res.value = cur->value;
            return res;
        }
        cur = cur->next;
    }
    return res;
}

int main(void) {
    HashTable* ht = ht_create(17);
    if (!ht) {
        printf("Failed to create hash table\n");
        return 1;
    }

    // Test 1: Insert and search basic
    int t1_insert = (ht_insert(ht, 1, 100) == 1) && (ht_insert(ht, 2, 200) == 1) && (ht_insert(ht, 3, 300) == 1);
    SearchResult s1 = ht_search(ht, 1);
    SearchResult s2 = ht_search(ht, 2);
    SearchResult s3 = ht_search(ht, 3);
    int t1_search = (s1.found && s1.value == 100) && (s2.found && s2.value == 200) && (s3.found && s3.value == 300);
    printf("Test1 Insert new keys: %s\n", t1_insert ? "true" : "false");
    printf("Test1 Search keys: %s\n", t1_search ? "true" : "false");

    // Test 2: Update existing key
    int updated = (ht_insert(ht, 2, 250) == 0);
    SearchResult s2b = ht_search(ht, 2);
    printf("Test2 Update existing: %s\n", (updated && s2b.found && s2b.value == 250) ? "true" : "false");

    // Test 3: Delete existing key
    int del3 = ht_delete(ht, 3);
    SearchResult s3b = ht_search(ht, 3);
    printf("Test3 Delete existing: %s\n", (del3 && !s3b.found) ? "true" : "false");

    // Test 4: Delete non-existing key
    int del99 = ht_delete(ht, 99);
    printf("Test4 Delete non-existing: %s\n", (!del99) ? "true" : "false");

    // Test 5: Search non-existing key after sequence
    SearchResult sNF = ht_search(ht, 42);
    printf("Test5 Search non-existing: %s\n", (!sNF.found) ? "true" : "false");

    ht_destroy(ht);
    return 0;
}