
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_USERNAME_LENGTH 50
#define MAX_EMAIL_LENGTH 100
#define MAX_DATE_LENGTH 20

typedef struct {
    int user_id;
    char username[MAX_USERNAME_LENGTH + 1];
    char email[MAX_EMAIL_LENGTH + 1];
    char created_at[MAX_DATE_LENGTH + 1];
    bool found;
} UserInfo;

bool validate_username(const char* username) {
    if (username == NULL || strlen(username) == 0) {
        return false;
    }
    
    size_t len = strlen(username);
    if (len > MAX_USERNAME_LENGTH) {
        return false;
    }
    
    // Check if username contains only alphanumeric and underscore
    for (size_t i = 0; i < len; i++) {
        if (!isalnum(username[i]) && username[i] != '_') {
            return false;
        }
    }
    
    return true;
}

UserInfo get_user_by_username(const char* db_path, const char* username) {
    UserInfo user_info = {0};
    user_info.found = false;
    
    // Input validation
    if (!validate_username(username)) {
        fprintf(stderr, "Invalid username provided\\n");
        return user_info;
    }
    
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    int rc;
    
    // Open database connection
    rc = sqlite3_open(db_path, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        return user_info;
    }
    
    // Prepare parameterized query to prevent SQL injection
    const char* sql = "SELECT user_id, username, email, created_at FROM users WHERE username = ?";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return user_info;
    }
    
    // Bind parameter safely
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    
    // Execute query
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        user_info.user_id = sqlite3_column_int(stmt, 0);
        strncpy(user_info.username, (const char*)sqlite3_column_text(stmt, 1), MAX_USERNAME_LENGTH);
        strncpy(user_info.email, (const char*)sqlite3_column_text(stmt, 2), MAX_EMAIL_LENGTH);
        strncpy(user_info.created_at, (const char*)sqlite3_column_text(stmt, 3), MAX_DATE_LENGTH);
        user_info.found = true;
    }
    
    // Clean up resources
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return user_info;
}

void setup_test_database(const char* db_path) {
    sqlite3* db;
    char* err_msg = NULL;
    
    sqlite3_open(db_path, &db);
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS users (user_id INTEGER PRIMARY KEY, username TEXT UNIQUE, email TEXT, created_at TEXT)", 
                 NULL, NULL, &err_msg);
    
    // Insert test data
    const char* insert_sql = "INSERT OR IGNORE INTO users VALUES (?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL);
    
    struct TestData {
        int id;
        const char* username;
        const char* email;
        const char* date;
    } test_data[] = {
        {1, "john_doe", "john@example.com", "2024-01-01"},
        {2, "alice", "alice@example.com", "2024-01-02"},
        {3, "bob123", "bob@example.com", "2024-01-03"}
    };
    
    for (int i = 0; i < 3; i++) {
        sqlite3_reset(stmt);
        sqlite3_bind_int(stmt, 1, test_data[i].id);
        sqlite3_bind_text(stmt, 2, test_data[i].username, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, test_data[i].email, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, test_data[i].date, -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

int main() {
    const char* db_path = "test_users.db";
    
    // Setup test database
    setup_test_database(db_path);
    
    // Test cases
    const char* test_users[] = {"john_doe", "alice", "bob123", "admin", "test_user"};
    
    printf("Testing User Retrieval:\\n\\n");
    for (int i = 0; i < 5; i++) {
        printf("Searching for user: %s\\n", test_users[i]);
        UserInfo user = get_user_by_username(db_path, test_users[i]);
        
        if (user.found) {
            printf("Found: UserInfo{userId=%d, username='%s', email='%s', createdAt='%s'}\\n",
                   user.user_id, user.username, user.email, user.created_at);
        } else {
            printf("User not found or error occurred\\n");
        }
        printf("---\\n");
    }
    
    return 0;
}
