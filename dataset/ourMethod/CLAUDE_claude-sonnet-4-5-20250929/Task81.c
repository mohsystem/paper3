
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/err.h>

/* Maximum certificate file size: 1MB to prevent excessive memory usage */
#define MAX_CERT_SIZE (1024 * 1024)
/* SHA-256 produces 32 bytes = 64 hex characters */
#define SHA256_HEX_LENGTH 64
#define SHA256_DIGEST_LENGTH 32

/**
 * Securely clears sensitive data from memory.
 * Uses explicit_bzero if available, otherwise volatile pointer technique.
 */
static void secure_zero(void *ptr, size_t len) {
    if (ptr == NULL || len == 0) {
        return;
    }
#if defined(__GLIBC__) && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 25))
    explicit_bzero(ptr, len);
#elif defined(_WIN32)
    SecureZeroMemory(ptr, len);
#else
    /* Fallback: use volatile to prevent compiler optimization */
    volatile unsigned char *vptr = (volatile unsigned char *)ptr;
    while (len--) {
        *vptr++ = 0;
    }
#endif
}

/**
 * Validates hex string format and length.
 * Returns 1 if valid, 0 otherwise.
 */
static int validate_hex_string(const char *hex_str) {
    /* Null check - fail closed */
    if (hex_str == NULL) {
        return 0;
    }
    
    size_t len = strlen(hex_str);
    
    /* Must be exactly 64 characters for SHA-256 */
    if (len != SHA256_HEX_LENGTH) {
        return 0;
    }
    
    /* Validate each character is valid hex */
    for (size_t i = 0; i < len; i++) {
        char c = hex_str[i];
        if (!((c >= '0' && c <= '9') || 
              (c >= 'a' && c <= 'f') || 
              (c >= 'A' && c <= 'F'))) {
            return 0;
        }
    }
    
    return 1;
}

/**
 * Converts hex string to binary data.
 * Returns 1 on success, 0 on failure.
 * Output buffer must be at least SHA256_DIGEST_LENGTH bytes.
 */
static int hex_to_bytes(const char *hex_str, unsigned char *output, size_t output_len) {
    /* Validate inputs - fail closed */
    if (hex_str == NULL || output == NULL) {
        return 0;
    }
    
    if (output_len < SHA256_DIGEST_LENGTH) {
        return 0;
    }
    
    /* Already validated in validate_hex_string, but double check */
    size_t hex_len = strlen(hex_str);
    if (hex_len != SHA256_HEX_LENGTH) {
        return 0;
    }
    
    /* Convert pairs of hex digits to bytes */
    for (size_t i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        unsigned int byte_val = 0;
        /* Use sscanf with bounds check to parse each hex byte */
        if (sscanf(&hex_str[i * 2], "%2x", &byte_val) != 1) {
            secure_zero(output, output_len);
            return 0;
        }
        output[i] = (unsigned char)byte_val;
    }
    
    return 1;
}

/**
 * Reads certificate file with bounds checking.
 * Returns X509 certificate or NULL on failure.
 * Caller must free returned X509 with X509_free.
 */
static X509 *read_certificate_file(const char *cert_path) {
    FILE *fp = NULL;
    X509 *cert = NULL;
    BIO *bio = NULL;
    
    /* Validate input - fail closed */
    if (cert_path == NULL) {
        fprintf(stderr, "Invalid certificate path\\n");
        return NULL;
    }
    
    /* Validate path length to prevent buffer issues */
    size_t path_len = strlen(cert_path);
    if (path_len == 0 || path_len > 4096) {
        fprintf(stderr, "Certificate path length invalid\\n");
        return NULL;
    }
    
    /* Open file first, then validate the handle (TOCTOU prevention) */
    fp = fopen(cert_path, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open certificate file\\n");
        return NULL;
    }
    
    /* Check file size to prevent excessive memory usage */
    if (fseek(fp, 0, SEEK_END) != 0) {
        fprintf(stderr, "Failed to seek certificate file\\n");
        fclose(fp);
        return NULL;
    }
    
    long file_size = ftell(fp);
    if (file_size < 0 || file_size > MAX_CERT_SIZE) {
        fprintf(stderr, "Certificate file size invalid\\n");
        fclose(fp);
        return NULL;
    }
    
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fprintf(stderr, "Failed to reset certificate file position\\n");
        fclose(fp);
        return NULL;
    }
    
    /* Use BIO for safe reading */
    bio = BIO_new_fp(fp, BIO_NOCLOSE);
    if (bio == NULL) {
        fprintf(stderr, "Failed to create BIO\\n");
        fclose(fp);
        return NULL;
    }
    
    /* Try PEM format first */
    cert = PEM_read_bio_X509(bio, NULL, NULL, NULL);
    
    if (cert == NULL) {
        /* Try DER format */
        BIO_reset(bio);
        cert = d2i_X509_bio(bio, NULL);
    }
    
    /* Clean up */
    BIO_free(bio);
    fclose(fp);
    
    if (cert == NULL) {
        fprintf(stderr, "Failed to parse certificate\\n");
    }
    
    return cert;
}

