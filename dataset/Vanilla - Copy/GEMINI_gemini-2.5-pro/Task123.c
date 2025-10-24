#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_SESSIONS 100
#define SESSION_ID_LEN 64
#define USERNAME_LEN 64

typedef struct {
    char sessionId[SESSION_ID_LEN];
    char username[USERNAME_LEN];
    int active; // 1 for active, 0 for inactive
} Session;

// Global in-memory array to store sessions
static Session session_store[MAX_SESSIONS];
static int session_count = 0; // Tracks the number of sessions ever created

/**
 * Simple session ID generator. Returns a pointer to a static buffer.
 * NOT THREAD-SAFE.
 */
const char* generateSessionId() {
    static char buffer[SESSION_ID_LEN];
    snprintf(buffer, SESSION_ID_LEN, "%ld-%d-%d", time(NULL), rand(), session_count);
    return buffer;
}

/**
 * Creates a new session for a given user.
 * @param username The username to associate with the session.
 * @return The unique session ID, or NULL if session store is full.
 */
const char* createSession(const char* username) {
    if (session_count >= MAX_SESSIONS) {
        printf("Session store is full. Cannot create new session.\n");
        return NULL;
    }

    const char* newId = generateSessionId();
    strncpy(session_store[session_count].sessionId, newId, SESSION_ID_LEN - 1);
    session_store[session_count].sessionId[SESSION_ID_LEN - 1] = '\0';

    strncpy(session_store[session_count].username, username, USERNAME_LEN - 1);
    session_store[session_count].username[USERNAME_LEN - 1] = '\0';
    
    session_store[session_count].active = 1;

    printf("Session created for %s with ID: %s\n", username, session_store[session_count].sessionId);

    // Return pointer to the ID within the persistent session_store array
    return session_store[session_count++].sessionId;
}

/**
 * Finds a session by its ID.
 * @param sessionId The session ID to look for.
 * @return A pointer to the Session struct, or NULL if not found.
 */
Session* findSession(const char* sessionId) {
    if (sessionId == NULL) return NULL;
    for (int i = 0; i < session_count; i++) {
        if (strcmp(session_store[i].sessionId, sessionId) == 0) {
            return &session_store[i];
        }
    }
    return NULL;
}

/**
 * Checks if a session ID is currently valid (exists and is active).
 * @param sessionId The session ID to check.
 * @return 1 if valid, 0 otherwise.
 */
int isSessionValid(const char* sessionId) {
    Session* session = findSession(sessionId);
    return (session != NULL && session->active == 1);
}

/**
 * Retrieves the username associated with a session ID.
 * @param sessionId The session ID to look up.
 * @return The username, or NULL if the session is not valid.
 */
const char* getSessionUser(const char* sessionId) {
    Session* session = findSession(sessionId);
    if (session != NULL && session->active == 1) {
        return session->username;
    }
    return NULL;
}

/**
 * Invalidates/removes a user session by marking it inactive.
 * @param sessionId The session ID to invalidate.
 */
void invalidateSession(const char* sessionId) {
    Session* session = findSession(sessionId);
    if (session != NULL) {
        if (session->active) {
            session->active = 0;
            printf("Session invalidated: %s\n", sessionId);
        }
    } else {
        printf("Session to invalidate not found: %s\n", sessionId);
    }
}

int main() {
    // Seed the random number generator
    srand(time(NULL));

    printf("--- Starting Session Management Test ---\n");

    // Test Case 1: Create a session for user "alice"
    printf("\n[Test Case 1: Create session for alice]\n");
    const char* aliceSessionId = createSession("alice");

    // Test Case 2: Check if alice's session is valid and get her username
    printf("\n[Test Case 2: Validate alice's session]\n");
    printf("Is alice's session valid? %s\n", isSessionValid(aliceSessionId) ? "true" : "false");
    const char* username = getSessionUser(aliceSessionId);
    printf("Retrieved user for session %s: %s\n", aliceSessionId, username ? username : "null");

    // Test Case 3: Create a second session for user "bob"
    printf("\n[Test Case 3: Create session for bob]\n");
    const char* bobSessionId = createSession("bob");
    printf("Is bob's session valid? %s\n", isSessionValid(bobSessionId) ? "true" : "false");
    
    // Test Case 4: Invalidate alice's session and verify
    printf("\n[Test Case 4: Invalidate alice's session]\n");
    invalidateSession(aliceSessionId);
    printf("Is alice's session now valid? %s\n", isSessionValid(aliceSessionId) ? "true" : "false");
    const char* invalidUser = getSessionUser(aliceSessionId);
    printf("Attempting to get user for invalidated session: %s\n", invalidUser ? invalidUser : "null");

    // Test Case 5: Verify that bob's session is still valid
    printf("\n[Test Case 5: Verify bob's session is unaffected]\n");
    printf("Is bob's session still valid? %s\n", isSessionValid(bobSessionId) ? "true" : "false");
    const char* bobUser = getSessionUser(bobSessionId);
    printf("Retrieved user for bob's session: %s\n", bobUser ? bobUser : "null");

    printf("\n--- Session Management Test Finished ---\n");

    return 0;
}