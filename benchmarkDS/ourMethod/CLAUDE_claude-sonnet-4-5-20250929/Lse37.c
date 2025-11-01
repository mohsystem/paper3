
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <sqlite3.h>

/* Constants for PBKDF2 (Rules#6, #7, #8) */
#define ITERATIONS 210000
#define KEY_LENGTH 32
#define SALT_LENGTH 16
#define MAX_INPUT_LENGTH 255

/* Securely clears sensitive data (Rules#1, #6) */
void secure_zero(void* ptr, size_t len) {
    if (ptr == NULL || len == 0) return;
    #ifdef _WIN32
        SecureZeroMemory(ptr, len);
    #else
        explicit_bzero(ptr, len);
    #endif
}

/* Generates cryptographically secure random salt (Rules#8, #9) */
int generate_salt(unsigned char* salt, size_t length) {
    if (salt == NULL || length == 0) return 0;
    return RAND_bytes(salt, (int)length) == 1;
}

/* Validates input to prevent injection attacks (Rules#1, #13) */
int validate_input(const char* input) {
    if (input == NULL || strlen(input) == 0 || strlen(input) > MAX_INPUT_LENGTH) {
        return 0;
    }
    
    /* Allow only alphanumeric and safe characters */
    for (size_t i = 0; i < strlen(input); i++) {
        unsigned char c = (unsigned char)input[i];
        if (!isalnum(c) && c != '_' && c != '@' && c != '.' && c != '-') {
            return 0;
        }
    }
    return 1;
}

/* Hashes password using PBKDF2-HMAC-SHA256 (Rules#1, #6, #7, #8) */
int hash_password(const char* password, const unsigned char* salt, 
                  size_t salt_len, unsigned char* out_hash, size_t hash_len) {
    if (password == NULL || salt == NULL || out_hash == NULL) {
        return 0;
    }
    
    int result = PKCS5_PBKDF2_HMAC(
        password, (int)strlen(password),
        salt, (int)salt_len,
        ITERATIONS,
        EVP_sha256(),
        (int)hash_len,
        out_hash
    );
    
    return result == 1;
}

/* Constant-time comparison to prevent timing attacks (Rules#6) */
int constant_time_compare(const unsigned char* a, const unsigned char* b, size_t len) {
    if (a == NULL || b == NULL) return 0;
    
    unsigned char result = 0;
    for (size_t i = 0; i < len; i++) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

/* Base64 encode helper */
char* base64_encode(const unsigned char* data, size_t len) {
    static const char encoding_table[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t output_len = ((len + 2) / 3) * 4;
    char* result = (char*)malloc(output_len + 1);
    if (result == NULL) return NULL;
    
    size_t j = 0;
    for (size_t i = 0; i < len; i += 3) {
        unsigned int val = data[i] << 16;
        if (i + 1 < len) val |= data[i + 1] << 8;
        if (i + 2 < len) val |= data[i + 2];
        
        result[j++] = encoding_table[(val >> 18) & 0x3F];
        result[j++] = encoding_table[(val >> 12) & 0x3F];
        result[j++] = (i + 1 < len) ? encoding_table[(val >> 6) & 0x3F] : '=';
        result[j++] = (i + 2 < len) ? encoding_table[val & 0x3F] : '=';
    }
    result[j] = '\\0';
    return result;
}

/* Base64 decode helper */
unsigned char* base64_decode(const char* input, size_t* out_len) {
    static const unsigned char decoding_table[256] = {
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,52,53,54,55,56,57,58,59,60,61,64,64,64,64,64,64,
        64,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,
        64,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
        64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64
    };
    
    size_t input_len = strlen(input);
    size_t decoded_len = (input_len / 4) * 3;
    unsigned char* result = (unsigned char*)malloc(decoded_len);
    if (result == NULL) return NULL;
    
    unsigned int val = 0;
    int bits = -8;
    size_t j = 0;
    
    for (size_t i = 0; i < input_len; i++) {
        unsigned char c = (unsigned char)input[i];
        if (decoding_table[c] == 64) break;
        val = (val << 6) | decoding_table[c];
        bits += 6;
        if (bits >= 0) {
            result[j++] = (val >> bits) & 0xFF;
            bits -= 8;
        }
    }
    *out_len = j;
    return result;
}

/* Authenticates user with parameterized queries (Rules#1, #2, #3, #4) */
const char* authenticate_user(const char* username, const char* password) {
    /* Input validation (Rules#1, #13) */
    if (!validate_input(username) || password == NULL || strlen(password) == 0) {
        return "cant_login";
    }
    
    /* Load database path from environment variable (Rules#3, #4) */
    const char* db_path = getenv("DB_PATH");
    if (db_path == NULL) {
        db_path = "users.db";
    }
    
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    unsigned char computed_hash[KEY_LENGTH];
    memset(computed_hash, 0, KEY_LENGTH);
    const char* result = "internal_server_error";
    
    /* Open database connection */
    if (sqlite3_open_v2(db_path, &db, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK) {
        fprintf(stderr, "Database error occurred\\n");
        return "internal_server_error";
    }
    
    /* Parameterized query prevents SQL injection (Rules#1, #2) */
    const char* query = "SELECT password_hash, salt FROM users WHERE username = ?";
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(db);
        fprintf(stderr, "Database error occurred\\n");
        return "internal_server_error";
    }
    
    /* Bind username parameter */
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        /* Retrieve stored hash and salt */
        const unsigned char* stored_hash_b64 = sqlite3_column_text(stmt, 0);
        const unsigned char* salt_b64 = sqlite3_column_text(stmt, 1);
        
        if (stored_hash_b64 == NULL || salt_b64 == NULL) {
            result = "internal_server_error";
            goto cleanup;
        }
        
        /* Decode base64 salt and hash */
        size_t salt_len = 0, hash_len = 0;
        unsigned char* salt = base64_decode((const char*)salt_b64, &salt_len);
        unsigned char* stored_hash = base64_decode((const char*)stored_hash_b64, &hash_len);
        
        if (salt == NULL || stored_hash == NULL) {
            free(salt);
            free(stored_hash);
            result = "internal_server_error";
            goto cleanup;
        }
        
        /* Hash provided password (Rules#6, #7, #8) */
        if (!hash_password(password, salt, salt_len, computed_hash, KEY_LENGTH)) {
            free(salt);
            free(stored_hash);
            result = "internal_server_error";
            goto cleanup;
        }
        
        /* Constant-time comparison (Rules#6) */
        int match = constant_time_compare(computed_hash, stored_hash, 
                                         (hash_len < KEY_LENGTH) ? hash_len : KEY_LENGTH);
        
        free(salt);
        free(stored_hash);
        result = match ? "success" : "cant_login";
        
    } else if (rc == SQLITE_DONE) {
        result = "cant_login";
    } else {
        fprintf(stderr, "Database error occurred\\n");
        result = "internal_server_error";
    }
    
cleanup:
    /* Securely clear sensitive data (Rules#1, #6) */
    secure_zero(computed_hash, KEY_LENGTH);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

int main(void) {
    /* Test cases */
    printf("Test 1 - Valid user: %s\\n", authenticate_user("validuser", "ValidPass123!"));
    printf("Test 2 - Invalid password: %s\\n", authenticate_user("validuser", "wrongpass"));
    printf("Test 3 - Invalid username: %s\\n", authenticate_user("invaliduser", "password"));
    printf("Test 4 - SQL injection attempt: %s\\n", authenticate_user("admin' OR '1'='1", "pass"));
    printf("Test 5 - Empty credentials: %s\\n", authenticate_user("", ""));
    
    return 0;
}
