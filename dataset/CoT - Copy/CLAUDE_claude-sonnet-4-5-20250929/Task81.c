
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <openssl/sha.h>

/**
 * Securely computes SHA-256 hash of a certificate
 * @param certBytes The certificate bytes
 * @param length Length of the certificate bytes
 * @param outputHash Output buffer for hex string (must be at least 65 bytes)
 * @return 0 on success, -1 on error
 */
int computeCertificateHash(const unsigned char* certBytes, size_t length, char* outputHash) {
    if (certBytes == NULL || length == 0 || outputHash == NULL) {
        fprintf(stderr, "Certificate bytes and output hash cannot be null or empty\\n");
        return -1;
    }
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(certBytes, length, hash);
    
    // Convert to hex string
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(outputHash + (i * 2), "%02x", hash[i]);
    }
    outputHash[64] = '\\0';
    
    return 0;
}

/**
 * Constant-time string comparison to prevent timing attacks
 * @param a First string
 * @param b Second string
 * @return 1 if strings are equal, 0 otherwise
 */
int constantTimeCompare(const char* a, const char* b) {
    if (a == NULL || b == NULL) {
        return 0;
    }
    
    size_t len_a = strlen(a);
    size_t len_b = strlen(b);
    
    if (len_a != len_b) {
        return 0;
    }
    
    volatile unsigned char result = 0;
    for (size_t i = 0; i < len_a; i++) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

/**
 * Normalizes hash string by removing colons and whitespace and converting to lowercase
 * @param hash The hash string to normalize
 * @param output Output buffer for normalized hash
 * @param outputSize Size of output buffer
 * @return 0 on success, -1 on error
 */
int normalizeHash(const char* hash, char* output, size_t outputSize) {
    if (hash == NULL || output == NULL || outputSize == 0) {
        return -1;
    }
    
    size_t j = 0;
    for (size_t i = 0; hash[i] != '\\0' && j < outputSize - 1; i++) {
        char c = hash[i];
        if (c != ':' && !isspace(c)) {
            output[j++] = tolower(c);
        }
    }
    output[j] = '\\0';
    return 0;
}

/**
 * Validates hash format
 * @param hash The hash string to validate
 * @return 1 if valid SHA-256 hash format, 0 otherwise
 */
int isValidHashFormat(const char* hash) {
    if (hash == NULL || strlen(hash) != 64) {
        return 0;
    }
    
    for (size_t i = 0; i < 64; i++) {
        if (!isxdigit(hash[i])) {
            return 0;
        }
    }
    return 1;
}

/**
 * Securely checks if certificate matches known hash using constant-time comparison
 * @param certBytes The certificate bytes to check
 * @param length Length of the certificate bytes
 * @param knownHash The known hash to compare against (hexadecimal string)
 * @return 1 if hashes match, 0 otherwise
 */
int verifyCertificateHash(const unsigned char* certBytes, size_t length, const char* knownHash) {
    if (knownHash == NULL || strlen(knownHash) == 0) {
        fprintf(stderr, "Known hash cannot be null or empty\\n");
        return 0;
    }
    
    // Normalize known hash
    char normalizedKnownHash[128];
    if (normalizeHash(knownHash, normalizedKnownHash, sizeof(normalizedKnownHash)) != 0) {
        fprintf(stderr, "Failed to normalize known hash\\n");
        return 0;
    }
    
    // Validate known hash format
    if (!isValidHashFormat(normalizedKnownHash)) {
        fprintf(stderr, "Invalid hash format. Expected 64 hexadecimal characters.\\n");
        return 0;
    }
    
    // Compute certificate hash
    char computedHash[65];
    if (computeCertificateHash(certBytes, length, computedHash) != 0) {
        fprintf(stderr, "Failed to compute certificate hash\\n");
        return 0;
    }
    
    // Use constant-time comparison to prevent timing attacks
    return constantTimeCompare(computedHash, normalizedKnownHash);
}

int main() {
    printf("=== SSL Certificate Hash Verification Tests ===\\n\\n");
    
    // Test Case 1: Valid certificate with matching hash
    {
        const char* testCert1 = "Test Certificate Data 1";
        const unsigned char* certBytes1 = (const unsigned char*)testCert1;
        char hash1[65];
        computeCertificateHash(certBytes1, strlen(testCert1), hash1);
        int result1 = verifyCertificateHash(certBytes1, strlen(testCert1), hash1);
        printf("Test 1 - Valid certificate with matching hash:\\n");
        printf("  Computed Hash: %s\\n", hash1);
        printf("  Result: %s\\n", result1 ? "PASS (Hashes match)" : "FAIL");
        printf("\\n");
    }
    
    // Test Case 2: Valid certificate with non-matching hash
    {
        const char* testCert2 = "Test Certificate Data 2";
        const unsigned char* certBytes2 = (const unsigned char*)testCert2;
        const char* wrongHash = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
        int result2 = verifyCertificateHash(certBytes2, strlen(testCert2), wrongHash);
        printf("Test 2 - Valid certificate with non-matching hash:\\n");
        printf("  Expected: FAIL (Hashes don't match)\\n");
        printf("  Result: %s\\n", !result2 ? "PASS (Correctly identified mismatch)" : "FAIL");
        printf("\\n");
    }
    
    // Test Case 3: Hash with colons (common format)
    {
        const char* testCert3 = "Test Certificate Data 3";
        const unsigned char* certBytes3 = (const unsigned char*)testCert3;
        char hash3[65];
        computeCertificateHash(certBytes3, strlen(testCert3), hash3);
        
        // Add colons to hash
        char hashWithColons[128];
        int j = 0;
        for (int i = 0; i < 64; i += 2) {
            if (i > 0) hashWithColons[j++] = ':';
            hashWithColons[j++] = hash3[i];
            hashWithColons[j++] = hash3[i + 1];
        }
        hashWithColons[j] = '\\0';
        
        int result3 = verifyCertificateHash(certBytes3, strlen(testCert3), hashWithColons);
        printf("Test 3 - Hash with colon separators:\\n");
        printf("  Hash format: %.20s...\\n", hashWithColons);
        printf("  Result: %s\\n", result3 ? "PASS (Correctly handled format)" : "FAIL");
        printf("\\n");
    }
    
    // Test Case 4: Invalid hash format
    {
        const char* testCert4 = "Test Certificate Data 4";
        const unsigned char* certBytes4 = (const unsigned char*)testCert4;
        const char* invalidHash = "invalid_hash_format";
        int result4 = verifyCertificateHash(certBytes4, strlen(testCert4), invalidHash);
        printf("Test 4 - Invalid hash format:\\n");
        printf("  Expected: FAIL (Invalid format)\\n");
        printf("  Result: %s\\n", !result4 ? "PASS (Correctly rejected invalid format)" : "FAIL");
        printf("\\n");
    }
    
    // Test Case 5: Empty certificate bytes
    {
        const unsigned char* emptyCert = (const unsigned char*)"";
        const char* someHash = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
        int result5 = verifyCertificateHash(emptyCert, 0, someHash);
        printf("Test 5 - Empty certificate:\\n");
        printf("  Expected: FAIL (Empty certificate)\\n");
        printf("  Result: %s\\n", !result5 ? "PASS (Correctly handled empty input)" : "FAIL");
        printf("\\n");
    }
    
    return 0;
}
