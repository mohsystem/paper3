
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define MAX_USERNAME_LEN 100
#define MAX_PASSWORD_LEN 100
#define MAX_HASH_LEN 256
#define DB_PATH "userdb.db"

void base64_encode(const unsigned char* input, int length, char* output) {
    static const char encoding_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int i, j;
    for (i = 0, j = 0; i < length;) {
        unsigned int octet_a = i < length ? input[i++] : 0;
        unsigned int octet_b = i < length ? input[i++] : 0;
        unsigned int octet_c = i < length ? input[i++] : 0;
        unsigned int triple = (octet_a << 16) + (octet_b << 8) + octet_c;
        
        output[j++] = encoding_table[(triple >> 18) & 0x3F];
        output[j++] = encoding_table[(triple >> 12) & 0x3F];
        output[j++] = encoding_table[(triple >> 6) & 0x3F];
        output[j++] = encoding_table[triple & 0x3F];
    }
    
    int mod = length % 3;
    if (mod == 1) {
        output[j - 2] = '=';
        output[j - 1] = '=';
    } else if (mod == 2) {
        output[j - 1] = '=';
    }
    output[j] = '\\0';
}

int base64_decode(const char* input, unsigned char* output) {
    static const unsigned char decoding_table[256] = {
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
        64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
        64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64
    };
    
    int i, j;
    int len = strlen(input);
    for (i = 0, j = 0; i < len;) {
        unsigned int sextet_a = input[i] == '=' ? 0 & i++ : decoding_table[(unsigned char)input[i++]];
        unsigned int sextet_b = input[i] == '=' ? 0 & i++ : decoding_table[(unsigned char)input[i++]];
        unsigned int sextet_c = input[i] == '=' ? 0 & i++ : decoding_table[(unsigned char)input[i++]];
        unsigned int sextet_d = input[i] == '=' ? 0 & i++ : decoding_table[(unsigned char)input[i++]];
        
        unsigned int triple = (sextet_a << 18) + (sextet_b << 12) + (sextet_c << 6) + sextet_d;
        
        if (j < len) output[j++] = (triple >> 16) & 0xFF;
        if (j < len) output[j++] = (triple >> 8) & 0xFF;
        if (j < len) output[j++] = triple & 0xFF;
    }
    return j;
}

void hash_password(const char* password, const char* salt_b64, char* output) {
    unsigned char salt[64];
    int salt_len = base64_decode(salt_b64, salt);
    
    unsigned char hash[32];
    PKCS5_PBKDF2_HMAC(password, strlen(password), salt, salt_len, 100000, EVP_sha256(), 32, hash);
    
    base64_encode(hash, 32, output);
}

const char* login(const char* username, const char* password) {
    if (username == NULL || password == NULL || strlen(username) == 0 || strlen(password) == 0) {
        return "Invalid credentials";
    }
    
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    
    int rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return "Database error: Cannot open database";
    }
    
    const char* query = "SELECT password_hash, salt FROM users WHERE username = ?";
    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return "Database error: Cannot prepare statement";
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    
    const char* result;
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_ROW) {
        const unsigned char* stored_hash = sqlite3_column_text(stmt, 0);
        const unsigned char* salt = sqlite3_column_text(stmt, 1);
        
        char input_hash[MAX_HASH_LEN];
        hash_password(password, (const char*)salt, input_hash);
        
        if (strcmp(input_hash, (const char*)stored_hash) == 0) {
            result = "Redirect to home page";
        } else {
            result = "Incorrect password";
        }
    } else {
        result = "User not found";
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

int main() {
    printf("Test Case 1: Valid credentials\\n");
    printf("%s\\n", login("user1", "password123"));
    
    printf("\\nTest Case 2: Invalid password\\n");
    printf("%s\\n", login("user1", "wrongpassword"));
    
    printf("\\nTest Case 3: Non-existent user\\n");
    printf("%s\\n", login("nonexistent", "password123"));
    
    printf("\\nTest Case 4: Empty username\\n");
    printf("%s\\n", login("", "password123"));
    
    printf("\\nTest Case 5: NULL values\\n");
    printf("%s\\n", login(NULL, NULL));
    
    return 0;
}
