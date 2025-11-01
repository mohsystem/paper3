
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sqlite3.h>
#include <openssl/rand.h>

/* Rules#2: Maximum lengths to prevent resource exhaustion */
#define MAX_USERNAME_LENGTH 50
#define MAX_MESSAGE_LENGTH 500
#define CSRF_TOKEN_LENGTH 64

/* Rules#11: Generic error message */
static const char* GENERIC_ERROR = "Unable to process request";

/**
 * Rules#2: Validates username input
 * Checks length and allowed character set
 */
bool validate_username(const char* username) {
    if (username == NULL) {
        return false;
    }
    
    /* Rules#2: Check length using strnlen to prevent unbounded reads */
    size_t len = strnlen(username, MAX_USERNAME_LENGTH + 1);
    if (len == 0 || len > MAX_USERNAME_LENGTH) {
        return false;
    }
    
    /* Rules#2: Allow only alphanumeric, underscore, hyphen */
    for (size_t i = 0; i < len; i++) {
        char c = username[i];
        if (!isalnum((unsigned char)c) && c != '_' && c != '-') {
            return false;
        }
    }
    
    return true;
}

/**
 * Rules#2: Validates message input
 * Checks length
 */
bool validate_message(const char* message) {
    if (message == NULL) {
        return false;
    }
    
    /* Rules#2: Check length */
    size_t len = strnlen(message, MAX_MESSAGE_LENGTH + 1);
    if (len == 0 || len > MAX_MESSAGE_LENGTH) {
        return false;
    }
    
    return true;
}

/**
 * Rules#5: Generate CSRF token using OpenSSL CSPRNG
 * Rules#8: Check allocation and return value
 */
char* generate_csrf_token(void) {
    /* Rules#8: Check allocation */
    char* token = (char*)malloc(CSRF_TOKEN_LENGTH + 1);
    if (token == NULL) {
        return NULL;
    }
    
    unsigned char buffer[32];
    /* Rules#5: Use CSPRNG */
    if (RAND_bytes(buffer, sizeof(buffer)) != 1) {
        free(token);
        return NULL;
    }
    
    /* Convert to hex string with bounds check */
    for (size_t i = 0; i < sizeof(buffer); i++) {
        /* Rules#8: Explicit bounds check */
        if (i * 2 + 1 >= CSRF_TOKEN_LENGTH) {
            break;
        }
        snprintf(token + (i * 2), 3, "%02x", buffer[i]);
    }
    token[CSRF_TOKEN_LENGTH] = '\\0';
    
    return token;
}

/**
 * Rules#32: Parameterized query to prevent SQL injection
 * Rules#8: Proper error handling and resource cleanup
 */
int insert_user_message_in_db(sqlite3* db, const char* username, const char* message) {
    /* Rules#32: Use prepared statement with parameters */
    const char* sql = "INSERT INTO messages (username, message, created_at) VALUES (?, ?, datetime('now'))";
    sqlite3_stmt* stmt = NULL;
    int rc;
    
    /* Rules#8: Check preparation result */
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Database error\\n");
        return -1;
    }
    
    /* Rules#32: Bind parameters safely - SQLite handles escaping */
    rc = sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return -1;
    }
    
    rc = sqlite3_bind_text(stmt, 2, message, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return -1;
    }
    
    /* Rules#8: Check execution result */
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Database error\\n");
        return -1;
    }
    
    return 0;
}

/**
 * Rules#24: HTML escaping to prevent XSS
 * Rules#8: Bounds checking and allocation verification
 */
char* escape_html(const char* input) {
    if (input == NULL) {
        return NULL;
    }
    
    size_t input_len = strlen(input);
    /* Rules#8: Check for overflow before allocation */
    if (input_len > SIZE_MAX / 6 - 1) {
        return NULL;
    }
    
    /* Allocate worst case: every char becomes 6 chars */
    char* output = (char*)malloc(input_len * 6 + 1);
    if (output == NULL) {
        return NULL;
    }
    
    size_t out_pos = 0;
    for (size_t i = 0; i < input_len; i++) {
        /* Rules#8: Bounds check */
        if (out_pos >= input_len * 6) {
            break;
        }
        
        switch (input[i]) {
            case '&':
                memcpy(output + out_pos, "&amp;", 5);
                out_pos += 5;
                break;
            case '<':
                memcpy(output + out_pos, "&lt;", 4);
                out_pos += 4;
                break;
            case '>':
                memcpy(output + out_pos, "&gt;", 4);
                out_pos += 4;
                break;
            case '"':
                memcpy(output + out_pos, "&quot;", 6);
                out_pos += 6;
                break;
            case '\\'':
                memcpy(output + out_pos, "&#x27;", 6);
                out_pos += 6;
                break;
            default:
                output[out_pos++] = input[i];
                break;
        }
    }
    output[out_pos] = '\\0';
    return output;
}

