#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

// Note: This code requires the OpenSSL library (version 1.1.1 or newer recommended).
// To compile on Linux/macOS: gcc your_file.c -o rsa_test -lssl -lcrypto
// You may need to install development packages like libssl-dev or openssl-devel.

// Function to generate an RSA key pair wrapped in an EVP_PKEY structure.
EVP_PKEY* generate_keys() {
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    if (!ctx) {
        fprintf(stderr, "EVP_PKEY_CTX_new_id failed.\n");
        return NULL;
    }

    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        fprintf(stderr, "EVP_PKEY_keygen_init failed.\n");
        EVP_PKEY_CTX_free(ctx);
        return NULL;
    }

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0) {
        fprintf(stderr, "EVP_PKEY_CTX_set_rsa_keygen_bits failed.\n");
        EVP_PKEY_CTX_free(ctx);
        return NULL;
    }
    
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        fprintf(stderr, "EVP_PKEY_keygen failed.\n");
        EVP_PKEY_CTX_free(ctx);
        return NULL;
    }

    EVP_PKEY_CTX_free(ctx);
    return pkey;
}

// Encrypts data using the provided public key.
// The caller is responsible for freeing the returned buffer (*encrypted_data).
int rsa_encrypt(const unsigned char* data, size_t data_len, EVP_PKEY* pkey, unsigned char** encrypted_data, size_t* encrypted_len) {
    EVP_PKEY_CTX *ctx = NULL;
    int ret = -1;

    ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (!ctx) goto cleanup;

    if (EVP_PKEY_encrypt_init(ctx) <= 0) goto cleanup;

    // Use OAEP padding for security, which is the recommended standard.
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) goto cleanup;

    // Determine buffer length
    if (EVP_PKEY_encrypt(ctx, NULL, encrypted_len, data, data_len) <= 0) goto cleanup;

    *encrypted_data = malloc(*encrypted_len);
    if (!*encrypted_data) goto cleanup;

    if (EVP_PKEY_encrypt(ctx, *encrypted_data, encrypted_len, data, data_len) <= 0) {
        free(*encrypted_data);
        *encrypted_data = NULL;
        goto cleanup;
    }

    ret = 0; // Success

cleanup:
    if (ctx) EVP_PKEY_CTX_free(ctx);
    if (ret != 0) ERR_print_errors_fp(stderr);
    return ret;
}

// Decrypts data using the provided private key.
// The caller is responsible for freeing the returned buffer (*decrypted_data).
int rsa_decrypt(const unsigned char* encrypted_data, size_t encrypted_len, EVP_PKEY* pkey, unsigned char** decrypted_data, size_t* decrypted_len) {
    EVP_PKEY_CTX *ctx = NULL;
    int ret = -1;

    ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if (!ctx) goto cleanup;

    if (EVP_PKEY_decrypt_init(ctx) <= 0) goto cleanup;

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) goto cleanup;

    // Determine buffer length
    if (EVP_PKEY_decrypt(ctx, NULL, decrypted_len, encrypted_data, encrypted_len) <= 0) goto cleanup;

    *decrypted_data = malloc(*decrypted_len);
    if (!*decrypted_data) goto cleanup;

    if (EVP_PKEY_decrypt(ctx, *decrypted_data, decrypted_len, encrypted_data, encrypted_len) <= 0) {
        free(*decrypted_data);
        *decrypted_data = NULL;
        goto cleanup;
    }

    ret = 0; // Success

cleanup:
    if (ctx) EVP_PKEY_CTX_free(ctx);
    if (ret != 0) ERR_print_errors_fp(stderr);
    return ret;
}

// Helper to print bytes in hex for readability
void print_hex(const char* label, const unsigned char* data, size_t len) {
    printf("%s", label);
    for (size_t i = 0; i < len; ++i) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

int main() {
    EVP_PKEY* pkey = generate_keys();
    if (!pkey) {
        fprintf(stderr, "Key generation failed.\n");
        return 1;
    }

    const char* test_cases[] = {
        "This is a test message.",
        "RSA is a public-key cryptosystem.",
        "Security is important.",
        "12345!@#$%^&*()",
        "A short message."
    };

    int all_tests_passed = 1;

    for (int i = 0; i < 5; ++i) {
        const char* original_data_str = test_cases[i];
        size_t original_len = strlen(original_data_str);
        
        printf("--- Test Case %d ---\n", i + 1);
        printf("Original: %s\n", original_data_str);

        unsigned char* encrypted_data = NULL;
        size_t encrypted_len = 0;
        
        if (rsa_encrypt((const unsigned char*)original_data_str, original_len, pkey, &encrypted_data, &encrypted_len) != 0) {
            fprintf(stderr, "Encryption failed for test case %d.\n", i + 1);
            all_tests_passed = 0;
            continue;
        }
        print_hex("Encrypted (hex): ", encrypted_data, encrypted_len);
        
        unsigned char* decrypted_data = NULL;
        size_t decrypted_len = 0;

        if (rsa_decrypt(encrypted_data, encrypted_len, pkey, &decrypted_data, &decrypted_len) != 0) {
            fprintf(stderr, "Decryption failed for test case %d.\n", i + 1);
            free(encrypted_data);
            all_tests_passed = 0;
            continue;
        }
        
        // Temporarily null-terminate the buffer for printing as a string
        printf("Decrypted: %.*s\n", (int)decrypted_len, (char*)decrypted_data);
        
        if (original_len == decrypted_len && strncmp(original_data_str, (char*)decrypted_data, original_len) == 0) {
            printf("Status: SUCCESS\n");
        } else {
            printf("Status: FAILURE\n");
            all_tests_passed = 0;
        }
        
        printf("\n");

        free(encrypted_data);
        free(decrypted_data);
    }

    EVP_PKEY_free(pkey);

    return all_tests_passed ? 0 : 1;
}