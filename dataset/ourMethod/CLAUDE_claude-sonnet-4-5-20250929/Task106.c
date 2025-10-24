
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Generate RSA keypair using OpenSSL EVP (minimum 2048 bits for security) */
EVP_PKEY* generateRSAKeyPair(int bits) {
    EVP_PKEY_CTX* ctx = NULL;
    EVP_PKEY* pkey = NULL;
    
    /* Validate key size - must be at least 2048 bits */
    if (bits < 2048) {
        fprintf(stderr, "RSA key size must be at least 2048 bits\\n");
        return NULL;
    }
    
    /* Create context for RSA key generation */
    ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    if (!ctx) {
        fprintf(stderr, "Failed to create EVP_PKEY_CTX\\n");
        return NULL;
    }
    
    /* Initialize key generation */
    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        fprintf(stderr, "Failed to initialize key generation\\n");
        EVP_PKEY_CTX_free(ctx);
        return NULL;
    }
    
    /* Set RSA key size */
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, bits) <= 0) {
        fprintf(stderr, "Failed to set RSA key size\\n");
        EVP_PKEY_CTX_free(ctx);
        return NULL;
    }
    
    /* Generate the keypair */
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        fprintf(stderr, "Failed to generate RSA keypair\\n");
        EVP_PKEY_CTX_free(ctx);
        return NULL;
    }
    
    EVP_PKEY_CTX_free(ctx);
    return pkey;
}

/* Encrypt data using RSA with OAEP padding (CWE-780 mitigation)
 * OAEP provides semantic security and prevents padding oracle attacks */
int rsaEncrypt(EVP_PKEY* publicKey, const uint8_t* plaintext, size_t plaintextLen,
               uint8_t** ciphertext, size_t* ciphertextLen) {
    EVP_PKEY_CTX* ctx = NULL;
    int keySize = 0;
    size_t maxPlaintextSize = 0;
    size_t outLen = 0;
    uint8_t* output = NULL;
    
    /* Validate inputs */
    if (!publicKey || !plaintext || plaintextLen == 0 || !ciphertext || !ciphertextLen) {
        fprintf(stderr, "Invalid input parameters\\n");
        return 0;
    }
    
    /* Get and validate key size */
    keySize = EVP_PKEY_size(publicKey);
    if (keySize <= 0 || keySize > 8192) {
        fprintf(stderr, "Invalid key size\\n");
        return 0;
    }
    
    /* Calculate maximum plaintext size for RSA-OAEP-SHA256
     * Max size = key_size_bytes - 2*hash_size - 2
     * For 2048-bit RSA: max = 256 - 2*32 - 2 = 190 bytes */
    maxPlaintextSize = (size_t)keySize - 2 * 32 - 2;
    if (plaintextLen > maxPlaintextSize) {
        fprintf(stderr, "Plaintext too large for RSA-OAEP encryption\\n");
        return 0;
    }
    
    /* Create encryption context */
    ctx = EVP_PKEY_CTX_new(publicKey, NULL);
    if (!ctx) {
        fprintf(stderr, "Failed to create encryption context\\n");
        return 0;
    }
    
    /* Initialize encryption */
    if (EVP_PKEY_encrypt_init(ctx) <= 0) {
        fprintf(stderr, "Failed to initialize encryption\\n");
        EVP_PKEY_CTX_free(ctx);
        return 0;
    }
    
    /* Set RSA OAEP padding for security */
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        fprintf(stderr, "Failed to set OAEP padding\\n");
        EVP_PKEY_CTX_free(ctx);
        return 0;
    }
    
    /* Set OAEP hash function to SHA-256 */
    if (EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha256()) <= 0) {
        fprintf(stderr, "Failed to set OAEP hash function\\n");
        EVP_PKEY_CTX_free(ctx);
        return 0;
    }
    
    /* Determine required ciphertext length */
    if (EVP_PKEY_encrypt(ctx, NULL, &outLen, plaintext, plaintextLen) <= 0) {
        fprintf(stderr, "Failed to determine ciphertext length\\n");
        EVP_PKEY_CTX_free(ctx);
        return 0;
    }
    
    /* Sanity check output length */
    if (outLen == 0 || outLen > 16384) {
        fprintf(stderr, "Invalid ciphertext length\\n");
        EVP_PKEY_CTX_free(ctx);
        return 0;
    }
    
    /* Allocate ciphertext buffer - checked for NULL */
    output = (uint8_t*)malloc(outLen);
    if (!output) {
        fprintf(stderr, "Failed to allocate ciphertext buffer\\n");
        EVP_PKEY_CTX_free(ctx);
        return 0;
    }
    
    /* Perform encryption */
    if (EVP_PKEY_encrypt(ctx, output, &outLen, plaintext, plaintextLen) <= 0) {
        fprintf(stderr, "Encryption failed\\n");
        free(output);
        EVP_PKEY_CTX_free(ctx);
        return 0;
    }
    
    *ciphertext = output;
    *ciphertextLen = outLen;
    
    EVP_PKEY_CTX_free(ctx);
    return 1;
}

