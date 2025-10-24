
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SESSION_TIMEOUT_MINUTES 30
#define TOKEN_LENGTH 32
#define MAX_SESSIONS 100
#define MAX_TOKEN_SIZE 64
#define MAX_USER_ID_SIZE 50

typedef struct {
    char userId[MAX_USER_ID_SIZE];
    char token[MAX_TOKEN_SIZE];
    time_t createdAt;
    time_t lastAccessedAt;
    int active;
} Session;

typedef struct {
    Session sessions[MAX_SESSIONS];
    int sessionCount;
} SessionManager;

void initSessionManager(SessionManager* manager) {
    manager->sessionCount = 0;
    for (int i = 0; i < MAX_SESSIONS; i++) {
        manager->sessions[i].active = 0;
    }
}

void generateToken(char* token, int length) {
    static const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    for (int i = 0; i < length; i++) {
        int key = rand() % (sizeof(charset) - 1);
        token[i] = charset[key];
    }
    token[length] = '\\0';
}

char* createSession(SessionManager* manager, const char* userId, char* tokenOut) {
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
    
    generateToken(tokenOut, TOKEN_LENGTH);
    
    strncpy(manager->sessions[index].userId, userId, MAX_USER_ID_SIZE - 1);
    manager->sessions[index].userId[MAX_USER_ID_SIZE - 1] = '\\0';
    
    strncpy(manager->sessions[index].token, tokenOut, MAX_TOKEN_SIZE - 1);
    manager->sessions[index].token[MAX_TOKEN_SIZE - 1] = '\\0';
    
    manager->sessions[index].createdAt = time(NULL);
    manager->sessions[index].lastAccessedAt = time(NULL);
    manager->sessions[index].active = 1;
    manager->sessionCount++;
    
    return tokenOut;
}

int isSessionExpired(Session* session) {
    time_t now = time(NULL);
    double seconds = difftime(now, session->lastAccessedAt);
    return (seconds / 60) > SESSION_TIMEOUT_MINUTES;
}

int validateSession(SessionManager* manager, const char* token) {
    if (token == NULL || strlen(token) == 0) {
        return 0;
    }
    
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (manager->sessions[i].active && 
            strcmp(manager->sessions[i].token, token) == 0) {
            
            if (isSessionExpired(&manager->sessions[i])) {
                manager->sessions[i].active = 0;
                manager->sessionCount--;
                return 0;
            }
            
            manager->sessions[i].lastAccessedAt = time(NULL);
            return 1;
        }
    }
    
    return 0;
}

char* getSessionUser(SessionManager* manager, const char* token) {
    if (!validateSession(manager, token)) {
        return NULL;
    }
    
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (manager->sessions[i].active && 
            strcmp(manager->sessions[i].token, token) == 0) {
            return manager->sessions[i].userId;
        }
    }
    
    return NULL;
}

int destroySession(SessionManager* manager, const char* token) {
    if (token == NULL || strlen(token) == 0) {
        return 0;
    }
    
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (manager->sessions[i].active && 
            strcmp(manager->sessions[i].token, token) == 0) {
            manager->sessions[i].active = 0;
            manager->sessionCount--;
            return 1;
        }
    }
    
    return 0;
}

void cleanupExpiredSessions(SessionManager* manager) {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (manager->sessions[i].active && isSessionExpired(&manager->sessions[i])) {
            manager->sessions[i].active = 0;
            manager->sessionCount--;
        }
    }
}

int main() {
    srand(time(NULL));
    SessionManager manager;
    initSessionManager(&manager);
    
    char token1[MAX_TOKEN_SIZE];
    
    // Test Case 1: Create a session for a user
    printf("Test Case 1: Create Session\\n");
    char* result = createSession(&manager, "user123", token1);
    printf("Token created: %s\\n", (result != NULL ? "Yes" : "No"));
    printf("\\n");
    
    // Test Case 2: Validate an existing session
    printf("Test Case 2: Validate Session\\n");
    int isValid = validateSession(&manager, token1);
    printf("Session valid: %s\\n", (isValid ? "true" : "false"));
    printf("\\n");
    
    // Test Case 3: Get user from session
    printf("Test Case 3: Get Session User\\n");
    char* userId = getSessionUser(&manager, token1);
    printf("User ID: %s\\n", (userId != NULL ? userId : "NULL"));
    printf("\\n");
    
    // Test Case 4: Destroy a session
    printf("Test Case 4: Destroy Session\\n");
    int destroyed = destroySession(&manager, token1);
    printf("Session destroyed: %s\\n", (destroyed ? "true" : "false"));
    int validAfterDestroy = validateSession(&manager, token1);
    printf("Session valid after destroy: %s\\n", (validAfterDestroy ? "true" : "false"));
    printf("\\n");
    
    // Test Case 5: Invalid session token
    printf("Test Case 5: Invalid Session Token\\n");
    int invalidToken = validateSession(&manager, "invalid_token_xyz");
    printf("Invalid token validation: %s\\n", (invalidToken ? "true" : "false"));
    char* userFromInvalid = getSessionUser(&manager, "invalid_token_xyz");
    printf("User from invalid token: %s\\n", (userFromInvalid != NULL ? userFromInvalid : "NULL"));
    
    return 0;
}
