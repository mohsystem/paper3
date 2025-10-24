#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

// For POSIX systems, to read from /dev/urandom for secure random numbers.
// For Windows, you would use the CryptoAPI (e.g., CryptGenRandom).
#include <fcntl.h>
#include <unistd.h>

#define MAX_SESSIONS 100
#define SESSION_ID_LEN 32 // Length of the hex string for the session ID
#define SESSION_ID_BYTES (SESSION_ID_LEN / 2) // Bytes needed for the hex string
#define USER_ID_MAX_LEN 50
#define SESSION_DURATION_SECONDS 1800 // 30 minutes

typedef struct {
    char sessionId[SESSION_ID_LEN + 1];
    char userId[USER_ID_MAX_LEN + 1];
    time_t expirationTime;
    bool isActive;
} Session;

// Global store for sessions. In a real application, this would be in a database
// or a more robust in-memory store, with proper mutex/locking for thread safety.
Session session_store[MAX_SESSIONS];

// Initialize the session store
void initialize_session_store() {
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        session_store[i].isActive = false;
    }
}

// Generates a cryptographically secure random session ID.
// Returns 0 on success, -1 on failure.
int generate_secure_session_id(char *hex_buffer) {
    unsigned char random_bytes[SESSION_ID_BYTES];
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1) {
        perror("Error opening /dev/urandom");
        return -1;
    }

    ssize_t bytes_read = read(fd, random_bytes, sizeof(random_bytes));
    close(fd);

    if (bytes_read != sizeof(random_bytes)) {
        fprintf(stderr, "Error reading from /dev/urandom\n");
        return -1;
    }

    for (size_t i = 0; i < SESSION_ID_BYTES; ++i) {
        sprintf(hex_buffer + (i * 2), "%02x", random_bytes[i]);
    }
    hex_buffer[SESSION_ID_LEN] = '\0';
    return 0;
}

// Creates a new session for a user.
// Returns the session ID on success, NULL on failure (e.g., no space or bad input).
const char* create_session(const char* userId) {
    if (userId == NULL || strlen(userId) == 0 || strlen(userId) > USER_ID_MAX_LEN) {
        return NULL;
    }

    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (!session_store[i].isActive) {
            if (generate_secure_session_id(session_store[i].sessionId) != 0) {
                fprintf(stderr, "Failed to generate session ID.\n");
                return NULL;
            }
            strncpy(session_store[i].userId, userId, USER_ID_MAX_LEN);
            session_store[i].userId[USER_ID_MAX_LEN] = '\0';
            session_store[i].expirationTime = time(NULL) + SESSION_DURATION_SECONDS;
            session_store[i].isActive = true;
            return session_store[i].sessionId;
        }
    }
    return NULL; // No available session slots
}

// Finds a user by session ID.
// Returns a pointer to the user ID string if the session is valid, otherwise NULL.
// The returned pointer is valid until the session is ended or expires.
const char* get_session_user(const char* sessionId) {
    if (sessionId == NULL) return NULL;

    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (session_store[i].isActive && strcmp(session_store[i].sessionId, sessionId) == 0) {
            if (time(NULL) > session_store[i].expirationTime) {
                // Session expired, clean it up
                session_store[i].isActive = false;
                return NULL;
            }
            return session_store[i].userId;
        }
    }
    return NULL; // Session not found
}

// Ends a session.
void end_session(const char* sessionId) {
    if (sessionId == NULL) return;

    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (session_store[i].isActive && strcmp(session_store[i].sessionId, sessionId) == 0) {
            session_store[i].isActive = false;
            return;
        }
    }
}

int main() {
    initialize_session_store();
    printf("--- Session Management Test Cases ---\n");

    // Test Case 1: Create a new session for 'dave'
    printf("\n1. Creating session for user 'dave'...\n");
    const char* daveSessionId = create_session("dave");
    if (daveSessionId) {
        printf("   Session created for dave with ID: %s\n", daveSessionId);
    } else {
        printf("   Failed to create session for dave.\n");
        return 1;
    }
    // We need a mutable copy for later use because daveSessionId points to the store
    char daveSessionIdCopy[SESSION_ID_LEN + 1];
    strcpy(daveSessionIdCopy, daveSessionId);

    // Test Case 2: Validate 'dave's' session
    printf("\n2. Validating dave's session...\n");
    const char* userId = get_session_user(daveSessionIdCopy);
    if (userId) {
        printf("   Success! Session is valid for user: %s\n", userId);
    } else {
        printf("   Failure! Session is not valid.\n");
    }

    // Test Case 3: Validate a non-existent session ID
    printf("\n3. Validating a fake session ID...\n");
    const char* fakeSessionId = "fake-session-id-0123456789abcdef";
    userId = get_session_user(fakeSessionId);
    if (userId) {
        printf("   Failure! Fake session was validated for user: %s\n", userId);
    } else {
        printf("   Success! Fake session was correctly identified as invalid.\n");
    }

    // Test Case 4: End 'dave's' session
    printf("\n4. Ending dave's session...\n");
    end_session(daveSessionIdCopy);
    printf("   Session %s has been ended.\n", daveSessionIdCopy);

    // Test Case 5: Re-validate 'dave's' ended session
    printf("\n5. Re-validating dave's ended session...\n");
    userId = get_session_user(daveSessionIdCopy);
    if (userId) {
        printf("   Failure! Ended session was validated for user: %s\n", userId);
    } else {
        printf("   Success! Ended session is no longer valid.\n");
    }

    return 0;
}