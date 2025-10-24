/*
NOTE: This C code requires the OpenSSL library (version 3.0 or newer recommended).
You must have OpenSSL installed on your system.
To compile:
gcc your_file_name.c -o your_program_name -lssl -lcrypto
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/bio.h>
#include <openssl/encoder.h>
#include <openssl/err.h>

/**
 * @brief Generates an RSA private key and exports it in the OpenSSH format.
 * 
 * @param key_size The size of the key in bits (e.g., 2048, 4096).
 * @return A dynamically allocated string containing the key in OpenSSH PEM format.
 *         The caller is responsible for freeing this memory with free().
 *         Returns NULL on failure.
 */
char* generateAndExportRsaKey(int key_size) {
    if (key_size < 2048) {
        fprintf(stderr, "Warning: Key size %d is smaller than the recommended 2048 bits.\n", key_size);
    }
    
    EVP_PKEY* pkey = NULL;
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    BIO* bio = NULL;
    OSSL_ENCODER_CTX* enc_ctx = NULL;
    char* result = NULL;

    if (!ctx) {
        fprintf(stderr, "Error: Failed to create EVP_PKEY_CTX.\n");
        goto cleanup;
    }

    if (EVP_PKEY_keygen_init(ctx) <= 0 ||
        EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, key_size) <= 0) {
        fprintf(stderr, "Error: Failed to initialize RSA keygen parameters.\n");
        goto cleanup;
    }
    
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        fprintf(stderr, "Error: Failed to generate RSA key pair.\n");
        goto cleanup;
    }

    bio = BIO_new(BIO_s_mem());
    if (!bio) {
        fprintf(stderr, "Error: Failed to create memory BIO.\n");
        goto cleanup;
    }

    // OSSL_ENCODER is the modern way (OpenSSL 3.0+) to export keys
    enc_ctx = OSSL_ENCODER_CTX_new_for_pkey(
        pkey, EVP_PKEY_KEYPAIR, "PEM", "OSSHPRIV", NULL);

    if (!enc_ctx) {
        fprintf(stderr, "Error: Failed to create OSSL_ENCODER_CTX. Is OpenSSL 3.0+ installed?\n");
        goto cleanup;
    }
    
    if (OSSL_ENCODER_to_bio(enc_ctx, bio) <= 0) {
        fprintf(stderr, "Error: Failed to encode key to BIO.\n");
        goto cleanup;
    }

    char* buffer_ptr = NULL;
    long len = BIO_get_mem_data(bio, &buffer_ptr);
    if (len > 0) {
        result = (char*)malloc(len + 1);
        if (result) {
            memcpy(result, buffer_ptr, len);
            result[len] = '\0';
        } else {
             fprintf(stderr, "Error: Failed to allocate memory for result.\n");
        }
    }

cleanup:
    if (enc_ctx) OSSL_ENCODER_CTX_free(enc_ctx);
    if (bio) BIO_free(bio);
    if (pkey) EVP_PKEY_free(pkey);
    if (ctx) EVP_PKEY_CTX_free(ctx);
    
    return result;
}

int main() {
    // 5 Test Cases
    int key_sizes[] = {2048, 2048, 3072, 4096, 4096};
    int num_tests = sizeof(key_sizes) / sizeof(key_sizes[0]);

    for (int i = 0; i < num_tests; ++i) {
        printf("--- Test Case %d (Key Size: %d) ---\n", i + 1, key_sizes[i]);
        char* openssh_key = generateAndExportRsaKey(key_sizes[i]);
        if (openssh_key) {
            printf("%s\n", openssh_key);
            free(openssh_key); // Free the allocated memory
        } else {
            fprintf(stderr, "Failed to generate key.\n");
            ERR_print_errors_fp(stderr);
        }
    }
    
    return 0;
}