/**
 * Rules#24: Constant-time CSRF token comparison
 */
bool validate_csrf_token(const char* session_token, const char* request_token) {
    if (session_token == NULL || request_token == NULL) {
        return false;
    }
    
    size_t session_len = strnlen(session_token, CSRF_TOKEN_LENGTH + 1);
    size_t request_len = strnlen(request_token, CSRF_TOKEN_LENGTH + 1);
    
    if (session_len != request_len || session_len != CSRF_TOKEN_LENGTH) {
        return false;
    }
    
    /* Rules#24: Constant-time comparison */
    volatile int result = 0;
    for (size_t i = 0; i < session_len; i++) {
        result |= session_token[i] ^ request_token[i];
    }
    
    return result == 0;
}

/**
 * Rules#9: Secure memory cleanup
 */
void secure_zero(void* ptr, size_t len) {
    if (ptr == NULL) {
        return;
    }
    /* Rules#9: Use volatile to prevent compiler optimization */
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    for (size_t i = 0; i < len; i++) {
        p[i] = 0;
    }
}

/**
 * POST handler for message submission
 * Rules#24-28: Comprehensive input validation
 */
int post(sqlite3* db, const char* username, const char* message, 
         const char* session_token, const char* request_token) {
    
    /* Rules#24-28: Validate CSRF token first */
    if (!validate_csrf_token(session_token, request_token)) {
        fprintf(stderr, "CSRF validation failed\\n");
        return -1;
    }
    
    /* Rules#24-25: Validate inputs */
    if (!validate_username(username)) {
        fprintf(stderr, "Invalid username\\n");
        return -1;
    }
    
    if (!validate_message(message)) {
        fprintf(stderr, "Invalid message\\n");
        return -1;
    }
    
    /* Rules#32: Use parameterized query */
    if (insert_user_message_in_db(db, username, message) != 0) {
        /* Rules#11: Return generic error */
        fprintf(stderr, "%s\\n", GENERIC_ERROR);
        return -1;
    }
    
    printf("Message posted successfully\\n");
    return 0;
}

/**
 * Initialize database
 */
int init_db(sqlite3** db) {
    int rc = sqlite3_open("messages.db", db);
    if (rc != SQLITE_OK) {
        return -1;
    }
    
    const char* create_table = 
        "CREATE TABLE IF NOT EXISTS messages ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL,"
        "message TEXT NOT NULL,"
        "created_at TEXT NOT NULL)";
    
    char* err_msg = NULL;
    rc = sqlite3_exec(*db, create_table, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        sqlite3_free(err_msg);
        sqlite3_close(*db);
        return -1;
    }
    
    return 0;
}

/* Test cases */
int main(void) {
    sqlite3* db = NULL;
    char* csrf_token = NULL;
    
    /* Initialize database */
    if (init_db(&db) != 0) {
        fprintf(stderr, "Failed to initialize database\\n");
        return 1;
    }
    
    /* Generate CSRF token */
    csrf_token = generate_csrf_token();
    if (csrf_token == NULL) {
        fprintf(stderr, "Failed to generate CSRF token\\n");
        sqlite3_close(db);
        return 1;
    }
    
    /* Test 1: Valid username */
    printf("Test 1: %s\\n", validate_username("john_doe") ? "true" : "false");
    
    /* Test 2: Invalid username with special chars */
    printf("Test 2: %s\\n", validate_username("john@doe") ? "true" : "false");
    
    /* Test 3: Username too long */
    char long_username[MAX_USERNAME_LENGTH + 10];
    memset(long_username, 'a', sizeof(long_username) - 1);
    long_username[sizeof(long_username) - 1] = '\\0';
    printf("Test 3: %s\\n", validate_username(long_username) ? "true" : "false");
    
    /* Test 4: Valid message */
    printf("Test 4: %s\\n", validate_message("Hello world!") ? "true" : "false");
    
    /* Test 5: Message too long */
    char long_message[MAX_MESSAGE_LENGTH + 10];
    memset(long_message, 'a', sizeof(long_message) - 1);
    long_message[sizeof(long_message) - 1] = '\\0';
    printf("Test 5: %s\\n", validate_message(long_message) ? "true" : "false");
    
    /* Cleanup */
    /* Rules#9: Securely zero sensitive data */
    secure_zero(csrf_token, CSRF_TOKEN_LENGTH);
    free(csrf_token);
    sqlite3_close(db);
    
    return 0;
}
