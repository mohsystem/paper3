
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

typedef struct {
    char username[256];
    char message[1024];
    char timestamp[64];
} Message;

sqlite3* db = NULL;

void initialize_database() {
    int rc = sqlite3_open("messages.db", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\\n", sqlite3_errmsg(db));
        return;
    }

    const char* sql = "CREATE TABLE IF NOT EXISTS messages ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                     "username TEXT NOT NULL, "
                     "message TEXT NOT NULL, "
                     "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);";
    
    char* err_msg = NULL;
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", err_msg);
        sqlite3_free(err_msg);
    }
}

void insert_user_message_in_db(const char* username, const char* message) {
    const char* sql = "INSERT INTO messages (username, message) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, message, -1, SQLITE_STATIC);
        
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            fprintf(stderr, "Execution failed: %s\\n", sqlite3_errmsg(db));
        }
        sqlite3_finalize(stmt);
    }
}

void get_messages(Message** messages, int* count) {
    const char* sql = "SELECT username, message, timestamp FROM messages ORDER BY timestamp DESC;";
    sqlite3_stmt* stmt;
    *count = 0;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            (*count)++;
        }
        sqlite3_reset(stmt);
        
        *messages = (Message*)malloc(sizeof(Message) * (*count));
        int i = 0;
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            strncpy((*messages)[i].username, (const char*)sqlite3_column_text(stmt, 0), 255);
            strncpy((*messages)[i].message, (const char*)sqlite3_column_text(stmt, 1), 1023);
            strncpy((*messages)[i].timestamp, (const char*)sqlite3_column_text(stmt, 2), 63);
            i++;
        }
        sqlite3_finalize(stmt);
    }
}

void display_messages() {
    Message* messages = NULL;
    int count = 0;
    
    get_messages(&messages, &count);
    printf("=== Message Board ===\\n");
    for (int i = 0; i < count; i++) {
        printf("%s: %s (%s)\\n", messages[i].username, messages[i].message, messages[i].timestamp);
    }
    
    free(messages);
}

void handle_post(const char* username, const char* message) {
    if (username && message && strlen(username) > 0 && strlen(message) > 0) {
        insert_user_message_in_db(username, message);
    }
}

int main() {
    initialize_database();
    
    printf("Test Case 1: Insert message from user1\\n");
    handle_post("user1", "Hello World!");
    
    printf("Test Case 2: Insert message from user2\\n");
    handle_post("user2", "This is a test message");
    
    printf("Test Case 3: Insert message from user3\\n");
    handle_post("user3", "C example");
    
    printf("Test Case 4: Insert message from user4\\n");
    handle_post("user4", "Another test message");
    
    printf("Test Case 5: Display all messages\\n");
    display_messages();
    
    if (db) {
        sqlite3_close(db);
    }
    
    return 0;
}
