
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

#define DB_NAME "users.db"
#define MAX_STRING_LEN 256

char* base64_encode(const unsigned char* buffer, size_t length) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, buffer, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    
    char* result = (char*)malloc(bufferPtr->length + 1);
    memcpy(result, bufferPtr->data, bufferPtr->length);
    result[bufferPtr->length] = '\\0';
    
    BIO_free_all(bio);
    return result;
}

char* hash_password(const char* password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    return base64_encode(hash, SHA256_DIGEST_LENGTH);
}

void initialize_database() {
    sqlite3* db;
    char* errMsg = NULL;
    
    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        return;
    }
    
    const char* createTableSQL = 
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE NOT NULL,"
        "password TEXT NOT NULL)";
    
    sqlite3_exec(db, createTableSQL, NULL, NULL, &errMsg);
    sqlite3_exec(db, "DELETE FROM users", NULL, NULL, &errMsg);
    
    const char* testUsers[][2] = {
        {"admin", "admin123"},
        {"testuser", "password"},
        {"john", "john123"},
        {"jane", "jane456"},
        {"guest", "guest"}
    };
    
    for (int i = 0; i < 5; i++) {
        char* hashedPwd = hash_password(testUsers[i][1]);
        sqlite3_stmt* stmt;
        const char* sql = "INSERT OR IGNORE INTO users (username, password) VALUES (?, ?)";
        
        sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        sqlite3_bind_text(stmt, 1, testUsers[i][0], -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, hashedPwd, -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        free(hashedPwd);
    }
    
    sqlite3_close(db);
}

char* authenticate_user(const char* username, const char* password) {
    static char result[MAX_STRING_LEN];
    
    if (!username || !password || strlen(username) == 0 || strlen(password) == 0) {
        strcpy(result, "redirect:/login?error=empty");
        return result;
    }
    
    char* hashedPassword = hash_password(password);
    sqlite3* db;
    
    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
        free(hashedPassword);
        strcpy(result, "redirect:/login?error=system");
        return result;
    }
    
    sqlite3_stmt* stmt;
    const char* sql = "SELECT username FROM users WHERE username = ? AND password = ?";
    
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hashedPassword, -1, SQLITE_TRANSIENT);
    
    int step_result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    free(hashedPassword);
    
    if (step_result == SQLITE_ROW) {
        strcpy(result, "redirect:/dashboard");
    } else {
        strcpy(result, "redirect:/login?error=invalid");
    }
    
    return result;
}

int main() {
    initialize_database();
    
    printf("=== Secure Login System Test Cases ===\\n\\n");
    
    printf("Test 1 - Valid login (admin/admin123):\\n");
    printf("Result: %s\\n\\n", authenticate_user("admin", "admin123"));
    
    printf("Test 2 - Invalid password (admin/wrongpass):\\n");
    printf("Result: %s\\n\\n", authenticate_user("admin", "wrongpass"));
    
    printf("Test 3 - Non-existent user (hacker/hack123):\\n");
    printf("Result: %s\\n\\n", authenticate_user("hacker", "hack123"));
    
    printf("Test 4 - Empty credentials:\\n");
    printf("Result: %s\\n\\n", authenticate_user("", ""));
    
    printf("Test 5 - SQL Injection attempt (admin' OR '1'='1/anything):\\n");
    printf("Result: %s\\n\\n", authenticate_user("admin' OR '1'='1", "anything"));
    
    return 0;
}
