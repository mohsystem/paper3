
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_SESSIONS 100
#define MAX_STRING_LEN 256

typedef struct {
    char sessionId[MAX_STRING_LEN];
    char username[MAX_STRING_LEN];
    char email[MAX_STRING_LEN];
    char ssn[MAX_STRING_LEN];
    int authenticated;
    int active;
} Session;

static Session sessionStore[MAX_SESSIONS];
static int sessionCount = 0;

void sanitize(char* output, const char* input) {
    if (!input || !output) return;
    
    int j = 0;
    for (int i = 0; input[i] != '\\0' && j < MAX_STRING_LEN - 1; i++) {
        char c = input[i];
        if (c != '<' && c != '>' && c != '"' && c != '\\'' && c != '&') {
            output[j++] = c;
        }
    }
    output[j] = '\\0';
}

void maskSSN(char* output, const char* ssn) {
    if (!ssn || strlen(ssn) < 4) {
        strcpy(output, "***");
        return;
    }
    sprintf(output, "XXX-XX-%.4s", ssn + strlen(ssn) - 4);
}

char* createSession(const char* sessionId, const char* username, 
                   const char* email, const char* ssn) {
    if (sessionCount >= MAX_SESSIONS) return NULL;
    
    Session* session = &sessionStore[sessionCount];
    strncpy(session->sessionId, sessionId, MAX_STRING_LEN - 1);
    sanitize(session->username, username);
    sanitize(session->email, email);
    maskSSN(session->ssn, ssn);
    session->authenticated = 1;
    session->active = 1;
    sessionCount++;
    
    return session->sessionId;
}

char* info(const char* sessionId, char* result) {
    if (!sessionId || strlen(sessionId) == 0) {
        strcpy(result, "Error: Invalid or expired session. Please log in.");
        return result;
    }
    
    for (int i = 0; i < sessionCount; i++) {
        if (sessionStore[i].active && 
            strcmp(sessionStore[i].sessionId, sessionId) == 0) {
            
            if (!sessionStore[i].authenticated) {
                strcpy(result, "Error: Unauthorized access.");
                return result;
            }
            
            sprintf(result, "User Information:\\nUsername: %s\\nEmail: %s\\nSSN: %s",
                   sessionStore[i].username,
                   sessionStore[i].email,
                   sessionStore[i].ssn);
            return result;
        }
    }
    
    strcpy(result, "Error: Invalid or expired session. Please log in.");
    return result;
}

int main() {
    char result[1024];
    
    printf("Testing /info route with session management\\n\\n");
    
    // Test case 1: Valid session
    createSession("sess001", "john_doe", "john@example.com", "123456789");
    printf("Test 1 - Valid session:\\n");
    printf("%s\\n\\n", info("sess001", result));
    
    // Test case 2: Another valid session
    createSession("sess002", "jane_smith", "jane@example.com", "987654321");
    printf("Test 2 - Another valid session:\\n");
    printf("%s\\n\\n", info("sess002", result));
    
    // Test case 3: Invalid session ID
    printf("Test 3 - Invalid session:\\n");
    printf("%s\\n\\n", info("invalid_session", result));
    
    // Test case 4: Empty session ID
    printf("Test 4 - Empty session:\\n");
    printf("%s\\n\\n", info("", result));
    
    // Test case 5: XSS attempt in username
    createSession("sess003", "<script>alert('xss')</script>user", 
                 "test@example.com", "555667777");
    printf("Test 5 - XSS attempt (sanitized):\\n");
    printf("%s\\n", info("sess003", result));
    
    return 0;
}
