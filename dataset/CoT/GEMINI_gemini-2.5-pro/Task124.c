#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// This code requires the OpenSSL library.
// To compile: gcc your_file.c -o your_app -lssl -lcrypto
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

// Cryptographic parameters
#define SALT_SIZE 16
#define IV_SIZE 12
#define TAG_SIZE 16
#define KEY_SIZE 32 // 256 bits
#define PBKDF2_ITERATIONS 65536

// A structure to hold the encrypted data components.
// In a real application, you would manage a collection of these.
typedef struct {
    unsigned char* data;
    size_t len;
} EncryptedBlob;

// Helper to handle OpenSSL errors
void handle_openssl_errors() {
    ERR_print_errors_fp(stderr);
    // In a real app, you might abort, log, or handle more gracefully
}

/**
 * Encrypts data using AES-256-GCM with a key derived via PBKDF2.
 * The output blob contains [salt][iv][tag][ciphertext].
 * The caller is responsible for freeing the returned EncryptedBlob->data.
 */
EncryptedBlob* encrypt_data(const char* data, const char* password) {
    unsigned char salt[SALT_SIZE];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        fprintf(stderr, "Failed to generate salt\n");
        return NULL;
    }

    unsigned char key[KEY_SIZE];
    if (PKCS5_PBKDF2_HMAC(password, strlen(password), salt, sizeof(salt), PBKDF2_ITERATIONS, EVP_sha256(), sizeof(key), key) != 1) {
        fprintf(stderr, "Failed to derive key\n");
        return NULL;
    }

    unsigned char iv[IV_SIZE];
    if (RAND_bytes(iv, sizeof(iv)) != 1) {
        fprintf(stderr, "Failed to generate IV\n");
        return NULL;
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        handle_openssl_errors();
        return NULL;
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) {
        handle_openssl_errors();
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, sizeof(iv), NULL) != 1) {
        handle_openssl_errors();
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }
    if (EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv) != 1) {
        handle_openssl_errors();
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }

    int data_len = strlen(data);
    int ciphertext_len = 0;
    int len;
    // Allocate enough space for ciphertext (can be slightly larger than plaintext)
    unsigned char* ciphertext = malloc(data_len); 

    if (EVP_EncryptUpdate(ctx, ciphertext, &len, (const unsigned char*)data, data_len) != 1) {
        handle_openssl_errors();
        free(ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }
    ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        handle_openssl_errors();
        free(ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }
    ciphertext_len += len;

    unsigned char tag[TAG_SIZE];
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, sizeof(tag), tag) != 1) {
        handle_openssl_errors();
        free(ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }
    
    EVP_CIPHER_CTX_free(ctx);
    OPENSSL_cleanse(key, sizeof(key)); // Securely zero the key

    // Combine all parts into a single blob
    size_t total_len = sizeof(salt) + sizeof(iv) + sizeof(tag) + ciphertext_len;
    unsigned char* combined = malloc(total_len);
    memcpy(combined, salt, sizeof(salt));
    memcpy(combined + sizeof(salt), iv, sizeof(iv));
    memcpy(combined + sizeof(salt) + sizeof(iv), tag, sizeof(tag));
    memcpy(combined + sizeof(salt) + sizeof(iv) + sizeof(tag), ciphertext, ciphertext_len);
    
    free(ciphertext);

    EncryptedBlob* blob = malloc(sizeof(EncryptedBlob));
    blob->data = combined;
    blob->len = total_len;
    return blob;
}

/**
 * Decrypts data from a blob created by encrypt_data.
 * Returns a dynamically allocated string with the plaintext.
 * The caller is responsible for freeing the returned string.
 * Returns NULL on any failure (wrong password, tampered data, etc.).
 */
