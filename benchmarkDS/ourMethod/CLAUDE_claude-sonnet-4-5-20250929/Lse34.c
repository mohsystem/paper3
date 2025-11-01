
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/crypto.h>

/* PBKDF2 parameters for secure key derivation */
#define ITERATIONS 210000
#define KEY_LENGTH 32
#define SALT_LENGTH 16
#define MAX_INPUT_LENGTH 100
#define MIN_INPUT_LENGTH 3

/* Secure zeroization of sensitive data */
void secure_zero(void* ptr, size_t len) {
    if (ptr == NULL || len == 0) return;
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (len--) *p++ = 0;
}

/* Base64 encoding */
char* base64_encode(const unsigned char* data, size_t len) {
    static const char base64_chars[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    size_t out_len = 4 * ((len + 2) / 3);
    char* result = (char*)malloc(out_len + 1);
    if (!result) return NULL;
    
    size_t i = 0, j = 0;
    unsigned int val = 0;
    int valb = -6;
    
    for (i = 0; i < len; i++) {
        val = (val << 8) + data[i];
        valb += 8;
        while (valb >= 0) {
            result[j++] = base64_chars[(val >> valb) & 0x3F];
            valb -= 6;
        }
    }
    if (valb > -6) result[j++] = base64_chars[((val << 8) >> (valb + 8)) & 0x3F];
    while (j % 4) result[j++] = '=';
    result[j] = '\\0';
    
    return result;
}

/* Base64 decoding */
unsigned char* base64_decode(const char* encoded, size_t* out_len) {
    static const unsigned char decode_table[256] = {
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
        64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
        64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
    };
    
    size_t len = strlen(encoded);
    size_t max_out = (len * 3) / 4;
    unsigned char* result = (unsigned char*)malloc(max_out);
    if (!result) {
        *out_len = 0;
        return NULL;
    }
    
    unsigned int val = 0;
    int valb = -8;
    size_t j = 0;
    
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)encoded[i];
        if (decode_table[c] == 64) break;
        val = (val << 6) + decode_table[c];
        valb += 6;
        if (valb >= 0) {
            result[j++] = (val >> valb) & 0xFF;
            valb -= 8;
        }
    }
    
    *out_len = j;
    return result;
}

/* Generate cryptographically secure random salt */
int generate_salt(unsigned char* salt, size_t len) {
    /* Use OpenSSL's CSPRNG */
    if (RAND_bytes(salt, len) != 1) {
        return 0;
    }
    return 1;
}

/* Hash password using PBKDF2-HMAC-SHA256 */
char* hash_password(const char* password, const unsigned char* salt, size_t salt_len) {
    unsigned char hash[KEY_LENGTH];
    
    /* Use PBKDF2 with SHA256 */
    if (PKCS5_PBKDF2_HMAC(password, strlen(password),
                          salt, salt_len,
                          ITERATIONS, EVP_sha256(),
                          KEY_LENGTH, hash) != 1) {
        secure_zero(hash, KEY_LENGTH);
        return NULL;
    }
    
    char* result = base64_encode(hash, KEY_LENGTH);
    secure_zero(hash, KEY_LENGTH);
    
    return result;
}

/* Validate input */
int validate_input(const char* value, const char* field_name) {
    if (!value || strlen(value) == 0) {
        fprintf(stderr, "Error: %s cannot be empty\\n", field_name);
        return 0;
    }
    
    size_t len = strlen(value);
    if (len < MIN_INPUT_LENGTH || len > MAX_INPUT_LENGTH) {
        fprintf(stderr, "Error: %s must be between %d and %d characters\\n",
                field_name, MIN_INPUT_LENGTH, MAX_INPUT_LENGTH);
        return 0;
    }
    
    return 1;
}

/* Initialize database */
int initialize_database(const char* db_path) {
    sqlite3* db = NULL;
    
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        fprintf(stderr, "Database initialization failed\\n");
        if (db) sqlite3_close(db);
        return 0;
    }
    
    const char* sql = 
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT UNIQUE NOT NULL, "
        "password_hash TEXT NOT NULL, "
        "salt TEXT NOT NULL)";
    
    char* err_msg = NULL;
    if (sqlite3_exec(db, sql, NULL, NULL, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "Table creation failed\\n");
        if (err_msg) {
            sqlite3_free(err_msg);
        }
        sqlite3_close(db);
        return 0;
    }
    
    sqlite3_close(db);
    return 1;
}

