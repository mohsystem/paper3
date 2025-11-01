
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>

#define DB_NAME "users.db"
#define MAX_LEN 256

void hash_password(const char* password, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\\0';
}

void init_database() {
    sqlite3* db;
    char* err_msg = NULL;
    
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        return;
    }
    
    const char* create_table = "CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY, password TEXT)";
    sqlite3_exec(db, create_table, NULL, NULL, &err_msg);
    sqlite3_exec(db, "DELETE FROM users", NULL, NULL, &err_msg);
    
    // Insert test users
    const char* test_users[] = {"admin", "user1", "user2", "user3", "user4"};
    const char* test_passwords[] = {"admin123", "pass123", "password", "test123", "user123"};
    
    for(int i = 0; i < 5; i++) {
        char hashed_pass[65];
        hash_password(test_passwords[i], hashed_pass);
        
        sqlite3_stmt* stmt;
        const char* sql = "INSERT INTO users VALUES (?, ?)";
        sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        sqlite3_bind_text(stmt, 1, test_users[i], -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, hashed_pass, -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    
    sqlite3_close(db);
}

int login(const char* username, const char* password) {
    char hashed_password[65];
    hash_password(password, hashed_password);
    
    sqlite3* db;
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK) {
        return 0;
    }
    
    sqlite3_stmt* stmt;
    const char* sql = "SELECT * FROM users WHERE username = ? AND password = ?";
    
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hashed_password, -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    int found = (rc == SQLITE_ROW);
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return found;
}

void authenticate(const char* username, const char* password, char* result) {
    if (login(username, password)) {
        sprintf(result, "REDIRECT: Dashboard - Welcome %s!", username);
    } else {
        strcpy(result, "REDIRECT: Login Page - Invalid credentials");
    }
}

int main() {
    init_database();
    
    char result[MAX_LEN];
    
    printf("=== Secure Login System Test Cases ===\\n\\n");
    
    // Test case 1: Valid admin login
    printf("Test 1 - Valid admin:\\n");
    authenticate("admin", "admin123", result);
    printf("%s\\n", result);
    
    // Test case 2: Valid user login
    printf("\\nTest 2 - Valid user1:\\n");
    authenticate("user1", "pass123", result);
    printf("%s\\n", result);
    
    // Test case 3: Invalid password
    printf("\\nTest 3 - Invalid password:\\n");
    authenticate("admin", "wrongpass", result);
    printf("%s\\n", result);
    
    // Test case 4: Non-existent user
    printf("\\nTest 4 - Non-existent user:\\n");
    authenticate("hacker", "hack123", result);
    printf("%s\\n", result);
    
    // Test case 5: SQL injection attempt
    printf("\\nTest 5 - SQL injection attempt:\\n");
    authenticate("admin' OR '1'='1", "anything", result);
    printf("%s\\n", result);
    
    return 0;
}
