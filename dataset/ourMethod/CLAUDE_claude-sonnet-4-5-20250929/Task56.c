
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#define TOKEN_BYTES 32
#define MAX_USER_ID_LEN 256
#define HEX_TOKEN_LEN (TOKEN_BYTES * 2 + 1)

/* Convert binary data to hex string
 * Prevents binary data injection and provides safe string representation
 * Returns 0 on success, -1 on failure */
int bytes_to_hex(const unsigned char* bytes, size_t bytes_len, char* hex_out, size_t hex_size) {
    if (!bytes || !hex_out || hex_size < (bytes_len * 2 + 1)) {
        return -1;
    }
    
    for (size_t i = 0; i < bytes_len; i++) {
        /* Use snprintf with bounds checking to prevent buffer overflow (CWE-120) */
        int written = snprintf(hex_out + (i * 2), hex_size - (i * 2), "%02x", bytes[i]);
        if (written != 2) {
            return -1;
        }
    }
    hex_out[bytes_len * 2] = '\\0';
    return 0;
}

/* Generate a cryptographically secure authentication token
 * Uses OpenSSL RAND_bytes (CSPRNG) to prevent CWE-338
 * Returns 0 on success, -1 on failure */
int generate_token(char* token_out, size_t token_size) {
    unsigned char token_bytes[TOKEN_BYTES];
    
    /* Validate output buffer */
    if (!token_out || token_size < HEX_TOKEN_LEN) {
        fprintf(stderr, "Invalid output buffer\\n");
        return -1;
    }
    
    /* Initialize buffer to prevent information leakage */
    memset(token_bytes, 0, TOKEN_BYTES);
    memset(token_out, 0, token_size);
    
    /* Generate cryptographically secure random bytes
     * RAND_bytes uses OpenSSL's CSPRNG, preventing weak PRNG issues (CWE-338) */\n    if (RAND_bytes(token_bytes, TOKEN_BYTES) != 1) {\n        fprintf(stderr, "Failed to generate secure random token\\n");\n        /* Clear sensitive data before returning */\n        memset(token_bytes, 0, TOKEN_BYTES);\n        return -1;\n    }\n    \n    /* Convert to hex string for safe storage/transmission */\n    int result = bytes_to_hex(token_bytes, TOKEN_BYTES, token_out, token_size);\n    \n    /* Clear sensitive data from memory */\n    memset(token_bytes, 0, TOKEN_BYTES);\n    \n    return result;\n}\n\n/* Generate user-bound authentication token\n * Binds token to specific user ID to prevent token reuse\n * Returns 0 on success, -1 on failure */\nint generate_user_token(const char* user_id, char* token_out, size_t token_size) {\n    unsigned char random_bytes[TOKEN_BYTES];\n    unsigned char hash[EVP_MAX_MD_SIZE];\n    unsigned int hash_len = 0;\n    EVP_MD_CTX* ctx = NULL;\n    int result = -1;\n    \n    /* Validate inputs to prevent injection and buffer overflow */\n    if (!user_id || !token_out) {\n        fprintf(stderr, "Invalid input parameters\\n");\n        return -1;\n    }\n    \n    size_t user_id_len = strnlen(user_id, MAX_USER_ID_LEN + 1);\n    if (user_id_len == 0 || user_id_len > MAX_USER_ID_LEN) {\n        fprintf(stderr, "Invalid userId: must be 1-256 characters\\n");\n        return -1;\n    }\n    \n    if (token_size < (SHA256_DIGEST_LENGTH * 2 + 1)) {\n        fprintf(stderr, "Output buffer too small\\n");\n        return -1;\n    }\n    \n    /* Initialize buffers */\n    memset(random_bytes, 0, TOKEN_BYTES);\n    memset(hash, 0, EVP_MAX_MD_SIZE);\n    memset(token_out, 0, token_size);\n    \n    /* Generate secure random bytes */\n    if (RAND_bytes(random_bytes, TOKEN_BYTES) != 1) {\n        fprintf(stderr, "Failed to generate secure random bytes\\n");\n        goto cleanup;\n    }\n    \n    /* Create hash context */\n    ctx = EVP_MD_CTX_new();\n    if (!ctx) {\n        fprintf(stderr, "Failed to create hash context\\n");\n        goto cleanup;\n    }\n    \n    /* Initialize SHA-256 hash */\n    if (EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) != 1) {\n        fprintf(stderr, "Failed to initialize hash\\n");\n        goto cleanup;\n    }\n    \n    /* Hash user ID - binds token to specific user */\n    if (EVP_DigestUpdate(ctx, user_id, user_id_len) != 1) {\n        fprintf(stderr, "Failed to update hash with userId\\n");\n        goto cleanup;\n    }\n    \n    /* Hash random bytes - adds unpredictability */\n    if (EVP_DigestUpdate(ctx, random_bytes, TOKEN_BYTES) != 1) {\n        fprintf(stderr, "Failed to update hash with random data\\n");\n        goto cleanup;\n    }\n    \n    /* Finalize hash */\n    if (EVP_DigestFinal_ex(ctx, hash, &hash_len) != 1) {\n        fprintf(stderr, "Failed to finalize hash\\n");\n        goto cleanup;\n    }\n    \n    /* Convert hash to hex string */\n    result = bytes_to_hex(hash, hash_len, token_out, token_size);\n    \ncleanup:\n    /* Securely clear sensitive data from memory */\n    memset(random_bytes, 0, TOKEN_BYTES);\n    memset(hash, 0, EVP_MAX_MD_SIZE);\n    \n    /* Free context if allocated */\n    if (ctx) {\n        EVP_MD_CTX_free(ctx);\n    }\n    \n    return result;\n}\n\nint main(void) {\n    char token[HEX_TOKEN_LEN];\n    int status = 0;\n    \n    printf("Authentication Token Generator\\n");\n    printf("================================\\n\\n");\n    \n    /* Test case 1: Generate basic authentication token */\n    printf("Test 1 - Basic token generation:\\n");\n    memset(token, 0, sizeof(token));\n    if (generate_token(token, sizeof(token)) == 0) {\n        printf("Token: %s\\n", token);\n        printf("Length: %zu characters\\n\\n", strlen(token));\n    } else {\n        fprintf(stderr, "Test 1 failed\\n");\n        status = 1;\n    }\n    \n    /* Test case 2: Generate another token (should be different) */\n    printf("Test 2 - Second token (should be unique):\\n");\n    char token2[HEX_TOKEN_LEN];\n    memset(token2, 0, sizeof(token2));\n    if (generate_token(token2, sizeof(token2)) == 0) {\n        printf("Token: %s\\n", token2);\n        printf("Unique: %s\\n\\n", (strcmp(token, token2) != 0) ? "Yes" : "No");\n    } else {\n        fprintf(stderr, "Test 2 failed\\n");\n        status = 1;\n    }\n    \n    /* Test case 3: Generate user-bound token */\n    printf("Test 3 - User-bound token for 'user123':\\n");\n    char user_token1[SHA256_DIGEST_LENGTH * 2 + 1];\n    memset(user_token1, 0, sizeof(user_token1));\n    if (generate_user_token("user123", user_token1, sizeof(user_token1)) == 0) {\n        printf("Token: %s\\n\\n", user_token1);\n    } else {\n        fprintf(stderr, "Test 3 failed\\n");\n        status = 1;\n    }\n    \n    /* Test case 4: Generate another token for same user */\n    printf("Test 4 - Another token for 'user123' (should differ):\\n");\n    char user_token2[SHA256_DIGEST_LENGTH * 2 + 1];\n    memset(user_token2, 0, sizeof(user_token2));\n    if (generate_user_token("user123", user_token2, sizeof(user_token2)) == 0) {\n        printf("Token: %s\\n", user_token2);\n        printf("Unique: %s\\n\\n", (strcmp(user_token1, user_token2) != 0) ? "Yes" : "No");\n    } else {\n        fprintf(stderr, "Test 4 failed\\n");\n        status = 1;\n    }\n    \n    /* Test case 5: Generate token for different user */\n    printf("Test 5 - Token for 'admin456':\
");\n    char user_token3[SHA256_DIGEST_LENGTH * 2 + 1];\n    memset(user_token3, 0, sizeof(user_token3));\n    if (generate_user_token("admin456", user_token3, sizeof(user_token3)) == 0) {\n        printf("Token: %s\
", user_token3);\n        printf("Different from user123: %s\
\
", (strcmp(user_token3, user_token1) != 0) ? "Yes" : "No");\n    } else {\n        fprintf(stderr, "Test 5 failed\
");
        status = 1;
    }
    
    /* Clear all tokens from memory before exit */
    memset(token, 0, sizeof(token));
    memset(token2, 0, sizeof(token2));
    memset(user_token1, 0, sizeof(user_token1));
    memset(user_token2, 0, sizeof(user_token2));
    memset(user_token3, 0, sizeof(user_token3));
    
    return status;
}