/* Decrypt data using RSA with OAEP padding */
int rsaDecrypt(EVP_PKEY* privateKey, const uint8_t* ciphertext, size_t ciphertextLen,
               uint8_t** plaintext, size_t* plaintextLen) {
    EVP_PKEY_CTX* ctx = NULL;
    int keySize = 0;
    size_t outLen = 0;
    uint8_t* output = NULL;
    
    /* Validate inputs */
    if (!privateKey || !ciphertext || ciphertextLen == 0 || !plaintext || !plaintextLen) {
        fprintf(stderr, "Invalid input parameters\\n");
        return 0;
    }
    
    /* Get and validate key size */
    keySize = EVP_PKEY_size(privateKey);
    if (keySize <= 0 || keySize > 8192) {
        fprintf(stderr, "Invalid key size\\n");
        return 0;
    }
    
    /* Validate ciphertext size matches key size */
    if (ciphertextLen != (size_t)keySize) {
        fprintf(stderr, "Invalid ciphertext size\\n");
        return 0;
    }
    
    /* Create decryption context */
    ctx = EVP_PKEY_CTX_new(privateKey, NULL);
    if (!ctx) {
        fprintf(stderr, "Failed to create decryption context\\n");
        return 0;
    }
    
    /* Initialize decryption */
    if (EVP_PKEY_decrypt_init(ctx) <= 0) {
        fprintf(stderr, "Failed to initialize decryption\\n");
        EVP_PKEY_CTX_free(ctx);
        return 0;
    }
    
    /* Set RSA OAEP padding (must match encryption) */
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        fprintf(stderr, "Failed to set OAEP padding\\n");
        EVP_PKEY_CTX_free(ctx);
        return 0;
    }
    
    /* Set OAEP hash function to SHA-256 (must match encryption) */
    if (EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha256()) <= 0) {
        fprintf(stderr, "Failed to set OAEP hash function\\n");
        EVP_PKEY_CTX_free(ctx);
        return 0;
    }
    
    /* Determine required plaintext length */
    if (EVP_PKEY_decrypt(ctx, NULL, &outLen, ciphertext, ciphertextLen) <= 0) {
        fprintf(stderr, "Failed to determine plaintext length\\n");
        EVP_PKEY_CTX_free(ctx);
        return 0;
    }
    
    /* Sanity check output length */
    if (outLen == 0 || outLen > 16384) {
        fprintf(stderr, "Invalid plaintext length\\n");
        EVP_PKEY_CTX_free(ctx);
        return 0;
    }
    
    /* Allocate plaintext buffer - checked for NULL */
    output = (uint8_t*)malloc(outLen);
    if (!output) {
        fprintf(stderr, "Failed to allocate plaintext buffer\\n");
        EVP_PKEY_CTX_free(ctx);
        return 0;
    }
    
    /* Perform decryption */
    if (EVP_PKEY_decrypt(ctx, output, &outLen, ciphertext, ciphertextLen) <= 0) {
        fprintf(stderr, "Decryption failed\\n");
        free(output);
        EVP_PKEY_CTX_free(ctx);
        return 0;
    }
    
    *plaintext = output;
    *plaintextLen = outLen;
    
    EVP_PKEY_CTX_free(ctx);
    return 1;
}

