
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#include <wincrypt.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

#define SESSION_TIMEOUT 1800
#define SESSION_ID_LENGTH 32
#define MAX_SESSIONS 10000
#define MAX_USER_ID_LENGTH 256

typedef struct {
    char userId[MAX_USER_ID_LENGTH + 1];
    time_t createdAt;
    time_t lastAccessedAt;
    int active;
} Session;

typedef struct {
    char sessionId[SESSION_ID_LENGTH + 1];
    Session session;
} SessionEntry;

typedef struct {
    SessionEntry* entries;
    size_t count;
    size_t capacity;
} SessionManager;

/* Securely clear sensitive memory */
static void secure_clear(void* ptr, size_t size) {
    if (ptr == NULL) return;
#ifdef _WIN32
    SecureZeroMemory(ptr, size);
#elif defined(__STDC_LIB_EXT1__)
    memset_s(ptr, size, 0, size);
#else
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (size--) *p++ = 0;
#endif
}

/* Generate cryptographically secure random bytes */
static int generate_random_bytes(unsigned char* buffer, size_t length) {
    if (buffer == NULL || length == 0) {
        return 0;
    }
    
#ifdef _WIN32
    HCRYPTPROV hProvider = 0;
    if (!CryptAcquireContextW(&hProvider, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
        return 0;
    }
    int result = CryptGenRandom(hProvider, (DWORD)length, buffer);
    CryptReleaseContext(hProvider, 0);
    return result;
#else
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        return 0;
    }
    
    ssize_t result = read(fd, buffer, length);
    close(fd);
    return (result == (ssize_t)length) ? 1 : 0;
#endif
}

/* Validate user ID format */
static int is_valid_user_id(const char* userId) {
    if (userId == NULL) {
        return 0;
    }
    
    size_t len = strnlen(userId, MAX_USER_ID_LENGTH + 1);
    if (len == 0 || len > MAX_USER_ID_LENGTH) {
        return 0;
    }
    
    /* Validate characters: alphanumeric, underscore, hyphen only */
    for (size_t i = 0; i < len; i++) {
        char c = userId[i];
        if (!isalnum((unsigned char)c) && c != '_' && c != '-') {
            return 0;
        }
    }
    return 1;
}

/* Validate session ID format */
static int is_valid_session_id(const char* sessionId) {
    if (sessionId == NULL) {
        return 0;
    }
    
    size_t len = strnlen(sessionId, SESSION_ID_LENGTH + 1);
    if (len != SESSION_ID_LENGTH) {
        return 0;
    }
    
    /* Validate hex characters only */
    for (size_t i = 0; i < len; i++) {
        if (!isxdigit((unsigned char)sessionId[i])) {
            return 0;
        }
    }
    return 1;
}

/* Initialize session manager */
SessionManager* create_session_manager(void) {
    SessionManager* manager = (SessionManager*)calloc(1, sizeof(SessionManager));
    if (manager == NULL) {
        return NULL;
    }
    
    manager->capacity = 16;
    manager->entries = (SessionEntry*)calloc(manager->capacity, sizeof(SessionEntry));
    if (manager->entries == NULL) {
        free(manager);
        return NULL;
    }
    
    manager->count = 0;
    return manager;
}

/* Generate unique session ID */
static int generate_session_id(char* sessionId, size_t size) {
    if (sessionId == NULL || size <= SESSION_ID_LENGTH) {
        return 0;
    }
    
    unsigned char random_bytes[SESSION_ID_LENGTH / 2];
    if (!generate_random_bytes(random_bytes, sizeof(random_bytes))) {
        return 0;
    }
    
    const char hex[] = "0123456789abcdef";
    for (size_t i = 0; i < sizeof(random_bytes); i++) {
        sessionId[i * 2] = hex[(random_bytes[i] >> 4) & 0x0F];
        sessionId[i * 2 + 1] = hex[random_bytes[i] & 0x0F];
    }
    sessionId[SESSION_ID_LENGTH] = '\\0';
    
    secure_clear(random_bytes, sizeof(random_bytes));
    return 1;
}

