#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// For POSIX systems (Linux, macOS) to get secure random bytes.
// For Windows, you'd need to use the CryptoAPI (e.g., CryptGenRandom).
#include <fcntl.h>
#include <unistd.h>

#define ABSOLUTE_TIMEOUT_SECONDS 5
#define IDLE_TIMEOUT_SECONDS 2

typedef struct {
    char* userId;
    char* sessionId;
    time_t creationTime;
    time_t lastAccessedTime;
} Session;

// Global session store (for simplicity in this single-file example)
// In a real application, this would be a more robust data structure like a hash map.
// NOTE: This implementation is NOT thread-safe. A real-world C application
// would require mutexes (e.g., pthreads) to protect this shared data.
static Session* session_store = NULL;
static size_t session_count = 0;
static size_t session_capacity = 0;

// A utility function to Base64 encode data.
// Caller is responsible for freeing the returned string.
char* base64_encode_c(const unsigned char* data, size_t input_length) {
    const char base64_chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    size_t output_length = 4 * ((input_length + 2) / 3);
    char* encoded_data = (char*)malloc(output_length + 1);
    if (encoded_data == NULL) return NULL;

    for (size_t i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;
        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = base64_chars[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 0 * 6) & 0x3F];
    }
    
    // Add padding
    for (size_t i = 0; i < (3 - input_length % 3) % 3; i++) {
        encoded_data[output_length - 1 - i] = '=';
    }
    encoded_data[output_length] = '\0';
    return encoded_data;
}


// Generates a secure session ID.
// Caller is responsible for freeing the returned string.
char* create_session_id() {
    unsigned char random_bytes[32];
    int urandom_fd = open("/dev/urandom", O_RDONLY);
    if (urandom_fd == -1) {
        perror("Failed to open /dev/urandom");
        return NULL;
    }
    ssize_t bytes_read = read(urandom_fd, random_bytes, sizeof(random_bytes));
    close(urandom_fd);
    if (bytes_read < sizeof(random_bytes)) {
        fprintf(stderr, "Failed to read enough bytes from /dev/urandom\n");
        return NULL;
    }
    return base64_encode_c(random_bytes, sizeof(random_bytes));
}

char* create_session(const char* userId) {
    if (session_count >= session_capacity) {
        size_t new_capacity = (session_capacity == 0) ? 10 : session_capacity * 2;
        Session* new_store = (Session*)realloc(session_store, new_capacity * sizeof(Session));
        if (new_store == NULL) {
            perror("Failed to reallocate session store");
            return NULL;
        }
        session_store = new_store;
        session_capacity = new_capacity;
    }

    char* new_session_id = create_session_id();
    if (new_session_id == NULL) return NULL;

    Session* new_session = &session_store[session_count];
    new_session->userId = strdup(userId);
    new_session->sessionId = new_session_id; // Already allocated
    new_session->creationTime = time(NULL);
    new_session->lastAccessedTime = new_session->creationTime;

    if (new_session->userId == NULL) {
        perror("Failed to duplicate userId string");
        free(new_session_id);
        return NULL;
    }

    session_count++;
    return new_session->sessionId;
}

void invalidate_session(const char* sessionId) {
    if (sessionId == NULL) return;
    for (size_t i = 0; i < session_count; i++) {
        if (strcmp(session_store[i].sessionId, sessionId) == 0) {
            free(session_store[i].userId);
            free(session_store[i].sessionId);
            // Shift remaining elements to fill the gap
            if (i < session_count - 1) {
                session_store[i] = session_store[session_count - 1];
            }
            session_count--;
            return;
        }
    }
}


Session* get_session(const char* sessionId) {
    if (sessionId == NULL) return NULL;

    time_t now = time(NULL);

    for (size_t i = 0; i < session_count; i++) {
        if (strcmp(session_store[i].sessionId, sessionId) == 0) {
            Session* session = &session_store[i];
            
            if (difftime(now, session->creationTime) > ABSOLUTE_TIMEOUT_SECONDS) {
                invalidate_session(sessionId);
                return NULL;
            }

            if (difftime(now, session->lastAccessedTime) > IDLE_TIMEOUT_SECONDS) {
                invalidate_session(sessionId);
                return NULL;
            }

            session->lastAccessedTime = now;
            return session;
        }
    }
    return NULL;
}

