
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>

#define DB_PATH "users.db"
#define MAX_HASH_LENGTH 65

void hash_password(const char* password, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\\0';
}

void initialize_database() {
    sqlite3* db;
    char* err_msg = NULL;
    
    int rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        return;
    }
    
    const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                     "username TEXT UNIQUE NOT NULL,"
                     "password_hash TEXT NOT NULL)";
    
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", err_msg);
        sqlite3_free(err_msg);
    }
    
    sqlite3_close(db);
}

int register_user(const char* username, const char* password) {
    if (username == NULL || password == NULL || 
        strlen(username) == 0 || strlen(password) == 0) {
        return 0;
    }
    
    char hashed_password[MAX_HASH_LENGTH];
    hash_password(password, hashed_password);
    
    sqlite3* db;
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        return 0;
    }
    
    const char* sql = "INSERT INTO users (username, password_hash) VALUES (?, ?)";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hashed_password, -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Registration error\\n");
        return 0;
    }
    
    return 1;
}

int main() {
    initialize_database();
    
    // Test case 1: Valid registration
    printf("Test 1 - Valid registration: %s\\n", 
           register_user("john_doe", "password123") ? "true" : "false");
    
    // Test case 2: Another valid registration
    printf("Test 2 - Valid registration: %s\\n", 
           register_user("jane_smith", "securePass456") ? "true" : "false");
    
    // Test case 3: Duplicate username (should fail)
    printf("Test 3 - Duplicate username: %s\\n", 
           register_user("john_doe", "differentPassword") ? "true" : "false");
    
    // Test case 4: Empty username (should fail)
    printf("Test 4 - Empty username: %s\\n", 
           register_user("", "password789") ? "true" : "false");
    
    // Test case 5: NULL password (should fail)
    printf("Test 5 - NULL password: %s\\n", 
           register_user("bob_jones", NULL) ? "true" : "false");
    
    return 0;
}
