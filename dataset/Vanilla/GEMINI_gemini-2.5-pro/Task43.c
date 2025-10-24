#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Note: C does not have a standard library for creating a web application.
// This code simulates the core logic of a secure session manager that would
// be part of a web application backend. It does not run a live server.

#define MAX_SESSIONS 100
#define SESSION_ID_LEN 33 // 32 hex chars + null terminator
#define USERNAME_LEN 50
#define SESSION_DURATION_SECONDS 300 // 5 minutes

typedef struct {
    char session_id[SESSION_ID_LEN];
    char username[USERNAME_LEN];
    time_t expires;
    int active;
} Session;

// In-memory session store
Session session_store[MAX_SESSIONS];

// Initializes the session store
void initialize_session_store() {
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        session_store[i].active = 0;
    }
}

// Generates a random session ID.
// WARNING: Uses rand(), which is not cryptographically secure.
// For a real application, a proper CSPRNG library should be used.
void generate_session_id(char* buffer) {
    const char* hex_chars = "0123456789abcdef";
    for (int i = 0; i < SESSION_ID_LEN - 1; ++i) {
        buffer[i] = hex_chars[rand() % 16];
    }
    buffer[SESSION_ID_LEN - 1] = '\0';
}

// Creates a new session for a user
const char* create_session(const char* username) {
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (!session_store[i].active) {
            session_store[i].active = 1;
            generate_session_id(session_store[i].session_id);
            strncpy(session_store[i].username, username, USERNAME_LEN - 1);
            session_store[i].username[USERNAME_LEN - 1] = '\0';
            session_store[i].expires = time(NULL) + SESSION_DURATION_SECONDS;
            return session_store[i].session_id;
        }
    }
    return NULL; // No available session slots
}

// Finds a session by its ID and returns a pointer to it if valid
Session* get_session(const char* session_id) {
    if (session_id == NULL) return NULL;

    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (session_store[i].active && strcmp(session_store[i].session_id, session_id) == 0) {
            if (time(NULL) > session_store[i].expires) {
                // Session expired, deactivate it
                session_store[i].active = 0;
                return NULL;
            }
            return &session_store[i];
        }
    }
    return NULL; // Session not found
}

// Terminates a session
void terminate_session(const char* session_id) {
     if (session_id == NULL) return;
     for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (session_store[i].active && strcmp(session_store[i].session_id, session_id) == 0) {
            session_store[i].active = 0;
            return;
        }
    }
}

int main() {
    // Seed the random number generator
    srand((unsigned int)time(NULL));
    initialize_session_store();
    
    printf("--- Session Management Test Cases ---\n");

    // Test Case 1: Create a new session for 'testuser'
    printf("\n1. Creating session for 'testuser'...\n");
    const char* sid = create_session("testuser");
    if (sid) {
        printf("   Session created with ID: %s\n", sid);
    } else {
        printf("   Failed to create session.\n");
        return 1;
    }
    // We need to copy the session ID because `sid` points to memory that might be reused.
    char session_id_copy[SESSION_ID_LEN];
    strncpy(session_id_copy, sid, SESSION_ID_LEN);

    // Test Case 2: Validate the session and get the user
    printf("\n2. Validating session...\n");
    Session* s = get_session(session_id_copy);
    if (s) {
        printf("   Success: Session is valid for user '%s'.\n", s->username);
    } else {
        printf("   Failure: Session is not valid.\n");
    }

    // Test Case 3: Try to validate a non-existent session
    printf("\n3. Validating a fake session ID...\n");
    s = get_session("fake_session_id");
    if (s) {
        printf("   Failure: Fake session was somehow valid for user '%s'.\n", s->username);
    } else {
        printf("   Success: Fake session is correctly identified as invalid.\n");
    }

    // Test Case 4: Terminate the session (logout)
    printf("\n4. Terminating session...\n");
    terminate_session(session_id_copy);
    printf("   Session with ID %s terminated.\n", session_id_copy);

    // Test Case 5: Try to validate the terminated session
    printf("\n5. Validating terminated session...\n");
    s = get_session(session_id_copy);
    if (s) {
        printf("   Failure: Terminated session was somehow still valid for user '%s'.\n", s->username);
    } else {
        printf("   Success: Terminated session is correctly identified as invalid.\n");
    }

    return 0;
}