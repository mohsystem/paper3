
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <ctype.h>
#include <stdbool.h>

#define SALT_LENGTH 16
#define MAX_USERNAME_LEN 50
#define MIN_USERNAME_LEN 3
#define MIN_PASSWORD_LEN 8
#define DB_NAME "users.db"

/* Base64 encoding lookup table */
static const char base64_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* Base64 encode function */
char* base64_encode(const unsigned char* data, size_t input_length, size_t* output_length) {
    *output_length = 4 * ((input_length + 2) / 3);
    
    char* encoded_data = malloc(*output_length + 1);
    if (encoded_data == NULL) return NULL;
    
    size_t i = 0, j = 0;
    for (i = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;
        
        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;
        
        encoded_data[j++] = base64_chars[(triple >> 18) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 12) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 6) & 0x3F];
        encoded_data[j++] = base64_chars[triple & 0x3F];
    }
    
    for (i = 0; i < (3 - input_length % 3) % 3; i++)
        encoded_data[*output_length - 1 - i] = '=';
    
    encoded_data[*output_length] = '\\0';
    return encoded_data;
}

/* Generate cryptographically secure random salt */
bool generate_salt(unsigned char* salt, size_t length) {
    if (salt == NULL) return false;
    
    /* Use OpenSSL RAND_bytes for cryptographically secure randomness */
    if (RAND_bytes(salt, length) != 1) {
        fprintf(stderr, "Failed to generate secure random salt\\n");
        return false;
    }
    
    return true;
}

/* Hash password with salt using SHA-256 */
char* hash_password(const char* password, const unsigned char* salt, size_t salt_len) {
    if (password == NULL || salt == NULL) return NULL;
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    unsigned char* combined = NULL;
    char* result = NULL;
    
    /* Create SHA-256 hash with salt */
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, salt, salt_len);
    SHA256_Update(&sha256, password, strlen(password));
    SHA256_Final(hash, &sha256);
    
    /* Combine salt and hash */
    size_t combined_len = salt_len + SHA256_DIGEST_LENGTH;
    combined = malloc(combined_len);
    if (combined == NULL) return NULL;
    
    memcpy(combined, salt, salt_len);
    memcpy(combined + salt_len, hash, SHA256_DIGEST_LENGTH);
    
    /* Base64 encode */
    size_t encoded_len = 0;
    result = base64_encode(combined, combined_len, &encoded_len);
    
    free(combined);
    return result;
}

/* Validate username format */
bool validate_username(const char* username) {
    if (username == NULL) return false;
    
    size_t len = strlen(username);
    if (len < MIN_USERNAME_LEN || len > MAX_USERNAME_LEN) {
        fprintf(stderr, "Invalid username length\\n");
        return false;
    }
    
    /* Check alphanumeric and underscore only */
    for (size_t i = 0; i < len; i++) {
        if (!isalnum(username[i]) && username[i] != '_') {
            fprintf(stderr, "Invalid username format\\n");
            return false;
        }
    }
    
    return true;
}

/* Validate password strength */
bool validate_password(const char* password) {
    if (password == NULL) return false;
    
    if (strlen(password) < MIN_PASSWORD_LEN) {
        fprintf(stderr, "Password must be at least 8 characters\\n");
        return false;
    }
    
    return true;
}

/* Insert user into database with parameterized query */
bool insert_user(const char* username, const char* password) {
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    unsigned char salt[SALT_LENGTH];
    char* hashed_password = NULL;
    bool success = false;
    
    /* Validate inputs */
    if (!validate_username(username) || !validate_password(password)) {
        return false;
    }
    
    /* Generate unique salt */
    if (!generate_salt(salt, SALT_LENGTH)) {
        return false;
    }
    
    /* Hash password with salt */
    hashed_password = hash_password(password, salt, SALT_LENGTH);
    if (hashed_password == NULL) {
        return false;
    }
    
    /* Open database */
    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
        fprintf(stderr, "Database error occurred\\n");
        free(hashed_password);
        if (db) sqlite3_close(db);
        return false;
    }
    
    /* Prepare parameterized statement to prevent SQL injection */
    const char* sql = "INSERT INTO users (username, password_hash) VALUES (?, ?)";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        /* Bind parameters - prevents SQL injection */
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, hashed_password, -1, SQLITE_STATIC);
        
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            success = true;
        } else {
            fprintf(stderr, "Database error occurred\\n");
        }
    }
    
    /* Clean up resources */
    if (stmt) sqlite3_finalize(stmt);
    if (db) sqlite3_close(db);
    free(hashed_password);
    
    return success;
}

/* Initialize database */
void initialize_database(void) {
    sqlite3* db = NULL;
    char* err_msg = NULL;
    
    if (sqlite3_open(DB_NAME, &db) == SQLITE_OK) {
        const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "username TEXT UNIQUE NOT NULL, "
                         "password_hash TEXT NOT NULL)";
        
        if (sqlite3_exec(db, sql, NULL, NULL, &err_msg) != SQLITE_OK) {
            fprintf(stderr, "Failed to initialize database\\n");
            if (err_msg) {
                sqlite3_free(err_msg);
            }
        }
        sqlite3_close(db);
    }
}

int main(void) {
    /* Initialize database */
    initialize_database();
    
    /* Test case 1: Valid user registration */
    printf("Test 1: %s\\n", insert_user("alice123", "SecurePass123!") ? "PASS" : "FAIL");
    
    /* Test case 2: Valid user with different credentials */
    printf("Test 2: %s\\n", insert_user("bob_user", "MyP@ssw0rd2024") ? "PASS" : "FAIL");
    
    /* Test case 3: Invalid username (too short) */
    printf("Test 3: %s\\n", !insert_user("ab", "ValidPass123") ? "PASS" : "FAIL");
    
    /* Test case 4: Invalid password (too short) */
    printf("Test 4: %s\\n", !insert_user("charlie", "short") ? "PASS" : "FAIL");
    
    /* Test case 5: Invalid username (special characters) */
    printf("Test 5: %s\\n", !insert_user("user@name", "ValidPass123") ? "PASS" : "FAIL");
    
    return 0;
}
