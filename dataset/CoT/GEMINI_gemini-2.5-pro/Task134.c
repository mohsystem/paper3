#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/rand.h>
#include <openssl/err.h>

// NOTE: This is a demonstration of cryptographic key management concepts.
// Storing raw key material in application memory is NOT a secure practice
// for production systems. Production systems should use Hardware Security
// Modules (HSMs) or dedicated key management services.
//
// To compile: gcc your_file_name.c -o your_program_name -lssl -lcrypto

typedef struct KeyNode {
    char* name;
    unsigned char* key;
    size_t key_len;
    struct KeyNode* next;
} KeyNode;

// Global head of the key store linked list
KeyNode* g_key_store = NULL;

// Helper to print keys in hex format
void print_hex_c(const unsigned char* data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

/**
 * Generates a new key using OpenSSL's cryptographically secure RNG.
 * @param keyName The alias to store the key under.
 * @param keySizeInBytes The size of the key in bytes.
 * @return 0 on success, -1 on failure.
 */
int generate_key(const char* keyName, size_t keySizeInBytes) {
    if (keyName == NULL || strlen(keyName) == 0) {
        fprintf(stderr, "Error: Key name cannot be empty.\n");
        return -1;
    }
    if (keySizeInBytes == 0) {
        fprintf(stderr, "Error: Key size must be greater than zero.\n");
        return -1;
    }

    // Check if key already exists
    for (KeyNode* current = g_key_store; current != NULL; current = current->next) {
        if (strcmp(current->name, keyName) == 0) {
            fprintf(stderr, "Error: Key with name '%s' already exists.\n", keyName);
            return -1;
        }
    }

    KeyNode* newNode = (KeyNode*)malloc(sizeof(KeyNode));
    if (!newNode) {
        perror("Failed to allocate memory for key node");
        return -1;
    }

    newNode->name = strdup(keyName);
    if (!newNode->name) {
        perror("Failed to duplicate key name");
        free(newNode);
        return -1;
    }

    newNode->key = (unsigned char*)malloc(keySizeInBytes);
    if (!newNode->key) {
        perror("Failed to allocate memory for key material");
        free(newNode->name);
        free(newNode);
        return -1;
    }
    newNode->key_len = keySizeInBytes;

    if (RAND_bytes(newNode->key, keySizeInBytes) != 1) {
        char err_buf[256];
        ERR_error_string_n(ERR_get_error(), err_buf, sizeof(err_buf));
        fprintf(stderr, "Failed to generate random bytes: %s\n", err_buf);
        free(newNode->key);
        free(newNode->name);
        free(newNode);
        return -1;
    }

    // Add to the front of the list
    newNode->next = g_key_store;
    g_key_store = newNode;

    return 0;
}

/**
 * Retrieves a copy of the key material for the given key name.
 * NOTE: The caller is responsible for freeing the memory allocated for `key_out`.
 * @param keyName The alias of the key to retrieve.
 * @param key_out Pointer to a buffer that will hold the copied key.
 * @param key_len_out Pointer to a size_t that will hold the key length.
 * @return 0 on success, -1 if key is not found.
 */
int get_key(const char* keyName, unsigned char** key_out, size_t* key_len_out) {
    for (KeyNode* current = g_key_store; current != NULL; current = current->next) {
        if (strcmp(current->name, keyName) == 0) {
            *key_out = (unsigned char*)malloc(current->key_len);
            if (*key_out == NULL) {
                perror("Failed to allocate memory for key copy");
                return -1;
            }
            memcpy(*key_out, current->key, current->key_len);
            *key_len_out = current->key_len;
            return 0;
        }
    }
    return -1; // Not found
}

/**
 * Deletes a key from the store and overwrites its memory.
 * @param keyName The alias of the key to delete.
 * @return 0 if the key was found and deleted, -1 otherwise.
 */
int delete_key(const char* keyName) {
    KeyNode* current = g_key_store;
    KeyNode* prev = NULL;

    while (current != NULL && strcmp(current->name, keyName) != 0) {
        prev = current;
        current = current->next;
    }

    if (current == NULL) {
        return -1; // Not found
    }

    if (prev == NULL) { // Deleting the head node
        g_key_store = current->next;
    } else {
        prev->next = current->next;
    }

    // Securely wipe memory before freeing
    memset(current->key, 0, current->key_len);
    free(current->key);
    free(current->name);
    free(current);

    return 0;
}

void list_keys() {
    printf("Current keys: ");
    if (g_key_store == NULL) {
        printf("(none)");
    } else {
        for (KeyNode* current = g_key_store; current != NULL; current = current->next) {
            printf("%s ", current->name);
        }
    }
    printf("\n");
}

void cleanup_store() {
    KeyNode* current = g_key_store;
    while (current != NULL) {
        KeyNode* next = current->next;
        // Securely wipe memory
        memset(current->key, 0, current->key_len);
        free(current->key);
        free(current->name);
        free(current);
        current = next;
    }
    g_key_store = NULL;
}

void Task134_run() {
    printf("--- C Key Manager Test ---\n");

    // Test Case 1: Generate a new key
    printf("\n1. Generating a 16-byte (128-bit) key named 'api_key_1'...\n");
    int result = generate_key("api_key_1", 16);
    printf("Key generated successfully: %s\n", result == 0 ? "true" : "false");
    list_keys();

    // Test Case 2: Retrieve the key
    printf("\n2. Retrieving key 'api_key_1'...\n");
    unsigned char* retrieved_key = NULL;
    size_t key_len = 0;
    if (get_key("api_key_1", &retrieved_key, &key_len) == 0) {
        printf("Retrieved key (Hex): ");
        print_hex_c(retrieved_key, key_len);
        printf("Retrieved key length: %zu bits\n", key_len * 8);
        free(retrieved_key); // IMPORTANT: Caller must free memory
    } else {
        printf("Failed to retrieve key.\n");
    }

    // Test Case 3: Attempt to retrieve a non-existent key
    printf("\n3. Attempting to retrieve non-existent key 'fake_key'...\n");
    if (get_key("fake_key", &retrieved_key, &key_len) == -1) {
        printf("Key not found (as expected).\n");
    } else {
        printf("FAIL: Retrieved 'fake_key' unexpectedly.\n");
        free(retrieved_key);
    }

    // Test Case 4: Delete the key
    printf("\n4. Deleting key 'api_key_1'...\n");
    result = delete_key("api_key_1");
    printf("Key deleted successfully: %s\n", result == 0 ? "true" : "false");
    list_keys();

    // Test Case 5: Attempt to retrieve the deleted key
    printf("\n5. Attempting to retrieve deleted key 'api_key_1'...\n");
    if (get_key("api_key_1", &retrieved_key, &key_len) == -1) {
        printf("Key not found (as expected).\n");
    } else {
        printf("FAIL: Retrieved 'api_key_1' after deletion.\n");
        free(retrieved_key);
    }

    // Generate another key to test cleanup
    generate_key("temp_key", 32);
    printf("\nGenerated a second key for cleanup test.\n");
    list_keys();
}

int main() {
    Task134_run();
    cleanup_store(); // Clean up all remaining keys at the end
    return 0;
}