char* decrypt_data(const EncryptedBlob* blob, const char* password) {
    if (blob == NULL || blob->data == NULL || blob->len < SALT_SIZE + IV_SIZE + TAG_SIZE) {
        return NULL;
    }

    const unsigned char* p = blob->data;
    const unsigned char* salt = p;
    p += SALT_SIZE;
    const unsigned char* iv = p;
    p += IV_SIZE;
    const unsigned char* tag = p;
    p += TAG_SIZE;
    const unsigned char* ciphertext = p;
    int ciphertext_len = blob->len - SALT_SIZE - IV_SIZE - TAG_SIZE;

    unsigned char key[KEY_SIZE];
    if (PKCS5_PBKDF2_HMAC(password, strlen(password), salt, SALT_SIZE, PBKDF2_ITERATIONS, EVP_sha256(), sizeof(key), key) != 1) {
        return NULL;
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return NULL;

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) { EVP_CIPHER_CTX_free(ctx); return NULL; }
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, NULL) != 1) { EVP_CIPHER_CTX_free(ctx); return NULL; }
    if (EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv) != 1) { EVP_CIPHER_CTX_free(ctx); return NULL; }

    int plaintext_len;
    int len;
    // Allocate space for plaintext (at most the size of ciphertext)
    char* plaintext = malloc(ciphertext_len + 1);

    if (EVP_DecryptUpdate(ctx, (unsigned char*)plaintext, &len, ciphertext, ciphertext_len) != 1) {
        free(plaintext);
        EVP_CIPHER_CTX_free(ctx);
        OPENSSL_cleanse(key, sizeof(key));
        return NULL;
    }
    plaintext_len = len;

    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, (void*)tag) != 1) {
        free(plaintext);
        EVP_CIPHER_CTX_free(ctx);
        OPENSSL_cleanse(key, sizeof(key));
        return NULL;
    }
    
    int ret = EVP_DecryptFinal_ex(ctx, (unsigned char*)plaintext + len, &len);
    
    EVP_CIPHER_CTX_free(ctx);
    OPENSSL_cleanse(key, sizeof(key));

    if (ret > 0) {
        plaintext_len += len;
        plaintext[plaintext_len] = '\0'; // Null-terminate the string
        return plaintext;
    } else {
        free(plaintext);
        return NULL; // Authentication failed
    }
}

int main() {
    const char* master_password = "aVery!Strong_Password123";

    // This simplified example stores 5 blobs in an array.
    // A real app would use a more dynamic data structure like a hash map.
    EncryptedBlob* vault[5] = {NULL};
    const char* ids[5] = {"CreditCard_1", "SSN_PersonA", "Email_Password", "API_Key_Prod", "MedicalRecord_ID"};
    const char* secrets[5] = {"1111-2222-3333-4444", "987-65-4321", "S3cr3tP@ssw0rd!", "xyz-abc-def-ghi-jkl-mno", "MRN-555-01-1234"};

    printf("--- Test Case 1: Storing multiple sensitive data entries ---\n");
    for (int i = 0; i < 5; ++i) {
        vault[i] = encrypt_data(secrets[i], master_password);
        if (vault[i]) {
            printf("Stored '%s' successfully.\n", ids[i]);
        } else {
            fprintf(stderr, "Failed to store '%s'.\n", ids[i]);
            return 1;
        }
    }
    printf("\n");

    printf("--- Test Case 2: Retrieving one entry with the correct password ---\n");
    char* credit_card = decrypt_data(vault[0], master_password);
    if (credit_card) {
        printf("Retrieved Credit Card: %s\n", credit_card);
        if (strcmp(credit_card, secrets[0]) != 0) { fprintf(stderr, "Test 2 FAILED\n"); return 1; }
        free(credit_card);
    } else {
        fprintf(stderr, "Test 2 FAILED: Decryption failed\n"); return 1;
    }
    printf("\n");

    printf("--- Test Case 3: Attempting to retrieve an entry with the WRONG password ---\n");
    char* failed_retrieval = decrypt_data(vault[1], "wrongPassword");
    printf("Result of retrieval with wrong password: %s\n", failed_retrieval ? failed_retrieval : "[NULL]");
    if (failed_retrieval != NULL) { fprintf(stderr, "Test 3 FAILED\n"); free(failed_retrieval); return 1; }
    printf("As expected, retrieval failed (returned NULL).\n\n");
    
    printf("--- Test Case 4: Attempting to retrieve with a NULL blob ---\n");
    char* non_existent = decrypt_data(NULL, master_password);
    printf("Result of retrieving with NULL blob: %s\n", non_existent ? non_existent : "[NULL]");
    if (non_existent != NULL) { fprintf(stderr, "Test 4 FAILED\n"); free(non_existent); return 1; }
    printf("As expected, retrieval failed (returned NULL).\n\n");
    
    printf("--- Test Case 5: Retrieving all stored entries successfully ---\n");
    for (int i=2; i<5; ++i) {
        char* retrieved = decrypt_data(vault[i], master_password);
        if (retrieved) {
             printf("Retrieved %s: %s\n", ids[i], retrieved);
             free(retrieved);
        } else {
             fprintf(stderr, "Failed to retrieve %s\n", ids[i]);
        }
    }
    printf("All tests completed.\n");

    // Cleanup
    for (int i = 0; i < 5; ++i) {
        if(vault[i]) {
            free(vault[i]->data);
            free(vault[i]);
        }
    }
    
    return 0;
}