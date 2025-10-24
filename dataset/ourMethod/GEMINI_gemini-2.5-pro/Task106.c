#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/err.h>

void handle_openssl_error() {
    fprintf(stderr, "OpenSSL error:\n");
    ERR_print_errors_fp(stderr);
}

EVP_PKEY* generate_rsa_key_pair() {
    EVP_PKEY* pkey = NULL;
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    if (!ctx) goto err;
    
    if (EVP_PKEY_keygen_init(ctx) <= 0) goto err;
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0) goto err;
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        pkey = NULL;
        goto err;
    }
    
err:
    if (!pkey) handle_openssl_error();
    if (ctx) EVP_PKEY_CTX_free(ctx);
    return pkey;
}

int encrypt_data(const unsigned char* plain_text, size_t plain_len, EVP_PKEY* public_key, unsigned char** cipher_text, size_t* cipher_len) {
    EVP_PKEY_CTX* ctx = NULL;
    int success = 0;
    
    ctx = EVP_PKEY_CTX_new(public_key, NULL);
    if (!ctx) goto cleanup;
    
    if (EVP_PKEY_encrypt_init(ctx) <= 0) goto cleanup;
    
    // Rule #8: Always incorporate OAEP when using the RSA algorithm
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) goto cleanup;

    if (EVP_PKEY_encrypt(ctx, NULL, cipher_len, plain_text, plain_len) <= 0) goto cleanup;

    *cipher_text = malloc(*cipher_len);
    if (!*cipher_text) {
        fprintf(stderr, "Memory allocation failed.\n");
        goto cleanup;
    }

    if (EVP_PKEY_encrypt(ctx, *cipher_text, cipher_len, plain_text, plain_len) <= 0) {
        free(*cipher_text);
        *cipher_text = NULL;
        goto cleanup;
    }
    
    success = 1;

cleanup:
    if (!success) handle_openssl_error();
    if (ctx) EVP_PKEY_CTX_free(ctx);
    return success ? 0 : -1;
}


int decrypt_data(const unsigned char* cipher_text, size_t cipher_len, EVP_PKEY* private_key, unsigned char** plain_text, size_t* plain_len) {
    EVP_PKEY_CTX* ctx = NULL;
    int success = 0;

    ctx = EVP_PKEY_CTX_new(private_key, NULL);
    if (!ctx) goto cleanup;

    if (EVP_PKEY_decrypt_init(ctx) <= 0) goto cleanup;

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) goto cleanup;

    if (EVP_PKEY_decrypt(ctx, NULL, plain_len, cipher_text, cipher_len) <= 0) goto cleanup;

    *plain_text = malloc(*plain_len);
    if (!*plain_text) {
        fprintf(stderr, "Memory allocation failed.\n");
        goto cleanup;
    }

    if (EVP_PKEY_decrypt(ctx, *plain_text, plain_len, cipher_text, cipher_len) <= 0) {
        free(*plain_text);
        *plain_text = NULL;
        goto cleanup;
    }
    
    success = 1;

cleanup:
    if (!success) handle_openssl_error();
    if (ctx) EVP_PKEY_CTX_free(ctx);
    return success ? 0 : -1;
}

void print_hex(const char* label, const unsigned char* data, size_t len) {
    printf("%s", label);
    for (size_t i = 0; i < len; ++i) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

int main() {
    EVP_PKEY* key_pair = generate_rsa_key_pair();
    if (!key_pair) {
        fprintf(stderr, "Failed to generate key pair.\n");
        return 1;
    }

    const char* test_cases[] = {
        "This is a test message.",
        "RSA with OAEP padding is secure.",
        "Another message for testing purposes.",
        "Short",
        "A much longer message to see how the encryption handles different lengths of input data."
    };
    int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_test_cases; ++i) {
        const char* original_message = test_cases[i];
        printf("--- Test Case %d ---\n", i + 1);
        printf("Original: %s\n", original_message);

        unsigned char* encrypted = NULL;
        size_t encrypted_len = 0;
        unsigned char* decrypted = NULL;
        size_t decrypted_len = 0;

        if (encrypt_data((const unsigned char*)original_message, strlen(original_message), key_pair, &encrypted, &encrypted_len) != 0) {
            printf("Status: ENCRYPTION FAILED\n\n");
            continue;
        }
        print_hex("Encrypted (Hex): ", encrypted, encrypted_len);
        
        if (decrypt_data(encrypted, encrypted_len, key_pair, &decrypted, &decrypted_len) != 0) {
            printf("Status: DECRYPTION FAILED\n\n");
            free(encrypted);
            continue;
        }
        
        printf("Decrypted: %.*s\n", (int)decrypted_len, (char*)decrypted);

        if (strlen(original_message) == decrypted_len && memcmp(original_message, decrypted, decrypted_len) == 0) {
            printf("Status: SUCCESS\n");
        } else {
            printf("Status: FAILED\n");
        }
        
        free(encrypted);
        free(decrypted);
        printf("\n");
    }

    EVP_PKEY_free(key_pair);
    return 0;
}