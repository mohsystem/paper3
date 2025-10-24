
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* generate_rsa_private_key_openssh(int key_size) {
    /* Validate minimum key size for security */
    if (key_size < 2048) {
        fprintf(stderr, "Key size must be at least 2048 bits for security\\n");
        return NULL;
    }
    
    /* Initialize OpenSSL */
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
    
    /* Generate RSA key pair */
    EVP_PKEY* pkey = NULL;
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    
    if (!ctx) {
        fprintf(stderr, "Failed to create EVP_PKEY_CTX\\n");
        return NULL;
    }
    
    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        fprintf(stderr, "Failed to initialize key generation\\n");
        EVP_PKEY_CTX_free(ctx);
        return NULL;
    }
    
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, key_size) <= 0) {
        fprintf(stderr, "Failed to set key size\\n");
        EVP_PKEY_CTX_free(ctx);
        return NULL;
    }
    
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        fprintf(stderr, "Failed to generate key pair\\n");
        EVP_PKEY_CTX_free(ctx);
        return NULL;
    }
    
    EVP_PKEY_CTX_free(ctx);
    
    /* Convert to PEM format (closest to OpenSSH in C) */
    BIO* bio = BIO_new(BIO_s_mem());
    if (!bio) {
        fprintf(stderr, "Failed to create BIO\\n");
        EVP_PKEY_free(pkey);
        return NULL;
    }
    
    if (!PEM_write_bio_PrivateKey(bio, pkey, NULL, NULL, 0, NULL, NULL)) {
        fprintf(stderr, "Failed to write private key\\n");
        BIO_free(bio);
        EVP_PKEY_free(pkey);
        return NULL;
    }
    
    /* Read the key from BIO */
    char* data;
    long data_len = BIO_get_mem_data(bio, &data);
    
    /* Allocate memory for result */
    char* result = (char*)malloc(data_len + 1);
    if (!result) {
        fprintf(stderr, "Memory allocation failed\\n");
        BIO_free(bio);
        EVP_PKEY_free(pkey);
        return NULL;
    }
    
    memcpy(result, data, data_len);
    result[data_len] = '\\0';
    
    /* Cleanup */
    BIO_free(bio);
    EVP_PKEY_free(pkey);
    
    return result;
}

int main() {
    printf("=== RSA Private Key Generator (OpenSSH Format) ===\\n\\n");
    
    /* Test case 1: 2048-bit key (minimum secure) */
    printf("Test Case 1: Generate 2048-bit RSA key\\n");
    char* key1 = generate_rsa_private_key_openssh(2048);
    if (key1) {
        printf("Success - Key length: %zu characters\\n", strlen(key1));
        printf("%.200s...\\n\\n", key1);
        free(key1);
    } else {
        printf("Error generating key\\n\\n");
    }
    
    /* Test case 2: 4096-bit key (highly secure) */
    printf("Test Case 2: Generate 4096-bit RSA key\\n");
    char* key2 = generate_rsa_private_key_openssh(4096);
    if (key2) {
        printf("Success - Key length: %zu characters\\n", strlen(key2));
        printf("%.200s...\\n\\n", key2);
        free(key2);
    } else {
        printf("Error generating key\\n\\n");
    }
    
    /* Test case 3: Invalid key size (should fail) */
    printf("Test Case 3: Attempt 1024-bit key (insecure, should fail)\\n");
    char* key3 = generate_rsa_private_key_openssh(1024);
    if (key3) {
        printf("Generated: %.100s...\\n\\n", key3);
        free(key3);
    } else {
        printf("Expected Error: Key size validation failed\\n\\n");
    }
    
    /* Test case 4: 3072-bit key */
    printf("Test Case 4: Generate 3072-bit RSA key\\n");
    char* key4 = generate_rsa_private_key_openssh(3072);
    if (key4) {
        printf("Success - Key length: %zu characters\\n", strlen(key4));
        printf("%.200s...\\n\\n", key4);
        free(key4);
    } else {
        printf("Error generating key\\n\\n");
    }
    
    /* Test case 5: Standard 2048-bit with full output */
    printf("Test Case 5: Generate 2048-bit key with full display\\n");
    char* key5 = generate_rsa_private_key_openssh(2048);
    if (key5) {
        printf("Full key generated:\\n%s\\n", key5);
        free(key5);
    } else {
        printf("Error generating key\\n\\n");
    }
    
    return 0;
}
