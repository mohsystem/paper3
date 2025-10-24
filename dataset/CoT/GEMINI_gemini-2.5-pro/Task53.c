#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#endif

// --- Configuration ---
#define MAX_SESSIONS 100
#define SESSION_ID_LEN 64
#define USERNAME_LEN 50
#define SESSION_TIMEOUT_SECONDS 3

// --- Data Structures ---
typedef struct {
    char sessionId[SESSION_ID_LEN + 1];
    char username[USERNAME_LEN + 1];
    time_t creationTime;
    int active; // Use 1 for active, 0 for inactive
} Session;

// Global session storage (simple array-based approach for demonstration)
static Session activeSessions[MAX_SESSIONS];

// --- Function Prototypes ---
void initializeSessionManager();
const char* createSession(const char* username);
int isSessionValid(const char* sessionId);
void invalidateSession(const char* sessionId);

// --- Implementation ---

/**
 * Generates a random session ID.
 * WARNING: This uses rand() which is NOT cryptographically secure.
 * For production systems, use a proper cryptographic library like OpenSSL's RAND_bytes.
 */
void generateSessionId(char* buffer, size_t bufferSize) {
    // Seed with time and process ID for better (but still weak) randomness
    srand((unsigned int)time(NULL) ^ (unsigned int)getpid());
    
    const char* chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    size_t chars_len = strlen(chars);
    for (size_t i = 0; i < bufferSize - 1; ++i) {
        buffer[i] = chars[rand() % chars_len];
    }
    buffer[bufferSize - 1] = '\0';
}

/**
 * Initializes the session manager, clearing all existing sessions.
 */
void initializeSessionManager() {
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        activeSessions[i].active = 0;
        memset(activeSessions[i].sessionId, 0, sizeof(activeSessions[i].sessionId));
        memset(activeSessions[i].username, 0, sizeof(activeSessions[i].username));
    }
}

/**
 * Creates a new session for a user.
 * @param username The username for whom to create the session.
 * @return A constant pointer to the session ID string if successful, NULL otherwise.
 */
const char* createSession(const char* username) {
    if (username == NULL || strlen(username) == 0) {
        return NULL;
    }
    
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (!activeSessions[i].active) {
            // Found an empty slot
            activeSessions[i].active = 1;
            generateSessionId(activeSessions[i].sessionId, SESSION_ID_LEN + 1);
            strncpy(activeSessions[i].username, username, USERNAME_LEN);
            activeSessions[i].username[USERNAME_LEN] = '\0'; // Ensure null-termination
            activeSessions[i].creationTime = time(NULL);

            printf("Session created for %s with ID: %s\n", username, activeSessions[i].sessionId);
            return activeSessions[i].sessionId;
        }
    }
    printf("Error: No available session slots.\n");
    return NULL; // No available slots
}

// Helper function to find a session by ID
Session* findSession(const char* sessionId) {
    if (sessionId == NULL) return NULL;
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (activeSessions[i].active && strcmp(activeSessions[i].sessionId, sessionId) == 0) {
            return &activeSessions[i];
        }
    }
    return NULL;
}

/**
 * Checks if a session is valid and not expired.
 * @param sessionId The session ID to validate.
 * @return 1 if the session is valid, 0 otherwise.
 */
int isSessionValid(const char* sessionId) {
    Session* session = findSession(sessionId);
    if (session == NULL) {
        return 0; // Session not found or inactive
    }

    if (difftime(time(NULL), session->creationTime) > SESSION_TIMEOUT_SECONDS) {
        // Session expired, invalidate it
        session->active = 0;
        printf("Session %s expired.\n", sessionId);
        return 0;
    }

    return 1;
}

/**
 * Invalidates/removes a user session.
 * @param sessionId The session ID to invalidate.
 */
void invalidateSession(const char* sessionId) {
    Session* session = findSession(sessionId);
    if (session != NULL) {
        session->active = 0;
        printf("Session %s invalidated.\n", sessionId);
    } else {
        printf("Session %s not found for invalidation.\n", sessionId);
    }
}

int main() {
    initializeSessionManager();
    printf("--- Session Management Test ---\n");

    // Test Case 1: Create a session and validate it
    printf("\n--- Test Case 1: Create and Validate ---\n");
    const char* user1SessionIdPtr = createSession("user1");
    // Copy the ID as the pointer might become invalid if the session slot is reused
    char user1SessionId[SESSION_ID_LEN + 1]; 
    if (user1SessionIdPtr) strncpy(user1SessionId, user1SessionIdPtr, sizeof(user1SessionId));
    user1SessionId[SESSION_ID_LEN] = '\0';
    printf("Is user1's session valid? %s\n", isSessionValid(user1SessionId) ? "true" : "false");

    // Test Case 2: Validate a non-existent session
    printf("\n--- Test Case 2: Validate Non-Existent Session ---\n");
    printf("Is 'invalid-session-id' valid? %s\n", isSessionValid("invalid-session-id") ? "true" : "false");

    // Test Case 3: Invalidate a session and check it again
    printf("\n--- Test Case 3: Invalidate and Re-Validate ---\n");
    invalidateSession(user1SessionId);
    printf("Is user1's session valid after invalidation? %s\n", isSessionValid(user1SessionId) ? "true" : "false");

    // Test Case 4: Test session timeout
    printf("\n--- Test Case 4: Session Timeout ---\n");
    const char* user2SessionIdPtr = createSession("user2");
    char user2SessionId[SESSION_ID_LEN + 1];
    if (user2SessionIdPtr) strncpy(user2SessionId, user2SessionIdPtr, sizeof(user2SessionId));
    user2SessionId[SESSION_ID_LEN] = '\0';
    printf("Is user2's session valid immediately? %s\n", isSessionValid(user2SessionId) ? "true" : "false");
    printf("Waiting for %d seconds to test timeout...\n", SESSION_TIMEOUT_SECONDS + 1);
    sleep(SESSION_TIMEOUT_SECONDS + 1);
    printf("Is user2's session valid after timeout? %s\n", isSessionValid(user2SessionId) ? "true" : "false");

    // Test Case 5: Create multiple sessions
    printf("\n--- Test Case 5: Multiple Sessions ---\n");
    const char* user3SessionIdPtr = createSession("user3");
    char user3SessionId[SESSION_ID_LEN + 1];
    if (user3SessionIdPtr) strncpy(user3SessionId, user3SessionIdPtr, sizeof(user3SessionId));
    user3SessionId[SESSION_ID_LEN] = '\0';
    const char* user4SessionIdPtr = createSession("user4");
    char user4SessionId[SESSION_ID_LEN + 1];
    if (user4SessionIdPtr) strncpy(user4SessionId, user4SessionIdPtr, sizeof(user4SessionId));
    user4SessionId[SESSION_ID_LEN] = '\0';
    printf("Is user3's session valid? %s\n", isSessionValid(user3SessionId) ? "true" : "false");
    printf("Is user4's session valid? %s\n", isSessionValid(user4SessionId) ? "true" : "false");
    invalidateSession(user3SessionId);
    printf("Is user3's session valid after invalidation? %s\n", isSessionValid(user3SessionId) ? "true" : "false");
    printf("Is user4's session still valid? %s\n", isSessionValid(user4SessionId) ? "true" : "false");

    return 0;
}