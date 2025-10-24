#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <openssl/rand.h>
#include <openssl/crypto.h>

#define SESSION_ID_BYTES 32
#define SESSION_ID_HEX_LEN (SESSION_ID_BYTES * 2)
#define SESSION_DURATION_SECONDS 1800 // 30 minutes
#define MAX_USERNAME_LEN 256

// Data structure for a single session
typedef struct {
    char username[MAX_USERNAME_LEN];
    time_t expiration;
} Session;

// Node for the linked list of sessions
typedef struct SessionNode {
    char session_id_hex[SESSION_ID_HEX_LEN + 1];
    Session data;
    struct SessionNode* next;
} SessionNode;

// Global head of the session list
// In a real multi-threaded app, this would need a mutex.
static SessionNode* g_session_list_head = NULL;

// Helper function to convert bytes to a hex string
// Returns 0 on success, -1 on failure
int bytes_to_hex(const unsigned char* data, size_t data_len, char* hex_str, size_t hex_str_len) {
    if (hex_str_len < (data_len * 2 + 1)) {
        return -1; // Not enough space
    }
    for (size_t i = 0; i < data_len; i++) {
        // snprintf is safer than sprintf
        if (snprintf(hex_str + (i * 2), 3, "%02x", data[i]) < 0) {
            return -1;
        }
    }
    hex_str[data_len * 2] = '\0';
    return 0;
}

// Safely free a session node
void free_session_node(SessionNode* node) {
    if (node) {
        // Securely zero out the session ID before freeing
        OPENSSL_cleanse(node->session_id_hex, sizeof(node->session_id_hex));
        free(node);
    }
}

