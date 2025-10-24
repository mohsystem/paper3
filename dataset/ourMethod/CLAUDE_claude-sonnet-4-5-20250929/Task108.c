
// C implementation with secure remote object manipulation
// Uses encryption and input validation to protect object state

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <curl/curl.h>

#define MAX_OBJECTS 100
#define MAX_ID_LENGTH 256
#define MAX_DATA_LENGTH 1048576
#define SALT_SIZE 16
#define IV_SIZE 12
#define TAG_SIZE 16
#define KEY_SIZE 32
#define ITERATIONS 210000

// Secure object structure
typedef struct {
    char id[MAX_ID_LENGTH + 1];
    uint8_t *encrypted_state;
    size_t encrypted_state_len;
    uint8_t salt[SALT_SIZE];
    uint8_t iv[IV_SIZE];
    uint8_t tag[TAG_SIZE];
    int is_active;
} SecureObject;

// Server structure
typedef struct {
    SecureObject objects[MAX_OBJECTS];
    char server_passphrase[256];
    size_t object_count;
} SecureObjectServer;

// Securely clear memory
void secure_clear(void *ptr, size_t len) {
    if (ptr != NULL && len > 0) {
        OPENSSL_cleanse(ptr, len);
    }
}

// Derive key using PBKDF2
int derive_key(const char *passphrase, const uint8_t *salt, uint8_t *key) {
    if (passphrase == NULL || salt == NULL || key == NULL) {
        return 0;
    }
    
    size_t pass_len = strlen(passphrase);
    if (pass_len == 0 || pass_len > 255) {
        return 0;
    }
    
    if (PKCS5_PBKDF2_HMAC(passphrase, (int)pass_len, salt, SALT_SIZE,
                          ITERATIONS, EVP_sha256(), KEY_SIZE, key) != 1) {
        return 0;
    }
    
    return 1;
}

// Encrypt data using AES-256-GCM
int encrypt_data(const char *plaintext, size_t plaintext_len,
                 const char *passphrase, SecureObject *obj) {
    if (plaintext == NULL || passphrase == NULL || obj == NULL) {
        return 0;
    }
    
    if (plaintext_len == 0 || plaintext_len > MAX_DATA_LENGTH) {
        return 0;
    }
    
    // Generate salt and IV
    if (RAND_bytes(obj->salt, SALT_SIZE) != 1 ||
        RAND_bytes(obj->iv, IV_SIZE) != 1) {
        return 0;
    }
    
    // Derive key
    uint8_t key[KEY_SIZE];
    if (!derive_key(passphrase, obj->salt, key)) {
        secure_clear(key, KEY_SIZE);
        return 0;
    }
    
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        secure_clear(key, KEY_SIZE);
        return 0;
    }
    
    int success = 0;
    int len = 0;
    
    do {
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, obj->iv) != 1) {
            break;
        }
        
        // Allocate buffer for ciphertext
        size_t max_ciphertext_len = plaintext_len + EVP_CIPHER_block_size(EVP_aes_256_gcm());
        obj->encrypted_state = (uint8_t *)malloc(max_ciphertext_len);
        if (obj->encrypted_state == NULL) {
            break;
        }
        
        int ciphertext_len = 0;
        if (EVP_EncryptUpdate(ctx, obj->encrypted_state, &len,
                             (const uint8_t *)plaintext, (int)plaintext_len) != 1) {
            free(obj->encrypted_state);
            obj->encrypted_state = NULL;
            break;
        }
        ciphertext_len = len;
        
        if (EVP_EncryptFinal_ex(ctx, obj->encrypted_state + len, &len) != 1) {
            free(obj->encrypted_state);
            obj->encrypted_state = NULL;
            break;
        }
        ciphertext_len += len;
        obj->encrypted_state_len = (size_t)ciphertext_len;
        
        // Get authentication tag
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, obj->tag) != 1) {
            free(obj->encrypted_state);
            obj->encrypted_state = NULL;
            obj->encrypted_state_len = 0;
            break;
        }
        
        success = 1;
    } while (0);
    
    secure_clear(key, KEY_SIZE);
    EVP_CIPHER_CTX_free(ctx);
    
    return success;
}

