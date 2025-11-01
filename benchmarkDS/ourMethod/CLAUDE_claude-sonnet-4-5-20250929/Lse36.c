
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

/* Rules#6: Use environment variables for credentials */
#define MAX_USERNAME_LEN 255
#define MAX_PASSWORD_LEN 128
#define MIN_PASSWORD_LEN 12
#define SALT_SIZE 16
#define HASH_SIZE 32
#define ITERATIONS 210000

/* Rules#12: Generate cryptographically secure salt */
char* generate_salt(void) {
    unsigned char salt[SALT_SIZE];
    char* encoded = NULL;
    
    /* Rules#12: Use OpenSSL CSPRNG */
    if (RAND_bytes(salt, SALT_SIZE) != 1) {
        return NULL;
    }
    
    encoded = malloc(25);
    if (!encoded) {
        return NULL;
    }
    
    EVP_EncodeBlock((unsigned char*)encoded, salt, SALT_SIZE);
    return encoded;
}

/* Rules#9, Rules#10: Hash password using PBKDF2-HMAC-SHA256 */
char* hash_password(const char* password, const char* salt_str) {
    unsigned char salt[SALT_SIZE];
    unsigned char hash[HASH_SIZE];
    char* encoded = NULL;
    int salt_len;
    
    if (!password || !salt_str) {
        return NULL;
    }
    
    /* Base64 decode salt */
    salt_len = EVP_DecodeBlock(salt, (unsigned char*)salt_str, strlen(salt_str));
    if (salt_len < 0) {
        return NULL;
    }
    
    /* Rules#9: Use PBKDF2 with at least 210,000 iterations */
    if (PKCS5_PBKDF2_HMAC(password, strlen(password),
                          salt, salt_len,
                          ITERATIONS,
                          EVP_sha256(),
                          HASH_SIZE, hash) != 1) {
        return NULL;
    }
    
    /* Base64 encode result */
    encoded = malloc(45);
    if (!encoded) {
        OPENSSL_cleanse(hash, HASH_SIZE);
        return NULL;
    }
    
    EVP_EncodeBlock((unsigned char*)encoded, hash, HASH_SIZE);
    
    /* Rules#7: Clear sensitive data from memory */
    OPENSSL_cleanse(hash, HASH_SIZE);
    
    return encoded;
}

/* Rules#10: Constant-time comparison to prevent timing attacks */
int constant_time_equals(const char* a, const char* b) {
    if (!a || !b) {
        return 0;
    }
    
    size_t len_a = strlen(a);
    size_t len_b = strlen(b);
    
    if (len_a != len_b) {
        return 0;
    }
    
    return CRYPTO_memcmp(a, b, len_a) == 0;
}

/**
 * Login function that authenticates user credentials
 * Rules#3: Validate and sanitize all inputs
 */
int login(const char* username, const char* password) {
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    const char* query = "SELECT password_hash, salt FROM users WHERE username = ? LIMIT 1";
    int result = 0;
    const char* db_path;
    
    /* Rules#3: Validate inputs are not NULL */
    if (!username || !password) {
        return 0;
    }
    
    /* Rules#3: Validate username length */
    size_t username_len = strlen(username);
    if (username_len == 0 || username_len > MAX_USERNAME_LEN) {
        return 0;
    }
    
    /* Rules#15: Enforce password policy */
    size_t password_len = strlen(password);
    if (password_len < MIN_PASSWORD_LEN || password_len > MAX_PASSWORD_LEN) {
        return 0;
    }
    
    /* Rules#3: Validate username format to prevent SQL injection */
    for (size_t i = 0; i < username_len; i++) {
        char c = username[i];
        if (!isalnum(c) && c != '_' && c != '@' && c != '.' && c != '+' && c != '-') {
            return 0;
        }
    }
    
    /* Rules#6: Get database path from environment */
    db_path = getenv("DB_PATH");
    if (!db_path) {
        db_path = ":memory:";
    }
    
    /* Open database connection */
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        fprintf(stderr, "Authentication failed\\n");
        return 0;
    }
    
    /* Rules#2: Use parameterized query to prevent SQL injection */
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(db);
        fprintf(stderr, "Authentication failed\\n");
        return 0;
    }
    
    /* Bind username parameter */
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* stored_hash = (const char*)sqlite3_column_text(stmt, 0);
        const char* salt = (const char*)sqlite3_column_text(stmt, 1);
        
        if (stored_hash && salt) {
            /* Rules#11: Use unique salt for password hashing */
            char* computed_hash = hash_password(password, salt);
            
            if (computed_hash) {
                /* Rules#10: Constant-time comparison */
                if (constant_time_equals(stored_hash, computed_hash)) {
                    result = 1;
                }
                
                /* Clean up computed hash */
                OPENSSL_cleanse(computed_hash, strlen(computed_hash));
                free(computed_hash);
            }
        }
    }
    
    /* Clean up */
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

int main(void) {
    /* Test cases */
    printf("Test Case 1 - Valid credentials:\\n");
    printf("%d\\n", login("user1", "ValidPassword123!"));
    
    printf("\\nTest Case 2 - Invalid password:\\n");
    printf("%d\\n", login("user1", "WrongPassword"));
    
    printf("\\nTest Case 3 - Non-existent user:\\n");
    printf("%d\\n", login("nonexistent", "SomePassword123!"));
    
    printf("\\nTest Case 4 - NULL inputs:\\n");
    printf("%d\\n", login(NULL, NULL));
    
    printf("\\nTest Case 5 - SQL injection attempt:\\n");
    printf("%d\\n", login("admin' OR '1'='1", "password"));
    
    return 0;
}
