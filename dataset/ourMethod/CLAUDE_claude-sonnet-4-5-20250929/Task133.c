
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

/* Password reset functionality with secure token generation and validation */
/* Uses PBKDF2-HMAC-SHA256 for password hashing with unique salts */

#define SALT_SIZE 16
#define HASH_SIZE 32
#define TOKEN_SIZE 32
#define PBKDF2_ITERATIONS 210000
#define TOKEN_EXPIRY_SECONDS 3600
#define MAX_USERNAME_LEN 256
#define MAX_PASSWORD_LEN 256

typedef struct {
    char username[MAX_USERNAME_LEN];
    unsigned char password_hash[HASH_SIZE];
    unsigned char salt[SALT_SIZE];
    unsigned char reset_token[TOKEN_SIZE];
    int has_token;
    long token_expiry;
} UserRecord;

/* Secure comparison to prevent timing attacks */
static int constant_time_compare(const unsigned char* a, const unsigned char* b, size_t len) {
    unsigned char result = 0;
    size_t i;
    
    if (!a || !b) {
        return 0;
    }
    
    for (i = 0; i < len; i++) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

/* Generate cryptographically secure random bytes */
static int generate_random_bytes(unsigned char* buffer, int size) {
    if (!buffer || size <= 0) {
        return 0;
    }
    /* Use OpenSSL's CSPRNG */\n    if (RAND_bytes(buffer, size) != 1) {\n        return 0;\n    }\n    return 1;\n}\n\n/* Hash password with PBKDF2-HMAC-SHA256 */\nstatic int hash_password(const char* password, const unsigned char* salt, unsigned char* hash) {\n    size_t password_len;\n    \n    if (!password || !salt || !hash) {\n        return 0;\n    }\n    \n    password_len = strnlen(password, MAX_PASSWORD_LEN);\n    if (password_len == 0 || password_len >= MAX_PASSWORD_LEN) {\n        return 0;\n    }\n    \n    /* Use PBKDF2 with SHA-256 and 210,000 iterations */\n    if (PKCS5_PBKDF2_HMAC(password, (int)password_len,\n                          salt, SALT_SIZE,\n                          PBKDF2_ITERATIONS,\n                          EVP_sha256(),\n                          HASH_SIZE, hash) != 1) {\n        return 0;\n    }\n    return 1;\n}\n\n/* Validate password strength */\nstatic int validate_password_strength(const char* password) {\n    size_t len;\n    size_t i;\n    int has_upper = 0, has_lower = 0, has_digit = 0, has_special = 0;\n    \n    if (!password) {\n        return 0;\n    }\n    \n    len = strnlen(password, MAX_PASSWORD_LEN);\n    if (len < 8 || len >= MAX_PASSWORD_LEN) {\n        return 0;\n    }\n    \n    for (i = 0; i < len; i++) {\n        if (isupper((unsigned char)password[i])) {\n            has_upper = 1;\n        } else if (islower((unsigned char)password[i])) {\n            has_lower = 1;\n        } else if (isdigit((unsigned char)password[i])) {\n            has_digit = 1;\n        } else {\n            has_special = 1;\n        }\n    }\n    \n    return has_upper && has_lower && has_digit && has_special;\n}\n\n/* Initialize user record */\nvoid init_user_record(UserRecord* user) {\n    if (!user) {\n        return;\n    }\n    memset(user, 0, sizeof(UserRecord));\n}\n\n/* Generate a secure password reset token */\nint generate_reset_token(UserRecord* user) {\n    if (!user || strnlen(user->username, MAX_USERNAME_LEN) == 0) {\n        return 0;\n    }\n    \n    /* Generate cryptographically secure random token */\n    if (!generate_random_bytes(user->reset_token, TOKEN_SIZE)) {\n        memset(user->reset_token, 0, TOKEN_SIZE);\n        user->has_token = 0;\n        return 0;\n    }\n    \n    /* Set token expiry (current time + 1 hour) */\n    user->token_expiry = (long)time(NULL) + TOKEN_EXPIRY_SECONDS;\n    user->has_token = 1;\n    return 1;\n}\n\n/* Validate reset token */\nint validate_reset_token(const UserRecord* user, const unsigned char* token) {\n    long current_time;\n    \n    if (!user || !token || !user->has_token) {\n        return 0;\n    }\n    \n    /* Check token expiry */\n    current_time = (long)time(NULL);\n    if (current_time > user->token_expiry) {\n        return 0;\n    }\n    \n    /* Constant-time comparison to prevent timing attacks */\n    return constant_time_compare(user->reset_token, token, TOKEN_SIZE);\n}\n\n/* Hash and store new password */\nint reset_password(UserRecord* user, const char* new_password) {\n    if (!user || !new_password) {\n        return 0;\n    }\n    \n    /* Validate password strength */\n    if (!validate_password_strength(new_password)) {\n        return 0;\n    }\n    \n    /* Generate new unique salt */\n    if (!generate_random_bytes(user->salt, SALT_SIZE)) {\n        memset(user->salt, 0, SALT_SIZE);\n        return 0;\n    }\n    \n    /* Hash password with new salt */\n    if (!hash_password(new_password, user->salt, user->password_hash)) {\n        /* Clear sensitive data on failure */\n        memset(user->password_hash, 0, HASH_SIZE);\n        memset(user->salt, 0, SALT_SIZE);\n        return 0;\n    }\n    \n    /* Clear reset token after successful password reset */\n    memset(user->reset_token, 0, TOKEN_SIZE);\n    user->has_token = 0;\n    user->token_expiry = 0;\n    \n    return 1;\n}\n\n/* Verify password against stored hash */\nint verify_password(const UserRecord* user, const char* password) {\n    unsigned char computed_hash[HASH_SIZE];\n    \n    if (!user || !password) {\n        return 0;\n    }\n    \n    memset(computed_hash, 0, HASH_SIZE);\n    \n    if (!hash_password(password, user->salt, computed_hash)) {\n        memset(computed_hash, 0, HASH_SIZE);\n        return 0;\n    }\n    \n    int result = constant_time_compare(user->password_hash, computed_hash, HASH_SIZE);\n    \n    /* Clear computed hash from memory */\n    memset(computed_hash, 0, HASH_SIZE);\n    \n    return result;\n}\n\nint main(void) {\n    UserRecord user;\n    unsigned char saved_token[TOKEN_SIZE];\n    unsigned char invalid_token[TOKEN_SIZE];\n    \n    printf("=== Password Reset Functionality Test Cases ===\\n");\n    \n    /* Test Case 1: Create user and set initial password */\n    printf("\\nTest 1: Create user with initial password\\n");\n    init_user_record(&user);\n    strncpy(user.username, "testuser1", MAX_USERNAME_LEN - 1);\n    user.username[MAX_USERNAME_LEN - 1] = '\\0';
    
    if (reset_password(&user, "SecureP@ss123")) {
        printf("Initial password set successfully\\n");
    } else {
        printf("Failed to set initial password\\n");
    }
    
    /* Test Case 2: Generate reset token */
    printf("\\nTest 2: Generate password reset token\\n");
    if (generate_reset_token(&user)) {
        printf("Reset token generated (length: %d bytes)\\n", TOKEN_SIZE);
        memcpy(saved_token, user.reset_token, TOKEN_SIZE);
    } else {
        printf("Failed to generate reset token\\n");
    }
    
    /* Test Case 3: Validate correct reset token */
    printf("\\nTest 3: Validate correct reset token\\n");
    if (validate_reset_token(&user, saved_token)) {
        printf("Token validation successful\\n");
    } else {
        printf("Token validation failed\\n");
    }
    
    /* Test Case 4: Validate incorrect reset token */
    printf("\\nTest 4: Validate incorrect reset token\\n");
    memset(invalid_token, 0xFF, TOKEN_SIZE);
    if (!validate_reset_token(&user, invalid_token)) {
        printf("Invalid token correctly rejected\\n");
    } else {
        printf("Invalid token incorrectly accepted\\n");
    }
    
    /* Test Case 5: Reset password with valid token and verify */
    printf("\\nTest 5: Reset password and verify\\n");
    if (reset_password(&user, "NewSecure@Pass456")) {
        printf("Password reset successfully\\n");
        if (verify_password(&user, "NewSecure@Pass456")) {
            printf("New password verified successfully\\n");
        } else {
            printf("New password verification failed\\n");
        }
        if (!verify_password(&user, "SecureP@ss123")) {
            printf("Old password correctly rejected\\n");
        } else {
            printf("Old password incorrectly accepted\\n");
        }
    } else {
        printf("Failed to reset password\\n");
    }
    
    /* Clear sensitive data before exit */
    memset(&user, 0, sizeof(UserRecord));
    memset(saved_token, 0, TOKEN_SIZE);
    memset(invalid_token, 0, TOKEN_SIZE);
    
    return 0;
}
