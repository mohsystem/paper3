
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_INPUT_LENGTH 500
#define DB_NAME "messages.db"

// Sanitize input to prevent XSS and SQL injection
void sanitize_input(const char* input, char* output, size_t output_size) {
    if (input == NULL || output == NULL || output_size == 0) {
        return;
    }
    
    size_t j = 0;
    for (size_t i = 0; i < strlen(input) && j < output_size - 1; i++) {
        char c = input[i];
        // Remove dangerous characters
        if (c != '<' && c != '>' && c != '"' && c != '\\'' && c != '&') {
            output[j++] = c;
        }
    }
    output[j] = '\\0';
}

// Get current timestamp
void get_current_timestamp(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

// Initialize database
sqlite3* init_database() {
    sqlite3* db;
    int rc = sqlite3_open(DB_NAME, &db);
    
    if (rc) {
        fprintf(stderr, "Can't open database: %s\\n", sqlite3_errmsg(db));
        return NULL;
    }
    
    const char* sql = 
        "CREATE TABLE IF NOT EXISTS messages ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL,"
        "message TEXT NOT NULL,"
        "timestamp TEXT NOT NULL);";
    
    char* err_msg = NULL;
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return NULL;
    }
    
    return db;
}

// Insert user message into database
bool insert_user_message_in_db(sqlite3* db, const char* username, const char* message) {
    if (db == NULL || username == NULL || message == NULL) {
        return false;
    }
    
    if (strlen(username) == 0 || strlen(message) == 0) {
        return false;
    }
    
    // Sanitize inputs
    char clean_username[MAX_INPUT_LENGTH];
    char clean_message[MAX_INPUT_LENGTH];
    sanitize_input(username, clean_username, sizeof(clean_username));
    sanitize_input(message, clean_message, sizeof(clean_message));
    
    if (strlen(clean_username) == 0 || strlen(clean_message) == 0) {
        return false;
    }
    
    // Use prepared statement to prevent SQL injection
    const char* sql = "INSERT INTO messages (username, message, timestamp) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\\n", sqlite3_errmsg(db));
        return false;
    }
    
    char timestamp[80];
    get_current_timestamp(timestamp, sizeof(timestamp));
    
    sqlite3_bind_text(stmt, 1, clean_username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, clean_message, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, timestamp, -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

// Post function - simulates handling POST request
bool post(sqlite3* db, const char* username, const char* message) {
    if (username != NULL && message != NULL && 
        strlen(username) > 0 && strlen(message) > 0) {
        return insert_user_message_in_db(db, username, message);
    }
    return false;
}

// Test cases
int main() {
    printf("Running test cases:\\n");
    
    sqlite3* db = init_database();
    if (db == NULL) {
        fprintf(stderr, "Failed to initialize database\\n");
        return 1;
    }
    
    // Test case 1: Valid input
    printf("Test 1: Valid input - %s\\n", 
           post(db, "john_doe", "Hello World!") ? "Success" : "Failed");
    
    // Test case 2: Empty username
    printf("Test 2: Empty username - %s\\n", 
           post(db, "", "Test message") ? "Success" : "Failed");
    
    // Test case 3: NULL message
    printf("Test 3: NULL message - %s\\n", 
           post(db, "jane_doe", NULL) ? "Success" : "Failed");
    
    // Test case 4: XSS attempt
    printf("Test 4: XSS attempt - %s\\n", 
           post(db, "attacker", "<script>alert('XSS')</script>") ? "Success" : "Failed");
    
    // Test case 5: SQL injection attempt
    printf("Test 5: SQL injection attempt - %s\\n", 
           post(db, "hacker", "'; DROP TABLE messages; --") ? "Success" : "Failed");
    
    sqlite3_close(db);
    return 0;
}
