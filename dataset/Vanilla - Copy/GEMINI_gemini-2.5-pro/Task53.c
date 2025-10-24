#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_SESSIONS 100
#define SESSION_ID_LEN 32
#define USERNAME_LEN 32

typedef struct {
    char sessionId[SESSION_ID_LEN];
    char username[USERNAME_LEN];
    int isActive;
} Session;

// Global session store
Session sessionStore[MAX_SESSIONS];
int session_count = 0;

// Initialize the session store
void initializeSessionStore() {
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        sessionStore[i].isActive = 0;
        sessionStore[i].sessionId[0] = '\0';
        sessionStore[i].username[0] = '\0';
    }
}

// Creates a new session for a user. Returns session ID on success, NULL on failure.
const char* createSession(const char* username) {
    if (session_count >= MAX_SESSIONS) {
        printf("Error: Maximum number of sessions reached.\n");
        return NULL;
    }

    int i = 0;
    while(sessionStore[i].isActive && i < MAX_SESSIONS) {
        i++;
    }

    if(i == MAX_SESSIONS) return NULL; // Should not happen if session_count is checked

    snprintf(sessionStore[i].sessionId, SESSION_ID_LEN, "sess_%d_%ld", i, time(NULL));
    strncpy(sessionStore[i].username, username, USERNAME_LEN - 1);
    sessionStore[i].username[USERNAME_LEN - 1] = '\0'; // Ensure null termination
    sessionStore[i].isActive = 1;
    session_count++;

    printf("Session created for %s with ID: %s\n", username, sessionStore[i].sessionId);
    return sessionStore[i].sessionId;
}

// Finds a session by ID and returns a pointer to it, or NULL if not found.
Session* findSession(const char* sessionId) {
    if (sessionId == NULL) return NULL;
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (sessionStore[i].isActive && strcmp(sessionStore[i].sessionId, sessionId) == 0) {
            return &sessionStore[i];
        }
    }
    return NULL;
}

// Retrieves the username for a given session ID.
const char* getUserFromSession(const char* sessionId) {
    Session* session = findSession(sessionId);
    if (session != NULL) {
        return session->username;
    }
    return NULL;
}

// Ends an active session.
void endSession(const char* sessionId) {
    Session* session = findSession(sessionId);
    if (session != NULL) {
        printf("Session ended for %s with ID: %s\n", session->username, session->sessionId);
        session->isActive = 0;
        session_count--;
    } else {
        printf("Session with ID %s not found.\n", sessionId);
    }
}

// Checks if a session is currently active.
int isSessionActive(const char* sessionId) {
    return findSession(sessionId) != NULL;
}

int main() {
    initializeSessionStore();
    printf("--- Running Test Cases ---\n");

    // Test Case 1: Create a session for user 'alice'
    printf("\n--- Test Case 1: Create Session ---\n");
    const char* aliceSessionId_const = createSession("alice");
    // Create a mutable copy for later use
    char aliceSessionId[SESSION_ID_LEN];
    if (aliceSessionId_const) {
        strncpy(aliceSessionId, aliceSessionId_const, SESSION_ID_LEN);
    } else {
        return 1; // Exit if session creation failed
    }


    // Test Case 2: Check if alice's session is active
    printf("\n--- Test Case 2: Check Active Session ---\n");
    printf("Is alice's session active? %s\n", isSessionActive(aliceSessionId) ? "true" : "false");

    // Test Case 3: Get user from session ID
    printf("\n--- Test Case 3: Get User from Session ---\n");
    const char* username = getUserFromSession(aliceSessionId);
    printf("User for session %s is: %s\n", aliceSessionId, username ? username : "Not Found");

    // Test Case 4: Create another session for 'bob' and end 'alice's session
    printf("\n--- Test Case 4: End Session ---\n");
    const char* bobSessionId = createSession("bob");
    endSession(aliceSessionId);

    // Test Case 5: Check if alice's session is still active and bob's is active
    printf("\n--- Test Case 5: Verify Session Status ---\n");
    printf("Is alice's session active after ending? %s\n", isSessionActive(aliceSessionId) ? "true" : "false");
    printf("Is bob's session active? %s\n", isSessionActive(bobSessionId) ? "true" : "false");
    const char* nonExistentUser = getUserFromSession(aliceSessionId);
    printf("Attempting to get user from ended session: %s\n", nonExistentUser ? nonExistentUser : "Not Found");
    
    return 0;
}