#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>

// --- Configuration ---
#define SESSION_ID_BYTES 24
#define SESSION_ID_HEX_LEN (SESSION_ID_BYTES * 2)
#define MAX_USERNAME_LEN 50
// Use a short timeout for demonstration purposes
#define SESSION_TIMEOUT_SECONDS 10

// --- Session Structure ---
typedef struct Session {
    char sessionId[SESSION_ID_HEX_LEN + 1];
    char username[MAX_USERNAME_LEN];
    time_t creationTime;
    time_t lastAccessedTime;
    struct Session* next;
} Session;

// In-memory session store (a simple linked list)
// In a real multi-threaded app, access to this list must be protected by a mutex.
static Session* g_session_list_head = NULL;


// Securely generates a random hex string. Returns true on success.
// NOTE: This implementation is POSIX-specific (uses /dev/urandom).
bool generate_session_id(char* hex_buffer, size_t buffer_len) {
    if (buffer_len < SESSION_ID_HEX_LEN + 1) return false;
    
    unsigned char random_bytes[SESSION_ID_BYTES];
    int rand_fd = open("/dev/urandom", O_RDONLY);
    if (rand_fd < 0) {
        perror("Failed to open /dev/urandom");
        return false;
    }
    
    ssize_t bytes_read = read(rand_fd, random_bytes, sizeof(random_bytes));
    close(rand_fd);
    
    if (bytes_read != sizeof(random_bytes)) {
        fprintf(stderr, "Failed to read enough bytes from /dev/urandom\n");
        return false;
    }
    
    for (size_t i = 0; i < SESSION_ID_BYTES; ++i) {
        sprintf(&hex_buffer[i * 2], "%02x", random_bytes[i]);
    }
    hex_buffer[SESSION_ID_HEX_LEN] = '\0';
    return true;
}

