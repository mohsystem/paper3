
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>

#define DB_NAME "users.db"

void hash_password(const char* password, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\\0';
}

int initialize_database() {
    sqlite3* db;
    char* err_msg = 0;
    
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }
    
    const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                     "name TEXT NOT NULL,"
                     "email TEXT UNIQUE NOT NULL,"
                     "password TEXT NOT NULL);";
    
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 0;
    }
    
    sqlite3_close(db);
    return 1;
}

int register_user(const char* name, const char* email, const char* password) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    char hashed_password[65];
    
    hash_password(password, hashed_password);
    
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        return 0;
    }
    
    const char* sql = "INSERT INTO users (name, email, password) VALUES (?, ?, ?);";
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }
    
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, email, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, hashed_password, -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    int success = (rc == SQLITE_DONE);
    
    if (!success) {
        fprintf(stderr, "Registration error: %s\\n", sqlite3_errmsg(db));
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return success;
}

int user_exists(const char* email) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK) return 0;
    
    const char* sql = "SELECT COUNT(*) FROM users WHERE email = ?;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return 0;
    }
    
    sqlite3_bind_text(stmt, 1, email, -1, SQLITE_STATIC);
    
    int exists = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        exists = sqlite3_column_int(stmt, 0) > 0;
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return exists;
}

int main() {
    printf("User Registration System - Test Cases\\n\\n");
    initialize_database();
    
    // Test Case 1
    printf("Test Case 1: Register new user\\n");
    int result1 = register_user("John Doe", "john@example.com", "password123");
    printf("Registration successful: %s\\n", result1 ? "true" : "false");
    printf("User exists: %s\\n\\n", user_exists("john@example.com") ? "true" : "false");
    
    // Test Case 2
    printf("Test Case 2: Register another user\\n");
    int result2 = register_user("Jane Smith", "jane@example.com", "securePass456");
    printf("Registration successful: %s\\n", result2 ? "true" : "false");
    printf("User exists: %s\\n\\n", user_exists("jane@example.com") ? "true" : "false");
    
    // Test Case 3
    printf("Test Case 3: Try to register duplicate email\\n");
    int result3 = register_user("John Another", "john@example.com", "newpassword");
    printf("Registration successful: %s\\n\\n", result3 ? "true" : "false");
    
    // Test Case 4
    printf("Test Case 4: Register user with special characters\\n");
    int result4 = register_user("Mike O'Brien", "mike.obrien@example.com", "P@ssw0rd!");
    printf("Registration successful: %s\\n", result4 ? "true" : "false");
    printf("User exists: %s\\n\\n", user_exists("mike.obrien@example.com") ? "true" : "false");
    
    // Test Case 5
    printf("Test Case 5: Register user with long name\\n");
    int result5 = register_user("Alexander Christopher Wellington", "alex@example.com", "myPassword789");
    printf("Registration successful: %s\\n", result5 ? "true" : "false");
    printf("User exists: %s\\n", user_exists("alex@example.com") ? "true" : "false");
    
    return 0;
}
