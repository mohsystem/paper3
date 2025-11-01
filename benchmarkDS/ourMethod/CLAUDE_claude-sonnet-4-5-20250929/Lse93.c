
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

#define SALT_LENGTH 16
#define USERNAME_MIN 3
#define USERNAME_MAX 50
#define PASSWORD_MIN 8
#define PASSWORD_MAX 128
#define BASE64_ENCODE_OUT_SIZE(s) (((s) + 2) / 3 * 4 + 1)

/* Base64 encoding table */
static const char base64_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
 * Encodes binary data to base64 string.
 * Caller must free the returned string.
 */
char* base64_encode(const unsigned char* data, size_t input_length) {
    size_t output_length = BASE64_ENCODE_OUT_SIZE(input_length);
    char* encoded_data = (char*)malloc(output_length);
    if (encoded_data == NULL) {
        return NULL;
    }
    
    size_t i, j;
    for (i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;
        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;
        
        encoded_data[j++] = base64_chars[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 0 * 6) & 0x3F];
    }
    
    /* Add padding */
    static const int mod_table[] = {0, 2, 1};
    for (i = 0; i < mod_table[input_length % 3]; i++) {
        encoded_data[output_length - 2 - i] = '=';
    }
    
    encoded_data[output_length - 1] = '\\0';
    return encoded_data;
}

/**
 * Generates cryptographically secure random salt using OpenSSL.
 * Caller must free the returned string.
 */
char* generate_salt(void) {
    unsigned char salt[SALT_LENGTH];
    
    /* Use OpenSSL's CSPRNG */\n    if (RAND_bytes(salt, SALT_LENGTH) != 1) {\n        return NULL;\n    }\n    \n    return base64_encode(salt, SALT_LENGTH);\n}\n\n/**\n * Hashes password with salt using SHA-256.\n * Note: For production, use bcrypt, scrypt, or Argon2id.\n * Caller must free the returned string.\n */\nchar* hash_password(const char* password, const char* salt) {\n    if (password == NULL || salt == NULL) {\n        return NULL;\n    }\n    \n    size_t password_len = strlen(password);\n    size_t salt_len = strlen(salt);\n    size_t salted_len = password_len + salt_len;\n    \n    /* Check for overflow */\n    if (salted_len < password_len || salted_len < salt_len) {\n        return NULL;\n    }\n    \n    char* salted_password = (char*)malloc(salted_len + 1);\n    if (salted_password == NULL) {\n        return NULL;\n    }\n    \n    /* Safely combine password and salt */\n    memcpy(salted_password, password, password_len);\n    memcpy(salted_password + password_len, salt, salt_len);\n    salted_password[salted_len] = '\\0';\n    \n    unsigned char hash[SHA256_DIGEST_LENGTH];\n    SHA256((unsigned char*)salted_password, salted_len, hash);\n    \n    /* Clear salted password from memory */\n    memset(salted_password, 0, salted_len);\n    free(salted_password);\n    \n    return base64_encode(hash, SHA256_DIGEST_LENGTH);\n}\n\n/**\n * Validates username format and length.\n */\nint is_valid_username(const char* username) {\n    if (username == NULL) {\n        return 0;\n    }\n    \n    size_t len = strlen(username);\n    if (len < USERNAME_MIN || len > USERNAME_MAX) {\n        return 0;\n    }\n    \n    /* Check for allowed characters only */\n    for (size_t i = 0; i < len; i++) {\n        char c = username[i];\n        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || \n              (c >= '0' && c <= '9') || c == '_' || c == '-')) {
            return 0;
        }
    }
    
    return 1;
}

/**
 * Validates password length.
 */
int is_valid_password(const char* password) {
    if (password == NULL) {
        return 0;
    }
    
    size_t len = strlen(password);
    return (len >= PASSWORD_MIN && len <= PASSWORD_MAX);
}

/**
 * Inserts user into database with hashed password.
 * Uses parameterized queries to prevent SQL injection.
 */
int insert_user(const char* username, const char* password) {
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    char* salt = NULL;
    char* hashed_password = NULL;
    int result = 0;
    
    /* Validate inputs */
    if (!is_valid_username(username)) {
        fprintf(stderr, "Invalid username format\\n");
        return 0;
    }
    
    if (!is_valid_password(password)) {
        fprintf(stderr, "Invalid password format\\n");
        return 0;
    }
    
    /* Generate salt */
    salt = generate_salt();
    if (salt == NULL) {
        fprintf(stderr, "Failed to generate salt\\n");
        return 0;
    }
    
    /* Hash password */
    hashed_password = hash_password(password, salt);
    if (hashed_password == NULL) {
        fprintf(stderr, "Failed to hash password\\n");
        free(salt);
        return 0;
    }
    
    /* Open database */
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        fprintf(stderr, "Database error occurred\\n");
        goto cleanup;
    }
    
    /* Create table */
    const char* create_table_sql = 
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT UNIQUE NOT NULL, "
        "password_hash TEXT NOT NULL, "
        "salt TEXT NOT NULL)";
    
    if (sqlite3_exec(db, create_table_sql, NULL, NULL, NULL) != SQLITE_OK) {
        fprintf(stderr, "Database error occurred\\n");
        goto cleanup;
    }
    
    /* Prepare parameterized statement */
    const char* insert_sql = 
        "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";
    
    if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Database error occurred\\n");
        goto cleanup;
    }
    
    /* Bind parameters to prevent SQL injection */
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hashed_password, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, salt, -1, SQLITE_TRANSIENT);
    
    /* Execute statement */
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        result = 1;
    }
    
cleanup:
    if (stmt != NULL) {
        sqlite3_finalize(stmt);
    }
    if (db != NULL) {
        sqlite3_close(db);
    }
    if (hashed_password != NULL) {
        memset(hashed_password, 0, strlen(hashed_password));
        free(hashed_password);
    }
    if (salt != NULL) {
        free(salt);
    }
    
    return result;
}

int main(void) {
    /* Test case 1: Valid username and password */
    printf("Test 1: %d\\n", insert_user("john_doe", "SecurePass123!"));
    
    /* Test case 2: Another valid user */
    printf("Test 2: %d\\n", insert_user("jane-smith", "MyP@ssw0rd2024"));
    
    /* Test case 3: Invalid username (too short) */
    printf("Test 3: %d\\n", insert_user("ab", "ValidPassword1"));
    
    /* Test case 4: Invalid password (too short) */
    printf("Test 4: %d\\n", insert_user("validuser", "short"));
    
    /* Test case 5: Valid edge case */
    printf("Test 5: %d\\n", insert_user("user123", "LongPassword123456"));
    
    return 0;
}
