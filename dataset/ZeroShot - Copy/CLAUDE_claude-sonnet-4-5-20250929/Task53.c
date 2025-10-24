
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SESSION_TIMEOUT_SECONDS 1800
#define TOKEN_LENGTH 64
#define MAX_SESSIONS 1000
#define MAX_ATTRIBUTES 10

typedef struct {
    char key[64];
    char value[256];
} Attribute;

typedef struct {
    char sessionId[TOKEN_LENGTH + 1];
    char userId[64];
    time_t createdAt;
    time_t lastAccessedAt;
    Attribute attributes[MAX_ATTRIBUTES];
    int attributeCount;
    int active;
} Session;

typedef struct {
    Session sessions[MAX_SESSIONS];
    int sessionCount;
} Task53;

void generateSecureToken(char* token) {
    const char* hex = "0123456789abcdef";
    srand(time(NULL) + rand());
    
    for (int i = 0; i < TOKEN_LENGTH; i++) {
        token[i] = hex[rand() % 16];
    }
    token[TOKEN_LENGTH] = '\\0';
}

Task53* createSessionManager() {
    Task53* manager = (Task53*)malloc(sizeof(Task53));
    manager->sessionCount = 0;
    
    for (int i = 0; i < MAX_SESSIONS; i++) {
        manager->sessions[i].active = 0;
    }
    
    return manager;
}

char* createSession(Task53* manager, const char* userId) {
    if (!manager || !userId || strlen(userId) == 0) {
        return NULL;
    }
    
    if (manager->sessionCount >= MAX_SESSIONS) {
        return NULL;
    }
    
    int index = -1;
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (!manager->sessions[i].active) {
            index = i;
            break;
        }
    }
    
    if (index == -1) {
        return NULL;
    }
    
    Session* session = &manager->sessions[index];
    generateSecureToken(session->sessionId);
    strncpy(session->userId, userId, sizeof(session->userId) - 1);
    session->userId[sizeof(session->userId) - 1] = '\\0';
    session->createdAt = time(NULL);
    session->lastAccessedAt = session->createdAt;
    session->attributeCount = 0;
    session->active = 1;
    manager->sessionCount++;
    
    char* sessionId = (char*)malloc(TOKEN_LENGTH + 1);
    strcpy(sessionId, session->sessionId);
    return sessionId;
}

int validateSession(Task53* manager, const char* sessionId) {
    if (!manager || !sessionId) {
        return 0;
    }
    
    for (int i = 0; i < MAX_SESSIONS; i++) {
        Session* session = &manager->sessions[i];
        
        if (session->active && strcmp(session->sessionId, sessionId) == 0) {
            time_t currentTime = time(NULL);
            
            if ((currentTime - session->lastAccessedAt) > SESSION_TIMEOUT_SECONDS) {
                session->active = 0;
                manager->sessionCount--;
                return 0;
            }
            
            session->lastAccessedAt = currentTime;
            return 1;
        }
    }
    
    return 0;
}

char* getUserId(Task53* manager, const char* sessionId) {
    if (!validateSession(manager, sessionId)) {
        return NULL;
    }
    
    for (int i = 0; i < MAX_SESSIONS; i++) {
        Session* session = &manager->sessions[i];
        
        if (session->active && strcmp(session->sessionId, sessionId) == 0) {
            char* userId = (char*)malloc(strlen(session->userId) + 1);
            strcpy(userId, session->userId);
            return userId;
        }
    }
    
    return NULL;
}

int invalidateSession(Task53* manager, const char* sessionId) {
    if (!manager || !sessionId) {
        return 0;
    }
    
    for (int i = 0; i < MAX_SESSIONS; i++) {
        Session* session = &manager->sessions[i];
        
        if (session->active && strcmp(session->sessionId, sessionId) == 0) {
            session->active = 0;
            manager->sessionCount--;
            return 1;
        }
    }
    
    return 0;
}

