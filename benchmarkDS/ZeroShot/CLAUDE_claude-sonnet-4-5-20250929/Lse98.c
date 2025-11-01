
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode(const unsigned char* data, size_t len, char* output) {
    int val = 0, valb = -6;
    size_t out_idx = 0;
    
    for (size_t i = 0; i < len; i++) {
        val = (val << 8) + data[i];
        valb += 8;
        while (valb >= 0) {
            output[out_idx++] = base64_chars[(val >> valb) & 0x3F];
            valb -= 6;
        }
    }
    if (valb > -6) output[out_idx++] = base64_chars[((val << 8) >> (valb + 8)) & 0x3F];
    while (out_idx % 4) output[out_idx++] = '=';
    output[out_idx] = '\\0';
}

void hash_password(const char* password, const unsigned char* salt, size_t salt_len, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, salt, salt_len);
    SHA256_Update(&sha256, password, strlen(password));
    SHA256_Final(hash, &sha256);
    base64_encode(hash, SHA256_DIGEST_LENGTH, output);
}

void generate_salt(unsigned char* salt, size_t len) {
    RAND_bytes(salt, len);
}

int register_user(const char* username, const char* password, sqlite3* db) {
    if (!username || !password || strlen(username) == 0 || strlen(password) == 0) {
        return 0;
    }
    
    unsigned char salt[16];
    generate_salt(salt, 16);
    
    char hashed_password[128];
    hash_password(password, salt, 16, hashed_password);
    
    char salt_string[64];
    base64_encode(salt, 16, salt_string);
    
    const char* sql = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hashed_password, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, salt_string, -1, SQLITE_TRANSIENT);
    
    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return result == SQLITE_DONE;
}

int main() {
    sqlite3* db;
    sqlite3_open(":memory:", &db);
    
    const char* create_table = "CREATE TABLE users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE NOT NULL, password_hash TEXT NOT NULL, salt TEXT NOT NULL)";
    sqlite3_exec(db, create_table, NULL, NULL, NULL);
    
    printf("Test 1: %d\\n", register_user("john_doe", "SecurePass123!", db));
    printf("Test 2: %d\\n", register_user("jane_smith", "MyP@ssw0rd", db));
    printf("Test 3: %d\\n", register_user("", "password123", db));
    printf("Test 4: %d\\n", register_user("user123", "", db));
    printf("Test 5: %d\\n", register_user(NULL, NULL, db));
    
    sqlite3_close(db);
    return 0;
}
