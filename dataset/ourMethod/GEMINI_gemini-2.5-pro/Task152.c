#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    int key;
    int value;
    struct Node* next;
} Node;

typedef struct {
    Node** table;
    int capacity;
} HashTable;

unsigned int hash_function(int key, int capacity) {
    // Ensure the hash is non-negative
    return abs(key) % capacity;
}

HashTable* create_hash_table(int capacity) {
    if (capacity <= 0) {
        fprintf(stderr, "Error: Capacity must be positive.\n");
        return NULL;
    }

    HashTable* ht = (HashTable*)malloc(sizeof(HashTable));
    if (ht == NULL) {
        perror("Failed to allocate memory for HashTable");
        return NULL;
    }

    ht->capacity = capacity;
    ht->table = (Node**)calloc(ht->capacity, sizeof(Node*));
    if (ht->table == NULL) {
        perror("Failed to allocate memory for table");
        free(ht);
        return NULL;
    }
    return ht;
}

void free_hash_table(HashTable* ht) {
    if (ht == NULL) return;

    for (int i = 0; i < ht->capacity; ++i) {
        Node* current = ht->table[i];
        while (current != NULL) {
            Node* to_delete = current;
            current = current->next;
            free(to_delete);
        }
    }
    free(ht->table);
    free(ht);
}

void hash_table_insert(HashTable* ht, int key, int value) {
    if (ht == NULL) return;
    unsigned int index = hash_function(key, ht->capacity);
    
    // Search for key. If it exists, update value.
    Node* current = ht->table[index];
    while (current != NULL) {
        if (current->key == key) {
            current->value = value;
            return;
        }
        current = current->next;
    }

    // If key not found, insert a new node at the beginning of the chain.
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (new_node == NULL) {
        perror("Failed to allocate memory for new node");
        return;
    }
    new_node->key = key;
    new_node->value = value;
    new_node->next = ht->table[index];
    ht->table[index] = new_node;
}

int hash_table_search(HashTable* ht, int key, int* value) {
    if (ht == NULL || value == NULL) return 0; // 0 for failure/not found
    unsigned int index = hash_function(key, ht->capacity);
    
    Node* current = ht->table[index];
    while (current != NULL) {
        if (current->key == key) {
            *value = current->value;
            return 1; // 1 for success
        }
        current = current->next;
    }
    return 0; // Key not found
}

void hash_table_delete(HashTable* ht, int key) {
    if (ht == NULL) return;
    unsigned int index = hash_function(key, ht->capacity);
    
    Node* current = ht->table[index];
    Node* prev = NULL;

    while (current != NULL) {
        if (current->key == key) {
            if (prev == NULL) {
                // Node to be deleted is the head
                ht->table[index] = current->next;
            } else {
                // Node is in the middle or at the end
                prev->next = current->next;
            }
            free(current);
            return; // Key found and deleted
        }
        prev = current;
        current = current->next;
    }
}

void print_hash_table(const HashTable* ht) {
    if (ht == NULL) return;
    printf("---- Hash Table ----\n");
    for (int i = 0; i < ht->capacity; ++i) {
        printf("Bucket %d: ", i);
        Node* current = ht->table[i];
        if (current == NULL) {
            printf("[]\n");
            continue;
        }
        while (current != NULL) {
            printf("[K:%d, V:%d] -> ", current->key, current->value);
            current = current->next;
        }
        printf("NULL\n");
    }
    printf("--------------------\n");
}


int main() {
    HashTable* ht = create_hash_table(10);
    if (ht == NULL) {
        return 1;
    }
    
    // Test Case 1: Insert new key-value pairs
    printf("Test Case 1: Inserting new elements\n");
    hash_table_insert(ht, 1, 10);
    hash_table_insert(ht, 11, 110); // Should collide with key 1 if capacity is 10
    hash_table_insert(ht, 2, 20);
    hash_table_insert(ht, 12, 120);
    print_hash_table(ht);

    // Test Case 2: Search for an existing key
    printf("\nTest Case 2: Searching for key 11\n");
    int value;
    if (hash_table_search(ht, 11, &value)) {
        printf("Value for key 11: %d\n", value);
    } else {
        printf("Value for key 11: Not Found\n");
    }

    // Test Case 3: Search for a non-existing key
    printf("\nTest Case 3: Searching for key 99\n");
    if (hash_table_search(ht, 99, &value)) {
        printf("Value for key 99: %d\n", value);
    } else {
        printf("Value for key 99: Not Found\n");
    }

    // Test Case 4: Update an existing key
    printf("\nTest Case 4: Updating key 1 to value 100\n");
    if (hash_table_search(ht, 1, &value)) {
        printf("Value for key 1 before update: %d\n", value);
    }
    hash_table_insert(ht, 1, 100);
    if (hash_table_search(ht, 1, &value)) {
        printf("Value for key 1 after update: %d\n", value);
    }
    print_hash_table(ht);

    // Test Case 5: Delete a key and verify
    printf("\nTest Case 5: Deleting key 11\n");
    hash_table_delete(ht, 11);
    printf("Searching for key 11 after deletion...\n");
    if (hash_table_search(ht, 11, &value)) {
        printf("Value for key 11: %d\n", value);
    } else {
        printf("Value for key 11: Not Found\n");
    }
    print_hash_table(ht);

    free_hash_table(ht);
    return 0;
}