/* Register user */
int register_user(const char* db_path, const char* username, const char* password) {
    if (!validate_input(username, "Username") || !validate_input(password, "Password")) {
        return 0;
    }
    
    unsigned char salt[SALT_LENGTH];
    if (!generate_salt(salt, SALT_LENGTH)) {
        fprintf(stderr, "Failed to generate salt\\n");
        return 0;
    }
    
    char* salt_base64 = base64_encode(salt, SALT_LENGTH);
    if (!salt_base64) {
        fprintf(stderr, "Memory allocation failed\\n");
        return 0;
    }
    
    char* password_hash = hash_password(password, salt, SALT_LENGTH);
    if (!password_hash) {
        free(salt_base64);
        fprintf(stderr, "Password hashing failed\\n");
        return 0;
    }
    
    sqlite3* db = NULL;
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        free(salt_base64);
        free(password_hash);
        fprintf(stderr, "Registration failed\\n");
        if (db) sqlite3_close(db);
        return 0;
    }
    
    /* Use parameterized query to prevent SQL injection */
    const char* sql = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";
    sqlite3_stmt* stmt = NULL;
    int success = 0;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, password_hash, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, salt_base64, -1, SQLITE_STATIC);
        
        int result = sqlite3_step(stmt);
        if (result == SQLITE_DONE) {
            success = 1;
        } else if (result == SQLITE_CONSTRAINT) {
            fprintf(stderr, "Username already exists\\n");
        } else {
            fprintf(stderr, "Registration failed\\n");
        }
    }
    
    if (stmt) sqlite3_finalize(stmt);
    sqlite3_close(db);
    free(salt_base64);
    free(password_hash);
    
    return success;
}

/* Authenticate user */
char* authenticate_user(const char* db_path, const char* username, const char* password) {
    if (!validate_input(username, "Username") || !validate_input(password, "Password")) {
        return strdup("login");
    }
    
    sqlite3* db = NULL;
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        fprintf(stderr, "Authentication error occurred\\n");
        if (db) sqlite3_close(db);
        return strdup("login");
    }
    
    /* Use parameterized query to prevent SQL injection */
    const char* sql = "SELECT password_hash, salt FROM users WHERE username = ?";
    sqlite3_stmt* stmt = NULL;
    char* result = strdup("login");
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char* stored_hash = (const char*)sqlite3_column_text(stmt, 0);
            const char* salt_base64 = (const char*)sqlite3_column_text(stmt, 1);
            
            if (stored_hash && salt_base64) {
                size_t salt_len = 0;
                unsigned char* salt = base64_decode(salt_base64, &salt_len);
                
                if (salt && salt_len == SALT_LENGTH) {
                    char* provided_hash = hash_password(password, salt, salt_len);
                    
                    if (provided_hash) {
                        /* Constant-time comparison to prevent timing attacks */
                        if (strlen(provided_hash) == strlen(stored_hash) &&
                            CRYPTO_memcmp(provided_hash, stored_hash, strlen(provided_hash)) == 0) {
                            free(result);
                            result = strdup("dashboard");
                        }
                        free(provided_hash);
                    }
                }
                
                if (salt) free(salt);
            }
        }
    }
    
    if (stmt) sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

int main() {
    const char* db_path = "users.db";
    
    if (!initialize_database(db_path)) {
        return 1;
    }
    
    /* Test case 1: Register and authenticate valid user */
    printf("Test 1: Valid user registration and login\\n");
    register_user(db_path, "testuser1", "SecurePass123!");
    char* result1 = authenticate_user(db_path, "testuser1", "SecurePass123!");
    printf("Result: %s (Expected: dashboard)\\n", result1);
    free(result1);
    
    /* Test case 2: Wrong password */
    printf("\\nTest 2: Wrong password\\n");
    char* result2 = authenticate_user(db_path, "testuser1", "WrongPassword");
    printf("Result: %s (Expected: login)\\n", result2);
    free(result2);
    
    /* Test case 3: Non-existent user */
    printf("\\nTest 3: Non-existent user\\n");
    char* result3 = authenticate_user(db_path, "nonexistent", "password123");
    printf("Result: %s (Expected: login)\\n", result3);
    free(result3);
    
    /* Test case 4: Empty username */
    printf("\\nTest 4: Empty username\\n");
    char* result4 = authenticate_user(db_path, "", "password");
    printf("Result: %s (Expected: login)\\n", result4);
    free(result4);
    
    /* Test case 5: Multiple users with unique salts */
    printf("\\nTest 5: Multiple users with unique salts\\n");
    register_user(db_path, "testuser2", "AnotherPass456!");
    char* result5a = authenticate_user(db_path, "testuser2", "AnotherPass456!");
    char* result5b = authenticate_user(db_path, "testuser1", "SecurePass123!");
    printf("User2 Result: %s (Expected: dashboard)\\n", result5a);
    printf("User1 Result: %s (Expected: dashboard)\\n", result5b);
    free(result5a);
    free(result5b);
    
    return 0;
}
