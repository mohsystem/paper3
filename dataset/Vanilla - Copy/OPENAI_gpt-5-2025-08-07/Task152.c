#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int key;
    int value;
    struct Node* next;
} Node;

typedef struct {
    int capacity;
    int size;
    Node** buckets;
} HashTable;

static unsigned int hashInt(int key) {
    unsigned int h = (unsigned int)key;
    h ^= (h >> 16);
    return h;
}

static int indexFor(HashTable* ht, int key) {
    unsigned int h = hashInt(key);
    return (int)(h % (unsigned int)ht->capacity);
}

HashTable* ht_create(int capacity) {
    if (capacity < 4) capacity = 4;
    HashTable* ht = (HashTable*)malloc(sizeof(HashTable));
    ht->capacity = capacity;
    ht->size = 0;
    ht->buckets = (Node**)calloc((size_t)capacity, sizeof(Node*));
    return ht;
}

void ht_free(HashTable* ht) {
    if (!ht) return;
    for (int i = 0; i < ht->capacity; ++i) {
        Node* cur = ht->buckets[i];
        while (cur) {
            Node* nxt = cur->next;
            free(cur);
            cur = nxt;
        }
    }
    free(ht->buckets);
    free(ht);
}

void ht_insert(HashTable* ht, int key, int value) {
    int idx = indexFor(ht, key);
    Node* cur = ht->buckets[idx];
    while (cur) {
        if (cur->key == key) {
            cur->value = value;
            return;
        }
        cur = cur->next;
    }
    Node* n = (Node*)malloc(sizeof(Node));
    n->key = key;
    n->value = value;
    n->next = ht->buckets[idx];
    ht->buckets[idx] = n;
    ht->size++;
}

int ht_search(HashTable* ht, int key, int* outValue) {
    int idx = indexFor(ht, key);
    Node* cur = ht->buckets[idx];
    while (cur) {
        if (cur->key == key) {
            if (outValue) *outValue = cur->value;
            return 1;
        }
        cur = cur->next;
    }
    return 0;
}

int ht_delete(HashTable* ht, int key) {
    int idx = indexFor(ht, key);
    Node* cur = ht->buckets[idx];
    Node* prev = NULL;
    while (cur) {
        if (cur->key == key) {
            if (prev) prev->next = cur->next;
            else ht->buckets[idx] = cur->next;
            free(cur);
            ht->size--;
            return 1;
        }
        prev = cur;
        cur = cur->next;
    }
    return 0;
}

int ht_size(HashTable* ht) {
    return ht->size;
}

int main() {
    HashTable* ht = ht_create(16);

    // Test 1: Insert and search
    ht_insert(ht, 1, 10);
    int v;
    int found = ht_search(ht, 1, &v);
    printf("Test1 search(1): %s", found ? "" : "None");
    if (found) printf("%d", v);
    printf("\n");

    // Test 2: Collision insert and search
    ht_insert(ht, 17, 20); // likely collides with 1 if capacity 16
    found = ht_search(ht, 17, &v);
    printf("Test2 search(17): %s", found ? "" : "None");
    if (found) printf("%d", v);
    printf("\n");

    // Test 3: Update existing key
    ht_insert(ht, 1, 15);
    found = ht_search(ht, 1, &v);
    printf("Test3 search(1): %s", found ? "" : "None");
    if (found) printf("%d", v);
    printf("\n");

    // Test 4: Delete existing key and verify
    printf("Test4 delete(17): %s\n", ht_delete(ht, 17) ? "true" : "false");
    found = ht_search(ht, 17, &v);
    printf("Test4 search(17): %s", found ? "" : "None");
    if (found) printf("%d", v);
    printf("\n");

    // Test 5: Delete non-existing key and size check
    printf("Test5 delete(99): %s\n", ht_delete(ht, 99) ? "true" : "false");
    printf("Test5 size(): %d\n", ht_size(ht));

    ht_free(ht);
    return 0;
}