#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h> // for sleep

#include <openssl/rand.h>
#include <openssl/crypto.h>

#define MAX_SESSIONS 100
#define SESSION_ID_BYTES 32
#define SESSION_ID_HEX_LEN (SESSION_ID_BYTES * 2)
#define MAX_USER_ID_LEN 64

// Global session state
static int g_session_timeout_seconds = 1800; // 30 minutes

typedef struct {
    char sessionId[SESSION_ID_HEX_LEN + 1];
    char userId[MAX_USER_ID_LEN + 1];
    time_t expiration;
    bool in_use;
} Session;

static Session g_sessions[MAX_SESSIONS];
static pthread_mutex_t g_session_mutex = PTHREAD_MUTEX_INITIALIZER;

void initialize_session_store() {
    pthread_mutex_lock(&g_session_mutex);
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        g_sessions[i].in_use = false;
        memset(g_sessions[i].sessionId, 0, sizeof(g_sessions[i].sessionId));
        memset(g_sessions[i].userId, 0, sizeof(g_sessions[i].userId));
    }
    pthread_mutex_unlock(&g_session_mutex);
}

// Helper to convert bytes to a hex string
void bytes_to_hex(const unsigned char* bytes, size_t len, char* hex_str) {
    for (size_t i = 0; i < len; ++i) {
        sprintf(hex_str + (i * 2), "%02x", bytes[i]);
    }
    hex_str[len * 2] = '\0';
}

char* create_session(const char* user_id) {
    if (user_id == NULL || strlen(user_id) == 0 || strlen(user_id) > MAX_USER_ID_LEN) {
        return NULL;
    }

    unsigned char session_bytes[SESSION_ID_BYTES];
    if (RAND_bytes(session_bytes, sizeof(session_bytes)) != 1) {
        return NULL;
    }

    char* session_id_hex = malloc(SESSION_ID_HEX_LEN + 1);
    if (!session_id_hex) {
        return NULL;
    }
    bytes_to_hex(session_bytes, SESSION_ID_BYTES, session_id_hex);

    pthread_mutex_lock(&g_session_mutex);

    int free_slot = -1;
    time_t now = time(NULL);

    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (!g_sessions[i].in_use || g_sessions[i].expiration < now) {
            free_slot = i;
            break;
        }
    }

    if (free_slot != -1) {
        strncpy(g_sessions[free_slot].sessionId, session_id_hex, SESSION_ID_HEX_LEN);
        g_sessions[free_slot].sessionId[SESSION_ID_HEX_LEN] = '\0';
        strncpy(g_sessions[free_slot].userId, user_id, MAX_USER_ID_LEN);
        g_sessions[free_slot].userId[MAX_USER_ID_LEN] = '\0';
        g_sessions[free_slot].expiration = now + g_session_timeout_seconds;
        g_sessions[free_slot].in_use = true;
    } else {
        free(session_id_hex);
        session_id_hex = NULL;
    }

    pthread_mutex_unlock(&g_session_mutex);
    return session_id_hex;
}

bool get_session_user(const char* session_id, char* user_id_out, size_t user_id_out_size) {
    if (session_id == NULL || strlen(session_id) != SESSION_ID_HEX_LEN || user_id_out == NULL || user_id_out_size == 0) {
        return false;
    }

    bool found = false;
    pthread_mutex_lock(&g_session_mutex);

    time_t now = time(NULL);
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (g_sessions[i].in_use) {
            if (CRYPTO_memcmp(g_sessions[i].sessionId, session_id, SESSION_ID_HEX_LEN) == 0) {
                if (g_sessions[i].expiration < now) {
                    g_sessions[i].in_use = false;
                } else {
                    strncpy(user_id_out, g_sessions[i].userId, user_id_out_size - 1);
                    user_id_out[user_id_out_size - 1] = '\0';
                    found = true;
                }
                break;
            }
        }
    }

    pthread_mutex_unlock(&g_session_mutex);
    return found;
}

