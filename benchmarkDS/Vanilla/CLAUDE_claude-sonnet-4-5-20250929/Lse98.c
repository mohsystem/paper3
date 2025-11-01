
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>

void hashPassword(const char* password, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\\0';
}

int registerUser(const char* username, const char* password) {
    char hashedPassword[65];
    hashPassword(password, hashedPassword);
    
    sqlite3* db;
    char* errMsg = 0;
    
    int rc = sqlite3_open("users.db", &db);
    if(rc) {
        fprintf(stderr, "Error opening database: %s\\n", sqlite3_errmsg(db));
        return 0;
    }
    
    const char* createTableSQL = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE, password TEXT);";
    rc = sqlite3_exec(db, createTableSQL, 0, 0, &errMsg);
    
    if(rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return 0;
    }
    
    sqlite3_stmt* stmt;
    const char* insertSQL = "INSERT INTO users (username, password) VALUES (?, ?);";
    rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, 0);
    
    if(rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, hashedPassword, -1, SQLITE_STATIC);
        rc = sqlite3_step(stmt);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return rc == SQLITE_DONE;
}

int main() {
    printf("Test Case 1: %d\\n", registerUser("alice", "password123"));
    printf("Test Case 2: %d\\n", registerUser("bob", "securePass456"));
    printf("Test Case 3: %d\\n", registerUser("charlie", "mySecret789"));
    printf("Test Case 4: %d\\n", registerUser("diana", "test@2024"));
    printf("Test Case 5: %d\\n", registerUser("eve", "strongPwd!"));
    return 0;
}
