
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

/**
 * Generates a cryptographically secure random salt
 * @param length The length of the salt in bytes
 * @param output Buffer to store the hexadecimal salt (must be at least length*2+1 bytes)
 * @return 0 on success, -1 on failure
 */
int generate_salt(int length, char* output) {
    if (output == NULL || length <= 0) {
        return -1;
    }
    
    unsigned char* salt = (unsigned char*)malloc(length);
    if (salt == NULL) {
        return -1;
    }
    
    if (RAND_bytes(salt, length) != 1) {
        free(salt);
        return -1;
    }
    
    // Convert to hexadecimal
    for (int i = 0; i < length; i++) {
        sprintf(output + (i * 2), "%02x", salt[i]);
    }
    output[length * 2] = '\\0';
    
    free(salt);
    return 0;
}

/**
 * Computes SHA-256 hash of password combined with salt
 * @param password The password to hash
 * @param salt The salt to combine with password
 * @param output Buffer to store the hexadecimal hash (must be at least 65 bytes)
 * @return 0 on success, -1 on failure
 */
int hash_password_with_salt(const char* password, const char* salt, char* output) {
    if (password == NULL || salt == NULL || output == NULL) {
        return -1;
    }
    
    // Combine password and salt
    size_t password_len = strlen(password);
    size_t salt_len = strlen(salt);
    size_t combined_len = password_len + salt_len;
    
    char* combined = (char*)malloc(combined_len + 1);
    if (combined == NULL) {
        return -1;
    }
    
    strcpy(combined, password);
    strcat(combined, salt);
    
    // Compute SHA-256 hash
    unsigned char hash[SHA256_DIGEST_LENGTH];
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    if (context == NULL) {
        free(combined);
        return -1;
    }
    
    if (EVP_DigestInit_ex(context, EVP_sha256(), NULL) != 1 ||
        EVP_DigestUpdate(context, combined, combined_len) != 1 ||
        EVP_DigestFinal_ex(context, hash, NULL) != 1) {
        EVP_MD_CTX_free(context);
        free(combined);
        return -1;
    }
    
    EVP_MD_CTX_free(context);
    free(combined);
    
    // Convert to hexadecimal string
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[SHA256_DIGEST_LENGTH * 2] = '\\0';
    
    return 0;
}

/**
 * Verifies a password against a stored hash using constant-time comparison
 * @param password The password to verify
 * @param salt The salt used in original hash
 * @param stored_hash The stored hash to compare against
 * @return 1 if password matches, 0 otherwise, -1 on error
 */
int verify_password(const char* password, const char* salt, const char* stored_hash) {
    if (password == NULL || salt == NULL || stored_hash == NULL) {
        return -1;
    }
    
    char computed_hash[SHA256_DIGEST_LENGTH * 2 + 1];
    if (hash_password_with_salt(password, salt, computed_hash) != 0) {
        return -1;
    }
    
    // Constant-time comparison to prevent timing attacks
    size_t len = strlen(stored_hash);
    if (strlen(computed_hash) != len) {
        return 0;
    }
    
    volatile unsigned char result = 0;
    for (size_t i = 0; i < len; i++) {
        result |= computed_hash[i] ^ stored_hash[i];
    }
    
    return result == 0 ? 1 : 0;
}

int main() {
    printf("=== Password Hashing with Salt - Test Cases ===\\n\\n");
    
    // Test Case 1: Basic password hashing
    printf("Test Case 1: Basic password hashing\\n");
    const char* password1 = "MySecurePassword123!";
    char salt1[33];
    char hash1[65];
    generate_salt(16, salt1);
    hash_password_with_salt(password1, salt1, hash1);
    printf("Password: %s\\n", password1);
    printf("Salt: %s\\n", salt1);
    printf("Hash: %s\\n", hash1);
    printf("Verification: %s\\n\\n", verify_password(password1, salt1, hash1) ? "true" : "false");
    
    // Test Case 2: Different password, same salt
    printf("Test Case 2: Different password, same salt\\n");
    const char* password2 = "DifferentPassword456#";
    char hash2[65];
    hash_password_with_salt(password2, salt1, hash2);
    printf("Password: %s\\n", password2);
    printf("Salt: %s\\n", salt1);
    printf("Hash: %s\\n", hash2);
    printf("Hashes are different: %s\\n\\n", strcmp(hash1, hash2) != 0 ? "true" : "false");
    
    // Test Case 3: Same password, different salt
    printf("Test Case 3: Same password, different salt\\n");
    char salt3[33];
    char hash3[65];
    generate_salt(16, salt3);
    hash_password_with_salt(password1, salt3, hash3);
    printf("Password: %s\\n", password1);
    printf("Salt: %s\\n", salt3);
    printf("Hash: %s\\n", hash3);
    printf("Hashes are different: %s\\n\\n", strcmp(hash1, hash3) != 0 ? "true" : "false");
    
    // Test Case 4: Complex password with special characters
    printf("Test Case 4: Complex password with special characters\\n");
    const char* password4 = "P@ssw0rd!#$%^&*()_+-=[]{}|;:',.<>?/~`";
    char salt4[65];
    char hash4[65];
    generate_salt(32, salt4);
    hash_password_with_salt(password4, salt4, hash4);
    printf("Password: %s\\n", password4);
    printf("Salt: %s\\n", salt4);
    printf("Hash: %s\\n", hash4);
    printf("Verification: %s\\n\\n", verify_password(password4, salt4, hash4) ? "true" : "false");
    
    // Test Case 5: Wrong password verification
    printf("Test Case 5: Wrong password verification\\n");
    const char* wrong_password = "WrongPassword";
    int verification_result = verify_password(wrong_password, salt1, hash1);
    printf("Correct Password: %s\\n", password1);
    printf("Wrong Password: %s\\n", wrong_password);
    printf("Verification with wrong password: %s\\n\\n", verification_result ? "true" : "false");
    
    return 0;
}
