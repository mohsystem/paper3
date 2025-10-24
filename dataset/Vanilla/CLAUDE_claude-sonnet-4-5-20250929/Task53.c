
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_SESSIONS 100
#define MAX_DATA_ITEMS 20
#define SESSION_ID_LENGTH 37
#define USER_ID_LENGTH 50
#define KEY_LENGTH 50
#define VALUE_LENGTH 100

typedef struct {
    char key[KEY_LENGTH];
    char value[VALUE_LENGTH];
} DataItem;

typedef struct {
    char sessionId[SESSION_ID_LENGTH];
    char userId[USER_ID_LENGTH];
    time_t createdAt;
    time_t lastAccessedAt;
    DataItem data[MAX_DATA_ITEMS];
    int dataCount;
    int active;
} Session;

typedef struct {
    Session sessions[MAX_SESSIONS];
    int sessionCount;
    int sessionTimeout;
} Task53;

void generateUUID(char* buffer) {
    const char* hex = "0123456789abcdef";
    int pos = 0;
    
    for (int i = 0; i < 32; i++) {
        if (i == 8 || i == 12 || i == 16 || i == 20) {
            buffer[pos++] = '-';
        }
        buffer[pos++] = hex[rand() % 16];
    }
    buffer[pos] = '\\0';
}

Task53* createTask53(int sessionTimeout) {
    Task53* manager = (Task53*)malloc(sizeof(Task53));
    manager->sessionCount = 0;
    manager->sessionTimeout = sessionTimeout;
    for (int i = 0; i < MAX_SESSIONS; i++) {
        manager->sessions[i].active = 0;
    }
    return manager;
}

int isExpired(Session* session, int timeoutMinutes) {
    time_t now = time(NULL);
    double seconds = difftime(now, session->lastAccessedAt);
    return (seconds / 60) >= timeoutMinutes;
}

void updateLastAccessed(Session* session) {
    session->lastAccessedAt = time(NULL);
}

char* createSession(Task53* manager, const char* userId) {
    if (manager->sessionCount >= MAX_SESSIONS) {
        return NULL;
    }
    
    int index = manager->sessionCount++;
    Session* session = &manager->sessions[index];
    
    generateUUID(session->sessionId);
    strncpy(session->userId, userId, USER_ID_LENGTH - 1);
    session->userId[USER_ID_LENGTH - 1] = '\\0';
    session->createdAt = time(NULL);
    session->lastAccessedAt = time(NULL);
    session->dataCount = 0;
    session->active = 1;
    
    static char returnBuffer[SESSION_ID_LENGTH];
    strcpy(returnBuffer, session->sessionId);
    return returnBuffer;
}

int validateSession(Task53* manager, const char* sessionId) {
    for (int i = 0; i < manager->sessionCount; i++) {
        Session* session = &manager->sessions[i];
        if (session->active && strcmp(session->sessionId, sessionId) == 0) {
            if (isExpired(session, manager->sessionTimeout)) {
                session->active = 0;
                return 0;
            }
            updateLastAccessed(session);
            return 1;
        }
    }
    return 0;
}

void destroySession(Task53* manager, const char* sessionId) {
    for (int i = 0; i < manager->sessionCount; i++) {
        if (strcmp(manager->sessions[i].sessionId, sessionId) == 0) {
            manager->sessions[i].active = 0;
            return;
        }
    }
}

void setSessionData(Task53* manager, const char* sessionId, const char* key, const char* value) {
    for (int i = 0; i < manager->sessionCount; i++) {
        Session* session = &manager->sessions[i];
        if (session->active && strcmp(session->sessionId, sessionId) == 0) {
            if (!isExpired(session, manager->sessionTimeout)) {
                if (session->dataCount < MAX_DATA_ITEMS) {
                    DataItem* item = &session->data[session->dataCount++];
                    strncpy(item->key, key, KEY_LENGTH - 1);
                    strncpy(item->value, value, VALUE_LENGTH - 1);
                    item->key[KEY_LENGTH - 1] = '\\0';
                    item->value[VALUE_LENGTH - 1] = '\\0';
                }
                updateLastAccessed(session);
            }
            return;
        }
    }
}

