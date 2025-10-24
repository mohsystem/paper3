#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>

#define MAX_SESSIONS 10
#define TOKEN_LENGTH 32
#define SESSION_ID_LENGTH 32

// Structure to hold session data
typedef struct {
    char sessionId[SESSION_ID_LENGTH + 1];
    char csrfToken[TOKEN_LENGTH + 1];
} Session;

// Simulates a server-side session store
static Session sessionStore[MAX_SESSIONS];
static int sessionCount = 0;

/**
 * WARNING: This token generation is NOT cryptographically secure.
 * It uses rand(), which is predictable. For production systems, use a proper
 * entropy source like /dev/urandom on Linux or the Cryptography API on Windows.
 */
void generate_token(char* buffer, size_t length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (size_t i = 0; i < length; i++) {
        int key = rand() % (int)(sizeof(charset) - 1);
        buffer[i] = charset[key];
    }
    buffer[length] = '\0';
}

/**
 * Finds a session by its ID. Returns a pointer to the session or NULL if not found.
 */
Session* find_session(const char* sessionId) {
    for (int i = 0; i < sessionCount; i++) {
        if (strcmp(sessionStore[i].sessionId, sessionId) == 0) {
            return &sessionStore[i];
        }
    }
    return NULL;
}

/**
 * Simulates a user requesting a page. Generates and stores a CSRF token.
 * Returns the generated token. The caller is responsible for freeing the memory.
 */
char* generate_csrf_token(const char* sessionId) {
    if (sessionId == NULL || strlen(sessionId) == 0) {
        fprintf(stderr, "Session ID cannot be null or empty.\n");
        return NULL;
    }

    Session* session = find_session(sessionId);
    if (session == NULL) {
        // Create a new session if not found and there is space
        if (sessionCount < MAX_SESSIONS) {
            session = &sessionStore[sessionCount++];
            strncpy(session->sessionId, sessionId, SESSION_ID_LENGTH);
            session->sessionId[SESSION_ID_LENGTH] = '\0';
        } else {
            fprintf(stderr, "Session store is full.\n");
            return NULL;
        }
    }

    // Generate and store new token
    generate_token(session->csrfToken, TOKEN_LENGTH);
    
    // Return a copy of the token
    char* token_copy = (char*)malloc(TOKEN_LENGTH + 1);
    if(token_copy) {
        strcpy(token_copy, session->csrfToken);
    }
    return token_copy;
}

/**
 * Simulates a form submission and validates the CSRF token.
 */
bool validate_csrf_token(const char* sessionId, const char* submittedToken) {
    if (sessionId == NULL || submittedToken == NULL) {
        return false;
    }

    Session* session = find_session(sessionId);
    if (session == NULL) {
        return false; // No session found
    }

    // IMPORTANT: In a real application, use a constant-time comparison function
    // to prevent timing attacks. strcmp is NOT secure for this.
    return strcmp(session->csrfToken, submittedToken) == 0;
}

int main() {
    // Seed the pseudo-random number generator
    // WARNING: Insecure for cryptographic purposes.
    srand((unsigned int)time(NULL));

    printf("--- CSRF Protection Simulation ---\n");

    const char* user1SessionId = "session_abc_123";
    const char* user2SessionId = "session_xyz_789";

    // Test Case 1: Valid request
    printf("\n--- Test Case 1: Valid Request ---\n");
    char* user1Token = generate_csrf_token(user1SessionId);
    printf("User 1 generated token: %s\n", user1Token);
    bool isValid1 = validate_csrf_token(user1SessionId, user1Token);
    printf("Form submission with correct token is valid: %s\n", isValid1 ? "true" : "false");
    assert(isValid1);
    
    // Test Case 2: Invalid request (wrong token)
    printf("\n--- Test Case 2: Invalid Request (Wrong Token) ---\n");
    const char* attackerToken = "fake_malicious_token";
    printf("User 1 submitting with a wrong token: %s\n", attackerToken);
    bool isValid2 = validate_csrf_token(user1SessionId, attackerToken);
    printf("Form submission with wrong token is valid: %s\n", isValid2 ? "true" : "false");
    assert(!isValid2);

    // Test Case 3: Invalid request (missing token)
    printf("\n--- Test Case 3: Invalid Request (Missing Token) ---\n");
    bool isValid3 = validate_csrf_token(user1SessionId, NULL);
    printf("Form submission with missing token is valid: %s\n", isValid3 ? "true" : "false");
    assert(!isValid3);

    // Test Case 4: Invalid request (token from a different user's session)
    printf("\n--- Test Case 4: Invalid Request (Token from another session) ---\n");
    char* user2Token = generate_csrf_token(user2SessionId);
    printf("User 2 generated token: %s\n", user2Token);
    printf("User 1 attempts to submit form with User 2's token.\n");
    bool isValid4 = validate_csrf_token(user1SessionId, user2Token);
    printf("Submission is valid: %s\n", isValid4 ? "true" : "false");
    assert(!isValid4);

    // Test Case 5: Valid request after token regeneration
    printf("\n--- Test Case 5: Valid Request after Token Regeneration ---\n");
    printf("User 1 old token: %s\n", user1Token);
    char* user1NewToken = generate_csrf_token(user1SessionId); // Token is regenerated
    printf("User 1 new token: %s\n", user1NewToken);
    bool isValid5_old = validate_csrf_token(user1SessionId, user1Token); // Old token is now invalid
    bool isValid5_new = validate_csrf_token(user1SessionId, user1NewToken); // New token is valid
    printf("Submission with old token is valid: %s\n", isValid5_old ? "true" : "false");
    printf("Submission with new token is valid: %s\n", isValid5_new ? "true" : "false");
    assert(!isValid5_old);
    assert(isValid5_new);
    
    // Cleanup
    free(user1Token);
    free(user2Token);
    free(user1NewToken);
    
    return 0;
}