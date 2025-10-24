/* Chain-of-Through secure implementation: C
 * 1) Problem understanding: Integer hash set with insert, delete, search.
 * 2) Security: safe hashing, power-of-two capacity, allocation checks.
 * 3) Secure coding: handle collisions, rehash safely, prevent overflow.
 * 4) Code review: verify pointer updates and memory management.
 * 5) Secure output: deterministic tests and proper cleanup.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct Node {
    int key;
    struct Node* next;
} Node;

typedef struct HashSet {
    size_t capacity;
    size_t size;
    size_t threshold;
    Node** table;
} HashSet;

static const float LOAD_FACTOR = 0.75f;
static const size_t MIN_CAP = 16;
static const size_t MAX_CAP = (size_t)1 << 30;

static uint64_t mix64(uint64_t z) {
    z = (z ^ (z >> 33)) * 0xff51afd7ed558ccdULL;
    z = (z ^ (z >> 33)) * 0xc4ceb9fe1a85ec53ULL;
    z = z ^ (z >> 33);
    return z;
}

static size_t index_for(int key, size_t cap) {
    return (size_t)(mix64((uint64_t)(int64_t)key)) & (cap - 1);
}

static void free_table(HashSet* hs) {
    if (!hs || !hs->table) return;
    for (size_t i = 0; i < hs->capacity; ++i) {
        Node* cur = hs->table[i];
        while (cur) {
            Node* nx = cur->next;
            free(cur);
            cur = nx;
        }
        hs->table[i] = NULL;
    }
    free(hs->table);
    hs->table = NULL;
}

static int resize_if_needed(HashSet* hs) {
    if (!hs) return 0;
    if (hs->size <= hs->threshold) return 1;
    if (hs->capacity >= MAX_CAP) return 1;
    size_t oldCap = hs->capacity;
    size_t newCap = oldCap << 1;
    if (newCap == 0 || newCap > MAX_CAP) newCap = MAX_CAP;

    Node** newTable = (Node**)calloc(newCap, sizeof(Node*));
    if (!newTable) return 0;

    for (size_t i = 0; i < oldCap; ++i) {
        Node* e = hs->table[i];
        while (e) {
            Node* next = e->next;
            size_t idx = index_for(e->key, newCap);
            e->next = newTable[idx];
            newTable[idx] = e;
            e = next;
        }
        hs->table[i] = NULL;
    }
    free(hs->table);
    hs->table = newTable;
    hs->capacity = newCap;
    size_t th = (size_t)(newCap * LOAD_FACTOR);
    if (th > (size_t)0x7fffffff) th = (size_t)0x7fffffff;
    hs->threshold = th;
    return 1;
}

static HashSet* hs_create(void) {
    HashSet* hs = (HashSet*)calloc(1, sizeof(HashSet));
    if (!hs) return NULL;
    hs->capacity = MIN_CAP;
    hs->size = 0;
    hs->threshold = (size_t)(hs->capacity * LOAD_FACTOR);
    hs->table = (Node**)calloc(hs->capacity, sizeof(Node*));
    if (!hs->table) {
        free(hs);
        return NULL;
    }
    return hs;
}

static void hs_destroy(HashSet* hs) {
    if (!hs) return;
    free_table(hs);
    free(hs);
}

static int hs_insert(HashSet* hs, int key) {
    if (!hs) return 0;
    size_t idx = index_for(key, hs->capacity);
    Node* cur = hs->table[idx];
    while (cur) {
        if (cur->key == key) return 0; // exists
        cur = cur->next;
    }
    Node* n = (Node*)malloc(sizeof(Node));
    if (!n) return 0;
    n->key = key;
    n->next = hs->table[idx];
    hs->table[idx] = n;
    hs->size++;
    if (!resize_if_needed(hs)) {
        // If resize failed, still valid state; proceed.
    }
    return 1;
}

static int hs_delete(HashSet* hs, int key) {
    if (!hs) return 0;
    size_t idx = index_for(key, hs->capacity);
    Node* prev = NULL;
    Node* cur = hs->table[idx];
    while (cur) {
        if (cur->key == key) {
            if (prev) prev->next = cur->next;
            else hs->table[idx] = cur->next;
            free(cur);
            hs->size--;
            return 1;
        }
        prev = cur;
        cur = cur->next;
    }
    return 0;
}

static int hs_search(HashSet* hs, int key) {
    if (!hs) return 0;
    size_t idx = index_for(key, hs->capacity);
    Node* cur = hs->table[idx];
    while (cur) {
        if (cur->key == key) return 1;
        cur = cur->next;
    }
    return 0;
}

int main(void) {
    HashSet* hs = hs_create();
    if (!hs) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }

    // Test 1
    hs_insert(hs, 10);
    hs_insert(hs, 20);
    hs_insert(hs, 30);
    printf("T1 contains(20): %s\n", hs_search(hs, 20) ? "true" : "false");

    // Test 2
    printf("T2 contains(15): %s\n", hs_search(hs, 15) ? "true" : "false");

    // Test 3
    printf("T3 delete(20): %s\n", hs_delete(hs, 20) ? "true" : "false");
    printf("T3 contains(20): %s\n", hs_search(hs, 20) ? "true" : "false");

    // Test 4
    printf("T4 insert(20): %s\n", hs_insert(hs, 20) ? "true" : "false");
    printf("T4 contains(20): %s\n", hs_search(hs, 20) ? "true" : "false");

    // Test 5
    printf("T5 delete(99): %s\n", hs_delete(hs, 99) ? "true" : "false");

    hs_destroy(hs);
    return 0;
}