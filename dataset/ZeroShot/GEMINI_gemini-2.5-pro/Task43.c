#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

/*
 * NOTE: This is a highly simplified simulation of session management logic.
 * C is not suited for writing a modern, secure web application without
 * extensive external libraries (e.g., for HTTP, crypto, data structures).
 *
 * SECURITY WARNINGS:
 * 1. The random session ID generation using rand() is NOT cryptographically
 *    secure and is predictable. It is used for demonstration only. Production
 *    code MUST use a proper source of entropy (e.g., /dev/urandom on Linux,
 *    or a library like libsodium/OpenSSL).
 * 2. The session store is a fixed-size array, which is not scalable, not
 *    efficient for lookups (O(n)), and vulnerable to denial-of-service if
 *    the limit is reached. A proper hash map implementation would be required
 *    in a real application.
 * 3. This code is not thread-safe.
 */

#define MAX_SESSIONS 100
#define SESSION_ID_LEN 33 // 32 hex chars + null terminator
#define USERNAME_LEN 50

typedef struct {
    char sessionId[SESSION_ID_LEN];
    char username[USERNAME_LEN];
    int active;
} Session;

// Global in-memory session store
Session sessionStore[MAX_SESSIONS];
int sessionCount = 0;

// Initializes the session store.
void initialize_session_store() {
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        sessionStore[i].active = 0;
        memset(sessionStore[i].sessionId, 0, SESSION_ID_LEN);
        memset(sessionStore[i].username, 0, USERNAME_LEN);
    }
    // Seed the insecure random number generator
    srand((unsigned int)time(NULL));
}

// WARNING: Insecure pseudo-random string generator. For demonstration only.
void generate_insecure_session_id(char* buffer) {
    const char* hex_chars = "0123456789abcdef";
    for (int i = 0; i < SESSION_ID_LEN - 1; ++i) {
        buffer[i] = hex_chars[rand() % 16];
    }
    buffer[SESSION_ID_LEN - 1] = '\0';
}

// Creates a session for a user. Returns the session ID or NULL on failure.
const char* create_session(const char* username) {
    if (sessionCount >= MAX_SESSIONS) {
        fprintf(stderr, "Error: Maximum number of sessions reached.\n");
        return NULL; // Store is full
    }
    if (username == NULL || strlen(username) >= USERNAME_LEN) {
        fprintf(stderr, "Error: Invalid username.\n");
        return NULL;
    }

    int session_index = -1;
    for(int i = 0; i < MAX_SESSIONS; ++i) {
        if (!sessionStore[i].active) {
            session_index = i;
            break;
        }
    }

    if (session_index != -1) {
        generate_insecure_session_id(sessionStore[session_index].sessionId);
        strncpy(sessionStore[session_index].username, username, USERNAME_LEN - 1);
        sessionStore[session_index].active = 1;
        sessionCount++;
        return sessionStore[session_index].sessionId;
    }
    
    return NULL; // Should not happen if sessionCount is managed correctly
}

// Finds a user by session ID. Returns username or NULL if not found/invalid.
const char* get_session_user(const char* session_id) {
    if (session_id == NULL) return NULL;

    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (sessionStore[i].active && strncmp(sessionStore[i].sessionId, session_id, SESSION_ID_LEN) == 0) {
            return sessionStore[i].username;
        }
    }
    return NULL;
}

// Invalidates a session.
void invalidate_session(const char* session_id) {
    if (session_id == NULL) return;

    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (sessionStore[i].active && strncmp(sessionStore[i].sessionId, session_id, SESSION_ID_LEN) == 0) {
            sessionStore[i].active = 0;
            sessionCount--;
            return;
        }
    }
}

int main() {
    initialize_session_store();
    printf("--- Secure Session Management Test (Simplified C Demo) ---\n");

    // Test Case 1: Create a session for user 'alice'
    printf("\n1. Creating session for 'alice'...\n");
    const char* alice_session_id_ptr = create_session("alice");
    assert(alice_session_id_ptr != NULL);
    char alice_session_id[SESSION_ID_LEN];
    strncpy(alice_session_id, alice_session_id_ptr, SESSION_ID_LEN);
    printf("   Alice's session ID created: %s\n", alice_session_id);
    const char* user = get_session_user(alice_session_id);
    printf("   Verified user for session ID: %s\n", user);
    assert(user != NULL && strcmp(user, "alice") == 0);

    // Test Case 2: Create a session for user 'bob'
    printf("\n2. Creating session for 'bob'...\n");
    const char* bob_session_id_ptr = create_session("bob");
    assert(bob_session_id_ptr != NULL);
    char bob_session_id[SESSION_ID_LEN];
    strncpy(bob_session_id, bob_session_id_ptr, SESSION_ID_LEN);
    printf("   Bob's session ID created: %s\n", bob_session_id);
    user = get_session_user(bob_session_id);
    printf("   Verified user for session ID: %s\n", user);
    assert(user != NULL && strcmp(user, "bob") == 0);

    // Test Case 3: Validate an invalid/non-existent session ID
    printf("\n3. Validating a non-existent session ID...\n");
    const char* fake_session_id = "invalid-session-id-12345";
    user = get_session_user(fake_session_id);
    printf("   User for fake session ID: %s\n", user == NULL ? "null" : user);
    assert(user == NULL);

    // Test Case 4: Invalidate Alice's session and try to use it again
    printf("\n4. Invalidating Alice's session...\n");
    invalidate_session(alice_session_id);
    printf("   Alice's session has been invalidated.\n");
    user = get_session_user(alice_session_id);
    printf("   Attempting to verify Alice's old session ID: %s\n", user == NULL ? "null" : user);
    assert(user == NULL);

    // Test Case 5: Ensure Bob's session is still valid
    printf("\n5. Verifying Bob's session is still active...\n");
    user = get_session_user(bob_session_id);
    printf("   User for Bob's session ID: %s\n", user);
    assert(user != NULL && strcmp(user, "bob") == 0);

    printf("\n--- All test cases passed ---\n");

    return 0;
}