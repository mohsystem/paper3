
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

/* Security: Maximum input lengths to prevent buffer overflow */
#define MAX_USERNAME_LEN 64
#define MAX_PASSWORD_LEN 128
#define MAX_INPUT_LEN 256

/* Security: Stored credentials would normally come from secure configuration
 * NOT hard-coded. This is for demonstration only. In production, use a 
 * credential manager or secure vault with hashed passwords. */
static const char VALID_USERNAME[] = "admin";
static const char VALID_PASSWORD[] = "securePass123!";

/* Security: Constant-time string comparison to prevent timing attacks */
static bool secure_strcmp(const char *a, const char *b, size_t max_len) {
    if (a == NULL || b == NULL) {
        return false;
    }
    
    size_t len_a = strnlen(a, max_len);
    size_t len_b = strnlen(b, max_len);
    
    /* Security: Compare lengths in constant time */
    unsigned char result = (unsigned char)(len_a ^ len_b);
    
    /* Security: Always compare up to the shorter length to prevent timing leaks */
    size_t compare_len = (len_a < len_b) ? len_a : len_b;
    
    for (size_t i = 0; i < compare_len; i++) {
        result |= (unsigned char)(a[i] ^ b[i]);
    }
    
    return (result == 0);
}

/* Security: Safe input reading with bounds checking */
static bool read_input(char *buffer, size_t buffer_size, const char *prompt) {
    if (buffer == NULL || buffer_size == 0) {
        return false;
    }
    
    /* Security: Initialize buffer to prevent info leaks */
    memset(buffer, 0, buffer_size);
    
    printf("%s", prompt);
    fflush(stdout);
    
    /* Security: Use fgets with explicit bound to prevent overflow */
    if (fgets(buffer, (int)buffer_size, stdin) == NULL) {
        return false;
    }
    
    /* Security: Remove trailing newline and validate length */
    size_t len = strnlen(buffer, buffer_size);
    if (len > 0 && buffer[len - 1] == '\\n') {
        buffer[len - 1] = '\\0';
        len--;
    }
    
    /* Security: Validate input length is within acceptable range */
    if (len == 0 || len >= buffer_size - 1) {
        return false;
    }
    
    return true;
}

/* Security: Input validation - check for valid characters only */
static bool validate_username(const char *username) {
    if (username == NULL) {
        return false;
    }
    
    size_t len = strnlen(username, MAX_USERNAME_LEN + 1);
    
    /* Security: Enforce length constraints */
    if (len == 0 || len > MAX_USERNAME_LEN) {
        return false;
    }
    
    /* Security: Allow list validation - only alphanumeric and underscore */
    for (size_t i = 0; i < len; i++) {
        char c = username[i];
        if (!((c >= 'a' && c <= 'z') || 
              (c >= 'A' && c <= 'Z') || 
              (c >= '0' && c <= '9') || 
              c == '_')) {
            return false;
        }
    }
    
    return true;
}

/* Security: Validate password meets basic requirements */
static bool validate_password(const char *password) {
    if (password == NULL) {
        return false;
    }
    
    size_t len = strnlen(password, MAX_PASSWORD_LEN + 1);
    
    /* Security: Enforce length constraints */
    if (len == 0 || len > MAX_PASSWORD_LEN) {
        return false;
    }
    
    /* Security: Check for printable ASCII characters only */
    for (size_t i = 0; i < len; i++) {
        if (password[i] < 32 || password[i] > 126) {
            return false;
        }
    }
    
    return true;
}

/* Main authentication function */
bool check_mod(const char *username, const char *password) {
    /* Security: Validate inputs are not NULL */
    if (username == NULL || password == NULL) {
        return false;
    }
    
    /* Security: Validate input format before comparison */
    if (!validate_username(username)) {
        return false;
    }
    
    if (!validate_password(password)) {
        return false;
    }
    
    /* Security: Use constant-time comparison to prevent timing attacks */
    bool username_match = secure_strcmp(username, VALID_USERNAME, MAX_USERNAME_LEN);
    bool password_match = secure_strcmp(password, VALID_PASSWORD, MAX_PASSWORD_LEN);
    
    /* Security: Return combined result without early exit */
    return (username_match && password_match);
}

/* Simulated form input and button click handler */
int main(void) {
    char username[MAX_USERNAME_LEN + 1];
    char password[MAX_PASSWORD_LEN + 1];
    
    printf("=== Authentication System Test Cases ===\\n\\n");
    
    /* Test Case 1: Valid credentials */
    printf("Test Case 1: Valid credentials\\n");
    memset(username, 0, sizeof(username));
    memset(password, 0, sizeof(password));
    strncpy(username, "admin", sizeof(username) - 1);
    strncpy(password, "securePass123!", sizeof(password) - 1);
    bool result1 = check_mod(username, password);
    printf("Result: %s\\n\\n", result1 ? "PASS (true)" : "FAIL (false)");
    
    /* Test Case 2: Invalid username */
    printf("Test Case 2: Invalid username\\n");
    memset(username, 0, sizeof(username));
    memset(password, 0, sizeof(password));
    strncpy(username, "wronguser", sizeof(username) - 1);
    strncpy(password, "securePass123!", sizeof(password) - 1);
    bool result2 = check_mod(username, password);
    printf("Result: %s\\n\\n", result2 ? "FAIL (true)" : "PASS (false)");
    
    /* Test Case 3: Invalid password */
    printf("Test Case 3: Invalid password\\n");
    memset(username, 0, sizeof(username));
    memset(password, 0, sizeof(password));
    strncpy(username, "admin", sizeof(username) - 1);
    strncpy(password, "wrongpass", sizeof(password) - 1);
    bool result3 = check_mod(username, password);
    printf("Result: %s\\n\\n", result3 ? "FAIL (true)" : "PASS (false)");
    
    /* Test Case 4: NULL inputs */
    printf("Test Case 4: NULL inputs\\n");
    bool result4 = check_mod(NULL, NULL);
    printf("Result: %s\\n\\n", result4 ? "FAIL (true)" : "PASS (false)");
    
    /* Test Case 5: Invalid characters in username */
    printf("Test Case 5: Invalid characters in username\\n");
    memset(username, 0, sizeof(username));
    memset(password, 0, sizeof(password));
    strncpy(username, "admin'; DROP TABLE users--", sizeof(username) - 1);
    strncpy(password, "securePass123!", sizeof(password) - 1);
    bool result5 = check_mod(username, password);
    printf("Result: %s\\n\\n", result5 ? "FAIL (true)" : "PASS (false)");
    
    /* Security: Clear sensitive data before program exit */
    memset(username, 0, sizeof(username));
    memset(password, 0, sizeof(password));
    
    return 0;
}
