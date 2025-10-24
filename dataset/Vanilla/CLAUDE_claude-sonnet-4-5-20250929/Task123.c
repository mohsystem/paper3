
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MAX_SESSIONS 100
#define UUID_LENGTH 37
#define USER_ID_LENGTH 50

typedef struct {
    char userId[USER_ID_LENGTH];
    char sessionId[UUID_LENGTH];
    time_t loginTime;
    time_t lastActivityTime;
    bool active;
} Session;

typedef struct {
    Session sessions[MAX_SESSIONS];
    int count;
} Task123;

void generateUUID(char* uuid) {
    const char* hex = "0123456789abcdef";
    int i, pos = 0;
    
    srand(time(NULL) + rand());
    
    for (i = 0; i < 32; i++) {
        if (i == 8 || i == 12 || i == 16 || i == 20) {
            uuid[pos++] = '-';
        }
        uuid[pos++] = hex[rand() % 16];
    }
    uuid[pos] = '\\0';
}

void initTask123(Task123* manager) {
    manager->count = 0;
}

void createSession(Task123* manager, const char* userId, char* sessionId) {
    if (manager->count >= MAX_SESSIONS) {
        strcpy(sessionId, "");
        return;
    }
    
    Session* session = &manager->sessions[manager->count];
    strcpy(session->userId, userId);
    generateUUID(session->sessionId);
    strcpy(sessionId, session->sessionId);
    session->loginTime = time(NULL);
    session->lastActivityTime = session->loginTime;
    session->active = true;
    manager->count++;
}

bool isSessionActive(Task123* manager, const char* sessionId) {
    for (int i = 0; i < manager->count; i++) {
        if (strcmp(manager->sessions[i].sessionId, sessionId) == 0) {
            return manager->sessions[i].active;
        }
    }
    return false;
}

void updateActivity(Task123* manager, const char* sessionId) {
    for (int i = 0; i < manager->count; i++) {
        if (strcmp(manager->sessions[i].sessionId, sessionId) == 0 && 
            manager->sessions[i].active) {
            manager->sessions[i].lastActivityTime = time(NULL);
            return;
        }
    }
}

void endSession(Task123* manager, const char* sessionId) {
    for (int i = 0; i < manager->count; i++) {
        if (strcmp(manager->sessions[i].sessionId, sessionId) == 0) {
            manager->sessions[i].active = false;
            return;
        }
    }
}

void getSessionInfo(Task123* manager, const char* sessionId, char* info) {
    for (int i = 0; i < manager->count; i++) {
        if (strcmp(manager->sessions[i].sessionId, sessionId) == 0) {
            Session* s = &manager->sessions[i];
            char loginStr[80], activityStr[80];
            strftime(loginStr, sizeof(loginStr), "%Y-%m-%d %H:%M:%S", localtime(&s->loginTime));
            strftime(activityStr, sizeof(activityStr), "%Y-%m-%d %H:%M:%S", localtime(&s->lastActivityTime));
            sprintf(info, "User: %s, Active: %s, Login: %s, Last Activity: %s",
                    s->userId, s->active ? "true" : "false", loginStr, activityStr);
            return;
        }
    }
    strcpy(info, "Session not found");
}

int getActiveSessions(Task123* manager, const char* userId, char sessions[][UUID_LENGTH]) {
    int count = 0;
    for (int i = 0; i < manager->count; i++) {
        if (strcmp(manager->sessions[i].userId, userId) == 0 && 
            manager->sessions[i].active) {
            strcpy(sessions[count], manager->sessions[i].sessionId);
            count++;
        }
    }
    return count;
}

int getActiveSessionCount(Task123* manager) {
    int count = 0;
    for (int i = 0; i < manager->count; i++) {
        if (manager->sessions[i].active) {
            count++;
        }
    }
    return count;
}

int main() {
    Task123 manager;
    initTask123(&manager);
    
    char session1[UUID_LENGTH], session2[UUID_LENGTH], session3[UUID_LENGTH];
    
    printf("Test Case 1: Create sessions for users\\n");
    createSession(&manager, "user1", session1);
    createSession(&manager, "user2", session2);
    createSession(&manager, "user1", session3);
    printf("Created sessions: %.8s..., %.8s..., %.8s...\\n", session1, session2, session3);
    printf("Active session count: %d\\n\\n", getActiveSessionCount(&manager));
    
    printf("Test Case 2: Check session status\\n");
    printf("Session 1 active: %s\\n", isSessionActive(&manager, session1) ? "true" : "false");
    printf("Invalid session active: %s\\n\\n", isSessionActive(&manager, "invalid-id") ? "true" : "false");
    
    printf("Test Case 3: Update activity and get session info\\n");
    updateActivity(&manager, session1);
    char info[256];
    getSessionInfo(&manager, session1, info);
    printf("%s\\n\\n", info);
    
    printf("Test Case 4: Get active sessions for user\\n");
    char activeSessions[MAX_SESSIONS][UUID_LENGTH];
    int activeCount = getActiveSessions(&manager, "user1", activeSessions);
    printf("User1 has %d active sessions\\n\\n", activeCount);
    
    printf("Test Case 5: End session and verify\\n");
    endSession(&manager, session1);
    printf("Session 1 active after ending: %s\\n", isSessionActive(&manager, session1) ? "true" : "false");
    printf("Active session count: %d\\n", getActiveSessionCount(&manager));
    activeCount = getActiveSessions(&manager, "user1", activeSessions);
    printf("User1 now has %d active sessions\\n", activeCount);
    
    return 0;
}