/* Find session by ID */
static SessionEntry* find_session(SessionManager* manager, const char* sessionId) {
    if (manager == NULL || sessionId == NULL) {
        return NULL;
    }
    
    for (size_t i = 0; i < manager->count; i++) {
        if (manager->entries[i].session.active &&
            strncmp(manager->entries[i].sessionId, sessionId, SESSION_ID_LENGTH + 1) == 0) {
            return &manager->entries[i];
        }
    }
    return NULL;
}

/* Create a new session */
char* create_session(SessionManager* manager, const char* userId, char* sessionIdOut, size_t outSize) {
    if (manager == NULL || userId == NULL || sessionIdOut == NULL || outSize <= SESSION_ID_LENGTH) {
        return NULL;
    }
    
    /* Validate user ID */
    if (!is_valid_user_id(userId)) {
        return NULL;
    }
    
    /* Enforce maximum sessions */
    if (manager->count >= MAX_SESSIONS) {
        return NULL;
    }
    
    /* Expand array if needed */
    if (manager->count >= manager->capacity) {
        size_t new_capacity = manager->capacity * 2;
        if (new_capacity > MAX_SESSIONS) {
            new_capacity = MAX_SESSIONS;
        }
        
        SessionEntry* new_entries = (SessionEntry*)realloc(manager->entries, 
                                                           new_capacity * sizeof(SessionEntry));
        if (new_entries == NULL) {
            return NULL;
        }
        
        manager->entries = new_entries;
        manager->capacity = new_capacity;
    }
    
    /* Generate unique session ID */
    char newSessionId[SESSION_ID_LENGTH + 1];
    int attempts = 0;
    const int MAX_ATTEMPTS = 100;
    
    do {
        if (!generate_session_id(newSessionId, sizeof(newSessionId))) {
            return NULL;
        }
        attempts++;
        if (attempts > MAX_ATTEMPTS) {
            return NULL;
        }
    } while (find_session(manager, newSessionId) != NULL);
    
    time_t now = time(NULL);
    if (now == (time_t)(-1)) {
        return NULL;
    }
    
    /* Add new session */
    SessionEntry* entry = &manager->entries[manager->count];
    strncpy(entry->sessionId, newSessionId, SESSION_ID_LENGTH + 1);
    entry->sessionId[SESSION_ID_LENGTH] = '\\0';
    
    strncpy(entry->session.userId, userId, MAX_USER_ID_LENGTH);
    entry->session.userId[MAX_USER_ID_LENGTH] = '\\0';
    entry->session.createdAt = now;
    entry->session.lastAccessedAt = now;
    entry->session.active = 1;
    
    manager->count++;
    
    strncpy(sessionIdOut, newSessionId, outSize);
    sessionIdOut[SESSION_ID_LENGTH] = '\\0';
    return sessionIdOut;
}

/* Validate session */
int validate_session(SessionManager* manager, const char* sessionId, char* userIdOut, size_t outSize) {
    if (manager == NULL || sessionId == NULL || userIdOut == NULL || outSize == 0) {
        return 0;
    }
    
    /* Validate session ID format */
    if (!is_valid_session_id(sessionId)) {
        return 0;
    }
    
    SessionEntry* entry = find_session(manager, sessionId);
    if (entry == NULL) {
        return 0;
    }
    
    time_t now = time(NULL);
    if (now == (time_t)(-1)) {
        return 0;
    }
    
    /* Check expiration */
    if (now - entry->session.lastAccessedAt > SESSION_TIMEOUT) {
        entry->session.active = 0;
        return 0;
    }
    
    /* Update last accessed time */
    entry->session.lastAccessedAt = now;
    
    strncpy(userIdOut, entry->session.userId, outSize - 1);
    userIdOut[outSize - 1] = '\\0';
    return 1;
}

/* Destroy session */
int destroy_session(SessionManager* manager, const char* sessionId) {
    if (manager == NULL || sessionId == NULL) {
        return 0;
    }
    
    /* Validate session ID format */
    if (!is_valid_session_id(sessionId)) {
        return 0;
    }
    
    SessionEntry* entry = find_session(manager, sessionId);
    if (entry == NULL) {
        return 0;
    }
    
    /* Clear session data securely */
    secure_clear(&entry->session, sizeof(Session));
    entry->session.active = 0;
    return 1;
}

