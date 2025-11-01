
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <time.h>
#include <stdbool.h>

#define MAX_USERNAME_LENGTH 50
#define MAX_MESSAGE_LENGTH 500
#define DB_NAME "messages.db"

typedef struct {
    char username[MAX_USERNAME_LENGTH + 1];
    char message[MAX_MESSAGE_LENGTH + 1];
} Request;

static void trim(char* str) {
    if (!str) return;
    
    char* start = str;
    while (*start == ' ' || *start == '\\t' || *start == '\\n' || *start == '\\r') {
        start++;
    }
    
    if (*start == 0) {
        str[0] = 0;
        return;
    }
    
    char* end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\\t' || *end == '\\n' || *end == '\\r')) {
        end--;
    }
    
    size_t len = end - start + 1;
    memmove(str, start, len);
    str[len] = 0;
}

static bool validate_and_sanitize_input(char* output, const char* input, int max_length) {
    if (!input || !output) return false;
    
    strncpy(output, input, max_length);
    output[max_length] = '\\0';
    
    trim(output);
    
    if (strlen(output) == 0) {
        return false;
    }
    
    // Remove dangerous characters
    char* src = output;
    char* dst = output;
    while (*src) {
        if (*src != '<' && *src != '>' && *src != '"' && *src != '\\'' && *src != '&') {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\\0';
    
    return strlen(output) > 0;
}

static void html_encode(char* output, const char* input, size_t max_len) {
    if (!input || !output) return;
    
    size_t out_idx = 0;
    for (size_t i = 0; input[i] && out_idx < max_len - 10; i++) {
        switch (input[i]) {
            case '&':
                if (out_idx + 5 < max_len) {
                    strcpy(output + out_idx, "&amp;");
                    out_idx += 5;
                }
                break;
            case '<':
                if (out_idx + 4 < max_len) {
                    strcpy(output + out_idx, "&lt;");
                    out_idx += 4;
                }
                break;
            case '>':
                if (out_idx + 4 < max_len) {
                    strcpy(output + out_idx, "&gt;");
                    out_idx += 4;
                }
                break;
            case '"':
                if (out_idx + 6 < max_len) {
                    strcpy(output + out_idx, "&quot;");
                    out_idx += 6;
                }
                break;
            case '\\'':
                if (out_idx + 6 < max_len) {
                    strcpy(output + out_idx, "&#x27;");
                    out_idx += 6;
                }
                break;
            default:
                output[out_idx++] = input[i];
                break;
        }
    }
    output[out_idx] = '\\0';
}

static void get_current_timestamp(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

static bool insert_user_message_in_db(const char* username, const char* message) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    char timestamp[80];
    
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Database error: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }
    
    const char* sql = "INSERT INTO messages (username, message, timestamp) VALUES (?, ?, ?)";
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Prepare error: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }
    
    get_current_timestamp(timestamp, sizeof(timestamp));
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, message, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, timestamp, -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return rc == SQLITE_DONE;
}

bool post(Request* request) {
    if (!request) return false;
    
    char username[MAX_USERNAME_LENGTH + 1];
    char message[MAX_MESSAGE_LENGTH + 1];
    
    if (!validate_and_sanitize_input(username, request->username, MAX_USERNAME_LENGTH)) {
        fprintf(stderr, "Invalid username\\n");
        return false;
    }
    
    if (!validate_and_sanitize_input(message, request->message, MAX_MESSAGE_LENGTH)) {
        fprintf(stderr, "Invalid message\\n");
        return false;
    }
    
    if (insert_user_message_in_db(username, message)) {
        printf("Redirecting to: /main\\n");
        return true;
    }
    
    return false;
}

void display_messages() {
    sqlite3* db;
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Database error: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    
    const char* sql = "SELECT username, message, timestamp FROM messages ORDER BY timestamp DESC LIMIT 100";
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Prepare error: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        char encoded_username[MAX_USERNAME_LENGTH * 6 + 1];
        char encoded_message[MAX_MESSAGE_LENGTH * 6 + 1];
        
        html_encode(encoded_username, (const char*)sqlite3_column_text(stmt, 0), sizeof(encoded_username));
        html_encode(encoded_message, (const char*)sqlite3_column_text(stmt, 1), sizeof(encoded_message));
        
        printf("[%s] %s: %s\\n", 
               sqlite3_column_text(stmt, 2),
               encoded_username,
               encoded_message);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void init_database() {
    sqlite3* db;
    char* err_msg = NULL;
    
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Database init error: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    
    const char* sql = "CREATE TABLE IF NOT EXISTS messages ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                     "username TEXT NOT NULL,"
                     "message TEXT NOT NULL,"
                     "timestamp DATETIME NOT NULL)";
    
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", err_msg);
        sqlite3_free(err_msg);
    }
    
    sqlite3_close(db);
}

int main() {
    init_database();
    
    printf("=== Test Cases ===\\n\\n");
    
    // Test 1: Valid message
    Request req1 = {0};
    strncpy(req1.username, "Alice", sizeof(req1.username) - 1);
    strncpy(req1.message, "Hello World!", sizeof(req1.message) - 1);
    printf("Test 1 - Valid message: %s\\n", post(&req1) ? "true" : "false");
    
    // Test 2: Another valid message
    Request req2 = {0};
    strncpy(req2.username, "Bob", sizeof(req2.username) - 1);
    strncpy(req2.message, "This is a test message", sizeof(req2.message) - 1);
    printf("Test 2 - Valid message: %s\\n", post(&req2) ? "true" : "false");
    
    // Test 3: Empty username (should fail)
    Request req3 = {0};
    strncpy(req3.username, "", sizeof(req3.username) - 1);
    strncpy(req3.message, "Message without user", sizeof(req3.message) - 1);
    printf("Test 3 - Empty username: %s\\n", post(&req3) ? "true" : "false");
    
    // Test 4: XSS attempt (should be sanitized)
    Request req4 = {0};
    strncpy(req4.username, "Hacker<script>", sizeof(req4.username) - 1);
    strncpy(req4.message, "Alert<script>alert('XSS')</script>", sizeof(req4.message) - 1);
    printf("Test 4 - XSS attempt: %s\\n", post(&req4) ? "true" : "false");
    
    // Test 5: Very long message (should be truncated)
    Request req5 = {0};
    strncpy(req5.username, "Charlie", sizeof(req5.username) - 1);
    memset(req5.message, 'A', sizeof(req5.message) - 1);
    printf("Test 5 - Long message: %s\\n", post(&req5) ? "true" : "false");
    
    // Display all messages
    printf("\\n=== Displaying Messages ===\\n");
    display_messages();
    
    return 0;
}
