
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#define SALT_SIZE 16
#define HASH_SIZE 32
#define ITERATIONS 210000
#define HEX_HASH_SIZE 96  // 32 hex salt + 64 hex hash

// Securely generates cryptographically random salt - CWE-330, CWE-338, CWE-759
int generate_salt(uint8_t *salt, size_t salt_size) {
    // Validate input pointer - CWE-476: NULL Pointer Dereference
    if (salt == NULL || salt_size != SALT_SIZE) {
        return 0;
    }
    
    // Use OpenSSL's cryptographically secure RNG\n    int result = RAND_bytes(salt, (int)salt_size);\n    if (result != 1) {\n        return 0;\n    }\n    \n    return 1;\n}\n\n// Computes PBKDF2-HMAC-SHA256 hash with salt - CWE-326, CWE-916\nint hash_password_with_salt(const char *password, size_t password_len,\n                            const uint8_t *salt, size_t salt_size,\n                            char *output_hex, size_t output_size) {\n    // Validate all inputs - CWE-20: Improper Input Validation\n    if (password == NULL || salt == NULL || output_hex == NULL) {\n        return 0;\n    }\n    \n    if (password_len == 0 || password_len > 1024) {\n        return 0;\n    }\n    \n    if (salt_size != SALT_SIZE) {\n        return 0;\n    }\n    \n    if (output_size < HEX_HASH_SIZE + 1) {\n        return 0;\n    }\n    \n    uint8_t hash[HASH_SIZE];\n    memset(hash, 0, HASH_SIZE);\n    \n    // Derive key using PBKDF2-HMAC-SHA256 with 210000 iterations\n    int result = PKCS5_PBKDF2_HMAC(\n        password,\n        (int)password_len,\n        salt,\n        (int)salt_size,\n        ITERATIONS,\n        EVP_sha256(),\n        HASH_SIZE,\n        hash\n    );\n    \n    if (result != 1) {\n        OPENSSL_cleanse(hash, HASH_SIZE);\n        return 0;\n    }\n    \n    // Convert salt and hash to hex string - CWE-134: Use of Externally-Controlled Format String\n    size_t pos = 0;\n    \n    // Append salt (16 bytes = 32 hex chars)\n    for (size_t i = 0; i < salt_size && pos < output_size - 1; i++) {\n        int written = snprintf(output_hex + pos, output_size - pos, "%02x", salt[i]);\n        if (written < 0 || written >= (int)(output_size - pos)) {\n            OPENSSL_cleanse(hash, HASH_SIZE);\n            return 0;\n        }\n        pos += (size_t)written;\n    }\n    \n    // Append hash (32 bytes = 64 hex chars)\n    for (size_t i = 0; i < HASH_SIZE && pos < output_size - 1; i++) {\n        int written = snprintf(output_hex + pos, output_size - pos, "%02x", hash[i]);\n        if (written < 0 || written >= (int)(output_size - pos)) {\n            OPENSSL_cleanse(hash, HASH_SIZE);\n            return 0;\n        }\n        pos += (size_t)written;\n    }\n    \n    output_hex[pos] = '\\0';\n    \n    // Securely clear sensitive data - CWE-244: Improper Clearing of Heap Memory\n    OPENSSL_cleanse(hash, HASH_SIZE);\n    \n    return 1;\n}\n\n// Verify password against stored hash with constant-time comparison\nint verify_password(const char *password, size_t password_len, const char *stored_hash) {\n    // Validate inputs\n    if (password == NULL || stored_hash == NULL) {\n        return 0;\n    }\n    \n    size_t stored_len = strlen(stored_hash);\n    if (stored_len != HEX_HASH_SIZE) {\n        return 0;\n    }\n    \n    // Extract salt from stored hash (first 32 hex chars)\n    uint8_t salt[SALT_SIZE];\n    for (size_t i = 0; i < SALT_SIZE; i++) {\n        char byte_str[3] = {stored_hash[i * 2], stored_hash[i * 2 + 1], '\\0'};
        unsigned int byte_val;
        if (sscanf(byte_str, "%02x", &byte_val) != 1) {
            return 0;
        }
        salt[i] = (uint8_t)byte_val;
    }
    
    // Compute hash with extracted salt
    char computed_hash[HEX_HASH_SIZE + 1];
    if (!hash_password_with_salt(password, password_len, salt, SALT_SIZE, 
                                  computed_hash, sizeof(computed_hash))) {
        return 0;
    }
    
    // Constant-time comparison - CWE-208: Observable Timing Discrepancy
    int matches = 1;
    for (size_t i = 0; i < HEX_HASH_SIZE; i++) {
        matches &= (computed_hash[i] == stored_hash[i]);
    }
    
    // Securely clear computed hash
    OPENSSL_cleanse(computed_hash, sizeof(computed_hash));
    
    return matches;
}