/* Cleanup expired sessions */
size_t cleanup_expired_sessions(SessionManager* manager) {
    if (manager == NULL) {
        return 0;
    }
    
    time_t now = time(NULL);
    if (now == (time_t)(-1)) {
        return 0;
    }
    
    size_t removed = 0;
    for (size_t i = 0; i < manager->count; i++) {
        if (manager->entries[i].session.active &&
            now - manager->entries[i].session.lastAccessedAt > SESSION_TIMEOUT) {
            secure_clear(&manager->entries[i].session, sizeof(Session));
            manager->entries[i].session.active = 0;
            removed++;
        }
    }
    return removed;
}

/* Free session manager */
void free_session_manager(SessionManager* manager) {
    if (manager == NULL) {
        return;
    }
    
    if (manager->entries != NULL) {
        /* Clear all session data securely */
        for (size_t i = 0; i < manager->count; i++) {
            secure_clear(&manager->entries[i], sizeof(SessionEntry));
        }
        free(manager->entries);
        manager->entries = NULL;
    }
    
    free(manager);
}

size_t get_session_count(SessionManager* manager) {
    if (manager == NULL) {
        return 0;
    }
    
    size_t active = 0;
    for (size_t i = 0; i < manager->count; i++) {
        if (manager->entries[i].session.active) {
            active++;
        }
    }
    return active;
}

int main(void) {
    SessionManager* manager = create_session_manager();
    if (manager == NULL) {
        fprintf(stderr, "Failed to create session manager\\n");
        return 1;
    }
    
    /* Test case 1: Create valid sessions */
    printf("Test 1: Create valid sessions\\n");
    char session1[SESSION_ID_LENGTH + 1] = {0};
    char session2[SESSION_ID_LENGTH + 1] = {0};
    char* result1 = create_session(manager, "user123", session1, sizeof(session1));
    char* result2 = create_session(manager, "admin_user", session2, sizeof(session2));
    printf("Created session 1: %s\\n", result1 ? "SUCCESS" : "FAILED");
    printf("Created session 2: %s\\n", result2 ? "SUCCESS" : "FAILED");
    printf("Active sessions: %zu\\n\\n", get_session_count(manager));
    
    /* Test case 2: Validate sessions */
    printf("Test 2: Validate sessions\\n");
    char userId[MAX_USER_ID_LENGTH + 1] = {0};
    int valid1 = validate_session(manager, session1, userId, sizeof(userId));
    printf("Session 1 valid: %s, User: %s\\n", valid1 ? "YES" : "NO", valid1 ? userId : "N/A");
    int valid2 = validate_session(manager, session2, userId, sizeof(userId));
    printf("Session 2 valid: %s, User: %s\\n\\n", valid2 ? "YES" : "NO", valid2 ? userId : "N/A");
    
    /* Test case 3: Invalid session ID */
    printf("Test 3: Invalid session ID\\n");
    int valid3 = validate_session(manager, "invalid_session_id", userId, sizeof(userId));
    printf("Invalid session validated: %s\\n\\n", valid3 ? "YES (BUG!)" : "NO (correct)");
    
    /* Test case 4: Destroy session */
    printf("Test 4: Destroy session\\n");
    int destroyed = destroy_session(manager, session1);
    printf("Session destroyed: %s\\n", destroyed ? "YES" : "NO");
    int validAfterDestroy = validate_session(manager, session1, userId, sizeof(userId));
    printf("Destroyed session still valid: %s\\n", validAfterDestroy ? "YES (BUG!)" : "NO (correct)");
    printf("Active sessions: %zu\\n\\n", get_session_count(manager));
    
    /* Test case 5: Invalid user ID */
    printf("Test 5: Invalid user ID with special characters\\n");
    char session3[SESSION_ID_LENGTH + 1] = {0};
    char* result3 = create_session(manager, "user'; DROP TABLE--", session3, sizeof(session3));
    printf("Session with invalid user ID created: %s\\n", result3 ? "YES (BUG!)" : "NO (correct)");
    char session4[SESSION_ID_LENGTH + 1] = {0};
    char* result4 = create_session(manager, "valid_user-123", session4, sizeof(session4));
    printf("Session with valid user ID created: %s\\n", result4 ? "YES (correct)" : "NO (BUG!)");
    
    free_session_manager(manager);
    return 0;
}
