
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

#define DB_NAME "users.db"

void initialize_database() {
    sqlite3* db;
    char* errMsg = NULL;
    
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\\n", sqlite3_errmsg(db));
        return;
    }
    
    const char* createTableSQL = 
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL UNIQUE,"
        "email TEXT,"
        "fullname TEXT)";
    
    rc = sqlite3_exec(db, createTableSQL, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", errMsg);
        sqlite3_free(errMsg);
    }
    
    const char* insertSQL = 
        "INSERT OR IGNORE INTO users (username, email, fullname) VALUES "
        "('john_doe', 'john@example.com', 'John Doe'),"
        "('jane_smith', 'jane@example.com', 'Jane Smith'),"
        "('bob_wilson', 'bob@example.com', 'Bob Wilson'),"
        "('alice_brown', 'alice@example.com', 'Alice Brown'),"
        "('charlie_davis', 'charlie@example.com', 'Charlie Davis')";
    
    rc = sqlite3_exec(db, insertSQL, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", errMsg);
        sqlite3_free(errMsg);
    }
    
    sqlite3_close(db);
}

void get_user_info(const char* username, char* result, size_t result_size) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc) {
        snprintf(result, result_size, "Can't open database: %s", sqlite3_errmsg(db));
        return;
    }
    
    const char* query = "SELECT * FROM users WHERE username = ?";
    
    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        snprintf(result, result_size, "Failed to prepare statement: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        snprintf(result, result_size, 
                "User found:\\nID: %d\\nUsername: %s\\nEmail: %s\\nFull Name: %s\\n",
                sqlite3_column_int(stmt, 0),
                sqlite3_column_text(stmt, 1),
                sqlite3_column_text(stmt, 2),
                sqlite3_column_text(stmt, 3));
    } else {
        snprintf(result, result_size, "User not found: %s", username);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

int main() {
    char result[1024];
    
    initialize_database();
    
    printf("Test Case 1:\\n");
    get_user_info("john_doe", result, sizeof(result));
    printf("%s\\n", result);
    printf("\\n==================================================\\n\\n");
    
    printf("Test Case 2:\\n");
    get_user_info("jane_smith", result, sizeof(result));
    printf("%s\\n", result);
    printf("\\n==================================================\\n\\n");
    
    printf("Test Case 3:\\n");
    get_user_info("alice_brown", result, sizeof(result));
    printf("%s\\n", result);
    printf("\\n==================================================\\n\\n");
    
    printf("Test Case 4:\\n");
    get_user_info("nonexistent_user", result, sizeof(result));
    printf("%s\\n", result);
    printf("\\n==================================================\\n\\n");
    
    printf("Test Case 5:\\n");
    get_user_info("charlie_davis", result, sizeof(result));
    printf("%s\\n", result);
    
    return 0;
}
