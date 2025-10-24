
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

char* hash_password(const char* password) {
    unsigned char salt[16];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        return NULL;
    }
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, salt, sizeof(salt));
    SHA256_Update(&sha256, password, strlen(password));
    SHA256_Final(hash, &sha256);
    
    unsigned char combined[48];
    memcpy(combined, salt, 16);
    memcpy(combined + 16, hash, SHA256_DIGEST_LENGTH);
    
    char* result = (char*)malloc(65);
    const char* hex = "0123456789abcdef";
    for (int i = 0; i < 48; i++) {
        result[i * 2] = hex[combined[i] >> 4];
        result[i * 2 + 1] = hex[combined[i] & 0x0F];
    }
    result[96] = '\\0';
    return result;
}

int update_user_password(const char* username, const char* new_password) {
    if (username == NULL || new_password == NULL || 
        strlen(username) == 0 || strlen(new_password) == 0 ||
        strlen(username) > 255 || strlen(new_password) > 255) {
        return 0;
    }
    
    char* hashed_password = hash_password(new_password);
    if (hashed_password == NULL) {
        return 0;
    }
    
    sqlite3* db;
    if (sqlite3_open("users.db", &db) != SQLITE_OK) {
        free(hashed_password);
        return 0;
    }
    
    const char* sql = "UPDATE users SET password = ? WHERE username = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        free(hashed_password);
        sqlite3_close(db);
        return 0;
    }
    
    sqlite3_bind_text(stmt, 1, hashed_password, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, username, -1, SQLITE_STATIC);
    
    sqlite3_step(stmt);
    int rows_affected = sqlite3_changes(db);
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    free(hashed_password);
    
    return rows_affected > 0;
}

int main() {
    printf("Test Case 1: %d\\n", update_user_password("user1", "SecurePass123!"));
    printf("Test Case 2: %d\\n", update_user_password("user2", "AnotherP@ssw0rd"));
    printf("Test Case 3: %d\\n", update_user_password("admin", "AdminSecure#456"));
    printf("Test Case 4: %d\\n", update_user_password("", "password"));
    printf("Test Case 5: %d\\n", update_user_password("user3", ""));
    
    return 0;
}