/**
 * Computes SHA-256 hash of X509 certificate DER encoding.
 * Returns 1 on success, 0 on failure.
 * Output buffer must be at least SHA256_DIGEST_LENGTH bytes.
 */
static int compute_certificate_hash(X509 *cert, unsigned char *hash_output, size_t output_len) {
    unsigned char *der_cert = NULL;
    int der_len = 0;
    EVP_MD_CTX *mdctx = NULL;
    unsigned int hash_len = 0;
    int result = 0;
    
    /* Validate inputs - fail closed */
    if (cert == NULL || hash_output == NULL) {
        fprintf(stderr, "Invalid input to hash computation\\n");
        return 0;
    }
    
    if (output_len < SHA256_DIGEST_LENGTH) {
        fprintf(stderr, "Output buffer too small\\n");
        return 0;
    }
    
    /* Initialize output to zero */
    memset(hash_output, 0, output_len);
    
    /* Convert certificate to DER format */
    der_len = i2d_X509(cert, &der_cert);
    if (der_len <= 0 || der_cert == NULL) {
        fprintf(stderr, "Failed to encode certificate to DER\\n");
        return 0;
    }
    
    /* Create digest context */
    mdctx = EVP_MD_CTX_new();
    if (mdctx == NULL) {
        fprintf(stderr, "Failed to create digest context\\n");
        OPENSSL_free(der_cert);
        return 0;
    }
    
    /* Initialize SHA-256 digest */
    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1) {
        fprintf(stderr, "Failed to initialize digest\\n");
        goto cleanup;
    }
    
    /* Update digest with certificate data */
    if (EVP_DigestUpdate(mdctx, der_cert, der_len) != 1) {
        fprintf(stderr, "Failed to update digest\\n");
        goto cleanup;
    }
    
    /* Finalize digest */
    if (EVP_DigestFinal_ex(mdctx, hash_output, &hash_len) != 1) {
        fprintf(stderr, "Failed to finalize digest\\n");
        goto cleanup;
    }
    
    /* Verify hash length */
    if (hash_len != SHA256_DIGEST_LENGTH) {
        fprintf(stderr, "Unexpected hash length\\n");
        secure_zero(hash_output, output_len);
        goto cleanup;
    }
    
    result = 1;
    
cleanup:
    /* Securely clear the DER encoding as it may contain sensitive data */
    if (der_cert != NULL) {
        secure_zero(der_cert, der_len);
        OPENSSL_free(der_cert);
    }
    
    if (mdctx != NULL) {
        EVP_MD_CTX_free(mdctx);
    }
    
    return result;
}

/**
 * Compares two byte arrays in constant time to prevent timing attacks.
 * Returns 1 if equal, 0 otherwise.
 */
static int constant_time_compare(const unsigned char *a, const unsigned char *b, size_t len) {
    /* Validate inputs */
    if (a == NULL || b == NULL || len == 0) {
        return 0;
    }
    
    unsigned char result = 0;
    
    /* XOR all bytes - result will be 0 only if all bytes match */
    for (size_t i = 0; i < len; i++) {
        result |= a[i] ^ b[i];
    }
    
    /* Return 1 if result is 0 (all bytes matched) */
    return result == 0 ? 1 : 0;
}

/**
 * Main function to check if certificate matches known hash.
 * Returns 1 if match, 0 if no match, -1 on error.
 */
