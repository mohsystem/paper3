#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Note: This code requires OpenSSL version 3.0 or newer.
// To compile: gcc your_file_name.c -o your_program_name -l crypto
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/bio.h>
#include <openssl/encoder.h>

/**
 * Generates an RSA private key and returns it in the OpenSSH format.
 *
 * @param keySize The size of the key in bits (e.g., 2048, 4096).
 * @return A dynamically allocated string containing the key. The caller is
 *         responsible for freeing this memory with free(). Returns NULL on failure.
 */
char* generateOpenSSHPrivateKey(int keySize) {
    EVP_PKEY_CTX* ctx = NULL;
    EVP_PKEY* pkey = NULL;
    BIO* bio = NULL;
    OSSL_ENCODER_CTX* ectx = NULL;
    char* key_string = NULL;
    int success = 0;

    // 1. Create a context for the key generation.
    ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    if (!ctx) {
        fprintf(stderr, "Error: Failed to create EVP_PKEY_CTX.\n");
        goto cleanup;
    }

    // 2. Initialize the context for key generation.
    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        fprintf(stderr, "Error: Failed to initialize keygen context.\n");
        goto cleanup;
    }

    // 3. Set the RSA key size.
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, keySize) <= 0) {
        fprintf(stderr, "Error: Failed to set RSA keygen bits.\n");
        goto cleanup;
    }

    // 4. Generate the key.
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        fprintf(stderr, "Error: Failed to generate key.\n");
        goto cleanup;
    }
    
    // 5. Create a memory BIO to write the key to.
    bio = BIO_new(BIO_s_mem());
    if (!bio) {
        fprintf(stderr, "Error: Failed to create memory BIO.\n");
        goto cleanup;
    }

    // 6. Create an encoder context to export the key in OpenSSH format.
    ectx = OSSL_ENCODER_CTX_new_for_pkey(
        pkey,
        OSSL_KEYMGMT_SELECT_PRIVATE_KEY,
        "PEM",
        "OpenSSH",
        NULL
    );
    if (!ectx) {
        fprintf(stderr, "Error: Failed to create OSSL_ENCODER_CTX. OpenSSL 3.0+ is required.\n");
        goto cleanup;
    }

    // 7. Perform the encoding and write to the BIO.
    if (!OSSL_ENCODER_to_bio(ectx, bio)) {
        fprintf(stderr, "Error: Failed to encode key to BIO.\n");
        goto cleanup;
    }

    // 8. Read the key from the BIO into a dynamically allocated string.
    char* data_ptr = NULL;
    long data_len = BIO_get_mem_data(bio, &data_ptr);
    if (data_len > 0 && data_ptr != NULL) {
        key_string = (char*)malloc(data_len + 1);
        if (key_string) {
            memcpy(key_string, data_ptr, data_len);
            key_string[data_len] = '\0';
            success = 1;
        } else {
            fprintf(stderr, "Error: Failed to allocate memory for key string.\n");
        }
    } else {
         fprintf(stderr, "Error: Failed to get data from BIO.\n");
    }

cleanup:
    // Free all allocated OpenSSL resources.
    OSSL_ENCODER_CTX_free(ectx);
    BIO_free_all(bio);
    EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(ctx);
    
    if (!success) {
        free(key_string); // In case of partial failure
        return NULL;
    }
    
    return key_string;
}

int main() {
    printf("--- C RSA OpenSSH Key Generation ---\n");
    printf("Note: This requires OpenSSL 3.0+ and linking with -lcrypto.\n");
    
    for (int i = 1; i <= 5; ++i) {
        printf("\n--- Test Case %d (2048 bits) ---\n", i);
        char* privateKey = generateOpenSSHPrivateKey(2048);
        if (privateKey != NULL) {
            printf("%s", privateKey);
            free(privateKey); // Free the memory allocated by the function.
        } else {
            printf("Key generation failed.\n");
        }
    }
    return 0;
}