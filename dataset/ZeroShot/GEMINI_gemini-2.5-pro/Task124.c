/*
 * DISCLAIMER: This is a demonstration of cryptographic concepts.
 * It is NOT a secure system for storing sensitive data in a production environment.
 * Real-world security requires a much more comprehensive approach, including secure
 * key management, which is not possible in this single-file example.
 *
 * COMPILE: gcc -Wall this_file.c -o task_c -lssl -lcrypto
 * You MUST have OpenSSL development libraries installed (e.g., `libssl-dev` on Debian/Ubuntu).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/kdf.h>

// --- Constants ---
#define AES_KEY_SIZE_BITS 256
#define AES_KEY_SIZE_BYTES (AES_KEY_SIZE_BITS / 8)
#define GCM_IV_LENGTH 12
#define GCM_TAG_LENGTH 16
#define SALT_LENGTH 16
#define PBKDF2_ITERATIONS 600000

// Helper to clean up and exit on error
void handle_openssl_error(const char* msg) {
    fprintf(stderr, "OpenSSL Error: %s\n", msg);
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
}

// Base64 encode. Caller must free the returned string.
char* base64_encode(const unsigned char* input, int length) {
    BIO *b64, *bmem;
    BUF_MEM *bptr;
    
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);

    if (BIO_write(b64, input, length) <= 0) {
        BIO_free_all(b64);
        return NULL;
    }
    BIO_flush(b64);

    BIO_get_mem_ptr(b64, &bptr);
    char* buff = (char*)malloc(bptr->length + 1);
    if (buff) {
        memcpy(buff, bptr->data, bptr->length);
        buff[bptr->length] = 0;
    }
    BIO_free_all(b64);
    return buff;
}

// Base64 decode. Caller must free the returned buffer.
unsigned char* base64_decode(const char* input, int* out_len) {
    BIO *b64, *bmem;
    int len = strlen(input);
    unsigned char* buffer = (unsigned char*)malloc(len);
    if (!buffer) return NULL;

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bmem = BIO_new_mem_buf(input, len);
    bmem = BIO_push(b64, bmem);

    *out_len = BIO_read(bmem, buffer, len);
    BIO_free_all(bmem);

    if (*out_len <= 0) {
        free(buffer);
        return NULL;
    }
    return buffer;
}


int encrypt_data(const char* plain_text, const char* password, char** encrypted_out) {
    int ret = -1;
    unsigned char salt[SALT_LENGTH];
    unsigned char iv[GCM_IV_LENGTH];
    unsigned char key[AES_KEY_SIZE_BYTES];
    unsigned char* ciphertext = NULL;
    unsigned char tag[GCM_TAG_LENGTH];
    
    EVP_CIPHER_CTX* ctx = NULL;

    if (RAND_bytes(salt, sizeof(salt)) != 1) goto cleanup;
    if (PKCS5_PBKDF2_HMAC(password, strlen(password), salt, sizeof(salt), PBKDF2_ITERATIONS, EVP_sha256(), sizeof(key), key) != 1) goto cleanup;
    if (RAND_bytes(iv, sizeof(iv)) != 1) goto cleanup;

    ctx = EVP_CIPHER_CTX_new();
    if (!ctx) goto cleanup;
    
    EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, sizeof(iv), NULL);
    EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv);

    int plain_text_len = strlen(plain_text);
    ciphertext = malloc(plain_text_len);
    if(!ciphertext) goto cleanup;
    
    int len;
    EVP_EncryptUpdate(ctx, ciphertext, &len, (const unsigned char*)plain_text, plain_text_len);
    int ciphertext_len = len;

    EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
    ciphertext_len += len;

    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, sizeof(tag), tag);

    // Format: base64(salt):base64(iv):base64(tag):base64(ciphertext)
    char* salt_b64 = base64_encode(salt, sizeof(salt));
    char* iv_b64 = base64_encode(iv, sizeof(iv));
    char* tag_b64 = base64_encode(tag, sizeof(tag));
    char* ciphertext_b64 = base64_encode(ciphertext, ciphertext_len);

    size_t total_len = strlen(salt_b64) + strlen(iv_b64) + strlen(tag_b64) + strlen(ciphertext_b64) + 4;
    *encrypted_out = malloc(total_len);
    if (*encrypted_out) {
        snprintf(*encrypted_out, total_len, "%s:%s:%s:%s", salt_b64, iv_b64, tag_b64, ciphertext_b64);
        ret = 0; // Success
    }

    free(salt_b64);
    free(iv_b64);
    free(tag_b64);
    free(ciphertext_b64);

cleanup:
    if (ctx) EVP_CIPHER_CTX_free(ctx);
    if (ciphertext) free(ciphertext);
    return ret;
}


int decrypt_data(const char* encrypted_data, const char* password, char** decrypted_out) {
    int ret = -1;
    char* data_copy = strdup(encrypted_data);
    if(!data_copy) return -1;
    
    unsigned char key[AES_KEY_SIZE_BYTES];
    unsigned char* salt = NULL, *iv = NULL, *tag = NULL, *ciphertext = NULL;
    int salt_len, iv_len, tag_len, ciphertext_len;
    EVP_CIPHER_CTX* ctx = NULL;

    // Parse input string
    char* s_salt_b64 = strtok(data_copy, ":");
    char* s_iv_b64 = strtok(NULL, ":");
    char* s_tag_b64 = strtok(NULL, ":");
    char* s_ciphertext_b64 = strtok(NULL, ":");
    if (!s_salt_b64 || !s_iv_b64 || !s_tag_b64 || !s_ciphertext_b64) goto cleanup;

    salt = base64_decode(s_salt_b64, &salt_len);
    iv = base64_decode(s_iv_b64, &iv_len);
    tag = base64_decode(s_tag_b64, &tag_len);
    ciphertext = base64_decode(s_ciphertext_b64, &ciphertext_len);
    if (!salt || !iv || !tag || !ciphertext) goto cleanup;

    if (PKCS5_PBKDF2_HMAC(password, strlen(password), salt, salt_len, PBKDF2_ITERATIONS, EVP_sha256(), sizeof(key), key) != 1) goto cleanup;
    
    ctx = EVP_CIPHER_CTX_new();
    if (!ctx) goto cleanup;

    EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv_len, NULL);
    EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv);
    
    *decrypted_out = malloc(ciphertext_len + 1);
    if(!*decrypted_out) goto cleanup;

    int len;
    EVP_DecryptUpdate(ctx, (unsigned char*)*decrypted_out, &len, ciphertext, ciphertext_len);
    int plaintext_len = len;

    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, tag_len, tag);

    if (EVP_DecryptFinal_ex(ctx, (unsigned char*)*decrypted_out + len, &len) > 0) {
        plaintext_len += len;
        (*decrypted_out)[plaintext_len] = '\0';
        ret = 0; // Success
    } else {
        free(*decrypted_out); // Decryption failed
        *decrypted_out = NULL;
    }

cleanup:
    free(data_copy);
    if(salt) free(salt);
    if(iv) free(iv);
    if(tag) free(tag);
    if(ciphertext) free(ciphertext);
    if(ctx) EVP_CIPHER_CTX_free(ctx);
    return ret;
}

int main() {
    printf("Running C Tests...\n");
    const char* password = "a-very-strong-password-!@#$";
    const char* test_data[] = {
        "1234-5678-9012-3456", // Test Case 1: Credit Card
        "{\"name\":\"John Doe\", \"ssn\":\"000-00-0000\"}", // Test Case 2: JSON data
        "My secret diary entry.", // Test Case 3: Simple text
        "", // Test Case 4: Empty string
        "Data with special characters: <>?`|~" // Test Case 5: Special characters
    };
    int num_tests = sizeof(test_data) / sizeof(test_data[0]);

    for (int i = 0; i < num_tests; ++i) {
        const char* original_data = test_data[i];
        printf("\n--- Test Case %d ---\n", i + 1);
        printf("Original:    %s\n", original_data);
        
        char* encrypted = NULL;
        if (encrypt_data(original_data, password, &encrypted) == 0) {
            printf("Encrypted:   %s\n", encrypted);
            
            char* decrypted = NULL;
            if (decrypt_data(encrypted, password, &decrypted) == 0) {
                printf("Decrypted:   %s\n", decrypted);
                if (strcmp(original_data, decrypted) == 0) {
                    printf("SUCCESS: Data integrity verified.\n");
                } else {
                    printf("FAILURE: Decrypted data does not match original data.\n");
                }
                free(decrypted);
            } else {
                printf("FAILURE: Decryption with correct password failed.\n");
            }

            printf("Testing with incorrect password...\n");
            char* bad_decryption = NULL;
            if (decrypt_data(encrypted, "wrong-password", &bad_decryption) != 0) {
                printf("SUCCESS: Decryption failed as expected with wrong password.\n");
            } else {
                printf("FAILURE: Decryption with wrong password should have failed but didn't.\n");
                free(bad_decryption);
            }
            free(encrypted);
        } else {
            fprintf(stderr, "FAILURE: Encryption failed for test case %d.\n", i + 1);
        }
    }

    return 0;
}