void setAttribute(Task53* manager, const char* sessionId, const char* key, const char* value) {
    if (!validateSession(manager, sessionId) || !key || !value) {
        return;
    }
    
    for (int i = 0; i < MAX_SESSIONS; i++) {
        Session* session = &manager->sessions[i];
        
        if (session->active && strcmp(session->sessionId, sessionId) == 0) {
            if (session->attributeCount < MAX_ATTRIBUTES) {
                strncpy(session->attributes[session->attributeCount].key, key, 
                       sizeof(session->attributes[session->attributeCount].key) - 1);
                strncpy(session->attributes[session->attributeCount].value, value,
                       sizeof(session->attributes[session->attributeCount].value) - 1);
                session->attributeCount++;
            }
            break;
        }
    }
}

char* getAttribute(Task53* manager, const char* sessionId, const char* key) {
    if (!validateSession(manager, sessionId) || !key) {
        return NULL;
    }
    
    for (int i = 0; i < MAX_SESSIONS; i++) {
        Session* session = &manager->sessions[i];
        
        if (session->active && strcmp(session->sessionId, sessionId) == 0) {
            for (int j = 0; j < session->attributeCount; j++) {
                if (strcmp(session->attributes[j].key, key) == 0) {
                    char* value = (char*)malloc(strlen(session->attributes[j].value) + 1);
                    strcpy(value, session->attributes[j].value);
                    return value;
                }
            }
        }
    }
    
    return NULL;
}

void cleanupExpiredSessions(Task53* manager) {
    if (!manager) {
        return;
    }
    
    time_t currentTime = time(NULL);
    
    for (int i = 0; i < MAX_SESSIONS; i++) {
        Session* session = &manager->sessions[i];
        
        if (session->active && (currentTime - session->lastAccessedAt) > SESSION_TIMEOUT_SECONDS) {
            session->active = 0;
            manager->sessionCount--;
        }
    }
}

void destroySessionManager(Task53* manager) {
    if (manager) {
        free(manager);
    }
}

int main() {
    Task53* sessionManager = createSessionManager();
    
    // Test Case 1: Create a session
    printf("Test Case 1: Create Session\\n");
    char* session1 = createSession(sessionManager, "user123");
    printf("Session created: %s\\n", session1 ? "true" : "false");
    printf("Session ID length: %lu\\n", session1 ? strlen(session1) : 0);
    
    // Test Case 2: Validate session
    printf("\\nTest Case 2: Validate Session\\n");
    int isValid = validateSession(sessionManager, session1);
    printf("Session valid: %s\\n", isValid ? "true" : "false");
    char* userId = getUserId(sessionManager, session1);
    printf("User ID: %s\\n", userId ? userId : "NULL");
    free(userId);
    
    // Test Case 3: Set and get attributes
    printf("\\nTest Case 3: Session Attributes\\n");
    setAttribute(sessionManager, session1, "username", "JohnDoe");
    setAttribute(sessionManager, session1, "role", "admin");
    char* username = getAttribute(sessionManager, session1, "username");
    char* role = getAttribute(sessionManager, session1, "role");
    printf("Username: %s\\n", username ? username : "NULL");
    printf("Role: %s\\n", role ? role : "NULL");
    free(username);
    free(role);
    
    // Test Case 4: Invalidate session
    printf("\\nTest Case 4: Invalidate Session\\n");
    int invalidated = invalidateSession(sessionManager, session1);
    printf("Session invalidated: %s\\n", invalidated ? "true" : "false");
    int stillValid = validateSession(sessionManager, session1);
    printf("Session still valid: %s\\n", stillValid ? "true" : "false");
    free(session1);
    
    // Test Case 5: Multiple sessions and cleanup
    printf("\\nTest Case 5: Multiple Sessions\\n");
    char* session2 = createSession(sessionManager, "user456");
    char* session3 = createSession(sessionManager, "user789");
    printf("Session 2 created: %s\\n", session2 ? "true" : "false");
    printf("Session 3 created: %s\\n", session3 ? "true" : "false");
    cleanupExpiredSessions(sessionManager);
    printf("Cleanup completed\\n");
    printf("Session 2 valid: %s\\n", validateSession(sessionManager, session2) ? "true" : "false");
    
    free(session2);
    free(session3);
    destroySessionManager(sessionManager);
    
    return 0;
}
