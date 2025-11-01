
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <sqlite3.h>

void hashPassword(const char* password, char* outputBuffer) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
    outputBuffer[64] = '\\0';
}

int insertUser(const char* username, const char* password) {
    char hashedPassword[65];
    hashPassword(password, hashedPassword);
    
    sqlite3* db;
    char* errMsg = 0;
    int rc = sqlite3_open("users.db", &db);
    
    if(rc) {
        printf("Can't open database: %s\\n", sqlite3_errmsg(db));
        return 0;
    }
    
    const char* createTableSQL = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE, password TEXT);";
    rc = sqlite3_exec(db, createTableSQL, 0, 0, &errMsg);
    
    if(rc != SQLITE_OK) {
        printf("SQL error: %s\\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return 0;
    }
    
    sqlite3_stmt* stmt;
    const char* insertSQL = "INSERT INTO users (username, password) VALUES (?, ?);";
    rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, 0);
    
    if(rc != SQLITE_OK) {
        printf("Failed to prepare statement\\n");
        sqlite3_close(db);
        return 0;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hashedPassword, -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return rc == SQLITE_DONE;
}

int main() {
    // Test cases
    printf("Test Case 1:\\n");
    int result1 = insertUser("user1", "password123");
    printf("User1 inserted: %s\\n", result1 ? "true" : "false");
    
    printf("\\nTest Case 2:\\n");
    int result2 = insertUser("user2", "securePass456");
    printf("User2 inserted: %s\\n", result2 ? "true" : "false");
    
    printf("\\nTest Case 3:\\n");
    int result3 = insertUser("user3", "mySecret789");
    printf("User3 inserted: %s\\n", result3 ? "true" : "false");
    
    printf("\\nTest Case 4:\\n");
    int result4 = insertUser("user4", "testPass000");
    printf("User4 inserted: %s\\n", result4 ? "true" : "false");
    
    printf("\\nTest Case 5:\\n");
    int result5 = insertUser("user5", "finalTest111");
    printf("User5 inserted: %s\\n", result5 ? "true" : "false");
    
    return 0;
}
