
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#define MAX_USERNAME_LEN 50
#define MAX_MESSAGE_LEN 500

typedef struct {
    char username[MAX_USERNAME_LEN + 1];
    char message[MAX_MESSAGE_LEN + 1];
    char created_at[30];
} Message;

sqlite3* db = NULL;

void init_db() {
    sqlite3_open("messages.db", &db);
    const char* sql = "CREATE TABLE IF NOT EXISTS messages ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                     "username TEXT NOT NULL, "
                     "message TEXT NOT NULL, "
                     "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)";
    sqlite3_exec(db, sql, NULL, NULL, NULL);
}

void escape_html(const char* input, char* output, size_t output_size) {
    size_t j = 0;
    for (size_t i = 0; input[i] != '\\0' && j < output_size - 6; i++) {
        switch (input[i]) {
            case '&': strcpy(&output[j], "&amp;"); j += 5; break;
            case '<': strcpy(&output[j], "&lt;"); j += 4; break;
            case '>': strcpy(&output[j], "&gt;"); j += 4; break;
            case '"': strcpy(&output[j], "&quot;"); j += 6; break;
            case '\\'': strcpy(&output[j], "&#x27;"); j += 6; break;
            default: output[j++] = input[i];
        }
    }
    output[j] = '\\0';
}

int insert_user_message_in_db(const char* username, const char* message) {
    if (!username || !message || strlen(username) == 0 || strlen(message) == 0) {
        return 0;
    }
    
    if (strlen(username) > MAX_USERNAME_LEN || strlen(message) > MAX_MESSAGE_LEN) {
        return 0;
    }
    
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO messages (username, message) VALUES (?, ?)";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, message, -1, SQLITE_TRANSIENT);
    
    int success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

void get_all_messages(Message** messages, int* count) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT username, message, created_at FROM messages ORDER BY created_at DESC";
    
    *count = 0;
    *messages = NULL;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        int capacity = 10;
        *messages = malloc(capacity * sizeof(Message));
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            if (*count >= capacity) {
                capacity *= 2;
                *messages = realloc(*messages, capacity * sizeof(Message));
            }
            
            const char* username = (const char*)sqlite3_column_text(stmt, 0);
            const char* message = (const char*)sqlite3_column_text(stmt, 1);
            const char* created_at = (const char*)sqlite3_column_text(stmt, 2);
            
            escape_html(username, (*messages)[*count].username, MAX_USERNAME_LEN + 1);
            escape_html(message, (*messages)[*count].message, MAX_MESSAGE_LEN + 1);
            strncpy((*messages)[*count].created_at, created_at, 29);
            (*messages)[*count].created_at[29] = '\\0';
            
            (*count)++;
        }
        sqlite3_finalize(stmt);
    }
}

void cleanup() {
    if (db) {
        sqlite3_close(db);
    }
}

int main() {
    init_db();
    
    // Test cases
    printf("\\nTest Cases:\\n");
    printf("1. Valid insertion: %d\\n", insert_user_message_in_db("Alice", "Hello World"));
    printf("2. Valid insertion: %d\\n", insert_user_message_in_db("Bob", "Test message"));
    printf("3. Empty username: %d\\n", insert_user_message_in_db("", "Message"));
    printf("4. Null values: %d\\n", insert_user_message_in_db(NULL, NULL));
    printf("5. XSS attempt: %d\\n", insert_user_message_in_db("Hacker", "<script>alert('xss')</script>"));
    
    // Display messages
    Message* messages;
    int count;
    get_all_messages(&messages, &count);
    
    printf("\\nStored Messages:\\n");
    for (int i = 0; i < count; i++) {
        printf("%s: %s (%s)\\n", messages[i].username, messages[i].message, messages[i].created_at);
    }
    
    free(messages);
    cleanup();
    return 0;
}
