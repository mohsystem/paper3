
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>

/* Rules#2, Rules#35, Rules#38: Maximum lengths to prevent DoS */
#define MAX_USERNAME_LENGTH 50
#define MAX_MESSAGE_LENGTH 1000
#define MAX_QUERY_LENGTH 2048

/* Rules#8: Initialize pointers to NULL for safety */
typedef struct {
    char* username;
    char* message;
} Request;

/**
 * Validate character is allowed
 * Rules#25: Allow list approach
 */
static bool is_allowed_char(char c) {
    return (isalnum((unsigned char)c) || 
            c == ' ' || c == '.' || c == ',' || 
            c == '!' || c == '?' || c == '\\'' || c == '-');
}

/**
 * Validate and sanitize input
 * Rules#2, Rules#24, Rules#35: Strict validation with bounds checks
 */
static char* validate_and_sanitize(const char* input, size_t max_length, 
                                   const char* field_name) {
    /* Rules#2: NULL check */
    if (input == NULL) {
        fprintf(stderr, "Invalid %s\\n", field_name);
        return NULL;
    }
    
    size_t len = strnlen(input, max_length + 1);
    
    /* Rules#35: Bounds check */
    if (len == 0 || len > max_length) {
        fprintf(stderr, "Invalid %s length\\n", field_name);
        return NULL;
    }
    
    /* Rules#8: Check allocation with overflow protection */
    if (len >= SIZE_MAX - 1) {
        fprintf(stderr, "Length overflow\\n");
        return NULL;
    }
    
    /* Rules#8: Allocate and check result */
    char* output = (char*)calloc(len + 1, sizeof(char));
    if (output == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        return NULL;
    }
    
    /* Rules#35: Bounded copy with character validation */
    size_t j = 0;
    for (size_t i = 0; i < len && j < len; i++) {
        /* Rules#25: Allow list validation */
        if (!is_allowed_char(input[i])) {
            fprintf(stderr, "Invalid character in %s\\n", field_name);
            free(output);
            return NULL;
        }
        
        /* Skip multiple spaces */
        if (input[i] == ' ' && j > 0 && output[j-1] == ' ') {
            continue;
        }
        
        output[j++] = input[i];
    }
    
    /* Rules#36: Ensure null termination */
    output[j] = '\\0';
    
    /* Trim trailing spaces */
    while (j > 0 && output[j-1] == ' ') {
        output[--j] = '\\0';
    }
    
    /* Check final length is valid */
    if (j == 0) {
        fprintf(stderr, "Invalid %s after sanitization\\n", field_name);
        free(output);
        return NULL;
    }
    
    return output;
}

/**
 * Insert message into database using parameterized approach
 * Rules#32: Prevent SQL injection with proper escaping
 * Note: In production use a proper database library with prepared statements
 */
static bool insert_user_message_in_db(const char* username, const char* message) {
    /* Rules#3: Get database path from environment */
    const char* db_path = getenv("DB_PATH");
    if (db_path == NULL) {
        /* Rules#11: Log error internally */
        fprintf(stderr, "Database configuration missing\\n");
        return false;
    }
    
    /* Rules#8: Initialize pointer */
    FILE* db_file = NULL;
    bool success = false;
    
    /* Rules#7: Open with error checking */
    db_file = fopen(db_path, "a");
    if (db_file == NULL) {
        fprintf(stderr, "Database error occurred\\n");
        return false;
    }
    
    /* Rules#31: Use constant format string for output */
    /* In production: Use actual database library with parameterized queries */
    /* This is a simplified file-based example */
    int written = fprintf(db_file, "USERNAME: %s\\nMESSAGE: %s\\n---\\n", 
                         username, message);
    
    if (written > 0) {
        /* Rules#7: Ensure data is written */
        if (fflush(db_file) == 0) {
            success = true;
        }
    }
    
    /* Rules#7: Close file handle */
    if (fclose(db_file) != 0) {
        fprintf(stderr, "Error closing database\\n");
        success = false;
    }
    
    return success;
}

/**
 * POST handler for message submission
 * Rules#2, Rules#28: Validate all inputs from request
 */
static void post(const Request* request) {
    /* Rules#8: Initialize pointers */
    char* username = NULL;
    char* message = NULL;
    
    /* Rules#2, Rules#28: Validate request is not NULL */
    if (request == NULL || request->username == NULL || request->message == NULL) {
        printf("Status: 400 Bad Request\\r\\n");
        printf("Content-Type: text/plain\\r\\n\\r\\n");
        printf("Invalid request\\n");
        return;
    }
    
    /* Rules#2, Rules#24: Validate and sanitize inputs */
    username = validate_and_sanitize(request->username, MAX_USERNAME_LENGTH, "username");
    if (username == NULL) {
        printf("Status: 400 Bad Request\\r\\n");
        printf("Content-Type: text/plain\\r\\n\\r\\n");
        printf("Invalid input\\n");
        return;
    }
    
    message = validate_and_sanitize(request->message, MAX_MESSAGE_LENGTH, "message");
    if (message == NULL) {
        free(username);
        printf("Status: 400 Bad Request\\r\\n");
        printf("Content-Type: text/plain\\r\\n\\r\\n");
        printf("Invalid input\\n");
        return;
    }
    
    /* Insert into database */
    bool success = insert_user_message_in_db(username, message);
    
    /* Rules#9: Clear sensitive data before free */
    memset(username, 0, strlen(username));
    memset(message, 0, strlen(message));
    
    /* Rules#8: Free exactly once */
    free(username);
    free(message);
    username = NULL;
    message = NULL;
    
    if (!success) {
        /* Rules#11: Generic error message */
        printf("Status: 500 Internal Server Error\\r\\n");
        printf("Content-Type: text/plain\\r\\n\\r\\n");
        printf("Unable to process request\\n");
        return;
    }
    
    /* Redirect to main page */
    printf("Status: 302 Found\\r\\n");
    printf("Location: /\\r\\n\\r\\n");
}

/**
 * GET handler for main page
 * Rules#31: Safe HTML output with constant format
 */
static void index(void) {
    printf("Status: 200 OK\\r\\n");
    printf("Content-Type: text/html; charset=UTF-8\\r\\n\\r\\n");
    
    /* Rules#31: Constant format string */
    printf("<!DOCTYPE html>\\n");
    printf("<html>\\n<head>\\n");
    printf("<title>Message Board</title>\\n");
    printf("<meta charset=\\"UTF-8\\">\\n");
    printf("</head>\\n<body>\\n");
    printf("<h1>Post a Message</h1>\\n");
    printf("<form method=\\"post\\" action=\\"/post\\">\\n");
    printf("<label>Username: <input type=\\"text\\" name=\\"username\\" maxlength=\\"%d\\" required></label><br>\\n", 
           MAX_USERNAME_LENGTH);
    printf("<label>Message: <textarea name=\\"message\\" maxlength=\\"%d\\" required></textarea></label><br>\\n", 
           MAX_MESSAGE_LENGTH);
    printf("<input type=\\"submit\\" value=\\"Post Message\\">\\n");
    printf("</form>\\n</body>\\n</html>\\n");
}

/* Test cases */
int main(void) {
    printf("Test Case 1: Valid username and message\\n");
    char* test1 = validate_and_sanitize("JohnDoe", MAX_USERNAME_LENGTH, "username");
    if (test1 != NULL) {
        printf("  Passed: %s\\n", test1);
        free(test1);
    } else {
        printf("  Failed\\n");
    }
    
    printf("\\nTest Case 2: Empty username\\n");
    char* test2 = validate_and_sanitize("", MAX_USERNAME_LENGTH, "username");
    if (test2 == NULL) {
        printf("  Passed: Correctly rejected\\n");
    } else {
        printf("  Failed: Should have rejected\\n");
        free(test2);
    }
    
    printf("\\nTest Case 3: Message exceeds max length\\n");
    char long_msg[MAX_MESSAGE_LENGTH + 2];
    memset(long_msg, 'a', MAX_MESSAGE_LENGTH + 1);
    long_msg[MAX_MESSAGE_LENGTH + 1] = '\\0';
    char* test3 = validate_and_sanitize(long_msg, MAX_MESSAGE_LENGTH, "message");
    if (test3 == NULL) {
        printf("  Passed: Correctly rejected\\n");
    } else {
        printf("  Failed: Should have rejected\\n");
        free(test3);
    }
    
    printf("\\nTest Case 4: Invalid characters in input\\n");
    char* test4 = validate_and_sanitize("test<script>", MAX_USERNAME_LENGTH, "username");
    if (test4 == NULL) {
        printf("  Passed: Correctly rejected\\n");
    } else {
        printf("  Failed: Should have rejected\\n");
        free(test4);
    }
    
    printf("\\nTest Case 5: SQL injection attempt\\n");
    char* test5 = validate_and_sanitize("'; DROP TABLE messages; --", 
                                       MAX_MESSAGE_LENGTH, "message");
    if (test5 == NULL) {
        printf("  Passed: Correctly rejected\\n");
    } else {
        printf("  Failed: Should have rejected\\n");
        free(test5);
    }
    
    return 0;
}