// Internal helper to remove a session node from the linked list and free it.
void remove_session_from_list(const char* session_id) {
    Session* current = g_session_list_head;
    Session* prev = NULL;
    while (current != NULL) {
        if (strcmp(current->sessionId, session_id) == 0) {
            if (prev == NULL) {
                g_session_list_head = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

/**
 * Creates a new session.
 * Returns a new heap-allocated string with the session ID. The caller MUST free this string.
 * Returns NULL on failure.
 */
char* create_session(const char* username) {
    if (username == NULL || strlen(username) >= MAX_USERNAME_LEN) return NULL;

    Session* new_session = (Session*)malloc(sizeof(Session));
    if (new_session == NULL) {
        perror("Failed to allocate memory for new session");
        return NULL;
    }

    if (!generate_session_id(new_session->sessionId, sizeof(new_session->sessionId))) {
        free(new_session);
        return NULL;
    }

    strncpy(new_session->username, username, MAX_USERNAME_LEN - 1);
    new_session->username[MAX_USERNAME_LEN - 1] = '\0';

    time_t now = time(NULL);
    new_session->creationTime = now;
    new_session->lastAccessedTime = now;

    new_session->next = g_session_list_head;
    g_session_list_head = new_session;
    
    return strdup(new_session->sessionId);
}

/**
 * Validates a session ID and retrieves the username.
 * Returns a new heap-allocated string with the username if valid. The caller MUST free this string.
 * Returns NULL if session is not found or has expired.
 */
char* get_session_user(const char* session_id) {
    if (session_id == NULL) return NULL;
    
    Session* current = g_session_list_head;
    while (current != NULL) {
        if (strcmp(current->sessionId, session_id) == 0) {
            time_t now = time(NULL);
            if (difftime(now, current->lastAccessedTime) > SESSION_TIMEOUT_SECONDS) {
                remove_session_from_list(session_id);
                return NULL;
            }
            current->lastAccessedTime = now;
            return strdup(current->username);
        }
        current = current->next;
    }
    return NULL;
}

/**
 * Invalidates (deletes) a session.
 */
void invalidate_session(const char* session_id) {
    if (session_id != NULL) {
        remove_session_from_list(session_id);
    }
}

// Frees all memory used by the session store.
void cleanup_all_sessions() {
    Session* current = g_session_list_head;
    while (current != NULL) {
        Session* next = current->next;
        free(current);
        current = next;
    }
    g_session_list_head = NULL;
}

int main() {
    printf("--- Session Management Test ---\n");
    char* alice_session_id = NULL;
    char* bob_session_id = NULL;
    char* charlie_session_id = NULL;
    char* dave_session_id = NULL;
    char* username = NULL;

    // Test Case 1: Create and validate a session
    printf("\n--- Test Case 1: Create and Validate Session ---\n");
    alice_session_id = create_session("alice");
    if (alice_session_id) {
        printf("Created session for 'alice': %s\n", alice_session_id);
        username = get_session_user(alice_session_id);
        if (username) {
            printf("Session is valid. User: %s\n", username);
            free(username);
        } else {
            printf("Session is invalid.\n");
        }
    }

    // Test Case 2: Invalidate a session
    printf("\n--- Test Case 2: Invalidate Session ---\n");
    bob_session_id = create_session("bob");
    if (bob_session_id) {
        printf("Created session for 'bob': %s\n", bob_session_id);
        invalidate_session(bob_session_id);
        printf("Invalidated session for 'bob'.\n");
        username = get_session_user(bob_session_id);
        if (username) {
            printf("Error: Session for 'bob' should be invalid.\n");
            free(username);
        } else {
            printf("Session for 'bob' is correctly invalidated.\n");
        }
        free(bob_session_id);
    }

    // Test Case 3: Session timeout
    printf("\n--- Test Case 3: Session Timeout ---\n");
    charlie_session_id = create_session("charlie");
    if(charlie_session_id) {
        printf("Created session for 'charlie': %s\n", charlie_session_id);
        printf("Waiting for session to time out (more than %d seconds)...\n", SESSION_TIMEOUT_SECONDS);
        sleep(SESSION_TIMEOUT_SECONDS + 1);
        username = get_session_user(charlie_session_id);
        if (username) {
            printf("Error: Session for 'charlie' should have timed out.\n");
            free(username);
        } else {
            printf("Session for 'charlie' correctly timed out.\n");
        }
        free(charlie_session_id);
    }

    // Test Case 4: Accessing invalid/non-existent session
    printf("\n--- Test Case 4: Access Invalid Session ---\n");
    username = get_session_user("invalid-session-id");
    if (username) {
        printf("Error: A non-existent session was found.\n");
        free(username);
    } else {
        printf("Correctly handled non-existent session ID.\n");
    }
    
    // Test Case 5: Session activity extends lifetime
    printf("\n--- Test Case 5: Session Activity Extends Lifetime ---\n");
    dave_session_id = create_session("dave");
    if (dave_session_id) {
        printf("Created session for 'dave': %s\n", dave_session_id);
        printf("Waiting for %d seconds...\n", SESSION_TIMEOUT_SECONDS - 3);
        sleep(SESSION_TIMEOUT_SECONDS - 3);
        username = get_session_user(dave_session_id);
        if (username) {
            printf("Accessed session for 'dave' successfully. Lifetime should be extended.\n");
            free(username);
        } else {
            printf("Error: Session for 'dave' expired prematurely.\n");
        }
        
        printf("Waiting for another %d seconds...\n", SESSION_TIMEOUT_SECONDS - 3);
        sleep(SESSION_TIMEOUT_SECONDS - 3);
        username = get_session_user(dave_session_id);
        if (username) {
            printf("Session for 'dave' is still valid after %d seconds due to activity.\n", 2 * (SESSION_TIMEOUT_SECONDS - 3));
            free(username);
        } else {
            printf("Error: Session for 'dave' expired despite recent activity.\n");
        }
    }
    
    free(alice_session_id);
    free(dave_session_id);
    cleanup_all_sessions();
    printf("\nAll sessions cleaned up.\n");

    return 0;
}