void cleanup_session_manager() {
    for (size_t i = 0; i < session_count; i++) {
        free(session_store[i].userId);
        free(session_store[i].sessionId);
    }
    free(session_store);
    session_store = NULL;
    session_count = 0;
    session_capacity = 0;
}

int main() {
    printf("Running C Session Manager Test Cases...\n");
    printf("Absolute Timeout: %ds, Idle Timeout: %ds\n\n", ABSOLUTE_TIMEOUT_SECONDS, IDLE_TIMEOUT_SECONDS);

    // Test Case 1: Create and validate a new session
    printf("--- Test Case 1: Create and Validate Session ---\n");
    char* sessionId1 = create_session("user123");
    if (sessionId1) {
        printf("Created session for user123: %s\n", sessionId1);
        Session* s1 = get_session(sessionId1);
        printf("Session valid? %s\n", (s1 ? "Yes, for user " : "No"));
        if(s1) printf("%s\n", s1->userId);
    } else {
        printf("Failed to create session.\n");
    }
    printf("\n");

    // Test Case 2: Invalidate a session
    printf("--- Test Case 2: Invalidate Session ---\n");
    invalidate_session(sessionId1);
    printf("Session invalidated.\n");
    Session* s1_after = get_session(sessionId1);
    printf("Session valid after invalidation? %s\n", (s1_after != NULL ? "Yes" : "No"));
    printf("\n");

    // Test Case 3: Idle timeout
    printf("--- Test Case 3: Idle Timeout ---\n");
    char* sessionId2 = create_session("user456");
    if (sessionId2) {
        printf("Created session for user456: %s\n", sessionId2);
        printf("Waiting for %d seconds to trigger idle timeout...\n", IDLE_TIMEOUT_SECONDS + 1);
        sleep(IDLE_TIMEOUT_SECONDS + 1);
        Session* s2 = get_session(sessionId2);
        printf("Session valid after idle period? %s\n", (s2 != NULL ? "Yes" : "No"));
    }
    printf("\n");

    // Test Case 4: Absolute timeout
    printf("--- Test Case 4: Absolute Timeout ---\n");
    char* sessionId3 = create_session("user789");
    if (sessionId3) {
        printf("Created session for user789: %s\n", sessionId3);
        printf("Accessing session every second to prevent idle timeout...\n");
        for (int i = 0; i < ABSOLUTE_TIMEOUT_SECONDS - 1; ++i) {
            sleep(1);
            get_session(sessionId3); // Keep it active
            printf("Accessed at second %d\n", i + 1);
        }
        printf("Waiting for absolute timeout...\n");
        sleep(2);
        Session* s3 = get_session(sessionId3);
        printf("Session valid after absolute timeout period? %s\n", (s3 != NULL ? "Yes" : "No"));
    }
    printf("\n");

    // Test Case 5: Prevent idle timeout by activity
    printf("--- Test Case 5: Prevent Idle Timeout ---\n");
    char* sessionId4 = create_session("userABC");
    if (sessionId4) {
        printf("Created session for userABC: %s\n", sessionId4);
        sleep(IDLE_TIMEOUT_SECONDS - 1);
        printf("Accessing session just before idle timeout...\n");
        Session* s4_active = get_session(sessionId4);
        printf("Session is still active? %s\n", s4_active ? "Yes" : "No");
        
        sleep(IDLE_TIMEOUT_SECONDS - 1);
        s4_active = get_session(sessionId4);
        printf("Accessing again. Session still active? %s\n", s4_active ? "Yes" : "No");
    }
    printf("\n");

    cleanup_session_manager();
    printf("Session manager cleaned up.\n");

    return 0;
}