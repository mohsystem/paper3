
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>

/* Constants for security parameters */
#define SALT_SIZE 16
#define HASH_SIZE 32
#define PBKDF2_ITERATIONS 210000
#define MAX_USERS 100
#define MAX_USERNAME_LEN 64
#define MAX_PASSWORD_LEN 128

/* Structure to hold user credentials */
typedef struct {
    char username[MAX_USERNAME_LEN + 1];
    uint8_t salt[SALT_SIZE];
    uint8_t password_hash[HASH_SIZE];
    int active;
} UserCredentials;

/* User database */
typedef struct {
    UserCredentials users[MAX_USERS];
    int user_count;
} UserDatabase;

/* Securely clear memory containing sensitive data - CWE-14 mitigation */
void secure_clear(void* ptr, size_t len) {
    if (ptr == NULL || len == 0) return;
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (len--) *p++ = 0;
}

/* Generate cryptographically secure random salt - CWE-338 mitigation */
int generate_salt(uint8_t* salt) {
    if (salt == NULL) {
        return 0;
    }
    
    /* Use OpenSSL's secure random generator */\n    if (RAND_bytes(salt, SALT_SIZE) != 1) {\n        return 0;\n    }\n    return 1;\n}\n\n/* Hash password with PBKDF2-HMAC-SHA256 - CWE-327, CWE-759 mitigation */\nint hash_password(const char* password, const uint8_t* salt, uint8_t* hash) {\n    size_t password_len;\n    \n    /* Input validation - CWE-20 mitigation */\n    if (password == NULL || salt == NULL || hash == NULL) {\n        return 0;\n    }\n    \n    password_len = strlen(password);\n    if (password_len == 0 || password_len > MAX_PASSWORD_LEN) {\n        return 0;\n    }\n    \n    /* Use PBKDF2 with SHA-256, unique salt, and sufficient iterations */\n    if (PKCS5_PBKDF2_HMAC(password, (int)password_len,\n                          salt, SALT_SIZE,\n                          PBKDF2_ITERATIONS, EVP_sha256(),\n                          HASH_SIZE, hash) != 1) {\n        return 0;\n    }\n    \n    return 1;\n}\n\n/* Constant-time comparison to prevent timing attacks - CWE-208 mitigation */\nint constant_time_compare(const uint8_t* a, const uint8_t* b, size_t len) {\n    size_t i;\n    int result = 0;\n    \n    if (a == NULL || b == NULL) {\n        return 0;\n    }\n    \n    for (i = 0; i < len; i++) {\n        result |= a[i] ^ b[i];\n    }\n    return result == 0;\n}\n\n/* Validate username format - CWE-20 mitigation */\nint validate_username(const char* username) {\n    size_t len;\n    size_t i;\n    \n    if (username == NULL) {\n        return 0;\n    }\n    \n    len = strlen(username);\n    if (len == 0 || len > MAX_USERNAME_LEN) {\n        return 0;\n    }\n    \n    /* Check for alphanumeric characters, underscore, and hyphen only */\n    for (i = 0; i < len; i++) {\n        if (!isalnum((unsigned char)username[i]) && \n            username[i] != '_' && username[i] != '-') {\n            return 0;\n        }\n    }\n    \n    return 1;\n}\n\n/* Initialize user database */\nvoid init_database(UserDatabase* db) {\n    if (db == NULL) return;\n    memset(db, 0, sizeof(UserDatabase));\n    db->user_count = 0;\n}\n\n/* Register a new user */\nint register_user(UserDatabase* db, const char* username, const char* password) {\n    UserCredentials* new_user;\n    int i;\n    size_t password_len;\n    \n    /* Input validation - CWE-20 mitigation */\n    if (db == NULL || username == NULL || password == NULL) {\n        return 0;\n    }\n    \n    if (!validate_username(username)) {\n        return 0;\n    }\n    \n    password_len = strlen(password);\n    if (password_len == 0 || password_len > MAX_PASSWORD_LEN) {\n        return 0;\n    }\n    \n    /* Check if database is full - CWE-119 mitigation */\n    if (db->user_count >= MAX_USERS) {\n        return 0;\n    }\n    \n    /* Check if user already exists */\n    for (i = 0; i < db->user_count; i++) {\n        if (db->users[i].active && strcmp(db->users[i].username, username) == 0) {\n            return 0; /* User already exists */\n        }\n    }\n    \n    /* Get pointer to new user slot */\n    new_user = &db->users[db->user_count];\n    \n    /* Copy username with bounds check - CWE-120 mitigation */\n    strncpy(new_user->username, username, MAX_USERNAME_LEN);\n    new_user->username[MAX_USERNAME_LEN] = '\\0';\n    \n    /* Generate unique salt - CWE-759 mitigation */\n    if (!generate_salt(new_user->salt)) {\n        memset(new_user, 0, sizeof(UserCredentials));\n        return 0;\n    }\n    \n    /* Hash password with salt */\n    if (!hash_password(password, new_user->salt, new_user->password_hash)) {\n        secure_clear(new_user, sizeof(UserCredentials));\n        return 0;\n    }\n    \n    new_user->active = 1;\n    db->user_count++;\n    \n    return 1;\n}\n\n/* Authenticate a user */\nint authenticate_user(UserDatabase* db, const char* username, const char* password) {\n    int i;\n    UserCredentials* user_creds = NULL;\n    uint8_t computed_hash[HASH_SIZE];\n    int result;\n    size_t password_len;\n    \n    /* Input validation - CWE-20 mitigation */\n    if (db == NULL || username == NULL || password == NULL) {\n        return 0;\n    }\n    \n    if (!validate_username(username)) {\n        return 0;\n    }\n    \n    password_len = strlen(password);\n    if (password_len == 0 || password_len > MAX_PASSWORD_LEN) {\n        return 0;\n    }\n    \n    /* Find user */\n    for (i = 0; i < db->user_count; i++) {\n        if (db->users[i].active && strcmp(db->users[i].username, username) == 0) {\n            user_creds = &db->users[i];\n            break;\n        }\n    }\n    \n    if (user_creds == NULL) {\n        /* User not found - perform dummy hash to prevent timing attacks */\n        uint8_t dummy_salt[SALT_SIZE] = {0};\n        hash_password(password, dummy_salt, computed_hash);\n        secure_clear(computed_hash, HASH_SIZE);\n        return 0;\n    }\n    \n    /* Hash provided password with stored salt */\n    if (!hash_password(password, user_creds->salt, computed_hash)) {\n        secure_clear(computed_hash, HASH_SIZE);\n        return 0;\n    }\n    \n    /* Constant-time comparison - CWE-208 mitigation */\n    result = constant_time_compare(computed_hash, user_creds->password_hash, HASH_SIZE);\n    \n    /* Securely clear computed hash */\n    secure_clear(computed_hash, HASH_SIZE);\n    \n    return result;\n}\n\nint main(void) {\n    UserDatabase db;\n    \n    init_database(&db);\n    \n    printf("User Authentication System\\n");\n    printf("==========================\\n\\n");\n    \n    /* Test case 1: Register valid user */\n    printf("Test 1: Register user 'alice' with password 'SecurePass123!': ");\n    if (register_user(&db, "alice", "SecurePass123!")) {\n        printf("SUCCESS\\n");\n    } else {\n        printf("FAILED\\n");\n    }\n    \n    /* Test case 2: Authenticate with correct credentials */\n    printf("Test 2: Authenticate 'alice' with correct password: ");\n    if (authenticate_user(&db, "alice", "SecurePass123!")) {\n        printf("SUCCESS\\n");\n    } else {\n        printf("FAILED\\n");\n    }\n    \n    /* Test case 3: Authenticate with incorrect password */\n    printf("Test 3: Authenticate 'alice' with wrong password: ");\n    if (!authenticate_user(&db, "alice", "WrongPassword")) {\n        printf("SUCCESS (correctly rejected)\\n");\n    } else {\n        printf("FAILED\\n");\n    }\n    \n    /* Test case 4: Register another user */\n    printf("Test 4: Register user 'bob' with password 'AnotherSecure456!': ");\n    if (register_user(&db, "bob", "AnotherSecure456!")) {\n        printf("SUCCESS\\n");\n    } else {\n        printf("FAILED\\n");\n    }\n    \n    /* Test case 5: Attempt to register duplicate username */\n    printf("Test 5: Attempt to register duplicate user 'alice': ");\n    if (!register_user(&db, "alice", "DifferentPass789!")) {\n        printf("SUCCESS (correctly rejected)\
");\n    } else {\n        printf("FAILED\
");
    }
    
    /* Securely clear database before exit */
    secure_clear(&db, sizeof(UserDatabase));
    
    return 0;
}
