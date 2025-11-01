
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <ctype.h>

/* Security constants */
#define SALT_LENGTH 16
#define PBKDF2_ITERATIONS 210000
#define HASH_LENGTH 32
#define MAX_USERNAME_LEN 100
#define MIN_PASSWORD_LEN 8
#define MAX_PASSWORD_LEN 128
#define DB_PATH "users.db"

/* Base64 encoding table */
static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* Base64 encode function */
char* base64_encode(const unsigned char* data, size_t len) {
    size_t out_len = ((len + 2) / 3) * 4;
    char* result = (char*)malloc(out_len + 1);
    if (!result) return NULL;
    
    size_t i, j;
    for (i = 0, j = 0; i < len; i += 3, j += 4) {
        unsigned int triple = (data[i] << 16);
        if (i + 1 < len) triple |= (data[i + 1] << 8);
        if (i + 2 < len) triple |= data[i + 2];
        
        result[j] = base64_chars[(triple >> 18) & 0x3F];
        result[j + 1] = base64_chars[(triple >> 12) & 0x3F];
        result[j + 2] = (i + 1 < len) ? base64_chars[(triple >> 6) & 0x3F] : '=';
        result[j + 3] = (i + 2 < len) ? base64_chars[triple & 0x3F] : '=';
    }
    result[out_len] = '\\0';
    return result;
}

/* Base64 decode function */
unsigned char* base64_decode(const char* input, size_t* out_len) {
    static const unsigned char decode_table[256] = {
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
        64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
        64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64
    };
    
    size_t in_len = strlen(input);
    *out_len = (in_len * 3) / 4;
    unsigned char* result = (unsigned char*)malloc(*out_len);
    if (!result) return NULL;
    
    size_t i, j;
    for (i = 0, j = 0; i < in_len; i += 4) {
        unsigned int sextet_a = decode_table[(unsigned char)input[i]];
        unsigned int sextet_b = decode_table[(unsigned char)input[i + 1]];
        unsigned int sextet_c = decode_table[(unsigned char)input[i + 2]];
        unsigned int sextet_d = decode_table[(unsigned char)input[i + 3]];
        
        unsigned int triple = (sextet_a << 18) + (sextet_b << 12) + (sextet_c << 6) + sextet_d;
        
        if (j < *out_len) result[j++] = (triple >> 16) & 0xFF;
        if (input[i + 2] != '=' && j < *out_len) result[j++] = (triple >> 8) & 0xFF;
        if (input[i + 3] != '=' && j < *out_len) result[j++] = triple & 0xFF;
    }
    *out_len = j;
    return result;
}

/* Generate cryptographically secure random salt */
int generate_salt(unsigned char* salt, int length) {
    /* Use OpenSSL CSPRNG */
    if (RAND_bytes(salt, length) != 1) {
        return 0;
    }
    return 1;
}

/* Hash password using PBKDF2-HMAC-SHA256 */
int hash_password(const char* password, const unsigned char* salt, 
                  unsigned char* hash, int hash_len) {
    /* Use PBKDF2 with SHA-256 and high iteration count */
    if (PKCS5_PBKDF2_HMAC(password, strlen(password),
                          salt, SALT_LENGTH,
                          PBKDF2_ITERATIONS,
                          EVP_sha256(),
                          hash_len, hash) != 1) {
        return 0;
    }
    return 1;
}

/* Validate username */
int validate_username(const char* username) {
    size_t len;
    size_t i;
    
    if (!username) return 0;
    
    len = strlen(username);
    if (len == 0 || len > MAX_USERNAME_LEN) return 0;
    
    /* Allow only alphanumeric and underscore */
    for (i = 0; i < len; i++) {
        if (!isalnum((unsigned char)username[i]) && username[i] != '_') {
            return 0;
        }
    }
    return 1;
}

