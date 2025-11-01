
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

/* Constants for security - Rules#1, Rules#13 */
#define MAX_USERNAME_LENGTH 50
#define MAX_PASSWORD_LENGTH 128
#define MAX_LOGIN_ATTEMPTS 5
#define PBKDF2_ITERATIONS 210000
#define SALT_LENGTH 16
#define HASH_LENGTH 32

/* Base64 encoding characters */
static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* Secure zeroization - Rules#1: clear sensitive data */
void secure_zero(void* ptr, size_t len) {
    if (ptr == NULL) return;
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (len--) {
        *p++ = 0;
    }
}

/* Generate cryptographically secure random salt - Rules#8, Rules#9 */
char* generate_salt(void) {
    unsigned char salt[SALT_LENGTH];
    char* result = NULL;
    int i;
    
    /* Use OpenSSL CSPRNG - Rules#1, Rules#6 */
    if (RAND_bytes(salt, SALT_LENGTH) != 1) {
        fprintf(stderr, "Failed to generate random salt\\n");
        return NULL;
    }
    
    /* Allocate memory for base64 encoded salt - Rules#1 */
    result = (char*)malloc(((SALT_LENGTH + 2) / 3) * 4 + 1);
    if (result == NULL) {
        return NULL;
    }
    
    /* Encode to base64 */
    int out_idx = 0;
    for (i = 0; i < SALT_LENGTH; i += 3) {
        unsigned int n = (salt[i] << 16) | 
                        ((i + 1 < SALT_LENGTH) ? (salt[i+1] << 8) : 0) | 
                        ((i + 2 < SALT_LENGTH) ? salt[i+2] : 0);
        result[out_idx++] = base64_chars[(n >> 18) & 63];
        result[out_idx++] = base64_chars[(n >> 12) & 63];
        result[out_idx++] = (i + 1 < SALT_LENGTH) ? base64_chars[(n >> 6) & 63] : '=';
        result[out_idx++] = (i + 2 < SALT_LENGTH) ? base64_chars[n & 63] : '=';
    }
    result[out_idx] = '\\0';
    
    return result;
}

/* Decode base64 string */
unsigned char* base64_decode(const char* encoded, size_t* out_len) {
    size_t len = strlen(encoded);
    size_t decoded_len = (len * 3) / 4;
    unsigned char* result = NULL;
    size_t i, j;
    
    /* Allocate memory - Rules#1 */
    result = (unsigned char*)malloc(decoded_len + 1);
    if (result == NULL) {
        return NULL;
    }
    
    /* Decode base64 */
    j = 0;
    for (i = 0; i < len; i += 4) {
        unsigned int n = 0;
        int k;
        for (k = 0; k < 4 && i + k < len; k++) {
            char* pos = strchr(base64_chars, encoded[i + k]);
            if (pos != NULL) {
                n = (n << 6) | (pos - base64_chars);
            }
        }
        if (j < decoded_len) result[j++] = (n >> 16) & 0xFF;
        if (j < decoded_len) result[j++] = (n >> 8) & 0xFF;
        if (j < decoded_len) result[j++] = n & 0xFF;
    }
    
    *out_len = j;
    return result;
}

/* Hash password using PBKDF2-HMAC-SHA256 - Rules#6, Rules#7 */
char* hash_password(const char* password, const char* salt_b64) {
    unsigned char hash[HASH_LENGTH];
    char* result = NULL;
    size_t salt_len = 0;
    unsigned char* salt_bytes = NULL;
    int i;
    
    if (password == NULL || salt_b64 == NULL) {
        return NULL;
    }
    
    /* Decode salt from base64 */
    salt_bytes = base64_decode(salt_b64, &salt_len);
    if (salt_bytes == NULL) {
        return NULL;
    }
    
    /* Use PBKDF2-HMAC-SHA256 with at least 210000 iterations - Rules#6 */
    if (PKCS5_PBKDF2_HMAC(password, strlen(password),
                          salt_bytes, salt_len,
                          PBKDF2_ITERATIONS,
                          EVP_sha256(),
                          HASH_LENGTH,
                          hash) != 1) {
        free(salt_bytes);
        return NULL;
    }
    
    /* Clean up salt bytes - Rules#1 */
    secure_zero(salt_bytes, salt_len);
    free(salt_bytes);
    
    /* Allocate memory for base64 encoded hash - Rules#1 */
    result = (char*)malloc(((HASH_LENGTH + 2) / 3) * 4 + 1);
    if (result == NULL) {
        secure_zero(hash, HASH_LENGTH);
        return NULL;
    }
    
    /* Encode hash to base64 */
    int out_idx = 0;
    for (i = 0; i < HASH_LENGTH; i += 3) {
        unsigned int n = (hash[i] << 16) | 
                        ((i + 1 < HASH_LENGTH) ? (hash[i+1] << 8) : 0) | 
                        ((i + 2 < HASH_LENGTH) ? hash[i+2] : 0);
        result[out_idx++] = base64_chars[(n >> 18) & 63];
        result[out_idx++] = base64_chars[(n >> 12) & 63];
        result[out_idx++] = (i + 1 < HASH_LENGTH) ? base64_chars[(n >> 6) & 63] : '=';
        result[out_idx++] = (i + 2 < HASH_LENGTH) ? base64_chars[n & 63] : '=';
    }
    result[out_idx] = '\\0';
    
    /* Clear sensitive data - Rules#1 */
    secure_zero(hash, HASH_LENGTH);
    
    return result;
}