// Decrypt data using AES-256-GCM
int decrypt_data(const SecureObject *obj, const char *passphrase,
                 char *plaintext, size_t *plaintext_len) {
    if (obj == NULL || passphrase == NULL || plaintext == NULL || plaintext_len == NULL) {
        return 0;
    }
    
    if (obj->encrypted_state == NULL || obj->encrypted_state_len == 0) {
        return 0;
    }
    
    // Derive key
    uint8_t key[KEY_SIZE];
    if (!derive_key(passphrase, obj->salt, key)) {
        secure_clear(key, KEY_SIZE);
        return 0;
    }
    
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        secure_clear(key, KEY_SIZE);
        return 0;
    }
    
    int success = 0;
    int len = 0;
    
    do {
        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, obj->iv) != 1) {
            break;
        }
        
        int decrypted_len = 0;
        if (EVP_DecryptUpdate(ctx, (uint8_t *)plaintext, &len,
                             obj->encrypted_state, (int)obj->encrypted_state_len) != 1) {
            break;
        }
        decrypted_len = len;
        
        // Set expected tag
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE,
                               (void *)obj->tag) != 1) {
            break;
        }
        
        // Verify tag during finalization
        if (EVP_DecryptFinal_ex(ctx, (uint8_t *)plaintext + len, &len) != 1) {
            secure_clear(plaintext, decrypted_len);
            break;
        }
        decrypted_len += len;
        
        *plaintext_len = (size_t)decrypted_len;
        plaintext[decrypted_len] = '\\0';
        success = 1;
    } while (0);
    
    secure_clear(key, KEY_SIZE);
    EVP_CIPHER_CTX_free(ctx);
    
    return success;
}

// Validate object ID (alphanumeric and underscore only)
int is_valid_object_id(const char *id) {
    if (id == NULL) return 0;
    
    size_t len = strlen(id);
    if (len == 0 || len > MAX_ID_LENGTH) return 0;
    
    for (size_t i = 0; i < len; i++) {
        if (!isalnum((unsigned char)id[i]) && id[i] != '_') {
            return 0;
        }
    }
    
    return 1;
}

// Validate operation
int is_valid_operation(const char *op) {
    if (op == NULL) return 0;
    
    return (strcmp(op, "create") == 0 ||
            strcmp(op, "read") == 0 ||
            strcmp(op, "update") == 0 ||
            strcmp(op, "delete") == 0);
}

// Initialize server
void init_server(SecureObjectServer *server, const char *passphrase) {
    if (server == NULL || passphrase == NULL) return;
    
    memset(server, 0, sizeof(SecureObjectServer));
    
    size_t pass_len = strlen(passphrase);
    if (pass_len < 16 || pass_len > 255) {
        fprintf(stderr, "Error: Invalid passphrase length\\n");
        return;
    }
    
    strncpy(server->server_passphrase, passphrase, sizeof(server->server_passphrase) - 1);
    server->server_passphrase[sizeof(server->server_passphrase) - 1] = '\\0';
    server->object_count = 0;
}

// Find object by ID
SecureObject *find_object(SecureObjectServer *server, const char *id) {
    if (server == NULL || id == NULL) return NULL;
    
    for (size_t i = 0; i < MAX_OBJECTS; i++) {
        if (server->objects[i].is_active &&
            strcmp(server->objects[i].id, id) == 0) {
            return &server->objects[i];
        }
    }
    
    return NULL;
}

