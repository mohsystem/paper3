
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

#define MAX_USERNAME 50
#define MAX_PASSWORD 50

typedef struct {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
} User;

// Simulated database
User mockDatabase[] = {
    {"admin", "password123"},
    {"user1", "pass1"},
    {"user2", "pass2"},
    {"testuser", "testpass"},
    {"john", "doe123"}
};

int mockDatabaseSize = 5;

const char* loginUser(const char* username, const char* password) {
    // Check if username and password are not NULL or empty
    if (username == NULL || password == NULL || strlen(username) == 0 || strlen(password) == 0) {
        return "cant_login";
    }
    
    // Check credentials in mock database
    for (int i = 0; i < mockDatabaseSize; i++) {
        if (strcmp(mockDatabase[i].username, username) == 0 && 
            strcmp(mockDatabase[i].password, password) == 0) {
            return "success";
        }
    }
    
    return "cant_login";
}

const char* loginUserWithDB(const char* username, const char* password, const char* dbPath) {
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    int rc;
    
    // Open database connection
    rc = sqlite3_open(dbPath, &db);
    if (rc != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return "internal_server_error";
    }
    
    // Prepare SQL query with parameterized statement
    const char* sql = "SELECT * FROM users WHERE username = ? AND password = ?";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        if (stmt) sqlite3_finalize(stmt);
        sqlite3_close(db);
        return "internal_server_error";
    }
    
    // Bind parameters
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);
    
    // Execute query
    rc = sqlite3_step(stmt);
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    if (rc == SQLITE_ROW) {
        return "success";
    } else if (rc == SQLITE_DONE) {
        return "cant_login";
    } else {
        return "internal_server_error";
    }
}

int main() {
    // Test case 1: Valid credentials
    printf("Test 1 - Valid login (admin/password123): %s\\n", loginUser("admin", "password123"));
    
    // Test case 2: Invalid username
    printf("Test 2 - Invalid username: %s\\n", loginUser("wronguser", "password123"));
    
    // Test case 3: Invalid password
    printf("Test 3 - Invalid password: %s\\n", loginUser("admin", "wrongpass"));
    
    // Test case 4: Empty credentials
    printf("Test 4 - Empty credentials: %s\\n", loginUser("", ""));
    
    // Test case 5: Valid user2
    printf("Test 5 - Valid login (user2/pass2): %s\\n", loginUser("user2", "pass2"));
    
    return 0;
}
