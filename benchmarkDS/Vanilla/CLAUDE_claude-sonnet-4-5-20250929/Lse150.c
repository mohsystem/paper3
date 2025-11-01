
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sqlite3.h>

#define MAX_USERNAME_LEN 100
#define MAX_MESSAGE_LEN 1000

typedef struct {
    char username[MAX_USERNAME_LEN];
    char message[MAX_MESSAGE_LEN];
} Request;

sqlite3* db = NULL;

void init_database() {
    int rc = sqlite3_open("messages.db", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\\n", sqlite3_errmsg(db));
        return;
    }
    
    const char* sql = "CREATE TABLE IF NOT EXISTS messages ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                     "username TEXT NOT NULL,"
                     "message TEXT NOT NULL,"
                     "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);";
    
    char* errMsg;
    rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", errMsg);
        sqlite3_free(errMsg);
    }
}

void close_database() {
    if (db) {
        sqlite3_close(db);
    }
}

void insert_user_message_in_db(const char* username, const char* message) {
    const char* sql = "INSERT INTO messages (username, message) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, message, -1, SQLITE_TRANSIENT);
        
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            fprintf(stderr, "Error inserting data\\n");
        }
        sqlite3_finalize(stmt);
    }
}

const char* post(Request* request) {
    // Get username and message from request
    const char* username = request->username;
    const char* message = request->message;
    
    // Insert message into database
    if (username != NULL && message != NULL && 
        strlen(username) > 0 && strlen(message) > 0) {
        insert_user_message_in_db(username, message);
    }
    
    // Return redirect instruction
    return "redirect:/";
}

void display_main_page() {
    printf("<!DOCTYPE html>\\n");
    printf("<html><head><title>Message Board</title></head>\\n");
    printf("<body>\\n");
    printf("<h1>Message Board</h1>\\n");
    printf("<form method='post' action='/post'>\\n");
    printf("<label>Username: <input type='text' name='username' required></label><br><br>\\n");
    printf("<label>Message: <textarea name='message' required></textarea></label><br><br>\\n");
    printf("<input type='submit' value='Submit Message'>\\n");
    printf("</form>\\n");
    printf("</body></html>\\n");
}

void get_messages() {
    const char* sql = "SELECT username, message, timestamp FROM messages ORDER BY timestamp DESC LIMIT 10;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        printf("\\nMessages in database:\\n");
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char* username = (const char*)sqlite3_column_text(stmt, 0);
            const char* message = (const char*)sqlite3_column_text(stmt, 1);
            const char* timestamp = (const char*)sqlite3_column_text(stmt, 2);
            printf("%s: %s (%s)\\n", username, message, timestamp);
        }
        sqlite3_finalize(stmt);
    }
}

int main() {
    init_database();
    
    printf("Running test cases...\\n");
    
    // Test case 1
    Request request1;
    strcpy(request1.username, "John");
    strcpy(request1.message, "Hello World");
    const char* result1 = post(&request1);
    printf("Test 1: %s\\n", result1);
    
    // Test case 2
    Request request2;
    strcpy(request2.username, "Alice");
    strcpy(request2.message, "Test message");
    const char* result2 = post(&request2);
    printf("Test 2: %s\\n", result2);
    
    // Test case 3
    Request request3;
    strcpy(request3.username, "Bob");
    strcpy(request3.message, "Another test");
    const char* result3 = post(&request3);
    printf("Test 3: %s\\n", result3);
    
    // Test case 4
    Request request4;
    strcpy(request4.username, "Charlie");
    strcpy(request4.message, "Fourth message");
    const char* result4 = post(&request4);
    printf("Test 4: %s\\n", result4);
    
    // Test case 5
    Request request5;
    strcpy(request5.username, "David");
    strcpy(request5.message, "Last test message");
    const char* result5 = post(&request5);
    printf("Test 5: %s\\n", result5);
    
    // Display all messages
    get_messages();
    
    close_database();
    return 0;
}
