
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <ctype.h>

#define DB_NAME "messages.db"
#define MAX_INPUT_LENGTH 1024

// Initialize database
int initialize_database() {
    sqlite3* db;
    char* err_msg = NULL;
    int rc = sqlite3_open(DB_NAME, &db);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }
    
    const char* sql = "CREATE TABLE IF NOT EXISTS messages ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                     "username TEXT NOT NULL,"
                     "message TEXT NOT NULL,"
                     "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);";
    
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 0;
    }
    
    sqlite3_close(db);
    return 1;
}

// Sanitize input
void sanitize_input(const char* input, char* output, size_t output_size) {
    if (input == NULL || output == NULL || output_size == 0) {
        return;
    }
    
    size_t j = 0;
    for (size_t i = 0; input[i] != '\\0' && j < output_size - 1; i++) {
        char c = input[i];
        // Skip dangerous characters
        if (c != '<' && c != '>' && c != '"' && c != '\\'' && c != '&') {
            output[j++] = c;
        }
    }
    output[j] = '\\0';
    
    // Trim whitespace
    while (j > 0 && isspace((unsigned char)output[j - 1])) {
        output[--j] = '\\0';
    }
}

// Insert user message in database
int insert_user_message_in_db(const char* username, const char* message) {
    if (username == NULL || message == NULL || strlen(username) == 0 || strlen(message) == 0) {
        return 0;
    }
    
    char sanitized_username[MAX_INPUT_LENGTH];
    char sanitized_message[MAX_INPUT_LENGTH];
    
    sanitize_input(username, sanitized_username, MAX_INPUT_LENGTH);
    sanitize_input(message, sanitized_message, MAX_INPUT_LENGTH);
    
    if (strlen(sanitized_username) == 0 || strlen(sanitized_message) == 0) {
        return 0;
    }
    
    sqlite3* db;
    int rc = sqlite3_open(DB_NAME, &db);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }
    
    const char* sql = "INSERT INTO messages (username, message) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }
    
    sqlite3_bind_text(stmt, 1, sanitized_username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, sanitized_message, -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return rc == SQLITE_DONE;
}

// Get all messages
void get_all_messages() {
    sqlite3* db;
    int rc = sqlite3_open(DB_NAME, &db);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    
    const char* sql = "SELECT username, message, timestamp FROM messages ORDER BY timestamp DESC;";
    sqlite3_stmt* stmt;
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    
    printf("<html><body><h1>Messages</h1>\\n");
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* username = sqlite3_column_text(stmt, 0);
        const unsigned char* message = sqlite3_column_text(stmt, 1);
        const unsigned char* timestamp = sqlite3_column_text(stmt, 2);
        
        printf("<div><strong>%s</strong>: %s <em>(%s)</em></div><br>\\n", 
               username, message, timestamp);
    }
    
    printf("</body></html>\\n");
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

// Handle POST route
const char* handle_post_route(const char* username, const char* message) {
    if (username == NULL || message == NULL || strlen(username) == 0 || strlen(message) == 0) {
        return "redirect:/error";
    }
    
    insert_user_message_in_db(username, message);
    return "redirect:/";
}

int main() {
    // Initialize database
    initialize_database();
    
    // Test case 1: Valid message
    printf("Test 1: Valid message\\n");
    printf("%s\\n", handle_post_route("Alice", "Hello World!"));
    get_all_messages();
    printf("\\n");
    
    // Test case 2: Another valid message
    printf("Test 2: Another valid message\\n");
    printf("%s\\n", handle_post_route("Bob", "Testing the system"));
    printf("\\n");
    
    // Test case 3: Empty username
    printf("Test 3: Empty username\\n");
    printf("%s\\n", handle_post_route("", "This should fail"));
    printf("\\n");
    
    // Test case 4: NULL message
    printf("Test 4: NULL message\\n");
    printf("%s\\n", handle_post_route("Charlie", NULL));
    printf("\\n");
    
    // Test case 5: XSS attempt
    printf("Test 5: XSS attempt\\n");
    printf("%s\\n", handle_post_route("Dave", "<script>alert('XSS')</script>Safe message"));
    get_all_messages();
    
    return 0;
}
