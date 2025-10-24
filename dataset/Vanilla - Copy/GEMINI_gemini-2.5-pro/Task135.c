#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_SESSIONS 10
#define SESSION_ID_LEN 32
#define TOKEN_LEN 33 // 32 chars + null terminator

// Simple structure to represent a session entry
typedef struct {
    char sessionId[SESSION_ID_LEN];
    char csrfToken[TOKEN_LEN];
} Session;

// Simulates server-side session storage
Session session_storage[MAX_SESSIONS];
int session_count = 0;

/**
 * @brief Generates a random CSRF token.
 * 
 * NOTE: This uses rand() which is NOT cryptographically secure and is for
 * demonstration purposes only. In a real-world application, use a proper
 * cryptographic library (like OpenSSL) or OS-provided entropy sources
 * (e.g., /dev/urandom).
 * 
 * @param buffer The character buffer to store the generated token.
 * @param length The length of the token to generate (excluding null terminator).
 */
void generate_csrf_token(char* buffer, size_t length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    if (length) {
        for (size_t n = 0; n < length; n++) {
            int key = rand() % (int)(sizeof(charset) - 1);
            buffer[n] = charset[key];
        }
        buffer[length] = '\0';
    }
}

/**
 * @brief Stores a session ID and its CSRF token.
 * Overwrites if session ID already exists.
 */
void store_token_in_session(const char* sessionId, const char* token) {
    if (session_count >= MAX_SESSIONS) {
        printf("Error: Max sessions reached.\n");
        return;
    }
    
    // Check if session already exists to update it
    for (int i = 0; i < session_count; ++i) {
        if (strncmp(session_storage[i].sessionId, sessionId, SESSION_ID_LEN) == 0) {
            strncpy(session_storage[i].csrfToken, token, TOKEN_LEN - 1);
            session_storage[i].csrfToken[TOKEN_LEN - 1] = '\0';
            return;
        }
    }
    
    // Add a new session
    strncpy(session_storage[session_count].sessionId, sessionId, SESSION_ID_LEN - 1);
    session_storage[session_count].sessionId[SESSION_ID_LEN - 1] = '\0';
    strncpy(session_storage[session_count].csrfToken, token, TOKEN_LEN - 1);
    session_storage[session_count].csrfToken[TOKEN_LEN - 1] = '\0';
    session_count++;
}

/**
 * @brief Validates a received token against the one stored for a session.
 *
 * NOTE: This uses strcmp which is not a constant-time comparison.
 * In a real application, use a constant-time comparison function to
 * prevent timing attacks.
 *
 * @return 1 if valid, 0 otherwise.
 */
int validate_csrf_token(const char* sessionId, const char* receivedToken) {
    if (sessionId == NULL || receivedToken == NULL) {
        return 0;
    }

    for (int i = 0; i < session_count; ++i) {
        if (strncmp(session_storage[i].sessionId, sessionId, SESSION_ID_LEN) == 0) {
            // Session found, now compare tokens
            return strcmp(session_storage[i].csrfToken, receivedToken) == 0;
        }
    }
    
    return 0; // Session ID not found
}

/**
 * @brief Simulates processing a protected action.
 */
void process_request(const char* sessionId, const char* receivedToken) {
    printf("Processing request for session: %s with token: %s\n", 
           sessionId ? sessionId : "(null)", 
           receivedToken ? receivedToken : "(null)");
           
    if (validate_csrf_token(sessionId, receivedToken)) {
        printf("SUCCESS: CSRF token is valid. Action allowed.\n");
    } else {
        printf("FAILURE: CSRF token is invalid or missing. Action rejected.\n");
    }
    printf("-------------------------------------------------\n");
}


int main() {
    // Seed the random number generator
    srand((unsigned int)time(NULL));

    // --- Test Case 1: Valid Request ---
    printf("Test Case 1: Valid Request\n");
    const char* sessionId1 = "user_session_abc123";
    char token1[TOKEN_LEN];
    generate_csrf_token(token1, TOKEN_LEN - 1);
    store_token_in_session(sessionId1, token1);
    process_request(sessionId1, token1);

    // --- Test Case 2: Invalid/Incorrect Token ---
    printf("Test Case 2: Invalid/Incorrect Token\n");
    const char* sessionId2 = "user_session_def456";
    char token2[TOKEN_LEN];
    generate_csrf_token(token2, TOKEN_LEN - 1);
    store_token_in_session(sessionId2, token2);
    process_request(sessionId2, "wrong_token_-_");

    // --- Test Case 3: Missing Token ---
    printf("Test Case 3: Missing Token\n");
    const char* sessionId3 = "user_session_ghi789";
    char token3[TOKEN_LEN];
    generate_csrf_token(token3, TOKEN_LEN - 1);
    store_token_in_session(sessionId3, token3);
    process_request(sessionId3, NULL);

    // --- Test Case 4: No Token in Session ---
    printf("Test Case 4: No Token in Session\n");
    const char* sessionId4 = "user_session_jkl012";
    // No token stored for this session
    process_request(sessionId4, "some_random_token");

    // --- Test Case 5: Token Mismatch between Users ---
    printf("Test Case 5: Token Mismatch between Users\n");
    const char* userA_sessionId = "user_A_session";
    char userA_token[TOKEN_LEN];
    generate_csrf_token(userA_token, TOKEN_LEN - 1);
    store_token_in_session(userA_sessionId, userA_token);

    const char* userB_sessionId = "user_B_session";
    char userB_token[TOKEN_LEN];
    generate_csrf_token(userB_token, TOKEN_LEN - 1);
    store_token_in_session(userB_sessionId, userB_token);

    printf("Attacker tries to use User B's token for User A's session:\n");
    process_request(userA_sessionId, userB_token);

    return 0;
}