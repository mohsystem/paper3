
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <time.h>
#include <ctype.h>

#define MAX_USERNAME_LENGTH 50
#define MAX_MESSAGE_LENGTH 500
#define MAX_INPUT_LENGTH 1024

typedef struct {
    char username[MAX_INPUT_LENGTH];
    char message[MAX_INPUT_LENGTH];
} Request;

void sanitize_input(char* input) {
    if (input == NULL) return;
    
    char* src = input;
    char* dst = input;
    
    while (*src) {
        if (*src != '<' && *src != '>' && *src != '"' && *src != '\\'' && *src != '&') {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\\0';
    
    // Trim whitespace
    while (isspace((unsigned char)*input)) input++;
    if (*input == 0) return;
    char* end = input + strlen(input) - 1;
    while (end > input && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\\0';
}

int validate_input(const char* username, const char* message) {
    return username != NULL && strlen(username) > 0 && strlen(username) <= MAX_USERNAME_LENGTH &&
           message != NULL && strlen(message) > 0 && strlen(message) <= MAX_MESSAGE_LENGTH;
}

int insert_user_message_in_db(const char* username, const char* message) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    int success = 0;
    
    if (sqlite3_open("messages.db", &db) != SQLITE_OK) {
        fprintf(stderr, "Database error: %s\\n", sqlite3_errmsg(db));
        return 0;
    }
    
    const char* sql = "INSERT INTO messages (username, message, created_at) VALUES (?, ?, ?)";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        char sanitized_username[MAX_INPUT_LENGTH];
        char sanitized_message[MAX_INPUT_LENGTH];
        
        strncpy(sanitized_username, username, MAX_INPUT_LENGTH - 1);
        strncpy(sanitized_message, message, MAX_INPUT_LENGTH - 1);
        sanitized_username[MAX_INPUT_LENGTH - 1] = '\\0';
        sanitized_message[MAX_INPUT_LENGTH - 1] = '\\0';
        
        sanitize_input(sanitized_username);
        sanitize_input(sanitized_message);
        
        sqlite3_bind_text(stmt, 1, sanitized_username, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, sanitized_message, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 3, (sqlite3_int64)time(NULL));
        
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            success = 1;
        }
        
        sqlite3_finalize(stmt);
    }
    
    sqlite3_close(db);
    return success;
}

const char* post(Request* request) {
    if (request == NULL) {
        return "error.html?msg=Invalid request";
    }
    
    if (!validate_input(request->username, request->message)) {
        return "error.html?msg=Invalid input";
    }
    
    if (insert_user_message_in_db(request->username, request->message)) {
        return "redirect:/index.html";
    } else {
        return "error.html?msg=Failed to save message";
    }
}

const char* get_main_page_form() {
    return "<!DOCTYPE html>\\n"
           "<html>\\n<head>\\n"
           "<meta charset=\\"UTF-8\\">\\n"
           "<title>Message Board</title>\\n"
           "</head>\\n<body>\\n"
           "<h1>Submit a Message</h1>\\n"
           "<form method=\\"POST\\" action=\\"/post\\">\\n"
           "  <label>Username:</label><br>\\n"
           "  <input type=\\"text\\" name=\\"username\\" maxlength=\\"50\\" required><br><br>\\n"
           "  <label>Message:</label><br>\\n"
           "  <textarea name=\\"message\\" maxlength=\\"500\\" required></textarea><br><br>\\n"
           "  <input type=\\"submit\\" value=\\"Submit\\">\\n"
           "</form>\\n"
           "</body>\\n</html>";
}

int main() {
    printf("Test Case 1: Valid input\\n");
    Request req1 = {.username = "John", .message = "Hello World"};
    printf("%s\\n", post(&req1));
    
    printf("\\nTest Case 2: Empty username\\n");
    Request req2 = {.username = "", .message = "Test message"};
    printf("%s\\n", post(&req2));
    
    printf("\\nTest Case 3: Empty message\\n");
    Request req3 = {.username = "Alice", .message = ""};
    printf("%s\\n", post(&req3));
    
    printf("\\nTest Case 4: XSS attempt\\n");
    Request req4 = {.username = "<script>alert('xss')</script>", .message = "Normal message"};
    printf("%s\\n", post(&req4));
    
    printf("\\nTest Case 5: Long input\\n");
    Request req5;
    memset(req5.username, 'A', 60);
    req5.username[60] = '\\0';
    strcpy(req5.message, "Test");
    printf("%s\\n", post(&req5));
    
    return 0;
}