/* Constant-time comparison to prevent timing attacks */
int constant_time_compare(const unsigned char* a, const unsigned char* b, size_t len) {
    unsigned char result = 0;
    size_t i;
    for (i = 0; i < len; i++) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

/* Register user */
int register_user(const char* username, const char* password) {
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    unsigned char salt[SALT_LENGTH];
    unsigned char hash[HASH_LENGTH];
    char* salt_b64 = NULL;
    char* hash_b64 = NULL;
    int result = 0;
    size_t password_len;
    
    /* Input validation */
    if (!validate_username(username)) {
        return 0;
    }
    
    if (!password) return 0;
    password_len = strlen(password);
    if (password_len < MIN_PASSWORD_LEN || password_len > MAX_PASSWORD_LEN) {
        return 0;
    }
    
    /* Open database */
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return 0;
    }
    
    /* Create table */
    const char* create_table = 
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT UNIQUE NOT NULL, "
        "password_hash TEXT NOT NULL, "
        "salt TEXT NOT NULL)";
    
    if (sqlite3_exec(db, create_table, NULL, NULL, NULL) != SQLITE_OK) {
        sqlite3_close(db);
        return 0;
    }
    
    /* Generate unique cryptographically secure salt */
    if (!generate_salt(salt, SALT_LENGTH)) {
        sqlite3_close(db);
        return 0;
    }
    
    /* Hash password with PBKDF2 */
    if (!hash_password(password, salt, hash, HASH_LENGTH)) {
        /* Clear sensitive data using memset_s or explicit_bzero if available */
        memset(hash, 0, HASH_LENGTH);
        memset(salt, 0, SALT_LENGTH);
        sqlite3_close(db);
        return 0;
    }
    
    /* Encode to base64 */
    salt_b64 = base64_encode(salt, SALT_LENGTH);
    hash_b64 = base64_encode(hash, HASH_LENGTH);
    
    /* Clear sensitive data from memory */
    memset(hash, 0, HASH_LENGTH);
    memset(salt, 0, SALT_LENGTH);
    
    if (!salt_b64 || !hash_b64) {
        goto cleanup;
    }
    
    /* Use parameterized query to prevent SQL injection */
    const char* insert_sql = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";
    
    if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL) != SQLITE_OK) {
        goto cleanup;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hash_b64, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, salt_b64, -1, SQLITE_TRANSIENT);
    
    result = (sqlite3_step(stmt) == SQLITE_DONE);
    
cleanup:
    if (stmt) sqlite3_finalize(stmt);
    if (salt_b64) free(salt_b64);
    if (hash_b64) free(hash_b64);
    sqlite3_close(db);
    
    return result;
}

/* Verify user credentials */
int verify_user(const char* username, const char* password) {
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    int authenticated = 0;
    unsigned char computed_hash[HASH_LENGTH];
    
    /* Input validation */
    if (!username || !password || strlen(username) == 0 || strlen(password) == 0) {
        return 0;
    }
    
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return 0;
    }
    
    /* Use parameterized query */
    const char* select_sql = "SELECT password_hash, salt FROM users WHERE username = ?";
    
    if (sqlite3_prepare_v2(db, select_sql, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(db);
        return 0;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* stored_hash_b64 = (const char*)sqlite3_column_text(stmt, 0);
        const char* stored_salt_b64 = (const char*)sqlite3_column_text(stmt, 1);
        
        if (stored_hash_b64 && stored_salt_b64) {
            size_t stored_hash_len, stored_salt_len;
            unsigned char* stored_hash = base64_decode(stored_hash_b64, &stored_hash_len);
            unsigned char* stored_salt = base64_decode(stored_salt_b64, &stored_salt_len);
            
            if (stored_hash && stored_salt && stored_salt_len == SALT_LENGTH) {
                /* Hash provided password with stored salt */
                if (hash_password(password, stored_salt, computed_hash, HASH_LENGTH)) {
                    /* Constant-time comparison */
                    authenticated = constant_time_compare(stored_hash, computed_hash, HASH_LENGTH);
                }
                
                /* Clear sensitive data */
                memset(computed_hash, 0, HASH_LENGTH);
            }
            
            if (stored_hash) free(stored_hash);
            if (stored_salt) free(stored_salt);
        }
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return authenticated;
}

int main(void) {
    /* Test case 1: Valid registration */
    printf("Test 1 - Valid registration: %s\\n", 
           register_user("user1", "StrongP@ss123") ? "true" : "false");
    
    /* Test case 2: Valid login */
    printf("Test 2 - Valid login: %s\\n", 
           verify_user("user1", "StrongP@ss123") ? "true" : "false");
    
    /* Test case 3: Invalid login (wrong password) */
    printf("Test 3 - Invalid password: %s\\n", 
           verify_user("user1", "WrongPassword") ? "true" : "false");
    
    /* Test case 4: Invalid username (SQL injection attempt) */
    printf("Test 4 - SQL injection attempt: %s\\n", 
           register_user("admin' OR '1'='1", "password123") ? "true" : "false");
    
    /* Test case 5: Weak password (too short) */
    printf("Test 5 - Weak password: %s\\n", 
           register_user("user2", "weak") ? "true" : "false");
    
    return 0;
}
