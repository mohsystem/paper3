
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <time.h>

#define SALT_LENGTH 16
#define HASH_LENGTH 32

void generate_salt(char* salt_out) {
    srand(time(NULL));
    for (int i = 0; i < SALT_LENGTH; i++) {
        salt_out[i] = rand() % 256;
    }
}

void hash_password(const char* password, const char* salt, char* hash_out) {
    char combined[256];
    int salt_len = SALT_LENGTH;
    int pass_len = strlen(password);
    
    memcpy(combined, salt, salt_len);
    memcpy(combined + salt_len, password, pass_len);
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)combined, salt_len + pass_len, hash);
    
    memcpy(hash_out, hash, SHA256_DIGEST_LENGTH);
}

int insert_user(const char* username, const char* password) {
    sqlite3* db;
    char* err_msg = NULL;
    
    if (sqlite3_open("users.db", &db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open database\\n");
        return 0;
    }
    
    const char* create_table_sql = 
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT UNIQUE NOT NULL, "
        "password BLOB NOT NULL, "
        "salt BLOB NOT NULL)";
    
    sqlite3_exec(db, create_table_sql, NULL, NULL, &err_msg);
    
    char salt[SALT_LENGTH];
    char hash[HASH_LENGTH];
    
    generate_salt(salt);
    hash_password(password, salt, hash);
    
    const char* insert_sql = "INSERT INTO users (username, password, salt) VALUES (?, ?, ?)";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
        sqlite3_bind_blob(stmt, 2, hash, HASH_LENGTH, SQLITE_STATIC);
        sqlite3_bind_blob(stmt, 3, salt, SALT_LENGTH, SQLITE_STATIC);
        
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            fprintf(stderr, "Error inserting user\\n");
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return 0;
        }
        sqlite3_finalize(stmt);
    }
    
    sqlite3_close(db);
    return 1;
}

int main() {
    printf("Test Case 1: %d\\n", insert_user("alice", "password123"));
    printf("Test Case 2: %d\\n", insert_user("bob", "securePass456"));
    printf("Test Case 3: %d\\n", insert_user("charlie", "myP@ssw0rd"));
    printf("Test Case 4: %d\\n", insert_user("david", "test1234"));
    printf("Test Case 5: %d\\n", insert_user("eve", "strongPassword!"));
    return 0;
}
