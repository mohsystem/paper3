
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

#define SALT_LENGTH 16
#define MAX_USERNAME 100
#define MAX_PASSWORD 256

static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char* base64_encode(const unsigned char* data, size_t input_length) {
    size_t output_length = 4 * ((input_length + 2) / 3);
    char* encoded = malloc(output_length + 1);
    if (!encoded) return NULL;
    
    for (size_t i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;
        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;
        
        encoded[j++] = base64_table[(triple >> 3 * 6) & 0x3F];
        encoded[j++] = base64_table[(triple >> 2 * 6) & 0x3F];
        encoded[j++] = base64_table[(triple >> 1 * 6) & 0x3F];
        encoded[j++] = base64_table[(triple >> 0 * 6) & 0x3F];
    }
    
    for (int i = 0; i < (3 - input_length % 3) % 3; i++) {
        encoded[output_length - 1 - i] = '=';
    }
    
    encoded[output_length] = '\\0';
    return encoded;
}

char* generate_salt() {
    unsigned char salt[SALT_LENGTH];
    if (RAND_bytes(salt, SALT_LENGTH) != 1) {
        return NULL;
    }
    return base64_encode(salt, SALT_LENGTH);
}

char* hash_password(const char* password, const char* salt) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, salt, strlen(salt));
    SHA256_Update(&sha256, password, strlen(password));
    SHA256_Final(hash, &sha256);
    
    return base64_encode(hash, SHA256_DIGEST_LENGTH);
}

int insert_user(const char* username, const char* password) {
    if (!username || strlen(username) == 0 || !password || strlen(password) < 8) {
        return 0;
    }
    
    char* salt = generate_salt();
    if (!salt) return 0;
    
    char* hashed_password = hash_password(password, salt);
    if (!hashed_password) {
        free(salt);
        return 0;
    }
    
    char combined_hash[512];
    snprintf(combined_hash, sizeof(combined_hash), "%s:%s", salt, hashed_password);
    
    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;
    
    rc = sqlite3_open("users.db", &db);
    if (rc != SQLITE_OK) {
        free(salt);
        free(hashed_password);
        return 0;
    }
    
    const char* sql = "INSERT INTO users (username, password_hash) VALUES (?, ?)";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        free(salt);
        free(hashed_password);
        return 0;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, combined_hash, -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    int success = (rc == SQLITE_DONE);
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    free(salt);
    free(hashed_password);
    
    return success;
}

int main() {
    printf("Test 1: %d\\n", insert_user("user1", "password123"));
    printf("Test 2: %d\\n", insert_user("user2", "securePass456"));
    printf("Test 3: %d\\n", insert_user("user3", "myP@ssw0rd"));
    printf("Test 4: %d\\n", insert_user("", "password123"));
    printf("Test 5: %d\\n", insert_user("user5", "short"));
    
    return 0;
}
