
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define TOKEN_LENGTH 32
#define TOKEN_EXPIRY_SECONDS 3600
#define MAX_SESSIONS 100
#define MAX_TOKEN_SIZE 64
#define MAX_SESSION_SIZE 64

typedef struct {
    char token[MAX_TOKEN_SIZE];
    time_t expiryTime;
    char sessionId[MAX_SESSION_SIZE];
    bool active;
} TokenData;

static TokenData tokenStore[MAX_SESSIONS];
static int tokenCount = 0;

static const char base64Chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

void base64Encode(const unsigned char* data, size_t length, char* output) {
    size_t outIndex = 0;
    
    for (size_t i = 0; i < length; i += 3) {
        unsigned int val = data[i] << 16;
        if (i + 1 < length) val |= data[i + 1] << 8;
        if (i + 2 < length) val |= data[i + 2];
        
        output[outIndex++] = base64Chars[(val >> 18) & 0x3F];
        output[outIndex++] = base64Chars[(val >> 12) & 0x3F];
        if (i + 1 < length) output[outIndex++] = base64Chars[(val >> 6) & 0x3F];
        if (i + 2 < length) output[outIndex++] = base64Chars[val & 0x3F];
    }
    
    output[outIndex] = '\\0';
}

bool constantTimeEquals(const char* a, const char* b) {
    size_t lenA = strlen(a);
    size_t lenB = strlen(b);
    
    if (lenA != lenB) {
        return false;
    }
    
    int result = 0;
    for (size_t i = 0; i < lenA; i++) {
        result |= a[i] ^ b[i];
    }
    
    return result == 0;
}

int findTokenIndex(const char* sessionId) {
    for (int i = 0; i < tokenCount; i++) {
        if (tokenStore[i].active && strcmp(tokenStore[i].sessionId, sessionId) == 0) {
            return i;
        }
    }
    return -1;
}

char* generateCSRFToken(const char* sessionId, char* outputToken) {
    if (sessionId == NULL || strlen(sessionId) == 0) {
        return NULL;
    }
    
    // Generate pseudo-random bytes (in production, use cryptographically secure random)
    unsigned char tokenBytes[TOKEN_LENGTH];
    srand((unsigned int)time(NULL) + tokenCount);
    
    for (int i = 0; i < TOKEN_LENGTH; i++) {
        tokenBytes[i] = (unsigned char)(rand() % 256);
    }
    
    base64Encode(tokenBytes, TOKEN_LENGTH, outputToken);
    
    // Store token
    if (tokenCount < MAX_SESSIONS) {
        int index = findTokenIndex(sessionId);
        if (index == -1) {
            index = tokenCount++;
        }
        
        strncpy(tokenStore[index].token, outputToken, MAX_TOKEN_SIZE - 1);
        tokenStore[index].token[MAX_TOKEN_SIZE - 1] = '\\0';
        tokenStore[index].expiryTime = time(NULL) + TOKEN_EXPIRY_SECONDS;
        strncpy(tokenStore[index].sessionId, sessionId, MAX_SESSION_SIZE - 1);
        tokenStore[index].sessionId[MAX_SESSION_SIZE - 1] = '\\0';
        tokenStore[index].active = true;
    }
    
    return outputToken;
}

bool validateCSRFToken(const char* sessionId, const char* token) {
    if (sessionId == NULL || strlen(sessionId) == 0) {
        return false;
    }
    
    if (token == NULL || strlen(token) == 0) {
        return false;
    }
    
    int index = findTokenIndex(sessionId);
    if (index == -1) {
        return false;
    }
    
    time_t currentTime = time(NULL);
    if (currentTime > tokenStore[index].expiryTime) {
        tokenStore[index].active = false;
        return false;
    }
    
    bool isValid = constantTimeEquals(tokenStore[index].token, token);
    
    if (isValid) {
        tokenStore[index].active = false;
    }
    
    return isValid;
}

void invalidateToken(const char* sessionId) {
    if (sessionId != NULL) {
        int index = findTokenIndex(sessionId);
        if (index != -1) {
            tokenStore[index].active = false;
        }
    }
}

int main() {
    printf("CSRF Protection Test Cases:\\n");
    printf("===========================\\n\\n");
    
    char token[MAX_TOKEN_SIZE];
    
    // Test Case 1: Valid token generation and validation
    printf("Test Case 1: Valid token generation and validation\\n");
    const char* session1 = "user_session_123";
    generateCSRFToken(session1, token);
    printf("Generated Token: %s\\n", token);
    bool result1 = validateCSRFToken(session1, token);
    printf("Validation Result: %s\\n", result1 ? "true" : "false");
    printf("Expected: true\\n\\n");
    
    // Test Case 2: Invalid token validation
    printf("Test Case 2: Invalid token validation\\n");
    const char* session2 = "user_session_456";
    generateCSRFToken(session2, token);
    bool result2 = validateCSRFToken(session2, "invalid_token_xyz");
    printf("Validation Result: %s\\n", result2 ? "true" : "false");
    printf("Expected: false\\n\\n");
    
    // Test Case 3: Token reuse prevention
    printf("Test Case 3: Token reuse prevention\\n");
    const char* session3 = "user_session_789";
    generateCSRFToken(session3, token);
    char savedToken[MAX_TOKEN_SIZE];
    strncpy(savedToken, token, MAX_TOKEN_SIZE);
    validateCSRFToken(session3, savedToken);
    bool result3 = validateCSRFToken(session3, savedToken);
    printf("Second Validation Result: %s\\n", result3 ? "true" : "false");
    printf("Expected: false (token already used)\\n\\n");
    
    // Test Case 4: Session mismatch
    printf("Test Case 4: Session mismatch\\n");
    const char* session4a = "user_session_aaa";
    const char* session4b = "user_session_bbb";
    generateCSRFToken(session4a, token);
    bool result4 = validateCSRFToken(session4b, token);
    printf("Validation Result: %s\\n", result4 ? "true" : "false");
    printf("Expected: false\\n\\n");
    
    // Test Case 5: NULL/empty input validation
    printf("Test Case 5: NULL/empty input validation\\n");
    bool result5a = validateCSRFToken(NULL, "some_token");
    bool result5b = validateCSRFToken("session", NULL);
    bool result5c = validateCSRFToken("", "token");
    printf("NULL session validation: %s\\n", result5a ? "true" : "false");
    printf("NULL token validation: %s\\n", result5b ? "true" : "false");
    printf("Empty session validation: %s\\n", result5c ? "true" : "false");
    printf("Expected: false, false, false\\n\\n");
    
    return 0;
}
