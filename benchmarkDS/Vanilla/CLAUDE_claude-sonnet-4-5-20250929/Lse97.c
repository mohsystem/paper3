
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/sha.h>

static sqlite3* db = NULL;

void hash_password(const char* password, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[SHA256_DIGEST_LENGTH * 2] = '\\0';
}

const char* register_user(const char* username, const char* password, 
                         const char* first_name, const char* last_name, 
                         const char* email) {
    // Initialize database if not exists
    if (db == NULL) {
        if (sqlite3_open("users.db", &db) != SQLITE_OK) {
            return "Database connection failed";
        }
        
        const char* create_table = "CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY, password TEXT, firstName TEXT, lastName TEXT, email TEXT)";
        char* err_msg;
        if (sqlite3_exec(db, create_table, NULL, NULL, &err_msg) != SQLITE_OK) {
            sqlite3_free(err_msg);
            return "Table creation failed";
        }
    }
    
    // Check if username exists
    sqlite3_stmt* stmt;
    const char* check_query = "SELECT username FROM users WHERE username = ?";
    
    if (sqlite3_prepare_v2(db, check_query, -1, &stmt, NULL) != SQLITE_OK) {
        return "Query preparation failed";
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return "Username already exists";
    }
    sqlite3_finalize(stmt);
    
    // Hash password
    char hashed_password[65];
    hash_password(password, hashed_password);
    
    // Insert user
    const char* insert_query = "INSERT INTO users (username, password, firstName, lastName, email) VALUES (?, ?, ?, ?, ?)";
    
    if (sqlite3_prepare_v2(db, insert_query, -1, &stmt, NULL) != SQLITE_OK) {
        return "Insert preparation failed";
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hashed_password, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, first_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, last_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, email, -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return "Registration failed";
    }
    
    sqlite3_finalize(stmt);
    return "Registration succeeded";
}

void close_database() {
    if (db != NULL) {
        sqlite3_close(db);
        db = NULL;
    }
}

int main() {
    // Test cases
    printf("%s\\n", register_user("john_doe", "password123", "John", "Doe", "john@example.com"));
    printf("%s\\n", register_user("jane_smith", "securePass456", "Jane", "Smith", "jane@example.com"));
    printf("%s\\n", register_user("john_doe", "anotherPass", "John", "Doe", "john2@example.com"));
    printf("%s\\n", register_user("bob_wilson", "bobPass789", "Bob", "Wilson", "bob@example.com"));
    printf("%s\\n", register_user("alice_brown", "alicePass321", "Alice", "Brown", "alice@example.com"));
    
    close_database();
    return 0;
}
