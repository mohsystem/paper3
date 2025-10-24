#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define KEY_SIZE_BYTES 32 // 256 bits

// Node for a linked list to store keys
typedef struct KeyNode {
    char* id;
    unsigned char* key;
    struct KeyNode* next;
} KeyNode;

// Global head of the key store list
KeyNode* g_key_store_head = NULL;

/**
 * Helper to print a byte array as a hexadecimal string.
 * @param data The byte array to print.
 * @param len The length of the array.
 */
void print_hex(const unsigned char* data, size_t len) {
    if (data == NULL) {
        printf("null");
        return;
    }
    for (size_t i = 0; i < len; ++i) {
        printf("%02x", data[i]);
    }
}

/**
 * Generates a new cryptographic key and stores it with the given ID.
 * NOTE: Uses rand() for simplicity, which is NOT cryptographically secure.
 *       In a real application, use a proper cryptographic library (e.g., OpenSSL).
 * @param id The identifier for the key.
 * @return 0 on success, -1 on failure.
 */
int generate_key(const char* id) {
    if (id == NULL || strlen(id) == 0) {
        return -1;
    }
    
    // Check if key already exists
    KeyNode* current = g_key_store_head;
    while(current != NULL) {
        if(strcmp(current->id, id) == 0) {
            // Key already exists, for this simple case we'll overwrite it.
            // A more robust implementation might return an error.
            for (int i = 0; i < KEY_SIZE_BYTES; ++i) {
                current->key[i] = rand() % 256;
            }
            return 0;
        }
        current = current->next;
    }
    
    // Create new node
    KeyNode* new_node = (KeyNode*)malloc(sizeof(KeyNode));
    if (new_node == NULL) return -1;

    new_node->id = (char*)malloc(strlen(id) + 1);
    if (new_node->id == NULL) {
        free(new_node);
        return -1;
    }
    strcpy(new_node->id, id);
    
    new_node->key = (unsigned char*)malloc(KEY_SIZE_BYTES);
    if (new_node->key == NULL) {
        free(new_node->id);
        free(new_node);
        return -1;
    }

    for (int i = 0; i < KEY_SIZE_BYTES; ++i) {
        new_node->key[i] = rand() % 256;
    }

    // Add to head of list
    new_node->next = g_key_store_head;
    g_key_store_head = new_node;

    return 0;
}

/**
 * Retrieves a key by its ID.
 * @param id The identifier for the key.
 * @return A pointer to the key bytes, or NULL if not found.
 */
unsigned char* get_key(const char* id) {
    KeyNode* current = g_key_store_head;
    while (current != NULL) {
        if (strcmp(current->id, id) == 0) {
            return current->key;
        }
        current = current->next;
    }
    return NULL;
}

/**
 * Deletes a key by its ID.
 * @param id The identifier for the key.
 * @return 0 if the key was deleted, -1 if not found.
 */
int delete_key(const char* id) {
    KeyNode* current = g_key_store_head;
    KeyNode* prev = NULL;

    while (current != NULL && strcmp(current->id, id) != 0) {
        prev = current;
        current = current->next;
    }

    if (current == NULL) return -1; // Not found

    if (prev == NULL) { // Head of list
        g_key_store_head = current->next;
    } else {
        prev->next = current->next;
    }

    free(current->id);
    free(current->key);
    free(current);

    return 0;
}

/**
 * Lists all key identifiers currently in the store.
 */
void list_keys() {
    KeyNode* current = g_key_store_head;
    printf("[");
    while (current != NULL) {
        printf("%s", current->id);
        if (current->next != NULL) {
            printf(", ");
        }
        current = current->next;
    }
    printf("]");
}

/**
 * Frees all allocated memory in the key store.
 */
void cleanup_key_store() {
    KeyNode* current = g_key_store_head;
    while (current != NULL) {
        KeyNode* next = current->next;
        free(current->id);
        free(current->key);
        free(current);
        current = next;
    }
    g_key_store_head = NULL;
}

int main() {
    // Seed the pseudo-random number generator
    srand((unsigned int)time(NULL));

    printf("Cryptographic Key Manager - C\n");
    printf("---------------------------------\n");

    // Test Case 1: Generate a key for "user1"
    printf("1. Generating key for 'user1'...\n");
    generate_key("user1");
    unsigned char* key1 = get_key("user1");
    printf("   Generated key for 'user1': ");
    print_hex(key1, KEY_SIZE_BYTES);
    printf("\n\n");

    // Test Case 2: Generate a key for "user2"
    printf("2. Generating key for 'user2'...\n");
    generate_key("user2");
    unsigned char* key2 = get_key("user2");
    printf("   Generated key for 'user2': ");
    print_hex(key2, KEY_SIZE_BYTES);
    printf("\n\n");

    // Test Case 3: Retrieve and print the key for "user1"
    printf("3. Retrieving key for 'user1'...\n");
    unsigned char* retrievedKey1 = get_key("user1");
    printf("   Retrieved key for 'user1': ");
    print_hex(retrievedKey1, KEY_SIZE_BYTES);
    printf("\n\n");

    // Test Case 4: Attempt to retrieve a key for "user3" (non-existent)
    printf("4. Attempting to retrieve non-existent key 'user3'...\n");
    unsigned char* retrievedKey3 = get_key("user3");
    printf("   Retrieved key for 'user3': %s\n\n", (retrievedKey3 == NULL ? "Not Found" : "Found? Error!"));

    // Test Case 5: Delete the key for "user2" and list remaining keys
    printf("5. Deleting key for 'user2' and listing keys...\n");
    int deleted = delete_key("user2");
    printf("   Key 'user2' deleted: %s\n", (deleted == 0 ? "true" : "false"));
    printf("   Current keys in store: ");
    list_keys();
    printf("\n");
    printf("---------------------------------\n");

    // Clean up all allocated memory
    cleanup_key_store();

    return 0;
}