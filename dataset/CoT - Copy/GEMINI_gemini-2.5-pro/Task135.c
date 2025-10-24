#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// This implementation uses /dev/urandom for random number generation.
// It will only work on POSIX-compliant systems (Linux, macOS, etc.).
// For Windows, you would need to use the BCryptGenRandom function from the
// Cryptography API: Next Generation (CNG).
#ifndef _WIN32
#include <fcntl.h>
#include <unistd.h>
#endif

#define TOKEN_BYTE_LENGTH 32
#define TOKEN_HEX_LENGTH (TOKEN_BYTE_LENGTH * 2 + 1)

// A simple linked list to simulate a session store
typedef struct Session {
    char* sessionId;
    char* token;
    struct Session* next;
} Session;

// Global head of the session store linked list
Session* g_session_store = NULL;

/**
 * Generates a cryptographically secure CSRF token.
 * Reads random bytes and converts them to a hex string.
 * @param token_buffer A buffer to store the generated hex token. Must be at least TOKEN_HEX_LENGTH bytes.
 * @return 0 on success, -1 on failure.
 */
int generate_csrf_token(char* token_buffer) {
#ifdef _WIN32
    // Windows implementation using BCryptGenRandom would go here.
    // For this example, we'll just report an error.
    fprintf(stderr, "CSRF token generation is not implemented for Windows in this example.\n");
    return -1;
#else
    unsigned char random_bytes[TOKEN_BYTE_LENGTH];
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1) {
        perror("Error opening /dev/urandom");
        return -1;
    }

    ssize_t bytes_read = read(fd, random_bytes, TOKEN_BYTE_LENGTH);
    close(fd);

    if (bytes_read != TOKEN_BYTE_LENGTH) {
        fprintf(stderr, "Error reading from /dev/urandom\n");
        return -1;
    }

    for (int i = 0; i < TOKEN_BYTE_LENGTH; i++) {
        sprintf(token_buffer + (i * 2), "%02x", random_bytes[i]);
    }
    token_buffer[TOKEN_HEX_LENGTH - 1] = '\0';
    return 0;
#endif
}

/**
 * Stores a token for a given session ID in the simulated store.
 * @param sessionId The user's session identifier.
 * @param token The CSRF token to store.
 */
void store_token(const char* sessionId, const char* token) {
    // Check if session already exists and update token
    Session* current = g_session_store;
    while (current != NULL) {
        if (strcmp(current->sessionId, sessionId) == 0) {
            free(current->token);
            current->token = strdup(token);
            return;
        }
        current = current->next;
    }

    // If not found, create a new session entry
    Session* new_session = (Session*)malloc(sizeof(Session));
    if (!new_session) {
        perror("Failed to allocate memory for session");
        return;
    }
    new_session->sessionId = strdup(sessionId);
    new_session->token = strdup(token);
    new_session->next = g_session_store;
    g_session_store = new_session;
}

/**
 * Performs a constant-time comparison of two C strings.
 * @return true if strings are equal, false otherwise.
 */
bool constant_time_equals(const char* a, const char* b) {
    if (a == NULL || b == NULL) return false;
    
    size_t len_a = strlen(a);
    size_t len_b = strlen(b);
    
    if (len_a != len_b) return false;
    
    int result = 0;
    for (size_t i = 0; i < len_a; ++i) {
        result |= (a[i] ^ b[i]);
    }
    
    return result == 0;
}

/**
 * Validates a received token against the one stored for the session.
 * @param sessionId The user's session identifier.
 * @param receivedToken The token received from the client.
 * @return true if the token is valid, false otherwise.
 */
bool validate_csrf_token(const char* sessionId, const char* receivedToken) {
    Session* current = g_session_store;
    while (current != NULL) {
        if (strcmp(current->sessionId, sessionId) == 0) {
            return constant_time_equals(current->token, receivedToken);
        }
        current = current->next;
    }
    return false; // Session not found
}

/**
 * Frees all memory allocated for the session store.
 */
void cleanup_session_store() {
    Session* current = g_session_store;
    while (current != NULL) {
        Session* temp = current;
        current = current->next;
        free(temp->sessionId);
        free(temp->token);
        free(temp);
    }
    g_session_store = NULL;
}

int main() {
    char token_buffer[TOKEN_HEX_LENGTH];

    // --- Test Cases ---
    const char* sessionId1 = "user_session_abc123";
    const char* sessionId2 = "user_session_def456";

    // Test Case 1: Successful Validation
    printf("--- Test Case 1: Successful Validation ---\n");
    if (generate_csrf_token(token_buffer) == 0) {
        store_token(sessionId1, token_buffer);
        printf("Generated Token: %s\n", token_buffer);
        printf("Is token valid? %s\n\n", validate_csrf_token(sessionId1, token_buffer) ? "true" : "false");
    }

    // Test Case 2: Failed Validation (Incorrect Token)
    printf("--- Test Case 2: Failed Validation (Incorrect Token) ---\n");
    if (generate_csrf_token(token_buffer) == 0) {
        store_token(sessionId2, token_buffer);
        const char* incorrectToken = "this_is_a_wrong_token";
        printf("Stored Token: %s\n", token_buffer);
        printf("Submitted Token: %s\n", incorrectToken);
        printf("Is token valid? %s\n\n", validate_csrf_token(sessionId2, incorrectToken) ? "true" : "false");
    }

    // Test Case 3: Failed Validation (No Token in Session)
    printf("--- Test Case 3: Failed Validation (No Token in Session) ---\n");
    if (generate_csrf_token(token_buffer) == 0) {
        const char* nonExistentSessionId = "non_existent_session_xyz789";
        printf("Submitted Token: %s\n", token_buffer);
        printf("Is token for non-existent session valid? %s\n\n", validate_csrf_token(nonExistentSessionId, token_buffer) ? "true" : "false");
    }

    // Test Case 4: Failed Validation (Submitted Token is NULL)
    printf("--- Test Case 4: Failed Validation (Submitted Token is NULL) ---\n");
    printf("Is NULL token valid? %s\n\n", validate_csrf_token(sessionId1, NULL) ? "true" : "false");

    // Test Case 5: Token Re-generation and Validation
    printf("--- Test Case 5: Token Re-generation and Validation ---\n");
    char old_token[TOKEN_HEX_LENGTH];
    // Find and copy the old token before overwriting it
    Session* s1 = g_session_store;
    while (s1 != NULL) {
        if (strcmp(s1->sessionId, sessionId1) == 0) {
            strncpy(old_token, s1->token, TOKEN_HEX_LENGTH);
            break;
        }
        s1 = s1->next;
    }
    
    if (generate_csrf_token(token_buffer) == 0) {
        store_token(sessionId1, token_buffer);
        printf("Old token: %s\n", old_token);
        printf("New token: %s\n", token_buffer);
        printf("Is old token still valid? %s\n", validate_csrf_token(sessionId1, old_token) ? "true" : "false");
        printf("Is new token valid? %s\n", validate_csrf_token(sessionId1, token_buffer) ? "true" : "false");
    }

    // Clean up all allocated memory
    cleanup_session_store();

    return 0;
}