int main(void) {
    printf("Password Hashing with Salt - Test Suite\\n");
    printf("========================================\\n\\n");
    
    // Test case 1: Basic password hashing
    printf("Test 1: Hash password with generated salt\\n");
    const char *password1 = "SecurePassword123!";
    uint8_t salt1[SALT_SIZE];
    char hash1[HEX_HASH_SIZE + 1];
    
    if (!generate_salt(salt1, SALT_SIZE)) {
        fprintf(stderr, "Failed to generate salt\\n");
        return 1;
    }
    
    if (!hash_password_with_salt(password1, strlen(password1), salt1, SALT_SIZE, 
                                  hash1, sizeof(hash1))) {
        fprintf(stderr, "Failed to hash password\\n");
        return 1;
    }
    
    printf("Hash (salt+hash): %.40s...\\n", hash1);
    printf("Length: %zu chars (32 salt + 64 hash)\\n\\n", strlen(hash1));
    
    // Test case 2: Verify correct password
    printf("Test 2: Verify correct password\\n");
    int valid = verify_password(password1, strlen(password1), hash1);
    printf("Verification result: %s\\n\\n", valid ? "PASS" : "FAIL");
    
    // Test case 3: Verify wrong password
    printf("Test 3: Verify incorrect password\\n");
    const char *wrong_password = "WrongPassword";
    int invalid = verify_password(wrong_password, strlen(wrong_password), hash1);
    printf("Verification result: %s (correctly rejected)\\n\\n", 
           invalid ? "FAIL" : "PASS");
    
    // Test case 4: Same password, different salt
    printf("Test 4: Same password with different salt\\n");
    uint8_t salt2[SALT_SIZE];
    char hash2[HEX_HASH_SIZE + 1];
    
    if (!generate_salt(salt2, SALT_SIZE)) {
        fprintf(stderr, "Failed to generate salt\\n");
        return 1;
    }
    
    if (!hash_password_with_salt(password1, strlen(password1), salt2, SALT_SIZE,
                                  hash2, sizeof(hash2))) {
        fprintf(stderr, "Failed to hash password\\n");
        return 1;
    }
    
    printf("Hash 1: %.40s...\\n", hash1);
    printf("Hash 2: %.40s...\\n", hash2);
    printf("Hashes different: %s\\n\\n", strcmp(hash1, hash2) != 0 ? "PASS" : "FAIL");
    
    // Test case 5: Complex password
    printf("Test 5: Complex password with special characters\\n");
    const char *password3 = "P@ssw0rd!#$%%^&*()_+";
    uint8_t salt3[SALT_SIZE];
    char hash3[HEX_HASH_SIZE + 1];
    
    if (!generate_salt(salt3, SALT_SIZE)) {
        fprintf(stderr, "Failed to generate salt\\n");
        return 1;
    }
    
    if (!hash_password_with_salt(password3, strlen(password3), salt3, SALT_SIZE,
                                  hash3, sizeof(hash3))) {
        fprintf(stderr, "Failed to hash password\\n");
        return 1;
    }
    
    int valid3 = verify_password(password3, strlen(password3), hash3);
    printf("Hash computed and verified: %s\\n", valid3 ? "PASS" : "FAIL");
    
    // Securely clear sensitive data before exit
    OPENSSL_cleanse(salt1, SALT_SIZE);
    OPENSSL_cleanse(salt2, SALT_SIZE);
    OPENSSL_cleanse(salt3, SALT_SIZE);
    OPENSSL_cleanse(hash1, sizeof(hash1));
    OPENSSL_cleanse(hash2, sizeof(hash2));
    OPENSSL_cleanse(hash3, sizeof(hash3));
    
    return 0;
}
