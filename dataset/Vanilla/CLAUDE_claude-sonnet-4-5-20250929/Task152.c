
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct HashNode {
    char* key;
    int value;
    struct HashNode* next;
} HashNode;

typedef struct {
    HashNode** table;
    int capacity;
    int size;
} Task152;

int hash_function(const char* key, int capacity) {
    int hash = 0;
    for (int i = 0; key[i] != '\\0'; i++) {
        hash = (hash * 31 + key[i]) % capacity;
    }
    return abs(hash);
}

Task152* create_hash_table(int capacity) {
    Task152* ht = (Task152*)malloc(sizeof(Task152));
    ht->capacity = capacity;
    ht->size = 0;
    ht->table = (HashNode**)calloc(capacity, sizeof(HashNode*));
    return ht;
}

void insert(Task152* ht, const char* key, int value) {
    int index = hash_function(key, ht->capacity);
    HashNode* current = ht->table[index];
    
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            current->value = value;
            return;
        }
        current = current->next;
    }
    
    HashNode* new_node = (HashNode*)malloc(sizeof(HashNode));
    new_node->key = (char*)malloc(strlen(key) + 1);
    strcpy(new_node->key, key);
    new_node->value = value;
    new_node->next = ht->table[index];
    ht->table[index] = new_node;
    ht->size++;
}

int* search(Task152* ht, const char* key) {
    int index = hash_function(key, ht->capacity);
    HashNode* current = ht->table[index];
    
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            int* result = (int*)malloc(sizeof(int));
            *result = current->value;
            return result;
        }
        current = current->next;
    }
    
    return NULL;
}

bool delete_key(Task152* ht, const char* key) {
    int index = hash_function(key, ht->capacity);
    HashNode* current = ht->table[index];
    HashNode* prev = NULL;
    
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            if (prev == NULL) {
                ht->table[index] = current->next;
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

int get_size(Task152* ht) {
    return ht->size;
}

void destroy_hash_table(Task152* ht) {
    for (int i = 0; i < ht->capacity; i++) {
        HashNode* current = ht->table[i];
        while (current != NULL) {
            HashNode* temp = current;
            current = current->next;
            free(temp->key);
            free(temp);
        }
    }
    free(ht->table);
    free(ht);
}

int main() {
    printf("Test Case 1: Basic insert and search\\n");
    Task152* ht1 = create_hash_table(10);
    insert(ht1, "apple", 100);
    insert(ht1, "banana", 200);
    insert(ht1, "orange", 300);
    int* result1 = search(ht1, "apple");
    printf("Search 'apple': %d\\n", result1 ? *result1 : -1);
    free(result1);
    int* result2 = search(ht1, "banana");
    printf("Search 'banana': %d\\n", result2 ? *result2 : -1);
    free(result2);
    int* result3 = search(ht1, "orange");
    printf("Search 'orange': %d\\n", result3 ? *result3 : -1);
    free(result3);
    
    printf("\\nTest Case 2: Delete operation\\n");
    Task152* ht2 = create_hash_table(10);
    insert(ht2, "cat", 50);
    insert(ht2, "dog", 60);
    int* result4 = search(ht2, "cat");
    printf("Before delete - Search 'cat': %d\\n", result4 ? *result4 : -1);
    free(result4);
    delete_key(ht2, "cat");
    int* result5 = search(ht2, "cat");
    printf("After delete - Search 'cat': %d\\n", result5 ? *result5 : -1);
    free(result5);
    int* result6 = search(ht2, "dog");
    printf("Search 'dog': %d\\n", result6 ? *result6 : -1);
    free(result6);
    
    printf("\\nTest Case 3: Update existing key\\n");
    Task152* ht3 = create_hash_table(10);
    insert(ht3, "key1", 10);
    int* result7 = search(ht3, "key1");
    printf("Initial value: %d\\n", result7 ? *result7 : -1);
    free(result7);
    insert(ht3, "key1", 20);
    int* result8 = search(ht3, "key1");
    printf("Updated value: %d\\n", result8 ? *result8 : -1);
    free(result8);
    
    printf("\\nTest Case 4: Search non-existent key\\n");
    Task152* ht4 = create_hash_table(10);
    insert(ht4, "exist", 100);
    int* result9 = search(ht4, "exist");
    printf("Search 'exist': %d\\n", result9 ? *result9 : -1);
    free(result9);
    int* result10 = search(ht4, "notexist");
    printf("Search 'notexist': %d\\n", result10 ? *result10 : -1);
    free(result10);
    
    printf("\\nTest Case 5: Multiple operations\\n");
    Task152* ht5 = create_hash_table(10);
    insert(ht5, "a", 1);
    insert(ht5, "b", 2);
    insert(ht5, "c", 3);
    printf("Size: %d\\n", get_size(ht5));
    delete_key(ht5, "b");
    printf("After delete, Size: %d\\n", get_size(ht5));
    int* result11 = search(ht5, "a");
    printf("Search 'a': %d\\n", result11 ? *result11 : -1);
    free(result11);
    int* result12 = search(ht5, "b");
    printf("Search 'b': %d\\n", result12 ? *result12 : -1);
    free(result12);
    int* result13 = search(ht5, "c");
    printf("Search 'c': %d\\n", result13 ? *result13 : -1);
    free(result13);
    
    destroy_hash_table(ht1);
    destroy_hash_table(ht2);
    destroy_hash_table(ht3);
    destroy_hash_table(ht4);
    destroy_hash_table(ht5);
    
    return 0;
}
