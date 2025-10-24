
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>

#define SESSION_TIMEOUT_SECONDS 1800
#define TOKEN_LENGTH 32
#define MAX_SESSIONS 1000
#define MAX_ATTRIBUTES 10
#define MAX_STRING_LENGTH 256

typedef struct {
    char key[MAX_STRING_LENGTH];
    char value[MAX_STRING_LENGTH];
} Attribute;

typedef struct {
    char userId[MAX_STRING_LENGTH];
    time_t createdAt;
    time_t lastAccessedAt;
    Attribute attributes[MAX_ATTRIBUTES];
    int attributeCount;
    bool active;
} Session;

typedef struct {
    char token[TOKEN_LENGTH + 1];
    Session session;
} SessionEntry;

static SessionEntry sessions[MAX_SESSIONS];
static int sessionCount = 0;

void sanitizeInput(const char* input, char* output, size_t maxLen) {
    if (input == NULL || output == NULL) return;
    
    size_t j = 0;
    for (size_t i = 0; input[i] != '\\0' && j < maxLen - 1; i++) {
        if (isalnum(input[i]) || input[i] == '@' || input[i] == '.' || 
            input[i] == '_' || input[i] == '-') {
            output[j++] = input[i];
        }
    }
    output[j] = '\\0';
}

void generateSecureToken(char* token) {
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    srand((unsigned int)time(NULL) + rand());
    
    for (int i = 0; i < TOKEN_LENGTH; i++) {
        token[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    token[TOKEN_LENGTH] = '\\0';
}

bool isSessionExpired(const Session* session) {
    if (session == NULL) return true;
    return (time(NULL) - session->lastAccessedAt) > SESSION_TIMEOUT_SECONDS;
}

char* createSession(const char* userId) {
    if (userId == NULL || strlen(userId) == 0 || sessionCount >= MAX_SESSIONS) {
        return NULL;
    }
    
    static char token[TOKEN_LENGTH + 1];
    generateSecureToken(token);
    
    SessionEntry* entry = &sessions[sessionCount];
    strncpy(entry->token, token, TOKEN_LENGTH);
    entry->token[TOKEN_LENGTH] = '\\0';
    
    sanitizeInput(userId, entry->session.userId, MAX_STRING_LENGTH);
    entry->session.createdAt = time(NULL);
    entry->session.lastAccessedAt = time(NULL);
    entry->session.attributeCount = 0;
    entry->session.active = true;
    
    sessionCount++;
    return entry->token;
}

bool validateSession(const char* sessionToken) {
    if (sessionToken == NULL || strlen(sessionToken) == 0) {
        return false;
    }
    
    for (int i = 0; i < sessionCount; i++) {
        if (sessions[i].session.active && 
            strcmp(sessions[i].token, sessionToken) == 0) {
            
            if (isSessionExpired(&sessions[i].session)) {
                sessions[i].session.active = false;
                return false;
            }
            
            sessions[i].session.lastAccessedAt = time(NULL);
            return true;
        }
    }
    return false;
}

char* getSessionUserId(const char* sessionToken) {
    if (!validateSession(sessionToken)) {
        return NULL;
    }
    
    for (int i = 0; i < sessionCount; i++) {
        if (sessions[i].session.active && 
            strcmp(sessions[i].token, sessionToken) == 0) {
            return sessions[i].session.userId;
        }
    }
    return NULL;
}

bool invalidateSession(const char* sessionToken) {
    if (sessionToken == NULL || strlen(sessionToken) == 0) {
        return false;
    }
    
    for (int i = 0; i < sessionCount; i++) {
        if (strcmp(sessions[i].token, sessionToken) == 0) {
            sessions[i].session.active = false;
            return true;
        }
    }
    return false;
}

bool setSessionAttribute(const char* sessionToken, const char* key, const char* value) {
    if (!validateSession(sessionToken) || key == NULL || value == NULL) {
        return false;
    }
    
    for (int i = 0; i < sessionCount; i++) {
        if (sessions[i].session.active && 
            strcmp(sessions[i].token, sessionToken) == 0) {
            
            Session* session = &sessions[i].session;
            if (session->attributeCount >= MAX_ATTRIBUTES) {
                return false;
            }
            
            Attribute* attr = &session->attributes[session->attributeCount];
            sanitizeInput(key, attr->key, MAX_STRING_LENGTH);
            sanitizeInput(value, attr->value, MAX_STRING_LENGTH);
            session->attributeCount++;
            return true;
        }
    }
    return false;
}

char* getSessionAttribute(const char* sessionToken, const char* key) {
    if (!validateSession(sessionToken) || key == NULL) {
        return NULL;
    }
    
    char sanitizedKey[MAX_STRING_LENGTH];
    sanitizeInput(key, sanitizedKey, MAX_STRING_LENGTH);
    
    for (int i = 0; i < sessionCount; i++) {
        if (sessions[i].session.active && 
            strcmp(sessions[i].token, sessionToken) == 0) {
            
            Session* session = &sessions[i].session;
            for (int j = 0; j < session->attributeCount; j++) {
                if (strcmp(session->attributes[j].key, sanitizedKey) == 0) {
                    return session->attributes[j].value;
                }
            }
        }
    }
    return NULL;
}

int main() {
    printf("=== User Session Management Tests ===\\n\\n");
    
    // Test Case 1: Create valid session
    printf("Test 1: Create Session\\n");
    char* token1 = createSession("user123");
    printf("Session created: %s\\n", token1 ? "Success" : "Failed");
    printf("Token length: %zu\\n", token1 ? strlen(token1) : 0);
    
    // Test Case 2: Validate existing session
    printf("\\nTest 2: Validate Session\\n");
    bool isValid = validateSession(token1);
    printf("Session validation: %s\\n", isValid ? "Valid" : "Invalid");
    
    // Test Case 3: Set and get session attributes
    printf("\\nTest 3: Session Attributes\\n");
    setSessionAttribute(token1, "email", "user@example.com");
    setSessionAttribute(token1, "role", "admin");
    char* email = getSessionAttribute(token1, "email");
    char* role = getSessionAttribute(token1, "role");
    printf("Email: %s\\n", email ? email : "NULL");
    printf("Role: %s\\n", role ? role : "NULL");
    
    // Test Case 4: Get user ID from session
    printf("\\nTest 4: Get User ID\\n");
    char* userId = getSessionUserId(token1);
    printf("User ID: %s\\n", userId ? userId : "NULL");
    
    // Test Case 5: Invalidate session
    printf("\\nTest 5: Invalidate Session\\n");
    bool invalidated = invalidateSession(token1);
    printf("Session invalidated: %s\\n", invalidated ? "true" : "false");
    bool stillValid = validateSession(token1);
    printf("Session still valid: %s\\n", stillValid ? "true" : "false");
    
    return 0;
}
