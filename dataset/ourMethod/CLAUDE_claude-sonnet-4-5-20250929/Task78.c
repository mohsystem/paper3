
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>

// Maximum size for exported key (100KB)
#define MAX_KEY_SIZE 102400

// Structure to hold generated key data
typedef struct {
    char* data;
    size_t length;
    int success;
} KeyResult;

// Initialize KeyResult structure
void init_key_result(KeyResult* result) {
    if (result) {
        result->data = NULL;
        result->length = 0;
        result->success = 0;
    }
}

// Free KeyResult and securely clear memory
void free_key_result(KeyResult* result) {
    if (result && result->data) {
        // Securely clear sensitive key material before freeing
        volatile char* ptr = result->data;
        for (size_t i = 0; i < result->length; i++) {
            ptr[i] = 0;
        }
        free(result->data);
        result->data = NULL;
        result->length = 0;
    }
}

// Generate RSA key pair and export in PEM format (compatible with OpenSSH)
// Returns KeyResult with allocated data that must be freed by caller
KeyResult generate_rsa_key_openssh(int bits) {
    KeyResult result;
    init_key_result(&result);

    // Validate key size - must be at least 2048 bits for security
    if (bits < 2048 || bits > 16384) {
        fprintf(stderr, "Invalid key size. Must be between 2048 and 16384 bits.\\n");
        return result;
    }

    // Initialize OpenSSL
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();

    EVP_PKEY_CTX* ctx = NULL;
    EVP_PKEY* pkey = NULL;
    BIO* bio = NULL;
    BUF_MEM* mem = NULL;

    // Create key generation context
    ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    if (!ctx) {
        fprintf(stderr, "Failed to create key generation context\\n");
        goto cleanup;
    }

    // Initialize key generation
    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        fprintf(stderr, "Failed to initialize key generation\\n");
        goto cleanup;
    }

    // Set RSA key size
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, bits) <= 0) {
        fprintf(stderr, "Failed to set key size\\n");
        goto cleanup;
    }

    // Generate the key pair using cryptographically secure RNG from OpenSSL
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        fprintf(stderr, "Failed to generate key pair\\n");
        goto cleanup;
    }

    // Create memory BIO for writing the key
    bio = BIO_new(BIO_s_mem());
    if (!bio) {
        fprintf(stderr, "Failed to create memory BIO\\n");
        goto cleanup;
    }

    // Write private key in PEM format
    // Pass NULL for cipher to write unencrypted key
    // In production, use passphrase protection with EVP_aes_256_cbc()
    if (PEM_write_bio_PrivateKey(bio, pkey, NULL, NULL, 0, NULL, NULL) != 1) {
        fprintf(stderr, "Failed to write private key in PEM format\\n");
        goto cleanup;
    }

    // Get the key data from BIO
    if (BIO_get_mem_ptr(bio, &mem) != 1 || !mem) {
        fprintf(stderr, "Failed to get memory pointer from BIO\\n");
        goto cleanup;
    }

    // Validate memory pointer and length
    if (!mem->data || mem->length == 0) {
        fprintf(stderr, "Invalid key data in BIO\\n");
        goto cleanup;
    }

    // Validate length is reasonable
    if (mem->length > MAX_KEY_SIZE) {
        fprintf(stderr, "Key data exceeds maximum expected size\\n");
        goto cleanup;
    }

    // Allocate memory for result - add 1 for null terminator
    result.data = (char*)malloc(mem->length + 1);
    if (!result.data) {
        fprintf(stderr, "Failed to allocate memory for key data\\n");
        goto cleanup;
    }

    // Copy key data - bounds checked by validation above
    memcpy(result.data, mem->data, mem->length);
    result.data[mem->length] = '\\0'; // Null terminate for string operations
    result.length = mem->length;
    result.success = 1;

cleanup:
    // Clean up OpenSSL structures
    if (bio) BIO_free_all(bio);
    if (pkey) EVP_PKEY_free(pkey);
    if (ctx) EVP_PKEY_CTX_free(ctx);

    return result;
}

int main(void) {
    printf("=== RSA Private Key Generator (OpenSSH Format) ===\\n\\n");

    // Test case 1: Generate 2048-bit RSA key (minimum secure size)
    printf("Test 1: Generating 2048-bit RSA private key...\\n");
    KeyResult key1 = generate_rsa_key_openssh(2048);
    if (key1.success && key1.data) {
        printf("Success! Key length: %zu bytes\\n", key1.length);
        printf("Key preview (first 100 chars):\\n");
        // Safely print first 100 characters
        size_t preview_len = key1.length < 100 ? key1.length : 100;
        for (size_t i = 0; i < preview_len; i++) {
            putchar(key1.data[i]);
        }
        printf("...\\n");
    } else {
        printf("Failed to generate key\\n");
    }
    printf("\\n");

    // Test case 2: Generate 3072-bit RSA key
    printf("Test 2: Generating 3072-bit RSA private key...\\n");
    KeyResult key2 = generate_rsa_key_openssh(3072);
    if (key2.success && key2.data) {
        printf("Success! Key length: %zu bytes\\n", key2.length);
    } else {
        printf("Failed to generate key\\n");
    }
    printf("\\n");

    // Test case 3: Generate 4096-bit RSA key (high security)
    printf("Test 3: Generating 4096-bit RSA private key...\\n");
    KeyResult key3 = generate_rsa_key_openssh(4096);
    if (key3.success && key3.data) {
        printf("Success! Key length: %zu bytes\\n", key3.length);
    } else {
        printf("Failed to generate key\\n");
    }
    printf("\\n");

    // Test case 4: Invalid key size (too small - should fail)
    printf("Test 4: Attempting to generate 1024-bit RSA key (should fail)...\\n");
    KeyResult key4 = generate_rsa_key_openssh(1024);
    if (!key4.success) {
        printf("Correctly rejected insecure key size\\n");
    } else {
        printf("Warning: Insecure key size was accepted\\n");
    }
    printf("\\n");

    // Test case 5: Invalid key size (too large - should fail)
    printf("Test 5: Attempting to generate 20480-bit RSA key (should fail)...\\n");
    KeyResult key5 = generate_rsa_key_openssh(20480);
    if (!key5.success) {
        printf("Correctly rejected excessive key size\\n");
    } else {
        printf("Warning: Excessive key size was accepted\\n");
    }

    // Securely free all key results
    free_key_result(&key1);
    free_key_result(&key2);
    free_key_result(&key3);
    free_key_result(&key4);
    free_key_result(&key5);

    return 0;
}
