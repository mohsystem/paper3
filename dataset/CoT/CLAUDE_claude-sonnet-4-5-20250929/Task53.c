
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define SESSION_TIMEOUT_SECONDS 1800
#define ABSOLUTE_TIMEOUT_SECONDS 7200
#define SESSION_ID_LENGTH 64
#define MAX_SESSIONS 1000
#define MAX_USER_ID_LENGTH 100

typedef struct {
    char sessionId[SESSION_ID_LENGTH + 1];
    char userId[MAX_USER_ID_LENGTH + 1];
    time_t createdAt;
    time_t lastAccessedAt;
    bool active;
} Session;

typedef struct {
    Session sessions[MAX_SESSIONS];
    int sessionCount;
} Task53;

void initSessionManager(Task53* manager) {
    manager->sessionCount = 0;
    for (int i = 0; i < MAX_SESSIONS; i++) {
        manager->sessions[i].active = false;
    }
}

void generateSessionId(char* sessionId) {
    const char* hexChars = "0123456789abcdef";
    for (int i = 0; i < SESSION_ID_LENGTH; i++) {
        sessionId[i] = hexChars[rand() % 16];
    }
    sessionId[SESSION_ID_LENGTH] = '\\0';
}

bool isSessionExpired(const Session* session) {
    time_t now = time(NULL);
    bool idleExpired = (now - session->lastAccessedAt) > SESSION_TIMEOUT_SECONDS;
    bool absoluteExpired = (now - session->createdAt) > ABSOLUTE_TIMEOUT_SECONDS;
    return idleExpired || absoluteExpired;
}

char* createSession(Task53* manager, const char* userId, char* outputSessionId) {
    if (userId == NULL || strlen(userId) == 0) {
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
    generateSessionId(session->sessionId);
    strncpy(session->userId, userId, MAX_USER_ID_LENGTH - 1);
    session->userId[MAX_USER_ID_LENGTH - 1] = '\\0';
    session->createdAt = time(NULL);
    session->lastAccessedAt = session->createdAt;
    session->active = true;
    manager->sessionCount++;
    
    strcpy(outputSessionId, session->sessionId);
    return outputSessionId;
}

bool validateSession(Task53* manager, const char* sessionId) {
    if (sessionId == NULL) {
        return false;
    }
    
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (manager->sessions[i].active && 
            strcmp(manager->sessions[i].sessionId, sessionId) == 0) {
            
            if (isSessionExpired(&manager->sessions[i])) {
                manager->sessions[i].active = false;
                manager->sessionCount--;
                return false;
            }
            
            manager->sessions[i].lastAccessedAt = time(NULL);
            return true;
        }
    }
    
    return false;
}

void invalidateSession(Task53* manager, const char* sessionId) {
    if (sessionId == NULL) {
        return;
    }
    
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (manager->sessions[i].active && 
            strcmp(manager->sessions[i].sessionId, sessionId) == 0) {
            manager->sessions[i].active = false;
            manager->sessionCount--;
            break;
        }
    }
}

void invalidateAllUserSessions(Task53* manager, const char* userId) {
    if (userId == NULL) {
        return;
    }
    
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (manager->sessions[i].active && 
            strcmp(manager->sessions[i].userId, userId) == 0) {
            manager->sessions[i].active = false;
            manager->sessionCount--;
        }
    }
}

int getActiveSessionCount(Task53* manager, const char* userId) {
    int count = 0;
    
    if (userId == NULL) {
        return 0;
    }
    
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (manager->sessions[i].active && 
            strcmp(manager->sessions[i].userId, userId) == 0) {
            if (!isSessionExpired(&manager->sessions[i])) {
                manager->sessions[i].lastAccessedAt = time(NULL);
                count++;
            } else {
                manager->sessions[i].active = false;
                manager->sessionCount--;
            }
        }
    }
    
    return count;
}

void cleanupExpiredSessions(Task53* manager) {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (manager->sessions[i].active && isSessionExpired(&manager->sessions[i])) {
            manager->sessions[i].active = false;
            manager->sessionCount--;
        }
    }
}

int main() {
    srand(time(NULL));
    Task53 sessionManager;
    initSessionManager(&sessionManager);
    
    char session1[SESSION_ID_LENGTH + 1];
    char session2[SESSION_ID_LENGTH + 1];
    char session3[SESSION_ID_LENGTH + 1];
    
    // Test Case 1: Create and validate session
    printf("Test Case 1: Create and validate session\\n");
    char* result1 = createSession(&sessionManager, "user1", session1);
    printf("Session created: %d\\n", result1 != NULL);
    printf("Session valid: %d\\n", validateSession(&sessionManager, session1));
    printf("\\n");
    
    // Test Case 2: Multiple sessions for same user
    printf("Test Case 2: Multiple sessions for same user\\n");
    createSession(&sessionManager, "user1", session2);
    createSession(&sessionManager, "user1", session3);
    printf("Active sessions for user1: %d\\n", getActiveSessionCount(&sessionManager, "user1"));
    printf("\\n");
    
    // Test Case 3: Invalidate specific session
    printf("Test Case 3: Invalidate specific session\\n");
    invalidateSession(&sessionManager, session2);
    printf("Session2 valid after invalidation: %d\\n", validateSession(&sessionManager, session2));
    printf("Active sessions for user1: %d\\n", getActiveSessionCount(&sessionManager, "user1"));
    printf("\\n");
    
    // Test Case 4: Invalidate all user sessions
    printf("Test Case 4: Invalidate all user sessions\\n");
    invalidateAllUserSessions(&sessionManager, "user1");
    printf("Session1 valid after user invalidation: %d\\n", validateSession(&sessionManager, session1));
    printf("Active sessions for user1: %d\\n", getActiveSessionCount(&sessionManager, "user1"));
    printf("\\n");
    
    // Test Case 5: Invalid session handling
    printf("Test Case 5: Invalid session handling\\n");
    printf("Null session valid: %d\\n", validateSession(&sessionManager, NULL));
    printf("Non-existent session valid: %d\\n", validateSession(&sessionManager, "invalid123"));
    char nullSession[SESSION_ID_LENGTH + 1];
    char* nullResult = createSession(&sessionManager, NULL, nullSession);
    printf("Session created with null user: %d\\n", nullResult != NULL);
    
    return 0;
}