char* getSessionData(Task53* manager, const char* sessionId, const char* key) {
    for (int i = 0; i < manager->sessionCount; i++) {
        Session* session = &manager->sessions[i];
        if (session->active && strcmp(session->sessionId, sessionId) == 0) {
            if (!isExpired(session, manager->sessionTimeout)) {
                updateLastAccessed(session);
                for (int j = 0; j < session->dataCount; j++) {
                    if (strcmp(session->data[j].key, key) == 0) {
                        return session->data[j].value;
                    }
                }
            }
            return NULL;
        }
    }
    return NULL;
}

char* getUserId(Task53* manager, const char* sessionId) {
    for (int i = 0; i < manager->sessionCount; i++) {
        Session* session = &manager->sessions[i];
        if (session->active && strcmp(session->sessionId, sessionId) == 0) {
            if (!isExpired(session, manager->sessionTimeout)) {
                return session->userId;
            }
        }
    }
    return NULL;
}

int getActiveSessionCount(Task53* manager) {
    int count = 0;
    for (int i = 0; i < manager->sessionCount; i++) {
        if (manager->sessions[i].active && 
            !isExpired(&manager->sessions[i], manager->sessionTimeout)) {
            count++;
        }
    }
    return count;
}

void freeTask53(Task53* manager) {
    free(manager);
}

int main() {
    srand(time(NULL));
    
    printf("Test Case 1: Create and validate session\\n");
    Task53* manager1 = createTask53(30);
    char* sessionId1 = createSession(manager1, "user123");
    printf("Session created: %s\\n", sessionId1);
    printf("Session valid: %s\\n", validateSession(manager1, sessionId1) ? "true" : "false");
    printf("User ID: %s\\n\\n", getUserId(manager1, sessionId1));
    
    printf("Test Case 2: Store and retrieve session data\\n");
    Task53* manager2 = createTask53(30);
    char* sessionId2 = createSession(manager2, "user456");
    setSessionData(manager2, sessionId2, "username", "JohnDoe");
    setSessionData(manager2, sessionId2, "email", "john@example.com");
    printf("Username: %s\\n", getSessionData(manager2, sessionId2, "username"));
    printf("Email: %s\\n\\n", getSessionData(manager2, sessionId2, "email"));
    
    printf("Test Case 3: Destroy session\\n");
    Task53* manager3 = createTask53(30);
    char* sessionId3 = createSession(manager3, "user789");
    printf("Before destroy - Valid: %s\\n", validateSession(manager3, sessionId3) ? "true" : "false");
    destroySession(manager3, sessionId3);
    printf("After destroy - Valid: %s\\n\\n", validateSession(manager3, sessionId3) ? "true" : "false");
    
    printf("Test Case 4: Multiple sessions\\n");
    Task53* manager4 = createTask53(30);
    char* session1 = createSession(manager4, "user1");
    char* session2 = createSession(manager4, "user2");
    char* session3 = createSession(manager4, "user3");
    printf("Active sessions: %d\\n", getActiveSessionCount(manager4));
    destroySession(manager4, session2);
    printf("After destroying one: %d\\n\\n", getActiveSessionCount(manager4));
    
    printf("Test Case 5: Invalid session operations\\n");
    Task53* manager5 = createTask53(30);
    printf("Validate non-existent session: %s\\n", validateSession(manager5, "invalid-id") ? "true" : "false");
    char* data = getSessionData(manager5, "invalid-id", "key");
    printf("Get data from non-existent session: %s\\n", data ? data : "(null)");
    char* validSession = createSession(manager5, "user999");
    destroySession(manager5, validSession);
    char* userId = getUserId(manager5, validSession);
    printf("Get user from destroyed session: %s\\n", userId ? userId : "(null)");
    
    freeTask53(manager1);
    freeTask53(manager2);
    freeTask53(manager3);
    freeTask53(manager4);
    freeTask53(manager5);
    
    return 0;
}
