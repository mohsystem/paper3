
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <time.h>

#define MAX_INPUT_LENGTH 500
#define MAX_MESSAGES 50

typedef struct {
    char username[MAX_INPUT_LENGTH];
    char message[MAX_INPUT_LENGTH];
    char timestamp[50];
} Message;

sqlite3 *db = NULL;

// Initialize database
int init_database() {
    int rc = sqlite3_open("messages.db", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\\n", sqlite3_errmsg(db));
        return 0;
    }
    
    const char *sql = "CREATE TABLE IF NOT EXISTS messages ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                     "username TEXT NOT NULL,"
                     "message TEXT NOT NULL,"
                     "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);";
    
    char *err_msg = NULL;
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", err_msg);
        sqlite3_free(err_msg);
        return 0;
    }
    
    return 1;
}

// Sanitize input
void sanitize_input(char *output, const char *input, size_t max_len) {
    if (!input || !output) return;
    
    size_t j = 0;
    for (size_t i = 0; input[i] && j < max_len - 1; i++) {
        char c = input[i];
        // Remove dangerous characters
        if (c != '<' && c != '>' && c != '"' && c != '\\'') {
            output[j++] = c;
        }
    }
    output[j] = '\\0';
}

// Insert user message in database
void insert_user_message_in_db(const char *username, const char *message) {
    if (!username || !message || strlen(username) == 0 || strlen(message) == 0) {
        return;
    }
    
    const char *sql = "INSERT INTO messages (username, message) VALUES (?, ?);";
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, message, -1, SQLITE_TRANSIENT);
        
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            fprintf(stderr, "Execution failed: %s\\n", sqlite3_errmsg(db));
        }
        
        sqlite3_finalize(stmt);
    }
}

// Get messages from database
int get_messages(Message *messages, int max_count) {
    const char *sql = "SELECT username, message, timestamp FROM messages "
                     "ORDER BY timestamp DESC LIMIT ?;";
    sqlite3_stmt *stmt;
    int count = 0;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, max_count);
        
        while (sqlite3_step(stmt) == SQLITE_ROW && count < max_count) {
            const char *username = (const char *)sqlite3_column_text(stmt, 0);
            const char *message = (const char *)sqlite3_column_text(stmt, 1);
            const char *timestamp = (const char *)sqlite3_column_text(stmt, 2);
            
            strncpy(messages[count].username, username, MAX_INPUT_LENGTH - 1);
            strncpy(messages[count].message, message, MAX_INPUT_LENGTH - 1);
            strncpy(messages[count].timestamp, timestamp, 49);
            
            messages[count].username[MAX_INPUT_LENGTH - 1] = '\\0';
            messages[count].message[MAX_INPUT_LENGTH - 1] = '\\0';
            messages[count].timestamp[49] = '\\0';
            
            count++;
        }
        sqlite3_finalize(stmt);
    }
    
    return count;
}

// POST function
void post(const char *username, const char *message) {
    char sanitized_username[MAX_INPUT_LENGTH];
    char sanitized_message[MAX_INPUT_LENGTH];
    
    // Sanitize input
    sanitize_input(sanitized_username, username, MAX_INPUT_LENGTH);
    sanitize_input(sanitized_message, message, MAX_INPUT_LENGTH);
    
    // Validate input
    if (strlen(sanitized_username) > 0 && strlen(sanitized_message) > 0) {
        // Insert message in database
        insert_user_message_in_db(sanitized_username, sanitized_message);
    }
    
    printf("Message posted successfully. Redirecting to main page...\\n");
}

// Display messages
void display_messages() {
    Message messages[MAX_MESSAGES];
    int count = get_messages(messages, MAX_MESSAGES);
    
    printf("\\n========== MESSAGE BOARD ==========\\n");
    printf("Total Messages: %d\\n", count);
    printf("===================================\\n");
    
    for (int i = 0; i < count; i++) {
        printf("\\n[%s] (%s)\\n", messages[i].username, messages[i].timestamp);
        printf("%s\\n", messages[i].message);
        printf("-----------------------------------\\n");
    }
}

// Close database
void close_database() {
    if (db) {
        sqlite3_close(db);
    }
}

int main() {
    if (!init_database()) {
        return 1;
    }
    
    printf("Running test cases...\\n\\n");
    
    // Test case 1: Normal message
    printf("Test 1: Posting normal message\\n");
    post("Alice", "Hello, this is my first message!");
    
    // Test case 2: Another user
    printf("\\nTest 2: Posting from another user\\n");
    post("Bob", "Great to be here!");
    
    // Test case 3: Message with special characters
    printf("\\nTest 3: Testing special characters handling\\n");
    post("Charlie", "Testing <script>alert('xss')</script> security");
    
    // Test case 4: Long message
    printf("\\nTest 4: Testing long message\\n");
    post("David", "This is a very long message to test the length limit functionality.");
    
    // Test case 5: Empty input
    printf("\\nTest 5: Testing empty input\\n");
    post("", "");
    
    // Display all messages
    printf("\\n\\nDisplaying all messages:\\n");
    display_messages();
    
    close_database();
    
    return 0;
}
