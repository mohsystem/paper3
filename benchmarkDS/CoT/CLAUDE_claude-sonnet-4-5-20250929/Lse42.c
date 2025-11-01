
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
    int active;
} Session;

Session sessions[MAX_SESSIONS];
int sessionCount = 0;

void escapeHtml(const char* input, char* output, size_t outputSize) {
    size_t j = 0;
    for (size_t i = 0; input[i] && j < outputSize - 1; i++) {
        if (input[i] == '<' && j < outputSize - 4) {
            strcpy(output + j, "&lt;");
            j += 4;
        } else if (input[i] == '>' && j < outputSize - 4) {
            strcpy(output + j, "&gt;");
            j += 4;
        } else if (input[i] == '&' && j < outputSize - 5) {
            strcpy(output + j, "&amp;");
            j += 5;
        } else {
            output[j++] = input[i];
        }
    }
    output[j] = '\\0';
}

void maskSSN(const char* ssn, char* masked, size_t maskedSize) {
    size_t len = strlen(ssn);
    if (len < 4) {
        strncpy(masked, "***-**-****", maskedSize - 1);
    } else {
        snprintf(masked, maskedSize, "***-**-%s", ssn + len - 4);
    }
    masked[maskedSize - 1] = '\\0';
}

void createSession(const char* sessionId, const char* username, 
                  const char* email, const char* ssn) {
    if (sessionCount >= MAX_SESSIONS) return;
    
    strncpy(sessions[sessionCount].sessionId, sessionId, MAX_STRING_LEN - 1);
    strncpy(sessions[sessionCount].username, username, MAX_STRING_LEN - 1);
    strncpy(sessions[sessionCount].email, email, MAX_STRING_LEN - 1);
    strncpy(sessions[sessionCount].ssn, ssn, MAX_STRING_LEN - 1);
    sessions[sessionCount].active = 1;
    sessionCount++;
}

void getInfo(const char* sessionId, char* result, size_t resultSize) {
    if (sessionId == NULL || strlen(sessionId) == 0) {
        strncpy(result, "Unauthorized: No session found", resultSize - 1);
        result[resultSize - 1] = '\\0';
        return;
    }
    
    for (int i = 0; i < sessionCount; i++) {
        if (sessions[i].active && strcmp(sessions[i].sessionId, sessionId) == 0) {
            char escapedUsername[MAX_STRING_LEN * 2];
            char escapedEmail[MAX_STRING_LEN * 2];
            char maskedSSN[MAX_STRING_LEN];
            
            escapeHtml(sessions[i].username, escapedUsername, sizeof(escapedUsername));
            escapeHtml(sessions[i].email, escapedEmail, sizeof(escapedEmail));
            maskSSN(sessions[i].ssn, maskedSSN, sizeof(maskedSSN));
            
            snprintf(result, resultSize, "Username: %s; Email: %s; SSN: %s",
                    escapedUsername, escapedEmail, maskedSSN);
            return;
        }
    }
    
    strncpy(result, "Unauthorized: Invalid session", resultSize - 1);
    result[resultSize - 1] = '\\0';
}

int main() {
    char result[1024];
    
    // Test case 1: Valid session
    createSession("session1", "john_doe", "john@example.com", "123-45-6789");
    getInfo("session1", result, sizeof(result));
    printf("Test Case 1: %s\\n", result);
    
    // Test case 2: Another valid session
    createSession("session2", "jane_smith", "jane@example.com", "987-65-4321");
    getInfo("session2", result, sizeof(result));
    printf("Test Case 2: %s\\n", result);
    
    // Test case 3: Invalid session
    getInfo("invalid_session", result, sizeof(result));
    printf("Test Case 3: %s\\n", result);
    
    // Test case 4: NULL session
    getInfo(NULL, result, sizeof(result));
    printf("Test Case 4: %s\\n", result);
    
    // Test case 5: XSS attempt
    createSession("session3", "<script>alert('XSS')</script>", "test@example.com", "111-11-1111");
    getInfo("session3", result, sizeof(result));
    printf("Test Case 5: %s\\n", result);
    
    return 0;
}
