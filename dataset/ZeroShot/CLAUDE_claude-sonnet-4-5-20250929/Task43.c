
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define SESSION_TIMEOUT_SECONDS 1800
#define SESSION_ID_LENGTH 32
#define MAX_SESSIONS 100
#define MAX_ATTRIBUTES 10
#define MAX_ATTR_KEY_LEN 50
#define MAX_ATTR_VALUE_LEN 200

typedef struct {
    char key[MAX_ATTR_KEY_LEN];
    char value[MAX_ATTR_VALUE_LEN];
} Attribute;

typedef struct {
    char sessionId[SESSION_ID_LENGTH + 1];
    char userId[100];
    time_t createdAt;
    time_t lastAccessedAt;
    Attribute attributes[MAX_ATTRIBUTES];
    int attributeCount;
    bool active;
} Session;

static Session sessions[MAX_SESSIONS];
static int sessionCount = 0;

void generateSecureToken(char* token, size_t length) {
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-_";
    srand(time(NULL) + rand());
    
    for (size_t i = 0; i < length; i++) {
        token[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    token[length] = '\\0';
}

bool isSessionExpired(Session* session) {
    time_t now = time(NULL);
    return difftime(now, session->lastAccessedAt) > SESSION_TIMEOUT_SECONDS;
}

void cleanExpiredSessions() {
    for (int i = 0; i < sessionCount; i++) {
        if (sessions[i].active && isSessionExpired(&sessions[i])) {
            sessions[i].active = false;
        }
    }
}

char* createSession(const char* userId, char* outputSessionId) {
    if (userId == NULL || strlen(userId) == 0) {
        return NULL;
    }
    
    cleanExpiredSessions();
    
    if (sessionCount >= MAX_SESSIONS) {
        return NULL;
    }
    
    Session* session = &sessions[sessionCount];
    generateSecureToken(session->sessionId, SESSION_ID_LENGTH);
    strncpy(session->userId, userId, sizeof(session->userId) - 1);
    session->userId[sizeof(session->userId) - 1] = '\\0';
    session->createdAt = time(NULL);
    session->lastAccessedAt = session->createdAt;
    session->attributeCount = 0;
    session->active = true;
    
    sessionCount++;
    
    if (outputSessionId != NULL) {
        strcpy(outputSessionId, session->sessionId);
    }
    
    return session->sessionId;
}

bool validateSession(const char* sessionId) {
    if (sessionId == NULL || strlen(sessionId) == 0) {
        return false;
    }
    
    for (int i = 0; i < sessionCount; i++) {
        if (sessions[i].active && strcmp(sessions[i].sessionId, sessionId) == 0) {
            if (isSessionExpired(&sessions[i])) {
                sessions[i].active = false;
                return false;
            }
            sessions[i].lastAccessedAt = time(NULL);
            return true;
        }
    }
    
    return false;
}

char* getSessionUserId(const char* sessionId) {
    if (!validateSession(sessionId)) {
        return NULL;
    }
    
    for (int i = 0; i < sessionCount; i++) {
        if (sessions[i].active && strcmp(sessions[i].sessionId, sessionId) == 0) {
            return sessions[i].userId;
        }
    }
    
    return NULL;
}

bool terminateSession(const char* sessionId) {
    if (sessionId == NULL) {
        return false;
    }
    
    for (int i = 0; i < sessionCount; i++) {
        if (sessions[i].active && strcmp(sessions[i].sessionId, sessionId) == 0) {
            sessions[i].active = false;
            return true;
        }
    }
    
    return false;
}

bool setSessionAttribute(const char* sessionId, const char* key, const char* value) {
    if (!validateSession(sessionId) || key == NULL || value == NULL) {
        return false;
    }
    
    for (int i = 0; i < sessionCount; i++) {
        if (sessions[i].active && strcmp(sessions[i].sessionId, sessionId) == 0) {
            if (sessions[i].attributeCount >= MAX_ATTRIBUTES) {
                return false;
            }
            
            Attribute* attr = &sessions[i].attributes[sessions[i].attributeCount];
            strncpy(attr->key, key, MAX_ATTR_KEY_LEN - 1);
            attr->key[MAX_ATTR_KEY_LEN - 1] = '\\0';
            strncpy(attr->value, value, MAX_ATTR_VALUE_LEN - 1);
            attr->value[MAX_ATTR_VALUE_LEN - 1] = '\\0';
            sessions[i].attributeCount++;
            return true;
        }
    }
    
    return false;
}

char* getSessionAttribute(const char* sessionId, const char* key) {
    if (!validateSession(sessionId) || key == NULL) {
        return NULL;
    }
    
    for (int i = 0; i < sessionCount; i++) {
        if (sessions[i].active && strcmp(sessions[i].sessionId, sessionId) == 0) {
            for (int j = 0; j < sessions[i].attributeCount; j++) {
                if (strcmp(sessions[i].attributes[j].key, key) == 0) {
                    return sessions[i].attributes[j].value;
                }
            }
        }
    }
    
    return NULL;
}

int getActiveSessionCount() {
    cleanExpiredSessions();
    int count = 0;
    for (int i = 0; i < sessionCount; i++) {
        if (sessions[i].active) {
            count++;
        }
    }
    return count;
}

int main() {
    printf("=== Secure Session Management Test Cases ===\\n\\n");
    
    // Test Case 1: Create and validate session
    printf("Test Case 1: Create and Validate Session\\n");
    char session1[SESSION_ID_LENGTH + 1];
    createSession("user123", session1);
    printf("Session created: %d\\n", strlen(session1) > 0);
    printf("Session valid: %d\\n", validateSession(session1));
    printf("User ID: %s\\n\\n", getSessionUserId(session1));
    
    // Test Case 2: Session attributes
    printf("Test Case 2: Session Attributes\\n");
    char session2[SESSION_ID_LENGTH + 1];
    createSession("user456", session2);
    setSessionAttribute(session2, "role", "admin");
    setSessionAttribute(session2, "loginTime", "2024-01-01");
    printf("Role attribute: %s\\n", getSessionAttribute(session2, "role"));
    printf("Login time set: %d\\n\\n", getSessionAttribute(session2, "loginTime") != NULL);
    
    // Test Case 3: Invalid session handling
    printf("Test Case 3: Invalid Session Handling\\n");
    printf("Null session valid: %d\\n", validateSession(NULL));
    printf("Empty session valid: %d\\n", validateSession(""));
    printf("Fake session valid: %d\\n\\n", validateSession("fake-session-id"));
    
    // Test Case 4: Session termination
    printf("Test Case 4: Session Termination\\n");
    char session3[SESSION_ID_LENGTH + 1];
    createSession("user789", session3);
    printf("Session created: %d\\n", validateSession(session3));
    bool terminated = terminateSession(session3);
    printf("Session terminated: %d\\n", terminated);
    printf("Session still valid: %d\\n\\n", validateSession(session3));
    
    // Test Case 5: Active session count
    printf("Test Case 5: Active Session Count\\n");
    char temp[SESSION_ID_LENGTH + 1];
    createSession("user001", temp);
    createSession("user002", temp);
    createSession("user003", temp);
    int count = getActiveSessionCount();
    printf("Active sessions: %d\\n", count);
    cleanExpiredSessions();
    printf("After cleanup: %d\\n\\n", getActiveSessionCount());
    
    return 0;
}