/* Constant-time string comparison - Rules#6 */
int constant_time_compare(const char* a, const char* b) {
    size_t len_a, len_b, i;
    volatile unsigned char result = 0;
    
    if (a == NULL || b == NULL) {
        return 0;
    }
    
    len_a = strlen(a);
    len_b = strlen(b);
    
    if (len_a != len_b) {
        return 0;
    }
    
    for (i = 0; i < len_a; i++) {
        result |= a[i] ^ b[i];
    }
    
    return result == 0;
}

/* Validate username format - Rules#1, Rules#13 */
int validate_username(const char* username) {
    size_t len;
    size_t i;
    
    if (username == NULL) {
        return 0;
    }
    
    len = strlen(username);
    if (len == 0 || len > MAX_USERNAME_LENGTH) {
        return 0;
    }
    
    /* Only allow alphanumeric, underscore, and hyphen - Rules#1 */
    for (i = 0; i < len; i++) {
        char c = username[i];
        if (!((c >= 'a' && c <= 'z') || 
              (c >= 'A' && c <= 'Z') || 
              (c >= '0' && c <= '9') || 
              c == '_' || c == '-')) {
            return 0;
        }
    }
    
    return 1;
}

/* Authenticate user against database - Rules#1: parameterized queries */
int authenticate_user(const char* username, const char* password) {
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    int authenticated = 0;
    const char* db_path;
    const char* query = "SELECT password_hash, salt FROM users WHERE username = ?";
    
    /* Get database path from environment - Rules#3, Rules#4 */
    db_path = getenv("DB_PATH");
    if (db_path == NULL) {
        db_path = "users.db";
    }
    
    /* Open database connection - Rules#1 */
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        fprintf(stderr, "Database connection error\\n");
        return 0;
    }
    
    /* Prepare parameterized query to prevent SQL injection - Rules#1 */
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(db);
        return 0;
    }
    
    /* Bind username parameter - Rules#1 */
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* stored_hash = sqlite3_column_text(stmt, 0);
        const unsigned char* salt = sqlite3_column_text(stmt, 1);
        
        if (stored_hash != NULL && salt != NULL) {
            char* computed_hash = hash_password(password, (const char*)salt);
            
            if (computed_hash != NULL) {
                /* Constant-time comparison to prevent timing attacks - Rules#6 */
                authenticated = constant_time_compare((const char*)stored_hash, computed_hash);
                
                /* Clear sensitive data - Rules#1 */
                secure_zero(computed_hash, strlen(computed_hash));
                free(computed_hash);
            }
        }
    }
    
    /* Clean up resources - Rules#1 */
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return authenticated;
}

/* Login function - main entry point */
int login(const char* username, const char* password) {
    /* Input validation - Rules#1, Rules#13 */
    if (username == NULL || password == NULL) {
        return 0;
    }
    
    if (strlen(username) == 0 || strlen(password) == 0) {
        return 0;
    }
    
    /* Check length constraints - Rules#1, Rules#13 */
    if (strlen(password) > MAX_PASSWORD_LENGTH) {
        return 0;
    }
    
    /* Validate username format - Rules#1 */
    if (!validate_username(username)) {
        return 0;
    }
    
    /* Authenticate user */
    return authenticate_user(username, password);
}

/* Test cases */
int main(void) {
    char* salt1 = NULL;
    char* hash1 = NULL;
    char* hash2 = NULL;
    char* salt2 = NULL;
    char* hash3 = NULL;
    char* hash4 = NULL;
    
    printf("Login System Test Cases:\\n");
    
    /* Test Case 1: Generate salt */
    salt1 = generate_salt();
    printf("Test 1 - Salt generated: %d\\n", salt1 != NULL && strlen(salt1) > 0);
    
    if (salt1 != NULL) {
        /* Test Case 2: Hash password with salt */
        hash1 = hash_password("testpass123", salt1);
        printf("Test 2 - Password hashed: %d\\n", hash1 != NULL && strlen(hash1) > 0);
        
        if (hash1 != NULL) {
            /* Test Case 3: Verify same password with same salt produces same hash */
            hash2 = hash_password("testpass123", salt1);
            printf("Test 3 - Hash consistency: %d\\n", 
                   hash2 != NULL && strcmp(hash1, hash2) == 0);
            
            /* Test Case 4: Verify different salt produces different hash */
            salt2 = generate_salt();
            if (salt2 != NULL) {
                hash3 = hash_password("testpass123", salt2);
                printf("Test 4 - Different salt, different hash: %d\\n", 
                       hash3 != NULL && strcmp(hash1, hash3) != 0);
                
                /* Clean up */
                if (hash3 != NULL) {
                    secure_zero(hash3, strlen(hash3));
                    free(hash3);
                }
                secure_zero(salt2, strlen(salt2));
                free(salt2);
            }
            
            /* Test Case 5: Verify different password produces different hash */
            hash4 = hash_password("differentpass", salt1);
            printf("Test 5 - Different password, different hash: %d\\n", 
                   hash4 != NULL && strcmp(hash1, hash4) != 0);
            
            /* Clean up */
            if (hash4 != NULL) {
                secure_zero(hash4, strlen(hash4));
                free(hash4);
            }
            if (hash2 != NULL) {
                secure_zero(hash2, strlen(hash2));
                free(hash2);
            }
            secure_zero(hash1, strlen(hash1));
            free(hash1);
        }
        
        secure_zero(salt1, strlen(salt1));
        free(salt1);
    }
    
    return 0;
}
