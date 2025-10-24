#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * WARNING: This is a non-secure, educational example of a Key Manager's structure.
 * - It uses a simple XOR operation for "wrapping" keys, which is insecure.
 * - It uses the non-cryptographic rand() function for key generation.
 * - Master key handling is trivial and insecure.
 *
 * In a real-world C application, you MUST use a well-vetted cryptographic
 * library like OpenSSL or libsodium. Do NOT use this code in production.
 */

// A secure way to zero out memory to prevent compilers from optimizing it away.
void secure_zero_memory(void* v, size_t n) {
    volatile unsigned char* p = (volatile unsigned char*)v;
    while (n--) {
        *p++ = 0;
    }
}

// Data structure for the key store (a simple linked list)
typedef struct KeyNode {
    char* alias;
    unsigned char* wrapped_key;
    size_t key_len;
    struct KeyNode* next;
} KeyNode;

// Represents the manager instance
typedef struct KeyManager {
    KeyNode* head;
    unsigned char* master_key;
    size_t master_key_len;
} KeyManager;

// INSECURE: Simple XOR cipher for demonstration.
void xor_cipher(const unsigned char* master_key, size_t master_len, const unsigned char* input, unsigned char* output, size_t data_len) {
    if (master_len == 0) return;
    for (size_t i = 0; i < data_len; ++i) {
        output[i] = input[i] ^ master_key[i % master_len];
    }
}

// Creates and initializes a KeyManager instance.
KeyManager* create_key_manager(const char* password) {
    if (password == NULL || strlen(password) == 0) {
        return NULL;
    }
    KeyManager* manager = (KeyManager*)malloc(sizeof(KeyManager));
    if (!manager) return NULL;
    
    manager->head = NULL;
    manager->master_key_len = strlen(password);
    // INSECURE: Uses password directly as key. Use a KDF in real code.
    manager->master_key = (unsigned char*)malloc(manager->master_key_len);
    if (!manager->master_key) {
        free(manager);
        return NULL;
    }
    memcpy(manager->master_key, password, manager->master_key_len);
    return manager;
}

// Securely destroys the KeyManager and frees all associated memory.
void destroy_key_manager(KeyManager* manager) {
    if (!manager) return;
    
    KeyNode* current = manager->head;
    while (current != NULL) {
        KeyNode* next = current->next;
        free(current->alias);
        secure_zero_memory(current->wrapped_key, current->key_len);
        free(current->wrapped_key);
        free(current);
        current = next;
    }
    
    secure_zero_memory(manager->master_key, manager->master_key_len);
    free(manager->master_key);
    free(manager);
}

// INSECURE: Uses rand(). Use a CSPRNG in real code.
int generate_key(KeyManager* manager, const char* alias, size_t key_size_bytes) {
    if (!manager || !alias || key_size_bytes == 0) return 0;

    // Check for duplicate alias
    KeyNode* current = manager->head;
    while(current != NULL) {
        if (strcmp(current->alias, alias) == 0) return 0; // Alias already exists
        current = current->next;
    }

    unsigned char* new_key = (unsigned char*)malloc(key_size_bytes);
    if (!new_key) return 0;

    for (size_t i = 0; i < key_size_bytes; ++i) {
        new_key[i] = rand() % 256;
    }

    unsigned char* wrapped_key = (unsigned char*)malloc(key_size_bytes);
    if (!wrapped_key) {
        free(new_key);
        return 0;
    }
    
    xor_cipher(manager->master_key, manager->master_key_len, new_key, wrapped_key, key_size_bytes);

    KeyNode* new_node = (KeyNode*)malloc(sizeof(KeyNode));
    if (!new_node) {
        free(new_key);
        free(wrapped_key);
        return 0;
    }
    new_node->alias = strdup(alias);
    new_node->wrapped_key = wrapped_key;
    new_node->key_len = key_size_bytes;
    new_node->next = manager->head;
    manager->head = new_node;

    secure_zero_memory(new_key, key_size_bytes);
    free(new_key);
    return 1;
}

// Caller is responsible for freeing the returned buffer after use.
unsigned char* get_key(KeyManager* manager, const char* alias, size_t* key_len_out) {
    if (!manager || !alias || !key_len_out) return NULL;
    
    KeyNode* current = manager->head;
    while (current != NULL) {
        if (strcmp(current->alias, alias) == 0) {
            unsigned char* unwrapped_key = (unsigned char*)malloc(current->key_len);
            if (!unwrapped_key) return NULL;
            
            xor_cipher(manager->master_key, manager->master_key_len, current->wrapped_key, unwrapped_key, current->key_len);
            *key_len_out = current->key_len;
            return unwrapped_key;
        }
        current = current->next;
    }
    return NULL;
}

int delete_key(KeyManager* manager, const char* alias) {
    if (!manager || !alias) return 0;
    
    KeyNode *current = manager->head, *prev = NULL;
    while (current != NULL && strcmp(current->alias, alias) != 0) {
        prev = current;
        current = current->next;
    }

    if (current == NULL) return 0; // Not found

    if (prev == NULL) { // Head of list
        manager->head = current->next;
    } else {
        prev->next = current->next;
    }

    free(current->alias);
    secure_zero_memory(current->wrapped_key, current->key_len);
    free(current->wrapped_key);
    free(current);
    
    return 1;
}

int main() {
    printf("--- C KeyManager Tests ---\n");
    printf("WARNING: This is a non-secure demonstration.\n");
    srand((unsigned int)time(NULL));

    KeyManager* km = create_key_manager("a-very-strong-master-password");
    if (!km) {
        fprintf(stderr, "Failed to create key manager.\n");
        return 1;
    }

    // Test Case 1: Generate a new key
    printf("1. Generating key 'api-key-1'...\n");
    int generated = generate_key(km, "api-key-1", 32);
    printf("   Key generated: %d\n", generated);
    if (!generated) exit(1);

    // Test Case 2: Retrieve the key
    printf("\n2. Retrieving key 'api-key-1'...\n");
    size_t key1_len;
    unsigned char* key1 = get_key(km, "api-key-1", &key1_len);
    printf("   Key retrieved: %d\n", key1 != NULL);
    if (key1) {
        printf("   Key length (bytes): %zu\n", key1_len);
        secure_zero_memory(key1, key1_len);
        free(key1);
    }
    if (!key1) exit(1);
    
    // Test Case 3: Generate a second key
    printf("\n3. Generating key 'db-key-2'...\n");
    int generated2 = generate_key(km, "db-key-2", 16);
    printf("   Key generated: %d\n", generated2);
    size_t key2_len;
    unsigned char* key2 = get_key(km, "db-key-2", &key2_len);
    printf("   Second key retrieved: %d\n", key2 != NULL);
    if (!generated2 || !key2) exit(1);
    secure_zero_memory(key2, key2_len);
    free(key2);

    // Test Case 4: Delete the first key
    printf("\n4. Deleting key 'api-key-1'...\n");
    int deleted = delete_key(km, "api-key-1");
    printf("   Key deleted: %d\n", deleted);
    if (!deleted) exit(1);

    // Test Case 5: Attempt to retrieve the deleted key
    printf("\n5. Attempting to retrieve deleted key 'api-key-1'...\n");
    size_t deleted_key_len;
    unsigned char* deleted_key = get_key(km, "api-key-1", &deleted_key_len);
    printf("   Key retrieved: %d\n", deleted_key != NULL);
    if (deleted_key != NULL) {
      free(deleted_key);
      exit(1);
    }

    printf("\nAll tests passed.\n");
    
    destroy_key_manager(km);
    return 0;
}