void invalidate_session(const char* session_id) {
    if (session_id == NULL || strlen(session_id) != SESSION_ID_HEX_LEN) {
        return;
    }

    pthread_mutex_lock(&g_session_mutex);
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (g_sessions[i].in_use) {
           if (CRYPTO_memcmp(g_sessions[i].sessionId, session_id, SESSION_ID_HEX_LEN) == 0) {
                g_sessions[i].in_use = false;
                break;
            }
        }
    }
    pthread_mutex_unlock(&g_session_mutex);
}

int main() {
    initialize_session_store();
    char user_id_buffer[MAX_USER_ID_LEN + 1];

    printf("--- Test Case 1: Create and validate a session ---\n");
    const char* user_id_1 = "user-123";
    char* session_id_1 = create_session(user_id_1);
    if (session_id_1) {
        printf("Created session for %s\n", user_id_1);
        if (get_session_user(session_id_1, user_id_buffer, sizeof(user_id_buffer))) {
            printf("Validated session, user is: %s\n", user_id_buffer);
            printf("Test Case 1 Passed: %s\n\n", strcmp(user_id_1, user_id_buffer) == 0 ? "True" : "False");
        } else {
            printf("Test Case 1 Failed: Could not validate session.\n\n");
        }
    } else {
        printf("Test Case 1 Failed: Could not create session.\n\n");
    }

    printf("--- Test Case 2: Invalidate a session ---\n");
    if (session_id_1) {
        invalidate_session(session_id_1);
        bool isValid = get_session_user(session_id_1, user_id_buffer, sizeof(user_id_buffer));
        printf("After invalidation, user is: %s\n", isValid ? user_id_buffer : "Not Found");
        printf("Test Case 2 Passed: %s\n\n", !isValid ? "True" : "False");
    }

    printf("--- Test Case 3: Validate a non-existent session ---\n");
    const char* fake_id = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
    bool isValid3 = get_session_user(fake_id, user_id_buffer, sizeof(user_id_buffer));
    printf("Validating non-existent session, user is: %s\n", isValid3 ? user_id_buffer : "Not Found");
    printf("Test Case 3 Passed: %s\n\n", !isValid3 ? "True" : "False");

    printf("--- Test Case 4: Session expiration ---\n");
    int original_timeout = g_session_timeout_seconds;
    g_session_timeout_seconds = 1; // Set timeout to 1 second
    const char* user_id_4 = "user-456";
    char* session_id_4 = create_session(user_id_4);
    if (session_id_4) {
        printf("Created short-lived session for %s\n", user_id_4);
        printf("Waiting for session to expire...\n");
        sleep(2);
        bool isValid4 = get_session_user(session_id_4, user_id_buffer, sizeof(user_id_buffer));
        printf("After expiration, user is: %s\n", isValid4 ? user_id_buffer : "Not Found");
        printf("Test Case 4 Passed: %s\n\n", !isValid4 ? "True" : "False");
        free(session_id_4);
    } else {
        printf("Test Case 4 Failed: Could not create session.\n\n");
    }
    g_session_timeout_seconds = original_timeout; // Reset timeout

    printf("--- Test Case 5: Handle null/empty inputs ---\n");
    bool test5_passed = true;
    char* null_session = create_session(NULL);
    if (null_session != NULL) {
        test5_passed = false; free(null_session);
    }
    printf("create_session(NULL) returned NULL: %s\n", null_session == NULL ? "True" : "False");
    bool invalid_get = get_session_user(NULL, user_id_buffer, sizeof(user_id_buffer));
    if (invalid_get) test5_passed = false;
    printf("get_session_user(NULL, ...) returned false: %s\n", !invalid_get ? "True" : "False");
    printf("Test Case 5 Passed: %s\n\n", test5_passed ? "True" : "False");
    
    if (session_id_1) free(session_id_1);

    return 0;
}