int main(void) {
    EVP_PKEY* keypair = NULL;
    uint8_t* ciphertext = NULL;
    uint8_t* decrypted = NULL;
    size_t ciphertextLen = 0;
    size_t decryptedLen = 0;
    int success = 1;
    
    /* Initialize OpenSSL */
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
    
    /* Test case 1: Basic encryption and decryption */
    printf("Test 1: Basic encryption/decryption\\n");
    {
        const char* message = "Hello, RSA with OAEP!";
        size_t messageLen = strlen(message);
        
        keypair = generateRSAKeyPair(2048);
        if (!keypair) {
            fprintf(stderr, "Failed to generate keypair\\n");
            success = 0;
            goto cleanup;
        }
        
        if (!rsaEncrypt(keypair, (const uint8_t*)message, messageLen, &ciphertext, &ciphertextLen)) {
            fprintf(stderr, "Encryption failed\\n");
            success = 0;
            goto cleanup;
        }
        
        if (!rsaDecrypt(keypair, ciphertext, ciphertextLen, &decrypted, &decryptedLen)) {
            fprintf(stderr, "Decryption failed\\n");
            success = 0;
            goto cleanup;
        }
        
        printf("Original: %s\\n", message);
        printf("Decrypted: %.*s\\n", (int)decryptedLen, decrypted);
        printf("Match: %s\\n\\n", (messageLen == decryptedLen && memcmp(message, decrypted, messageLen) == 0) ? "YES" : "NO");
        
        free(ciphertext);
        free(decrypted);
        ciphertext = NULL;
        decrypted = NULL;
        EVP_PKEY_free(keypair);
        keypair = NULL;
    }
    
    /* Test case 2: Binary data encryption */
    printf("Test 2: Binary data encryption\\n");
    {
        uint8_t binaryData[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0xFF, 0xFE, 0xFD};
        size_t binaryLen = sizeof(binaryData);
        
        keypair = generateRSAKeyPair(2048);
        if (!keypair) {
            fprintf(stderr, "Failed to generate keypair\\n");
            success = 0;
            goto cleanup;
        }
        
        if (!rsaEncrypt(keypair, binaryData, binaryLen, &ciphertext, &ciphertextLen)) {
            fprintf(stderr, "Encryption failed\\n");
            success = 0;
            goto cleanup;
        }
        
        if (!rsaDecrypt(keypair, ciphertext, ciphertextLen, &decrypted, &decryptedLen)) {
            fprintf(stderr, "Decryption failed\\n");
            success = 0;
            goto cleanup;
        }
        
        printf("Binary data match: %s\\n\\n", (binaryLen == decryptedLen && memcmp(binaryData, decrypted, binaryLen) == 0) ? "YES" : "NO");
        
        free(ciphertext);
        free(decrypted);
        ciphertext = NULL;
        decrypted = NULL;
        EVP_PKEY_free(keypair);
        keypair = NULL;
    }
    
    /* Test case 3: Maximum plaintext size */
    printf("Test 3: Maximum plaintext size\\n");
    {
        /* For 2048-bit RSA with OAEP-SHA256: max = 256 - 2*32 - 2 = 190 bytes */
        uint8_t maxPlaintext[190];
        size_t maxLen = sizeof(maxPlaintext);
        size_t i;
        
        /* Initialize with pattern */
        for (i = 0; i < maxLen; i++) {
            maxPlaintext[i] = 0xAA;
        }
        
        keypair = generateRSAKeyPair(2048);
        if (!keypair) {
            fprintf(stderr, "Failed to generate keypair\\n");
            success = 0;
            goto cleanup;
        }
        
        if (!rsaEncrypt(keypair, maxPlaintext, maxLen, &ciphertext, &ciphertextLen)) {
            fprintf(stderr, "Encryption failed\\n");
            success = 0;
            goto cleanup;
        }
        
        if (!rsaDecrypt(keypair, ciphertext, ciphertextLen, &decrypted, &decryptedLen)) {
            fprintf(stderr, "Decryption failed\\n");
            success = 0;
            goto cleanup;
        }
        
        printf("Max size data match: %s\\n\\n", (maxLen == decryptedLen && memcmp(maxPlaintext, decrypted, maxLen) == 0) ? "YES" : "NO");
        
        free(ciphertext);
        free(decrypted);
        ciphertext = NULL;
        decrypted = NULL;
        EVP_PKEY_free(keypair);
        keypair = NULL;
    }
    
    /* Test case 4: Different key size (3072-bit) */
    printf("Test 4: 3072-bit RSA key\\n");
    {
        const char* message = "Testing 3072-bit RSA";
        size_t messageLen = strlen(message);
        
        keypair = generateRSAKeyPair(3072);
        if (!keypair) {
            fprintf(stderr, "Failed to generate keypair\\n");
            success = 0;
            goto cleanup;
        }
        
        if (!rsaEncrypt(keypair, (const uint8_t*)message, messageLen, &ciphertext, &ciphertextLen)) {
            fprintf(stderr, "Encryption failed\\n");
            success = 0;
            goto cleanup;
        }
        
        if (!rsaDecrypt(keypair, ciphertext, ciphertextLen, &decrypted, &decryptedLen)) {
            fprintf(stderr, "Decryption failed\\n");
            success = 0;
            goto cleanup;
        }
        
        printf("3072-bit match: %s\\n\\n", (messageLen == decryptedLen && memcmp(message, decrypted, messageLen) == 0) ? "YES" : "NO");
        
        free(ciphertext);
        free(decrypted);
        ciphertext = NULL;
        decrypted = NULL;
        EVP_PKEY_free(keypair);
        keypair = NULL;
    }
    
    /* Test case 5: Error handling - plaintext too large */
    printf("Test 5: Error handling\\n");
    {
        uint8_t tooLarge[300];
        size_t tooLargeLen = sizeof(tooLarge);
        size_t i;
        
        /* Initialize with pattern */
        for (i = 0; i < tooLargeLen; i++) {
            tooLarge[i] = 0xBB;
        }
        
        keypair = generateRSAKeyPair(2048);
        if (!keypair) {
            fprintf(stderr, "Failed to generate keypair\\n");
            success = 0;
            goto cleanup;
        }
        
        /* This should fail because plaintext is too large */
        if (!rsaEncrypt(keypair, tooLarge, tooLargeLen, &ciphertext, &ciphertextLen)) {
            printf("Caught expected error: Plaintext too large\\n");
        } else {
            printf("ERROR: Should have failed with too large plaintext\\n");
            free(ciphertext);
            ciphertext = NULL;
        }
        
        EVP_PKEY_free(keypair);
        keypair = NULL;
    }
    
cleanup:
    /* Clean up any remaining resources */
    if (ciphertext) {
        free(ciphertext);
    }
    if (decrypted) {
        free(decrypted);
    }
    if (keypair) {
        EVP_PKEY_free(keypair);
    }
    
    /* Clean up OpenSSL */
    EVP_cleanup();
    ERR_free_strings();
    
    return success ? 0 : 1;
}
