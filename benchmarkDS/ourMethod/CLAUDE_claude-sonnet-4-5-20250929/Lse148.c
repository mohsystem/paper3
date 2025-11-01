
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <time.h>
#include <ctype.h>

/* Rule#2: Define maximum input lengths */
#define MAX_USERNAME_LENGTH 50
#define MAX_MESSAGE_LENGTH 500
#define MAX_CSRF_TOKEN_LENGTH 128

/* Rule#2: Validate input contains no dangerous control characters */
int is_valid_input(const char* input) {
    if (!input) return 0;
    
    size_t len = strlen(input);
    for (size_t i = 0; i < len; i++) {
        /* Allow printable chars, newline, and tab only */
        if (iscntrl((unsigned char)input[i]) && input[i] != '\\n' && input[i] != '\\t') {
            return 0;
        }
    }
    return 1;
}

/* Rule#2: HTML escape to prevent XSS attacks */
char* escape_html(const char* input) {
    if (!input) return NULL;
    
    size_t len = strlen(input);
    /* Rule#35: Check for overflow before allocation */
    if (len > SIZE_MAX / 6) return NULL;
    
    /* Rule#34: Allocate sufficient space for worst case */
    char* output = calloc(len * 6 + 1, sizeof(char));
    if (!output) return NULL; /* Rule#36: Check allocation */
    
    size_t pos = 0;
    for (size_t i = 0; i < len; i++) {
        switch (input[i]) {
            case '&':
                memcpy(output + pos, "&amp;", 5);
                pos += 5;
                break;
            case '<':
                memcpy(output + pos, "&lt;", 4);
                pos += 4;
                break;
            case '>':
                memcpy(output + pos, "&gt;", 4);
                pos += 4;
                break;
            case '"':
                memcpy(output + pos, "&quot;", 6);
                pos += 6;
                break;
            case '\\'':
                memcpy(output + pos, "&#x27;", 6);
                pos += 6;
                break;
            default:
                output[pos++] = input[i];
        }
    }
    output[pos] = '\\0'; /* Rule#36: Ensure null termination */
    return output;
}

/* Rule#32: Use parameterized queries to prevent SQL injection */
int insert_user_message_in_db(const char* username, const char* message) {
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    int result = -1;
    
    /* Rule#3: Read database path from environment */
    const char* db_path = getenv("DB_PATH");
    if (!db_path) {
        db_path = "messages.db";
    }
    
    /* Open database */
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        goto cleanup;
    }
    
    /* Rule#32: Parameterized query prevents SQL injection */
    const char* sql = "INSERT INTO messages (username, message, created_at) VALUES (?, ?, datetime('now'))";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        goto cleanup;
    }
    
    /* Rule#32: Bind parameters safely */
    if (sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC) != SQLITE_OK) {
        goto cleanup;
    }
    if (sqlite3_bind_text(stmt, 2, message, -1, SQLITE_STATIC) != SQLITE_OK) {
        goto cleanup;
    }
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        goto cleanup;
    }
    
    result = 0;
    
cleanup:
    /* Rule#7: Cleanup resources in deterministic order */
    if (stmt) sqlite3_finalize(stmt);
    if (db) sqlite3_close(db);
    return result;
}

/* Rule#5: Generate CSRF token using secure random */
void generate_csrf_token(char* token, size_t token_size) {
    /* Rule#36: Initialize buffer */
    memset(token, 0, token_size);
    
    FILE* urandom = fopen("/dev/urandom", "rb");
    if (!urandom) {
        /* Fallback to time-based token (less secure) */
        snprintf(token, token_size, "%ld", (long)time(NULL));
        return;
    }
    
    unsigned char random_bytes[32];
    if (fread(random_bytes, 1, sizeof(random_bytes), urandom) != sizeof(random_bytes)) {
        fclose(urandom);
        snprintf(token, token_size, "%ld", (long)time(NULL));
        return;
    }
    fclose(urandom);
    
    /* Convert to hex string */
    for (size_t i = 0; i < sizeof(random_bytes) && i * 2 < token_size - 1; i++) {
        snprintf(token + i * 2, 3, "%02x", random_bytes[i]);
    }
}