// Finds a session by ID. Does NOT check for expiration.
// Uses constant-time comparison.
SessionNode* find_session_node(const char* session_id) {
    if (!session_id) return NULL;
    
    SessionNode* current = g_session_list_head;
    while (current != NULL) {
        // Constant time comparison to prevent timing attacks
        if (CRYPTO_memcmp(current->session_id_hex, session_id, SESSION_ID_HEX_LEN) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}


// Creates a new session for a user.
// Returns a dynamically allocated session ID string on success (caller must free).
// Returns NULL on failure.
char* create_session(const char* username) {
    if (!username || strlen(username) >= MAX_USERNAME_LEN) {
        return NULL;
    }

    unsigned char session_id_bytes[SESSION_ID_BYTES];
    char* session_id_hex = (char*)malloc(SESSION_ID_HEX_LEN + 1);
    if (!session_id_hex) {
        return NULL; // Malloc failed
    }

    // Generate a unique session ID
    do {
        if (RAND_bytes(session_id_bytes, sizeof(session_id_bytes)) != 1) {
            free(session_id_hex);
            return NULL; // RNG failure
        }
        if (bytes_to_hex(session_id_bytes, sizeof(session_id_bytes), session_id_hex, SESSION_ID_HEX_LEN + 1) != 0) {
             free(session_id_hex);
             return NULL; // Hex conversion failed
        }
    } while (find_session_node(session_id_hex) != NULL); // Extremely unlikely collision

    SessionNode* new_node = (SessionNode*)malloc(sizeof(SessionNode));
    if (!new_node) {
        free(session_id_hex);
        return NULL;
    }

    strncpy(new_node->session_id_hex, session_id_hex, SESSION_ID_HEX_LEN);
    new_node->session_id_hex[SESSION_ID_HEX_LEN] = '\0';
    
    strncpy(new_node->data.username, username, MAX_USERNAME_LEN - 1);
    new_node->data.username[MAX_USERNAME_LEN - 1] = '\0';

    new_node->data.expiration = time(NULL) + SESSION_DURATION_SECONDS;
    
    // Add to the head of the list
    new_node->next = g_session_list_head;
    g_session_list_head = new_node;

    return session_id_hex;
}

// Retrieves the username for a given session ID.
// Returns a dynamically allocated username string (caller must free).
// Returns NULL if the session is not found or is expired.
char* get_session_user(const char* session_id) {
    if (!session_id || strlen(session_id) != SESSION_ID_HEX_LEN) {
        return NULL;
    }

    SessionNode* node = find_session_node(session_id);

    if (node) {
        if (time(NULL) > node->data.expiration) {
            // Session expired.
            return NULL;
        }

        char* username_copy = (char*)malloc(strlen(node->data.username) + 1);
        if (username_copy) {
            strcpy(username_copy, node->data.username);
        }
        return username_copy;
    }

    return NULL;
}

// Ends a session.
// Returns 1 if the session was found and removed, 0 otherwise.
int end_session(const char* session_id) {
     if (!session_id || strlen(session_id) != SESSION_ID_HEX_LEN) {
        return 0;
    }

    SessionNode* current = g_session_list_head;
    SessionNode* prev = NULL;

    while (current != NULL) {
        if (CRYPTO_memcmp(current->session_id_hex, session_id, SESSION_ID_HEX_LEN) == 0) {
            if (prev) {
                prev->next = current->next;
            } else { // It's the head of the list
                g_session_list_head = current->next;
            }
            free_session_node(current);
            return 1;
        }
        prev = current;
        current = current->next;
    }

    return 0;
}

// Cleanup function to free all sessions
void cleanup_all_sessions() {
    SessionNode* current = g_session_list_head;
    while (current != NULL) {
        SessionNode* to_delete = current;
        current = current->next;
        free_session_node(to_delete);
    }
    g_session_list_head = NULL;
}


int main() {
    printf("--- Test Case 1: Create and validate a session ---\n");
    const char* user1 = "alice";
    char* session1_id = create_session(user1);
    if (session1_id) {
        printf("Created session for %s.\n", user1);
        char* found_user = get_session_user(session1_id);
        if (found_user) {
            printf("Session ID belongs to: %s\n", found_user);
            if (strcmp(found_user, user1) != 0) {
                printf("TEST FAILED: User mismatch.\n");
            } else {
                printf("TEST PASSED\n");
            }
            free(found_user);
        } else {
            fprintf(stderr, "TEST FAILED: Could not retrieve user for valid session.\n");
        }
    } else {
        fprintf(stderr, "TEST FAILED: Session creation failed.\n");
    }
    printf("\n");

    printf("--- Test Case 2: Validate a non-existent session ---\n");
    const char* non_existent_id = "0000000000000000000000000000000000000000000000000000000000000000";
    char* found_user2 = get_session_user(non_existent_id);
    printf("Searching for non-existent session ID...\n");
    if (found_user2 == NULL) {
        printf("Session not found, as expected.\n");
        printf("TEST PASSED\n");
    } else {
        fprintf(stderr, "TEST FAILED: Found a user for a non-existent session.\n");
        free(found_user2);
    }
    printf("\n");

    printf("--- Test Case 3: Create and then end a session ---\n");
    const char* user2 = "bob";
    char* session2_id = create_session(user2);
    if (session2_id) {
        printf("Created session for %s.\n", user2);
        int ended = end_session(session2_id);
        if (ended) {
            printf("Session ended successfully.\n");
            char* found_user3 = get_session_user(session2_id);
            if (found_user3 == NULL) {
                printf("Verified session is gone.\n");
                printf("TEST PASSED\n");
            } else {
                fprintf(stderr, "TEST FAILED: Session still exists after being ended.\n");
                free(found_user3);
            }
        } else {
            fprintf(stderr, "TEST FAILED: Failed to end session.\n");
        }
        free(session2_id);
        session2_id = NULL;
    } else {
        fprintf(stderr, "TEST FAILED: Session creation failed.\n");
    }
    printf("\n");

    printf("--- Test Case 4: Try to end a session that doesn't exist ---\n");
    int ended_non_existent = end_session(non_existent_id);
    if (!ended_non_existent) {
        printf("Attempt to end non-existent session failed, as expected.\n");
        printf("TEST PASSED\n");
    } else {
        fprintf(stderr, "TEST FAILED: end_session returned true for a non-existent session.\n");
    }
    printf("\n");
    
    printf("--- Test Case 5: Create session for another user ---\n");
    const char* user3 = "charlie";
    char* session3_id = create_session(user3);
    if (session3_id) {
        printf("Created session for %s.\n", user3);
        char* found_user4 = get_session_user(session3_id);
        if (found_user4) {
            printf("Session ID belongs to: %s\n", found_user4);
            if (strcmp(found_user4, user3) == 0) {
                char* still_found_user1 = get_session_user(session1_id);
                if (still_found_user1) {
                    if(strcmp(still_found_user1, user1) == 0) {
                        printf("Original session for 'alice' is still valid.\n");
                        printf("TEST PASSED\n");
                    } else {
                        fprintf(stderr, "TEST FAILED: User mismatch for 'alice'.\n");
                    }
                    free(still_found_user1);
                } else {
                     fprintf(stderr, "TEST FAILED: Original session for 'alice' is gone.\n");
                }
            } else {
                 fprintf(stderr, "TEST FAILED: User mismatch for 'charlie'.\n");
            }
            free(found_user4);
        } else {
            fprintf(stderr, "TEST FAILED: Session validation failed for 'charlie'.\n");
        }
        free(session3_id);
    } else {
        fprintf(stderr, "TEST FAILED: Session creation failed for 'charlie'.\n");
    }
    
    free(session1_id);
    cleanup_all_sessions();

    return 0;
}