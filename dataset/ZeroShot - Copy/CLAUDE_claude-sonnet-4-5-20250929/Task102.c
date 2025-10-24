
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

#define SALT_SIZE 16
#define HASH_SIZE 32
#define BASE64_SIZE(n) (((n + 2) / 3) * 4)

static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode(const unsigned char* data, size_t length, char* output) {
    size_t i = 0, j = 0;
    unsigned char array_3[3], array_4[4];
    
    while (length--) {
        array_3[i++] = *(data++);
        if (i == 3) {
            array_4[0] = (array_3[0] & 0xfc) >> 2;
            array_4[1] = ((array_3[0] & 0x03) << 4) + ((array_3[1] & 0xf0) >> 4);
            array_4[2] = ((array_3[1] & 0x0f) << 2) + ((array_3[2] & 0xc0) >> 6);
            array_4[3] = array_3[2] & 0x3f;
            
            for (i = 0; i < 4; i++)
                output[j++] = base64_chars[array_4[i]];
            i = 0;
        }
    }
    
    if (i) {
        for (size_t k = i; k < 3; k++)
            array_3[k] = '\\0';
        
        array_4[0] = (array_3[0] & 0xfc) >> 2;
        array_4[1] = ((array_3[0] & 0x03) << 4) + ((array_3[1] & 0xf0) >> 4);
        array_4[2] = ((array_3[1] & 0x0f) << 2) + ((array_3[2] & 0xc0) >> 6);
        
        for (size_t k = 0; k < i + 1; k++)
            output[j++] = base64_chars[array_4[k]];
        
        while (i++ < 3)
            output[j++] = '=';
    }
    output[j] = '\\0';
}

void generate_salt(char* salt_b64) {
    unsigned char salt[SALT_SIZE];
    RAND_bytes(salt, SALT_SIZE);
    base64_encode(salt, SALT_SIZE, salt_b64);
}

void hash_password(const char* password, const char* salt_b64, char* hash_b64) {
    unsigned char salt[SALT_SIZE];
    unsigned char hash[SHA256_DIGEST_LENGTH];
    
    // Simple base64 decode for salt (simplified version)
    size_t salt_len = strlen(salt_b64);
    memcpy(salt, salt_b64, SALT_SIZE < salt_len ? SALT_SIZE : salt_len);
    
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, salt, SALT_SIZE);
    SHA256_Update(&sha256, password, strlen(password));
    SHA256_Final(hash, &sha256);
    
    base64_encode(hash, SHA256_DIGEST_LENGTH, hash_b64);
}

int update_user_password(const char* username, const char* new_password, sqlite3* db) {
    if (!username || !new_password || strlen(username) == 0 || strlen(new_password) == 0) {
        return 0;
    }
    
    char salt_b64[BASE64_SIZE(SALT_SIZE) + 1];
    char hash_b64[BASE64_SIZE(SHA256_DIGEST_LENGTH) + 1];
    
    generate_salt(salt_b64);
    hash_password(new_password, salt_b64, hash_b64);
    
    const char* sql = "UPDATE users SET password_hash = ?, salt = ? WHERE username = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error preparing statement: %s\\n", sqlite3_errmsg(db));
        return 0;
    }
    
    sqlite3_bind_text(stmt, 1, hash_b64, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, salt_b64, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, username, -1, SQLITE_TRANSIENT);
    
    int result = sqlite3_step(stmt);
    int changes = sqlite3_changes(db);
    sqlite3_finalize(stmt);
    
    return result == SQLITE_DONE && changes > 0;
}

int main() {
    sqlite3* db;
    sqlite3_open(":memory:", &db);
    
    // Create test table
    const char* create_table = "CREATE TABLE users (id INTEGER PRIMARY KEY AUTOINCREMENT, "
                              "username TEXT UNIQUE NOT NULL, password_hash TEXT, salt TEXT)";
    sqlite3_exec(db, create_table, NULL, NULL, NULL);
    
    // Insert test users
    const char* test_users[] = {"user1", "user2", "user3", "user4", "user5"};
    for (int i = 0; i < 5; i++) {
        char sql[256];
        snprintf(sql, sizeof(sql), "INSERT INTO users (username) VALUES ('%s')", test_users[i]);
        sqlite3_exec(db, sql, NULL, NULL, NULL);
    }
    
    // Test cases
    printf("Test 1: %d\\n", update_user_password("user1", "SecurePass123!", db));
    printf("Test 2: %d\\n", update_user_password("user2", "MyP@ssw0rd", db));
    printf("Test 3: %d\\n", update_user_password("nonexistent", "password", db));
    printf("Test 4: %d\\n", update_user_password("user3", "", db));
    printf("Test 5: %d\\n", update_user_password(NULL, "password", db));
    
    sqlite3_close(db);
    return 0;
}
