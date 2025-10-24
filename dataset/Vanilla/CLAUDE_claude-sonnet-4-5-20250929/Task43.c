
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MAX_SESSIONS 1000
#define SESSION_ID_LENGTH 64
#define USER_ID_LENGTH 64
#define SESSION_TIMEOUT_SECONDS 1800
#define MAX_ATTRIBUTES 10

typedef struct {
    char key[64];
    char value[256];
} Attribute;

typedef struct {
    char sessionId[SESSION_ID_LENGTH];
    char userId[USER_ID_LENGTH];
    time_t createdAt;
    time_t lastAccessedAt;
    bool active;
    Attribute attributes[MAX_ATTRIBUTES];
    int attributeCount;
} Session;

typedef struct {
    Session sessions[MAX_SESSIONS];
    int sessionCount;
} SessionManager;

void generateSessionId(char* sessionId) {
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    for (int i = 0; i < SESSION_ID_LENGTH - 1; i++) {
        sessionId[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    sessionId[SESSION_ID_LENGTH - 1] = '\\0';
}

void initSessionManager(SessionManager* manager) {
    manager->sessionCount = 0;
    for (int i = 0; i < MAX_SESSIONS; i++) {
        manager->sessions[i].active = false;
        manager->sessions[i].attributeCount = 0;
    }
}

bool isSessionExpired(Session* session) {
    time_t now = time(NULL);
    return difftime(now, session->lastAccessedAt) > SESSION_TIMEOUT_SECONDS;
}

void cleanupExpiredSessions(SessionManager* manager) {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (manager->sessions[i].active && isSessionExpired(&manager->sessions[i])) {
            manager->sessions[i].active = false;
            manager->sessionCount--;
        }
    }
}

char* createSession(SessionManager* manager, const char* userId, char* outSessionId) {
    cleanupExpiredSessions(manager);
    
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
    generateSessionId(session->sessionId);
    strncpy(session->userId, userId, USER_ID_LENGTH - 1);
    session->userId[USER_ID_LENGTH - 1] = '\\0';
    session->createdAt = time(NULL);
    session->lastAccessedAt = session->createdAt;
    session->active = true;
    session->attributeCount = 0;
    manager->sessionCount++;
    
    strcpy(outSessionId, session->sessionId);
    return outSessionId;
}

Session* getSession(SessionManager* manager, const char* sessionId) {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (manager->sessions[i].active && 
            strcmp(manager->sessions[i].sessionId, sessionId) == 0) {
            if (isSessionExpired(&manager->sessions[i])) {
                manager->sessions[i].active = false;
                manager->sessionCount--;
                return NULL;
            }
            manager->sessions[i].lastAccessedAt = time(NULL);
            return &manager->sessions[i];
        }
    }
    return NULL;
}

bool validateSession(SessionManager* manager, const char* sessionId) {
    return getSession(manager, sessionId) != NULL;
}

void terminateSession(SessionManager* manager, const char* sessionId) {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (manager->sessions[i].active && 
            strcmp(manager->sessions[i].sessionId, sessionId) == 0) {
            manager->sessions[i].active = false;
            manager->sessionCount--;
            break;
        }
    }
}

void terminateUserSessions(SessionManager* manager, const char* userId) {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (manager->sessions[i].active && 
            strcmp(manager->sessions[i].userId, userId) == 0) {
            manager->sessions[i].active = false;
            manager->sessionCount--;
        }
    }
}

int getActiveSessionCount(SessionManager* manager) {
    cleanupExpiredSessions(manager);
    return manager->sessionCount;
}

void setSessionAttribute(Session* session, const char* key, const char* value) {
    if (session->attributeCount < MAX_ATTRIBUTES) {
        strncpy(session->attributes[session->attributeCount].key, key, 63);
        session->attributes[session->attributeCount].key[63] = '\\0';
        strncpy(session->attributes[session->attributeCount].value, value, 255);
        session->attributes[session->attributeCount].value[255] = '\\0';
        session->attributeCount++;
    }
}

const char* getSessionAttribute(Session* session, const char* key) {
    for (int i = 0; i < session->attributeCount; i++) {
        if (strcmp(session->attributes[i].key, key) == 0) {
            return session->attributes[i].value;
        }
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    SessionManager manager;
    initSessionManager(&manager);
    
    printf("=== Test Case 1: Create Session ===\\n");
    char sessionId1[SESSION_ID_LENGTH];
    createSession(&manager, "user123", sessionId1);
    printf("Created session: %s\\n", sessionId1);
    printf("Session valid: %s\\n", validateSession(&manager, sessionId1) ? "true" : "false");
    
    printf("\\n=== Test Case 2: Get Session and Update Access Time ===\\n");
    Session* session1 = getSession(&manager, sessionId1);
    if (session1) {
        printf("Session userId: %s\\n", session1->userId);
        setSessionAttribute(session1, "username", "JohnDoe");
        printf("Username attribute: %s\\n", getSessionAttribute(session1, "username"));
    }
    
    printf("\\n=== Test Case 3: Multiple Sessions ===\\n");
    char sessionId2[SESSION_ID_LENGTH], sessionId3[SESSION_ID_LENGTH];
    createSession(&manager, "user456", sessionId2);
    createSession(&manager, "user789", sessionId3);
    printf("Active sessions: %d\\n", getActiveSessionCount(&manager));
    
    printf("\\n=== Test Case 4: Terminate Specific Session ===\\n");
    terminateSession(&manager, sessionId2);
    printf("Session 2 valid after termination: %s\\n", 
           validateSession(&manager, sessionId2) ? "true" : "false");
    printf("Active sessions: %d\\n", getActiveSessionCount(&manager));
    
    printf("\\n=== Test Case 5: Terminate All User Sessions ===\\n");
    char sessionId4[SESSION_ID_LENGTH];
    createSession(&manager, "user123", sessionId4);
    printf("Created another session for user123\\n");
    printf("Active sessions before cleanup: %d\\n", getActiveSessionCount(&manager));
    terminateUserSessions(&manager, "user123");
    printf("Active sessions after user123 cleanup: %d\\n", getActiveSessionCount(&manager));
    printf("Session 1 valid: %s\\n", validateSession(&manager, sessionId1) ? "true" : "false");
    printf("Session 4 valid: %s\\n", validateSession(&manager, sessionId4) ? "true" : "false");
    
    return 0;
}
