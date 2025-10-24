#include <stdio.h>
#include <stdlib.h>

// Node for the hash table chain
typedef struct Node {
    int key;
    int value;
    struct Node* next;
} Node;

// Hash Table structure
typedef struct HashTable {
    int capacity;
    Node** buckets;
} HashTable;

// Create a new node
Node* createNode(int key, int value) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        perror("Failed to allocate memory for new node");
        exit(EXIT_FAILURE);
    }
    newNode->key = key;
    newNode->value = value;
    newNode->next = NULL;
    return newNode;
}

// Create a hash table
HashTable* createHashTable(int capacity) {
    if (capacity <= 0) {
        fprintf(stderr, "Capacity must be a positive integer.\n");
        return NULL;
    }
    HashTable* table = (HashTable*)malloc(sizeof(HashTable));
    if (table == NULL) {
        perror("Failed to allocate memory for hash table");
        exit(EXIT_FAILURE);
    }
    table->capacity = capacity;
    table->buckets = (Node**)calloc(capacity, sizeof(Node*));
    if (table->buckets == NULL) {
        perror("Failed to allocate memory for buckets");
        free(table);
        exit(EXIT_FAILURE);
    }
    return table;
}

// Free the entire hash table
void freeHashTable(HashTable* table) {
    if (table == NULL) return;
    for (int i = 0; i < table->capacity; i++) {
        Node* current = table->buckets[i];
        while (current != NULL) {
            Node* toDelete = current;
            current = current->next;
            free(toDelete);
        }
    }
    free(table->buckets);
    free(table);
}

// Hash function
int getBucketIndex(HashTable* table, int key) {
    return abs(key) % table->capacity;
}

// Insert a key-value pair
void insert(HashTable* table, int key, int value) {
    if (table == NULL) return;
    int index = getBucketIndex(table, key);
    Node* current = table->buckets[index];

    // Check if key already exists to update it
    while (current != NULL) {
        if (current->key == key) {
            current->value = value;
            return;
        }
        current = current->next;
    }

    // Key not found, insert new node at the head of the chain
    Node* newNode = createNode(key, value);
    newNode->next = table->buckets[index];
    table->buckets[index] = newNode;
}

// Search for a key
int search(HashTable* table, int key) {
    if (table == NULL) return -1;
    int index = getBucketIndex(table, key);
    Node* current = table->buckets[index];

    while (current != NULL) {
        if (current->key == key) {
            return current->value;
        }
        current = current->next;
    }
    return -1; // Sentinel value for not found
}

// Delete a key
void deleteKey(HashTable* table, int key) {
    if (table == NULL) return;
    int index = getBucketIndex(table, key);
    Node* current = table->buckets[index];
    Node* prev = NULL;

    // Find the node with the given key
    while (current != NULL && current->key != key) {
        prev = current;
        current = current->next;
    }

    // If key was not found
    if (current == NULL) {
        return;
    }

    // Unlink the node
    if (prev == NULL) {
        // The node to be deleted is the head
        table->buckets[index] = current->next;
    } else {
        prev->next = current->next;
    }
    free(current);
}

// Main function with test cases
void runTests() {
    HashTable* hashTable = createHashTable(10);
    printf("C Hash Table Implementation Test\n");

    // Test Case 1: Insert key-value pairs. (5 and 15 will collide)
    insert(hashTable, 5, 50);
    insert(hashTable, 15, 150);
    insert(hashTable, 2, 20);
    printf("Test 1: Insert (5, 50), (15, 150), (2, 20)\n");
    printf("Value for key 15: %d\n", search(hashTable, 15));
    printf("Value for key 5: %d\n", search(hashTable, 5));
    printf("Value for key 2: %d\n", search(hashTable, 2));
    printf("--------------------\n");

    // Test Case 2: Search for an existing key.
    printf("Test 2: Search for existing key 15\n");
    printf("Value: %d\n", search(hashTable, 15));
    printf("--------------------\n");

    // Test Case 3: Search for a non-existing key.
    printf("Test 3: Search for non-existing key 25\n");
    printf("Value: %d\n", search(hashTable, 25));
    printf("--------------------\n");

    // Test Case 4: Update an existing key's value.
    printf("Test 4: Update value for key 5 to 55\n");
    insert(hashTable, 5, 55);
    printf("New value for key 5: %d\n", search(hashTable, 5));
    printf("--------------------\n");

    // Test Case 5: Delete a key and verify deletion.
    printf("Test 5: Delete key 15\n");
    deleteKey(hashTable, 15);
    printf("Searching for deleted key 15: %d\n", search(hashTable, 15));
    printf("Searching for key 5 to ensure it's still there: %d\n", search(hashTable, 5));
    printf("--------------------\n");

    freeHashTable(hashTable);
}

int main() {
    runTests();
    return 0;
}