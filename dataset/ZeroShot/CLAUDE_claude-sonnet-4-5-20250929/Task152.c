
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct HashNode {
    char* key;
    int value;
    struct HashNode* next;
} HashNode;

typedef struct HashTable {
    HashNode** buckets;
    int capacity;
    int size;
} HashTable;

unsigned int hash(const char* key, int capacity) {
    unsigned int hash = 0;
    while (*key) {
        hash = (hash * 31) + *key;
        key++;
    }
    return hash % capacity;
}

HashTable* createHashTable(int capacity) {
    HashTable* ht = (HashTable*)malloc(sizeof(HashTable));
    ht->capacity = capacity;
    ht->size = 0;
    ht->buckets = (HashNode**)calloc(capacity, sizeof(HashNode*));
    return ht;
}

void insert(HashTable* ht, const char* key, int value) {
    unsigned int index = hash(key, ht->capacity);
    HashNode* current = ht->buckets[index];
    
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            current->value = value;
            return;
        }
        current = current->next;
    }
    
    HashNode* newNode = (HashNode*)malloc(sizeof(HashNode));
    newNode->key = (char*)malloc(strlen(key) + 1);
    strcpy(newNode->key, key);
    newNode->value = value;
    newNode->next = ht->buckets[index];
    ht->buckets[index] = newNode;
    ht->size++;
}

int* search(HashTable* ht, const char* key) {
    unsigned int index = hash(key, ht->capacity);
    HashNode* current = ht->buckets[index];
    
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            return &(current->value);
        }
        current = current->next;
    }
    
    return NULL;
}

bool deleteKey(HashTable* ht, const char* key) {
    unsigned int index = hash(key, ht->capacity);
    HashNode* current = ht->buckets[index];
    HashNode* prev = NULL;
    
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            if (prev == NULL) {
                ht->buckets[index] = current->next;
            } else {
                prev->next = current->next;
            }
            free(current->key);
            free(current);
            ht->size--;
            return true;
        }
        prev = current;
        current = current->next;
    }
    
    return false;
}

void freeHashTable(HashTable* ht) {
    for (int i = 0; i < ht->capacity; i++) {
        HashNode* current = ht->buckets[i];
        while (current != NULL) {
            HashNode* temp = current;
            current = current->next;
            free(temp->key);
            free(temp);
        }
    }
    free(ht->buckets);
    free(ht);
}

int main() {
    printf("Test Case 1: Basic Insert and Search\\n");
    HashTable* ht1 = createHashTable(10);
    insert(ht1, "apple", 5);
    insert(ht1, "banana", 10);
    insert(ht1, "orange", 15);
    int* result1 = search(ht1, "apple");
    printf("Search 'apple': %d\\n", result1 ? *result1 : -1);
    int* result2 = search(ht1, "banana");
    printf("Search 'banana': %d\\n", result2 ? *result2 : -1);
    printf("Size: %d\\n", ht1->size);
    freeHashTable(ht1);
    
    printf("\\nTest Case 2: Delete Operation\\n");
    HashTable* ht2 = createHashTable(10);
    insert(ht2, "key1", 100);
    insert(ht2, "key2", 200);
    printf("Before delete - Size: %d\\n", ht2->size);
    printf("Delete 'key1': %s\\n", deleteKey(ht2, "key1") ? "true" : "false");
    printf("After delete - Size: %d\\n", ht2->size);
    int* result3 = search(ht2, "key1");
    printf("Search 'key1': %s\\n", result3 ? "found" : "not found");
    freeHashTable(ht2);
    
    printf("\\nTest Case 3: Update Existing Key\\n");
    HashTable* ht3 = createHashTable(10);
    insert(ht3, "counter", 1);
    int* result4 = search(ht3, "counter");
    printf("Initial value: %d\\n", result4 ? *result4 : -1);
    insert(ht3, "counter", 2);
    int* result5 = search(ht3, "counter");
    printf("Updated value: %d\\n", result5 ? *result5 : -1);
    printf("Size: %d\\n", ht3->size);
    freeHashTable(ht3);
    
    printf("\\nTest Case 4: Search Non-existent Key\\n");
    HashTable* ht4 = createHashTable(10);
    insert(ht4, "exists", 99);
    int* result6 = search(ht4, "exists");
    printf("Search 'exists': %d\\n", result6 ? *result6 : -1);
    int* result7 = search(ht4, "notexists");
    printf("Search 'notexists': %s\\n", result7 ? "found" : "not found");
    freeHashTable(ht4);
    
    printf("\\nTest Case 5: Multiple Operations\\n");
    HashTable* ht5 = createHashTable(10);
    insert(ht5, "one", 1);
    insert(ht5, "two", 2);
    insert(ht5, "three", 3);
    int* result8 = search(ht5, "two");
    printf("Search 'two': %d\\n", result8 ? *result8 : -1);
    deleteKey(ht5, "two");
    int* result9 = search(ht5, "two");
    printf("After delete 'two': %s\\n", result9 ? "found" : "not found");
    insert(ht5, "four", 4);
    int* result10 = search(ht5, "four");
    printf("Search 'four': %d\\n", result10 ? *result10 : -1);
    printf("Final size: %d\\n", ht5->size);
    freeHashTable(ht5);
    
    return 0;
}
