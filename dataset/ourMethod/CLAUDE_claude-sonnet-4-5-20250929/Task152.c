
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* Hash table implementation with security hardening */
/* All inputs validated, all allocations checked, all strings null-terminated */

#define MAX_KEY_LENGTH 10000
#define MIN_CAPACITY 16
#define MAX_CAPACITY 1000000

typedef struct Node {
    char* key;           /* Dynamically allocated, null-terminated */
    int value;
    struct Node* next;
} Node;

typedef struct HashTable {
    Node** table;        /* Array of linked list heads */
    size_t capacity;
    size_t count;
} HashTable;

/* Secure string duplication with bounds checking */
static char* safe_strdup(const char* str, size_t max_len) {
    if (str == NULL) {
        return NULL;
    }
    
    /* Validate string length */
    size_t len = strnlen(str, max_len + 1);
    if (len == 0 || len > max_len) {
        return NULL;
    }
    
    /* Allocate with extra byte for null terminator */
    char* copy = (char*)malloc(len + 1);
    if (copy == NULL) {
        return NULL;
    }
    
    /* Safe copy with explicit null termination */
    memcpy(copy, str, len);
    copy[len] = '\\0';
    
    return copy;
}

/* Hash function with overflow protection */
static size_t hash_function(const char* key, size_t capacity) {
    if (key == NULL || capacity == 0) {
        return 0;
    }
    
    size_t hash = 0;
    size_t len = strnlen(key, MAX_KEY_LENGTH + 1);
    
    /* Reject invalid length */
    if (len == 0 || len > MAX_KEY_LENGTH) {
        return 0;
    }
    
    /* Compute hash with modulo to prevent overflow */
    for (size_t i = 0; i < len; i++) {
        hash = (hash * 31 + (unsigned char)key[i]) % capacity;
    }
    
    return hash;
}

/* Validate key input */
static bool is_valid_key(const char* key) {
    if (key == NULL) {
        return false;
    }
    
    size_t len = strnlen(key, MAX_KEY_LENGTH + 1);
    return len > 0 && len <= MAX_KEY_LENGTH;
}

/* Create hash table with validated capacity */
HashTable* hashtable_create(size_t capacity) {
    /* Validate and clamp capacity */
    if (capacity < MIN_CAPACITY) {
        capacity = MIN_CAPACITY;
    }
    if (capacity > MAX_CAPACITY) {
        capacity = MAX_CAPACITY;
    }
    
    /* Allocate hash table structure */
    HashTable* ht = (HashTable*)malloc(sizeof(HashTable));
    if (ht == NULL) {
        return NULL;
    }
    
    /* Initialize all fields */
    ht->capacity = capacity;
    ht->count = 0;
    ht->table = NULL;
    
    /* Allocate table array with overflow check */
    if (capacity > SIZE_MAX / sizeof(Node*)) {
        free(ht);
        return NULL;
    }
    
    ht->table = (Node**)calloc(capacity, sizeof(Node*));
    if (ht->table == NULL) {
        free(ht);
        return NULL;
    }
    
    /* calloc initializes all pointers to NULL */
    return ht;
}

/* Insert or update key-value pair */
bool hashtable_insert(HashTable* ht, const char* key, int value) {
    /* Validate inputs */
    if (ht == NULL || ht->table == NULL || !is_valid_key(key)) {
        return false;
    }
    
    size_t index = hash_function(key, ht->capacity);
    
    /* Bounds check */
    if (index >= ht->capacity) {
        return false;
    }
    
    /* Search for existing key */
    Node* current = ht->table[index];
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            /* Update existing value */
            current->value = value;
            return true;
        }
        current = current->next;
    }
    
    /* Create new node */
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (new_node == NULL) {
        return false;
    }
    
    /* Initialize node fields */
    new_node->key = safe_strdup(key, MAX_KEY_LENGTH);
    if (new_node->key == NULL) {
        free(new_node);
        return false;
    }
    
    new_node->value = value;
    new_node->next = ht->table[index];
    
    /* Insert at head of chain */
    ht->table[index] = new_node;
    ht->count++;
    
    return true;
}

