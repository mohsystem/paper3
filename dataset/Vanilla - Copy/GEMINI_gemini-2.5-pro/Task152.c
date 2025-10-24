#include <stdio.h>
#include <stdlib.h>

// Node for the linked list
typedef struct HashNode {
    int key;
    int value;
    struct HashNode* next;
} HashNode;

// Hash Table structure
typedef struct HashTable {
    int capacity;
    HashNode** table;
} HashTable;

// Hash function
int hashFunction(int key, int capacity) {
    // Ensure the result is non-negative
    int hash = key % capacity;
    return hash < 0 ? hash + capacity : hash;
}

// Create a new hash node
HashNode* createNode(int key, int value) {
    HashNode* newNode = (HashNode*)malloc(sizeof(HashNode));
    if (!newNode) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    newNode->key = key;
    newNode->value = value;
    newNode->next = NULL;
    return newNode;
}

// Create a new hash table
HashTable* createHashTable(int capacity) {
    HashTable* ht = (HashTable*)malloc(sizeof(HashTable));
    if (!ht) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    ht->capacity = capacity;
    ht->table = (HashNode**)calloc(capacity, sizeof(HashNode*));
    if (!ht->table) {
        fprintf(stderr, "Memory allocation failed\n");
        free(ht);
        exit(EXIT_FAILURE);
    }
    return ht;
}

// Free the hash table and all its nodes
void freeHashTable(HashTable* ht) {
    if (!ht) return;
    for (int i = 0; i < ht->capacity; i++) {
        HashNode* current = ht->table[i];
        while (current != NULL) {
            HashNode* temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(ht->table);
    free(ht);
}

// Insert a key-value pair
void insert(HashTable* ht, int key, int value) {
    int index = hashFunction(key, ht->capacity);
    HashNode* head = ht->table[index];
    HashNode* current = head;

    // Check if key exists and update value
    while (current != NULL) {
        if (current->key == key) {
            current->value = value;
            return;
        }
        current = current->next;
    }

    // Key not found, insert new node at the beginning
    HashNode* newNode = createNode(key, value);
    newNode->next = head;
    ht->table[index] = newNode;
}

// Search for a key and return its value
// Returns -1 if not found
int search(HashTable* ht, int key) {
    int index = hashFunction(key, ht->capacity);
    HashNode* current = ht->table[index];

    while (current != NULL) {
        if (current->key == key) {
            return current->value;
        }
        current = current->next;
    }
    return -1; // Not found
}

// Delete a key
void delete_key(HashTable* ht, int key) {
    int index = hashFunction(key, ht->capacity);
    HashNode* head = ht->table[index];
    HashNode* prev = NULL;
    HashNode* current = head;

    while (current != NULL && current->key != key) {
        prev = current;
        current = current->next;
    }

    if (current == NULL) return;

    if (prev != NULL) {
        prev->next = current->next;
    } else {
        ht->table[index] = current->next;
    }
    free(current); // Free the memory of the deleted node
}

void display(HashTable* ht) {
    printf("---- HASH TABLE ----\n");
    for (int i = 0; i < ht->capacity; i++) {
        printf("Index %d:", i);
        HashNode* current = ht->table[i];
        while (current != NULL) {
            printf(" -> (%d, %d)", current->key, current->value);
            current = current->next;
        }
        printf("\n");
    }
    printf("--------------------\n");
}

int main() {
    HashTable* hashTable = createHashTable(10);

    printf("----- Testing Insert and Search -----\n");
    insert(hashTable, 1, 10);
    insert(hashTable, 2, 20);
    insert(hashTable, 12, 120); // Collision with key 2
    display(hashTable);

    printf("\nSearch for key 2: %d\n", search(hashTable, 2));
    printf("Search for key 12: %d\n", search(hashTable, 12));
    printf("Search for key 5 (non-existent): %d\n", search(hashTable, 5));

    printf("\n----- Testing Update -----\n");
    insert(hashTable, 2, 22); // Update value for key 2
    printf("Search for key 2 after update: %d\n", search(hashTable, 2));
    display(hashTable);

    printf("\n----- Testing Delete -----\n");
    delete_key(hashTable, 12);
    printf("Search for key 12 after delete: %d\n", search(hashTable, 12));
    printf("Search for key 2 after deleting 12: %d\n", search(hashTable, 2));
    display(hashTable);

    printf("\n----- More Test Cases -----\n");
    insert(hashTable, 22, 220); // Another collision at index 2
    insert(hashTable, 32, 320); // Another collision at index 2
    display(hashTable);
    printf("Deleting head of a chain (key 32)...\n");
    delete_key(hashTable, 32);
    display(hashTable);
    printf("Search for 32: %d\n", search(hashTable, 32));
    printf("Search for 22: %d\n", search(hashTable, 22));

    freeHashTable(hashTable); // Clean up memory
    return 0;
}