// Process remote request
void process_request(SecureObjectServer *server, const char *operation,
                    const char *object_id, const char *data,
                    char *response, size_t response_size) {
    if (server == NULL || operation == NULL || object_id == NULL ||
        response == NULL || response_size == 0) {
        return;
    }
    
    // Validate inputs
    if (!is_valid_operation(operation) || !is_valid_object_id(object_id)) {
        snprintf(response, response_size, "ERROR: Invalid request parameters");
        return;
    }
    
    if (data != NULL && strlen(data) > MAX_DATA_LENGTH) {
        snprintf(response, response_size, "ERROR: Data too large");
        return;
    }
    
    if (strcmp(operation, "create") == 0) {
        if (find_object(server, object_id) != NULL) {
            snprintf(response, response_size, "ERROR: Object already exists");
            return;
        }
        
        if (server->object_count >= MAX_OBJECTS) {
            snprintf(response, response_size, "ERROR: Maximum objects reached");
            return;
        }
        
        // Find empty slot
        SecureObject *obj = NULL;
        for (size_t i = 0; i < MAX_OBJECTS; i++) {
            if (!server->objects[i].is_active) {
                obj = &server->objects[i];
                break;
            }
        }
        
        if (obj == NULL) {
            snprintf(response, response_size, "ERROR: No available slots");
            return;
        }
        
        strncpy(obj->id, object_id, MAX_ID_LENGTH);
        obj->id[MAX_ID_LENGTH] = '\\0';
        
        if (!encrypt_data(data != NULL ? data : "", data != NULL ? strlen(data) : 0,
                         server->server_passphrase, obj)) {
            snprintf(response, response_size, "ERROR: Failed to encrypt object state");
            return;
        }
        
        obj->is_active = 1;
        server->object_count++;
        snprintf(response, response_size, "SUCCESS: Object created");
        
    } else if (strcmp(operation, "read") == 0) {
        SecureObject *obj = find_object(server, object_id);
        if (obj == NULL) {
            snprintf(response, response_size, "ERROR: Object not found");
            return;
        }
        
        char plaintext[MAX_DATA_LENGTH + 1];
        size_t plaintext_len = 0;
        
        if (!decrypt_data(obj, server->server_passphrase, plaintext, &plaintext_len)) {
            snprintf(response, response_size, "ERROR: Failed to decrypt object state");
            return;
        }
        
        snprintf(response, response_size, "SUCCESS: %s", plaintext);
        secure_clear(plaintext, sizeof(plaintext));
        
    } else if (strcmp(operation, "update") == 0) {
        SecureObject *obj = find_object(server, object_id);
        if (obj == NULL) {
            snprintf(response, response_size, "ERROR: Object not found");
            return;
        }
        
        // Free old encrypted state
        if (obj->encrypted_state != NULL) {
            secure_clear(obj->encrypted_state, obj->encrypted_state_len);
            free(obj->encrypted_state);
            obj->encrypted_state = NULL;
            obj->encrypted_state_len = 0;
        }
        
        if (!encrypt_data(data != NULL ? data : "", data != NULL ? strlen(data) : 0,
                         server->server_passphrase, obj)) {
            snprintf(response, response_size, "ERROR: Failed to update object state");
            return;
        }
        
        snprintf(response, response_size, "SUCCESS: Object updated");
        
    } else if (strcmp(operation, "delete") == 0) {
        SecureObject *obj = find_object(server, object_id);
        if (obj == NULL) {
            snprintf(response, response_size, "ERROR: Object not found");
            return;
        }
        
        if (obj->encrypted_state != NULL) {
            secure_clear(obj->encrypted_state, obj->encrypted_state_len);
            free(obj->encrypted_state);
            obj->encrypted_state = NULL;
        }
        
        secure_clear(obj, sizeof(SecureObject));
        server->object_count--;
        snprintf(response, response_size, "SUCCESS: Object deleted");
        
    } else {
        snprintf(response, response_size, "ERROR: Unknown operation");
    }
}

// Cleanup server
void cleanup_server(SecureObjectServer *server) {
    if (server == NULL) return;
    
    for (size_t i = 0; i < MAX_OBJECTS; i++) {
        if (server->objects[i].encrypted_state != NULL) {
            secure_clear(server->objects[i].encrypted_state,
                        server->objects[i].encrypted_state_len);
            free(server->objects[i].encrypted_state);
        }
    }
    
    secure_clear(server, sizeof(SecureObjectServer));
}

int main(void) {
    printf("=== Secure Remote Object Manipulation System (C) ===\\n");
    
    SecureObjectServer server;
    char response[4096];
    
    // Initialize with strong passphrase
    init_server(&server, "SecureServerPass123!@#$%^&*()");
    
    printf("\\n--- Test Case 1: Create Object ---\\n");
    process_request(&server, "create", "user_001", "John Doe, Age: 30",
                   response, sizeof(response));
    printf("Create user_001: %s\\n", response);
    
    printf("\\n--- Test Case 2: Read Object ---\\n");
    process_request(&server, "read", "user_001", NULL, response, sizeof(response));
    printf("Read user_001: %s\\n", response);
    
    printf("\\n--- Test Case 3: Update Object ---\\n");
    process_request(&server, "update", "user_001", "John Doe, Age: 31",
                   response, sizeof(response));
    printf("Update user_001: %s\\n", response);
    
    printf("\\n--- Test Case 4: Invalid Operation ---\\n");
    process_request(&server, "hack", "user_001", "malicious", response, sizeof(response));
    printf("Invalid operation: %s\\n", response);
    
    printf("\\n--- Test Case 5: Delete Object ---\\n");
    process_request(&server, "delete", "user_001", NULL, response, sizeof(response));
    printf("Delete user_001: %s\\n", response);
    
    printf("\\n--- Additional Security Tests ---\\n");
    
    process_request(&server, "create", "../../../etc/passwd", "data",
                   response, sizeof(response));
    printf("Path traversal attempt: %s\\n", response);
    
    printf("\\nFinal object count: %zu\\n", server.object_count);
    
    cleanup_server(&server);
    
    printf("\\n=== All tests completed successfully ===\\n");
    
    return 0;
}
