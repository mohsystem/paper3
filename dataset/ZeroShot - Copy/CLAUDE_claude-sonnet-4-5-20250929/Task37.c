
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <stdlib.h>

#define DB_PATH "users.db"
#define MAX_RESULT_SIZE 1024

// Retrieve user information securely using prepared statements
char* get_user_info(const char* username, char* result, size_t result_size) {
    if (username == NULL || strlen(username) == 0) {
        snprintf(result, result_size, "Invalid username");
        return result;
    }
    
    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;
    
    rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        snprintf(result, result_size, "Cannot open database: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return result;
    }
    
    // Use parameterized query to prevent SQL injection
    const char* query = "SELECT id, username, email, created_date FROM users WHERE username = ?";
    
    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        snprintf(result, result_size, "Failed to prepare statement: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return result;
    }
    
    // Bind parameter
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        snprintf(result, result_size, 
                "User ID: %d\\nUsername: %s\\nEmail: %s\\nCreated Date: %s",
                sqlite3_column_int(stmt, 0),
                sqlite3_column_text(stmt, 1),
                sqlite3_column_text(stmt, 2),
                sqlite3_column_text(stmt, 3));
    } else if (rc == SQLITE_DONE) {
        snprintf(result, result_size, "User not found");
    } else {
        snprintf(result, result_size, "Query execution error: %s", sqlite3_errmsg(db));
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

// Initialize database with test data
void initialize_database() {
    sqlite3* db;
    char* err_msg = NULL;
    int rc;
    
    rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        return;
    }
    
    const char* create_table_sql = 
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT NOT NULL UNIQUE, "
        "email TEXT NOT NULL, "
        "created_date TEXT NOT NULL)";
    
    rc = sqlite3_exec(db, create_table_sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return;
    }
    
    // Insert test data
    const char* test_data[5][3] = {
        {"john_doe", "john@example.com", "2024-01-15"},
        {"jane_smith", "jane@example.com", "2024-02-20"},
        {"bob_wilson", "bob@example.com", "2024-03-10"},
        {"alice_jones", "alice@example.com", "2024-04-05"},
        {"charlie_brown", "charlie@example.com", "2024-05-12"}
    };
    
    const char* insert_sql = "INSERT OR IGNORE INTO users (username, email, created_date) VALUES (?, ?, ?)";
    sqlite3_stmt* stmt;
    
    for (int i = 0; i < 5; i++) {
        sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL);
        sqlite3_bind_text(stmt, 1, test_data[i][0], -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, test_data[i][1], -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, test_data[i][2], -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    
    sqlite3_close(db);
}

int main() {
    char result[MAX_RESULT_SIZE];
    
    // Initialize database with test data
    initialize_database();
    
    printf("=== Secure Database Query - Test Cases ===\\n\\n");
    
    // Test Case 1: Valid username
    printf("Test Case 1: Valid username 'john_doe'\\n");
    printf("%s\\n", get_user_info("john_doe", result, MAX_RESULT_SIZE));
    printf("\\n==================================================\\n\\n");
    
    // Test Case 2: Another valid username
    printf("Test Case 2: Valid username 'jane_smith'\\n");
    printf("%s\\n", get_user_info("jane_smith", result, MAX_RESULT_SIZE));
    printf("\\n==================================================\\n\\n");
    
    // Test Case 3: Non-existent username
    printf("Test Case 3: Non-existent username 'unknown_user'\\n");
    printf("%s\\n", get_user_info("unknown_user", result, MAX_RESULT_SIZE));
    printf("\\n==================================================\\n\\n");
    
    // Test Case 4: SQL injection attempt (secured by prepared statement)
    printf("Test Case 4: SQL injection attempt\\n");
    printf("%s\\n", get_user_info("' OR '1'='1", result, MAX_RESULT_SIZE));
    printf("\\n==================================================\\n\\n");
    
    // Test Case 5: Empty username
    printf("Test Case 5: Empty username\\n");
    printf("%s\\n", get_user_info("", result, MAX_RESULT_SIZE));
    printf("\\n==================================================\\n\\n");
    
    return 0;
}