int check_certificate_hash(const char *cert_path, const char *known_hash_hex) {
    X509 *cert = NULL;
    unsigned char computed_hash[SHA256_DIGEST_LENGTH];
    unsigned char expected_hash[SHA256_DIGEST_LENGTH];
    int result = -1;
    
    /* Initialize buffers to zero */
    memset(computed_hash, 0, sizeof(computed_hash));
    memset(expected_hash, 0, sizeof(expected_hash));
    
    /* Validate inputs - fail closed */
    if (cert_path == NULL || known_hash_hex == NULL) {
        fprintf(stderr, "Error: Invalid input parameters\\n");
        return -1;
    }
    
    /* Initialize OpenSSL */
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
    
    /* Validate hex string format */
    if (!validate_hex_string(known_hash_hex)) {
        fprintf(stderr, "Error: Invalid hash format (expected 64 hex characters)\\n");
        goto cleanup;
    }
    
    /* Convert expected hash from hex to bytes */
    if (!hex_to_bytes(known_hash_hex, expected_hash, sizeof(expected_hash))) {
        fprintf(stderr, "Error: Failed to parse hash\\n");
        goto cleanup;
    }
    
    /* Read and parse certificate */
    cert = read_certificate_file(cert_path);
    if (cert == NULL) {
        fprintf(stderr, "Error: Failed to read certificate\\n");
        goto cleanup;
    }
    
    /* Compute certificate hash */
    if (!compute_certificate_hash(cert, computed_hash, sizeof(computed_hash))) {
        fprintf(stderr, "Error: Failed to compute certificate hash\\n");
        goto cleanup;
    }
    
    /* Compare hashes using constant-time comparison to prevent timing attacks */
    if (constant_time_compare(computed_hash, expected_hash, SHA256_DIGEST_LENGTH)) {
        result = 1;  /* Match */
    } else {
        result = 0;  /* No match */
    }
    
cleanup:
    /* Securely clear sensitive data */
    secure_zero(computed_hash, sizeof(computed_hash));
    secure_zero(expected_hash, sizeof(expected_hash));
    
    /* Free certificate */
    if (cert != NULL) {
        X509_free(cert);
    }
    
    /* Clean up OpenSSL */
    EVP_cleanup();
    ERR_free_strings();
    
    return result;
}

int main(void) {
    int test_result = 0;
    
    printf("SSL Certificate Hash Verification Test Suite\\n");
    printf("=============================================\\n\\n");
    
    /* Test Case 1: Valid certificate with matching hash */
    printf("Test 1: Valid certificate with matching hash\\n");
    /* Note: In real use, create a test certificate file and compute its hash */
    /* For demonstration, using placeholder - replace with actual test data */
    test_result = check_certificate_hash(
        "test_cert1.pem",
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"
    );
    printf("Result: %s\\n\\n", test_result == 1 ? "MATCH" : test_result == 0 ? "NO MATCH" : "ERROR");
    
    /* Test Case 2: Valid certificate with non-matching hash */
    printf("Test 2: Valid certificate with non-matching hash\\n");
    test_result = check_certificate_hash(
        "test_cert1.pem",
        "0000000000000000000000000000000000000000000000000000000000000000"
    );
    printf("Result: %s\\n\\n", test_result == 1 ? "MATCH" : test_result == 0 ? "NO MATCH" : "ERROR");
    
    /* Test Case 3: Invalid hash format (too short) */
    printf("Test 3: Invalid hash format (too short)\\n");
    test_result = check_certificate_hash(
        "test_cert1.pem",
        "e3b0c442"
    );
    printf("Result: %s\\n\\n", test_result == 1 ? "MATCH" : test_result == 0 ? "NO MATCH" : "ERROR");
    
    /* Test Case 4: Invalid hash format (non-hex characters) */
    printf("Test 4: Invalid hash format (non-hex characters)\\n");
    test_result = check_certificate_hash(
        "test_cert1.pem",
        "g3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"
    );
    printf("Result: %s\\n\\n", test_result == 1 ? "MATCH" : test_result == 0 ? "NO MATCH" : "ERROR");
    
    /* Test Case 5: Non-existent certificate file */
    printf("Test 5: Non-existent certificate file\\n");
    test_result = check_certificate_hash(
        "nonexistent_cert.pem",
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"
    );
    printf("Result: %s\\n\\n", test_result == 1 ? "MATCH" : test_result == 0 ? "NO MATCH" : "ERROR");
    
    printf("Test suite completed.\\n");
    printf("Note: For tests to pass, create test_cert1.pem and compute its actual SHA-256 hash.\\n");
    
    return 0;
}
