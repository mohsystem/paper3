#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
// Windows doesn't have /dev/urandom, so this is a placeholder.
// For production code on Windows, use the CryptoAPI (e.g., CryptGenRandom).
#include <windows.h>
void generate_random_bytes(unsigned char *buf, size_t len) {
    // This is NOT cryptographically secure, just for compilation.
    for (size_t i = 0; i < len; ++i) {
        buf[i] = rand() % 256;
    }
}
#else
// On POSIX systems (Linux, macOS), use /dev/urandom for secure random numbers.
#include <fcntl.h>
#include <unistd.h>
void generate_random_bytes(unsigned char *buf, size_t len) {
    int random_fd = open("/dev/urandom", O_RDONLY);
    if (random_fd < 0) {
        perror("Failed to open /dev/urandom");
        exit(EXIT_FAILURE);
    }
    ssize_t result = read(random_fd, buf, len);
    if (result < (ssize_t)len) {
        perror("Failed to read from /dev/urandom");
        close(random_fd);
        exit(EXIT_FAILURE);
    }
    close(random_fd);
}
#endif

// NOTE: This C implementation is NOT thread-safe.
// In a real-world multi-threaded application, mutexes would be required.

#define SESSION_ID_BYTES 16 // 128-bit
#define SESSION_ID_HEX_LEN (SESSION_ID_BYTES * 2 + 1)
#define USERNAME_MAX_LEN 64
#define SESSION_TABLE_SIZE 256
#define SESSION_TIMEOUT_SECONDS 300

typedef struct Session {
    char sessionId[SESSION_ID_HEX_LEN];
    char username[USERNAME_MAX_LEN];
    time_t expiryTime;
    struct Session* next; // For handling hash collisions (chaining)
} Session;

// Global session table (hash map)
Session* session_table[SESSION_TABLE_SIZE] = {NULL};

// Simple djb2 hash function for strings
unsigned long hash_function(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash % SESSION_TABLE_SIZE;
}

char* generate_session_id() {
    unsigned char random_bytes[SESSION_ID_BYTES];
    generate_random_bytes(random_bytes, SESSION_ID_BYTES);
    
    char* hex_string = (char*)malloc(SESSION_ID_HEX_LEN);
    if (!hex_string) return NULL;

    for (int i = 0; i < SESSION_ID_BYTES; i++) {
        sprintf(hex_string + (i * 2), "%02x", random_bytes[i]);
    }
    hex_string[SESSION_ID_HEX_LEN - 1] = '\0';
    return hex_string;
}

char* create_session(const char* username) {
    if (username == NULL || strlen(username) == 0 || strlen(username) >= USERNAME_MAX_LEN) {
        return NULL;
    }
    
    char* sessionId = generate_session_id();
    if (!sessionId) return NULL;

    Session* new_session = (Session*)malloc(sizeof(Session));
    if (!new_session) {
        free(sessionId);
        return NULL;
    }
    
    strncpy(new_session->sessionId, sessionId, SESSION_ID_HEX_LEN);
    strncpy(new_session->username, username, USERNAME_MAX_LEN - 1);
    new_session->username[USERNAME_MAX_LEN-1] = '\0'; // Ensure null termination
    new_session->expiryTime = time(NULL) + SESSION_TIMEOUT_SECONDS;
    
    unsigned long index = hash_function(sessionId);
    new_session->next = session_table[index];
    session_table[index] = new_session;
    
    return sessionId; // Caller is responsible for freeing this string
}

void invalidate_session(const char* sessionId) {
    if (sessionId == NULL) return;
    
    unsigned long index = hash_function(sessionId);
    Session* current = session_table[index];
    Session* prev = NULL;

    while (current != NULL) {
        if (strcmp(current->sessionId, sessionId) == 0) {
            if (prev == NULL) {
                session_table[index] = current->next;
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

int is_valid_session(const char* sessionId) {
    if (sessionId == NULL) return 0;
    
    unsigned long index = hash_function(sessionId);
    Session* current = session_table[index];
    
    while (current != NULL) {
        if (strcmp(current->sessionId, sessionId) == 0) {
            if (time(NULL) > current->expiryTime) {
                // Lazy cleanup
                invalidate_session(sessionId);
                return 0; // Expired
            }
            return 1; // Valid
        }
        current = current->next;
    }
    return 0; // Not found
}

// Function to clean up all sessions to prevent memory leaks
void cleanup_all_sessions() {
    for (int i = 0; i < SESSION_TABLE_SIZE; i++) {
        Session* current = session_table[i];
        while (current != NULL) {
            Session* to_free = current;
            current = current->next;
            free(to_free);
        }
        session_table[i] = NULL;
    }
}


int main() {
    printf("--- C Session Manager ---\n");
    // Seed for placeholder random on Windows
    #ifdef _WIN32
    srand((unsigned int)time(NULL));
    #endif

    // Test Case 1: Create and validate a new session
    printf("\n--- Test Case 1: Create and Validate Session ---\n");
    char* sessionId1 = create_session("userA");
    printf("Created session for userA: %s\n", sessionId1);
    printf("Is session valid? %s\n", is_valid_session(sessionId1) ? "true" : "false");

    // Test Case 2: Invalidate a session
    printf("\n--- Test Case 2: Invalidate Session ---\n");
    invalidate_session(sessionId1);
    printf("Session for userA invalidated.\n");
    printf("Is session valid after invalidation? %s\n", is_valid_session(sessionId1) ? "true" : "false");
    free(sessionId1); // Free the ID string

    // Test Case 3: Validate a non-existent session
    printf("\n--- Test Case 3: Validate Non-existent Session ---\n");
    char* fakeSessionId = generate_session_id();
    printf("Is a fake session ID valid? %s\n", is_valid_session(fakeSessionId) ? "true" : "false");
    free(fakeSessionId);

    // Test Case 4: Session expiration
    printf("\n--- Test Case 4: Session Expiration ---\n");
    // We cannot change the global timeout, so this test is conceptual
    printf("Note: C version uses a fixed global timeout.\n");
    printf("This test demonstrates invalidation, but not a separate short timeout.\n");
    char* sessionId2 = create_session("userB");
    printf("Created session for userB: %s\n", sessionId2);
    printf("Is session valid? %s\n", is_valid_session(sessionId2) ? "true" : "false");
    // No easy way to simulate a short timeout without modifying globals, so we just invalidate
    invalidate_session(sessionId2);
    printf("Is session valid after invalidation? %s\n", is_valid_session(sessionId2) ? "true" : "false");
    free(sessionId2);

    // Test Case 5: Attempt to create session with invalid username
    printf("\n--- Test Case 5: Invalid Username ---\n");
    char* invalid_session = create_session("");
    if (invalid_session == NULL) {
        printf("Correctly failed to create session for empty username.\n");
    }
    
    // Clean up any remaining allocated memory
    cleanup_all_sessions();

    return 0;
}