#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 16
#define LOAD_FACTOR_THRESHOLD 0.75

typedef struct Node {
    char* key;
    char* value;
    struct Node* next;
} Node;

typedef struct {
    Node** buckets;
    size_t capacity;
    size_t size;
} HashTable;

// djb2 hash function - a well-regarded non-cryptographic hash function
static unsigned long hash_function(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

// Safely duplicates a string, returns NULL on failure
char* safe_strdup(const char* s) {
    if (!s) return NULL;
    char* d = malloc(strlen(s) + 1);
    if (!d) return NULL;
    strcpy(d, s);
    return d;
}

HashTable* create_table(size_t capacity) {
    if (capacity < 1) capacity = INITIAL_CAPACITY;
    HashTable* table = (HashTable*)malloc(sizeof(HashTable));
    if (!table) return NULL;

    table->capacity = capacity;
    table->size = 0;
    table->buckets = (Node**)calloc(table->capacity, sizeof(Node*));
    if (!table->buckets) {
        free(table);
        return NULL;
    }
    return table;
}

void free_table(HashTable* table) {
    if (!table) return;
    for (size_t i = 0; i < table->capacity; i++) {
        Node* node = table->buckets[i];
        while (node) {
            Node* temp = node;
            node = node->next;
            free(temp->key);
            free(temp->value);
            free(temp);
        }
    }
    free(table->buckets);
    free(table);
}

static int ht_rehash(HashTable* table) {
    size_t old_capacity = table->capacity;
    Node** old_buckets = table->buckets;

    size_t new_capacity = old_capacity * 2;
    Node** new_buckets = (Node**)calloc(new_capacity, sizeof(Node*));
    if (!new_buckets) return 0; // Failure

    table->capacity = new_capacity;
    table->buckets = new_buckets;
    table->size = 0; // Size will be recounted on re-insertion

    for (size_t i = 0; i < old_capacity; i++) {
        Node* node = old_buckets[i];
        while (node) {
            // Re-insert into new table (simplified insert logic)
            size_t index = hash_function(node->key) % new_capacity;
            Node* newNode = (Node*)malloc(sizeof(Node));
            if (!newNode) {
                 // In a real-world scenario, you'd need a more robust cleanup
                 // For this example, we'll signal failure and leak memory
                 return 0;
            }
            newNode->key = node->key; // Transfer ownership of strings
            newNode->value = node->value;
            newNode->next = table->buckets[index];
            table->buckets[index] = newNode;
            table->size++;

            Node* temp = node;
            node = node->next;
            free(temp); // Free the old node container
        }
    }
    free(old_buckets);
    return 1; // Success
}

int ht_insert(HashTable* table, const char* key, const char* value) {
    if (!table || !key || !value) return 0;

    if ((double)table->size / table->capacity >= LOAD_FACTOR_THRESHOLD) {
        if (!ht_rehash(table)) return 0; // Rehash failed
    }

    size_t index = hash_function(key) % table->capacity;
    Node* current = table->buckets[index];

    while (current) {
        if (strcmp(current->key, key) == 0) {
            char* new_value = safe_strdup(value);
            if (!new_value) return 0; // Allocation failure
            free(current->value);
            current->value = new_value;
            return 1; // Success (updated)
        }
        current = current->next;
    }

    Node* new_node = (Node*)malloc(sizeof(Node));
    if (!new_node) return 0;
    
    new_node->key = safe_strdup(key);
    new_node->value = safe_strdup(value);
    if (!new_node->key || !new_node->value) {
        free(new_node->key);
        free(new_node->value);
        free(new_node);
        return 0; // Allocation failure
    }

    new_node->next = table->buckets[index];
    table->buckets[index] = new_node;
    table->size++;
    return 1; // Success (inserted)
}

const char* ht_search(HashTable* table, const char* key) {
    if (!table || !key) return NULL;
    size_t index = hash_function(key) % table->capacity;
    Node* current = table->buckets[index];
    while (current) {
        if (strcmp(current->key, key) == 0) {
            return current->value;
        }
        current = current->next;
    }
    return NULL;
}

void ht_delete(HashTable* table, const char* key) {
    if (!table || !key) return;
    size_t index = hash_function(key) % table->capacity;
    Node* current = table->buckets[index];
    Node* prev = NULL;
    while (current) {
        if (strcmp(current->key, key) == 0) {
            if (prev) {
                prev->next = current->next;
            } else {
                table->buckets[index] = current->next;
            }
            free(current->key);
            free(current->value);
            free(current);
            table->size--;
            return;
        }
        prev = current;
        current = current->next;
    }
}


int main() {
    HashTable* ht = create_table(INITIAL_CAPACITY);
    if (!ht) {
        perror("Failed to create hash table");
        return 1;
    }

    // Test Case 1: Basic insertion and search
    printf("--- Test Case 1: Insert and Search ---\n");
    ht_insert(ht, "apple", "10");
    ht_insert(ht, "banana", "20");
    printf("Value for 'apple': %s\n", ht_search(ht, "apple"));
    printf("Value for 'banana': %s\n", ht_search(ht, "banana"));
    printf("\n");

    // Test Case 2: Search for a non-existent key
    printf("--- Test Case 2: Search Non-existent Key ---\n");
    const char* result = ht_search(ht, "cherry");
    printf("Value for 'cherry': %s\n", result ? result : "Not found");
    printf("\n");

    // Test Case 3: Update an existing key
    printf("--- Test Case 3: Update Key ---\n");
    printf("Value for 'apple' before update: %s\n", ht_search(ht, "apple"));
    ht_insert(ht, "apple", "15");
    printf("Value for 'apple' after update: %s\n", ht_search(ht, "apple"));
    printf("\n");

    // Test Case 4: Deletion
    printf("--- Test Case 4: Deletion ---\n");
    printf("Value for 'banana' before deletion: %s\n", ht_search(ht, "banana"));
    ht_delete(ht, "banana");
    result = ht_search(ht, "banana");
    printf("Value for 'banana' after deletion: %s\n", result ? result : "Not found");
    printf("\n");

    // Test Case 5: Trigger rehash
    printf("--- Test Case 5: Trigger Rehash ---\n");
    printf("Initial capacity: %zu\n", ht->capacity);
    // Initial capacity is 16, threshold is 0.75 * 16 = 12.
    // We have 1 element ('apple'). Let's add 11 more to trigger rehash.
    for (int i = 0; i < 12; i++) {
        char key_buf[16];
        char val_buf[16];
        sprintf(key_buf, "key%d", i);
        sprintf(val_buf, "%d", i);
        ht_insert(ht, key_buf, val_buf);
    }
    printf("Size: %zu\n", ht->size);
    printf("Capacity after rehash: %zu\n", ht->capacity); // Expected: 32
    printf("Value for 'key5': %s\n", ht_search(ht, "key5"));
    printf("Value for 'apple': %s\n", ht_search(ht, "apple"));

    free_table(ht);
    return 0;
}