/* Search for key */
bool hashtable_search(const HashTable* ht, const char* key, int* value) {
    /* Validate inputs */
    if (ht == NULL || ht->table == NULL || value == NULL || !is_valid_key(key)) {
        return false;
    }
    
    size_t index = hash_function(key, ht->capacity);
    
    /* Bounds check */
    if (index >= ht->capacity) {
        return false;
    }
    
    /* Search chain */
    Node* current = ht->table[index];
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            *value = current->value;
            return true;
        }
        current = current->next;
    }
    
    return false;
}

/* Delete key from hash table */
bool hashtable_delete(HashTable* ht, const char* key) {
    /* Validate inputs */
    if (ht == NULL || ht->table == NULL || !is_valid_key(key)) {
        return false;
    }
    
    size_t index = hash_function(key, ht->capacity);
    
    /* Bounds check */
    if (index >= ht->capacity) {
        return false;
    }
    
    Node* current = ht->table[index];
    Node* prev = NULL;
    
    /* Search and delete */
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            if (prev == NULL) {
                /* Remove head */
                ht->table[index] = current->next;
            } else {
                /* Remove from middle/end */
                prev->next = current->next;
            }
            
            /* Free memory securely */
            free(current->key);
            free(current);
            ht->count--;
            return true;
        }
        prev = current;
        current = current->next;
    }
    
    return false;
}

/* Get size of hash table */
size_t hashtable_size(const HashTable* ht) {
    return (ht != NULL) ? ht->count : 0;
}

/* Destroy hash table and free all memory */
void hashtable_destroy(HashTable* ht) {
    if (ht == NULL) {
        return;
    }
    
    if (ht->table != NULL) {
        /* Free all chains */
        for (size_t i = 0; i < ht->capacity; i++) {
            Node* current = ht->table[i];
            while (current != NULL) {
                Node* temp = current;
                current = current->next;
                
                /* Free node memory */
                free(temp->key);
                free(temp);
            }
        }
        
        /* Free table array */
        free(ht->table);
    }
    
    /* Free hash table structure */
    free(ht);
}

int main(void) {
    HashTable* ht = hashtable_create(16);
    if (ht == NULL) {
        fprintf(stderr, "Failed to create hash table\\n");
        return 1;
    }
    
    /* Test case 1: Basic insert and search */
    printf("Test 1: Insert and search\\n");
    hashtable_insert(ht, "apple", 100);
    int value = 0;
    if (hashtable_search(ht, "apple", &value)) {
        printf("Found: apple = %d\\n", value);
    } else {
        printf("Not found: apple\\n");
    }
    
    /* Test case 2: Multiple inserts */
    printf("\\nTest 2: Multiple inserts\\n");
    hashtable_insert(ht, "banana", 200);
    hashtable_insert(ht, "cherry", 300);
    hashtable_insert(ht, "date", 400);
    if (hashtable_search(ht, "cherry", &value)) {
        printf("Found: cherry = %d\\n", value);
    }
    
    /* Test case 3: Update existing key */
    printf("\\nTest 3: Update existing key\\n");
    hashtable_insert(ht, "apple", 150);
    if (hashtable_search(ht, "apple", &value)) {
        printf("Updated: apple = %d\\n", value);
    }
    
    /* Test case 4: Delete operation */
    printf("\\nTest 4: Delete operation\\n");
    hashtable_delete(ht, "banana");
    if (!hashtable_search(ht, "banana", &value)) {
        printf("Successfully deleted: banana\\n");
    }
    
    /* Test case 5: Edge cases */
    printf("\\nTest 5: Edge cases\\n");
    if (!hashtable_search(ht, "grape", &value)) {
        printf("Not found: grape (expected)\\n");
    }
    if (!hashtable_search(ht, "", &value)) {
        printf("Empty key rejected (expected)\\n");
    }
    
    printf("\\nTotal items in hash table: %zu\\n", hashtable_size(ht));
    
    /* Clean up all allocated memory */
    hashtable_destroy(ht);
    
    return 0;
}
