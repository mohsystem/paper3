
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sqlite3.h>
#include <openssl/sha.h>

void generate_salt(char* salt_out, size_t len) {
    srand(time(NULL));
    for (size_t i = 0; i < len; i += 2) {
        sprintf(salt_out + i, "%02x", rand() % 256);
    }
}

void hash_password(const char* password, const char* salt, char* hash_out) {
    char combined[512];
    sprintf(combined, "%s%s", salt, password);
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)combined, strlen(combined), hash);
    
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hash_out + (i * 2), "%02x", hash[i]);
    }
    hash_out[SHA256_DIGEST_LENGTH * 2] = '\\0';
}

int insert_user(const char* username, const char* password) {
    char salt[33];
    char hashed_password[65];
    
    generate_salt(salt, 32);
    salt[32] = '\\0';
    hash_password(password, salt, hashed_password);
    
    sqlite3* db;
    char* errMsg = NULL;
    
    if (sqlite3_open("users.db", &db) != SQLITE_OK) {
        fprintf(stderr, "Error opening database\\n");
        return 0;
    }
    
    const char* createTableSQL = "CREATE TABLE IF NOT EXISTS users ("
                                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                "username TEXT NOT NULL UNIQUE,"
                                "password TEXT NOT NULL,"
                                "salt TEXT NOT NULL)";
    
    if (sqlite3_exec(db, createTableSQL, NULL, NULL, &errMsg) != SQLITE_OK) {
        fprintf(stderr, "Error creating table: %s\\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return 0;
    }
    
    sqlite3_stmt* stmt;
    const char* insertSQL = "INSERT INTO users (username, password, salt) VALUES (?, ?, ?)";
    
    if (sqlite3_prepare_v2(db, insertSQL, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error preparing statement\\n");
        sqlite3_close(db);
        return 0;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hashed_password, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, salt, -1, SQLITE_TRANSIENT);
    
    int success = (sqlite3_step(stmt) == SQLITE_DONE);
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return success;
}

int main() {
    printf("Test 1: %d\\n", insert_user("alice", "password123"));
    printf("Test 2: %d\\n", insert_user("bob", "securePass456"));
    printf("Test 3: %d\\n", insert_user("charlie", "mySecret789"));
    printf("Test 4: %d\\n", insert_user("diana", "p@ssw0rd!"));
    printf("Test 5: %d\\n", insert_user("eve", "test12345"));
    
    return 0;
}
