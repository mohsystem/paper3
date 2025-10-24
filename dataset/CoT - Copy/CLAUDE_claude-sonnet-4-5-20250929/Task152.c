
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define TABLE_SIZE 10
#define MAX_KEY_LENGTH 100

typedef struct Entry {
    char key[MAX_KEY_LENGTH];
    int value;
    struct Entry* next;
} Entry;

typedef struct {
    Entry* table[TABLE_SIZE];
} Task152;

int hash_function(const char* key) {
    if (key == NULL || strlen(key) == 0) return 0;
    
    long long hash_value = 0;
    size_t len = strlen(key);
    
    for (size_t i = 0; i < len; i++) {
        hash_value = (hash_value * 31 + (unsigned char)key[i]) % TABLE_SIZE;
    }
    
    return abs((int)hash_value);
}

Task152* create_hash_table() {
    Task152* ht = (Task152*)malloc(sizeof(Task152));
    if (ht == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        return NULL;
    }
    
    for (int i = 0; i < TABLE_SIZE; i++) {
        ht->table[i] = NULL;
    }
    
    return ht;
}

void insert(Task152* ht, const char* key, int value) {
    if (ht == NULL || key == NULL || strlen(key) == 0) {
        printf("Error: Invalid input\\n");
        return;
    }
    
    int index = hash_function(key);
    Entry* current = ht->table[index];
    
    // Update if key exists
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            current->value = value;
            return;
        }
        current = current->next;
    }
    
    // Insert new entry
    Entry* new_entry = (Entry*)malloc(sizeof(Entry));
    if (new_entry == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        return;
    }
    
    strncpy(new_entry->key, key, MAX_KEY_LENGTH - 1);
    new_entry->key[MAX_KEY_LENGTH - 1] = '\\0';
    new_entry->value = value;
    new_entry->next = ht->table[index];
    ht->table[index] = new_entry;
}

int* search(Task152* ht, const char* key) {
    if (ht == NULL || key == NULL || strlen(key) == 0) {
        printf("Error: Invalid input\\n");
        return NULL;
    }
    
    int index = hash_function(key);
    Entry* current = ht->table[index];
    
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            return &current->value;
        }
        current = current->next;
    }
    
    return NULL;
}

bool delete_key(Task152* ht, const char* key) {
    if (ht == NULL || key == NULL || strlen(key) == 0) {
        printf("Error: Invalid input\\n");
        return false;
    }
    
    int index = hash_function(key);
    Entry* current = ht->table[index];
    Entry* prev = NULL;
    
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            if (prev == NULL) {
                ht->table[index] = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            return true;
        }
        prev = current;
        current = current->next;
    }
    
    return false;
}

void free_hash_table(Task152* ht) {
    if (ht == NULL) return;
    
    for (int i = 0; i < TABLE_SIZE; i++) {
        Entry* current = ht->table[i];
        while (current != NULL) {
            Entry* temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(ht);
}

int main() {
    Task152* hashTable = create_hash_table();
    if (hashTable == NULL) return 1;
    
    // Test Case 1: Insert and search
    printf("Test 1: Insert and Search\\n");
    insert(hashTable, "apple", 100);
    insert(hashTable, "banana", 200);
    int* result = search(hashTable, "apple");
    printf("Search 'apple': %s\\n", result ? (sprintf((char[20]){0}, "%d", *result), (char[20]){0}) : "Not found");
    result = search(hashTable, "banana");
    printf("Search 'banana': %d\\n", result ? *result : -1);
    
    // Test Case 2: Update existing key
    printf("\\nTest 2: Update Existing Key\\n");
    insert(hashTable, "apple", 150);
    result = search(hashTable, "apple");
    printf("Search 'apple' after update: %d\\n", result ? *result : -1);
    
    // Test Case 3: Delete operation
    printf("\\nTest 3: Delete Operation\\n");
    printf("Delete 'banana': %s\\n", delete_key(hashTable, "banana") ? "true" : "false");
    result = search(hashTable, "banana");
    printf("Search 'banana' after delete: %s\\n", result ? "Found" : "Not found");
    
    // Test Case 4: Search non-existent key
    printf("\\nTest 4: Search Non-existent Key\\n");
    result = search(hashTable, "orange");
    printf("Search 'orange': %s\\n", result ? "Found" : "Not found");
    
    // Test Case 5: NULL key handling
    printf("\\nTest 5: NULL Key Handling\\n");
    insert(hashTable, NULL, 300);
    result = search(hashTable, NULL);
    delete_key(hashTable, NULL);
    
    free_hash_table(hashTable);
    return 0;
}