/* Handle POST request with security validations */
int post(const char* username, const char* message, 
         const char* csrf_token, const char* session_token) {
    /* Rule#2: Validate CSRF token */
    if (!csrf_token || !session_token || strcmp(csrf_token, session_token) != 0) {
        return -1; /* Rule#11: Generic error */
    }
    
    /* Rule#2: Validate inputs exist */
    if (!username || !message) {
        return -1;
    }
    
    /* Rule#2: Validate length constraints */
    size_t username_len = strlen(username);
    size_t message_len = strlen(message);
    
    if (username_len == 0 || username_len > MAX_USERNAME_LENGTH) {
        return -1;
    }
    if (message_len == 0 || message_len > MAX_MESSAGE_LENGTH) {
        return -1;
    }
    
    /* Rule#2: Validate content */
    if (!is_valid_input(username) || !is_valid_input(message)) {
        return -1;
    }
    
    /* Insert into database */
    if (insert_user_message_in_db(username, message) != 0) {
        /* Rule#11: Do not leak internal details */
        fprintf(stderr, "Error processing post request\\n");
        return -1;
    }
    
    return 0; /* Success - redirect to main */
}

/* Retrieve and display messages with XSS protection */
void display_messages(void) {
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    
    const char* db_path = getenv("DB_PATH");
    if (!db_path) {
        db_path = "messages.db";
    }
    
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        goto cleanup;
    }
    
    /* Rule#32: Safe query */
    const char* sql = "SELECT username, message, created_at FROM messages ORDER BY created_at DESC LIMIT 100";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        goto cleanup;
    }
    
    printf("\\nMessages:\\n");
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* username = sqlite3_column_text(stmt, 0);
        const unsigned char* message = sqlite3_column_text(stmt, 1);
        
        /* Rule#2: Escape output to prevent XSS */
        char* escaped_username = escape_html((const char*)username);
        char* escaped_message = escape_html((const char*)message);
        
        if (escaped_username && escaped_message) {
            printf("%s: %s\\n", escaped_username, escaped_message);
        }
        
        /* Rule#36: Free allocated memory */
        free(escaped_username);
        free(escaped_message);
    }
    
cleanup:
    /* Rule#7: Cleanup resources */
    if (stmt) sqlite3_finalize(stmt);
    if (db) sqlite3_close(db);
}

int main(void) {
    /* Setup: Create in-memory database for testing */
    sqlite3* db = NULL;
    sqlite3_open(":memory:", &db);
    sqlite3_exec(db, "CREATE TABLE messages (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT, message TEXT, created_at TEXT)", NULL, NULL, NULL);
    sqlite3_close(db);
    setenv("DB_PATH", ":memory:", 1);
    
    char token1[MAX_CSRF_TOKEN_LENGTH];
    char token2[MAX_CSRF_TOKEN_LENGTH];
    char token4[MAX_CSRF_TOKEN_LENGTH];
    char token5[MAX_CSRF_TOKEN_LENGTH];
    
    /* Test case 1: Valid post */
    generate_csrf_token(token1, sizeof(token1));
    printf("Test 1: %d\\n", post("alice", "Hello world!", token1, token1));
    
    /* Test case 2: Another valid post */
    generate_csrf_token(token2, sizeof(token2));
    printf("Test 2: %d\\n", post("bob", "Test message", token2, token2));
    
    /* Test case 3: Invalid CSRF token */
    printf("Test 3 (should error): %d\\n", post("eve", "Attack", "wrong", "correct"));
    
    /* Test case 4: Oversized input */
    generate_csrf_token(token4, sizeof(token4));
    char oversized[150];
    memset(oversized, 'x', sizeof(oversized) - 1);
    oversized[sizeof(oversized) - 1] = '\\0';
    printf("Test 4 (should error): %d\\n", post(oversized, "msg", token4, token4));
    
    /* Test case 5: XSS attempt */
    generate_csrf_token(token5, sizeof(token5));
    printf("Test 5: %d\\n", post("hacker", "<script>alert('xss')</script>", token5, token5));
    
    /* Display all messages */
    display_messages();
    
    return 0;
}
