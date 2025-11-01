
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SESSIONS 100
#define MAX_STRING_LEN 256

/* Simple session structure */
typedef struct {
    char sessionId[MAX_STRING_LEN];
    char username[MAX_STRING_LEN];
    char email[MAX_STRING_LEN];
    char ssn[MAX_STRING_LEN];
    int active;
} Session;

/* Global session storage (in production, use proper session management) */
static Session sessionStore[MAX_SESSIONS];
static int sessionCount = 0;

/**
 * Sanitizes output to prevent injection attacks.
 * SECURITY: Removes potentially dangerous characters.
 */
void sanitizeForOutput(char* dest, const char* src, size_t destSize) {
    if (dest == NULL || src == NULL || destSize == 0) {
        return;
    }
    
    size_t j = 0;
    /* Copy only safe characters, bounds checked */
    for (size_t i = 0; i < strlen(src) && j < destSize - 1; i++) {
        char c = src[i];
        /* Skip potentially dangerous characters */
        if (c != '<' && c != '>' && c != '&' && c != '"' && c != '\\'') {
            dest[j++] = c;
        }
    }
    dest[j] = '\\0'; /* Ensure null termination */
}

/**
 * Masks email to show only first character and domain.
 * SECURITY: Prevents full email exposure in responses.
 */
void maskEmail(char* dest, const char* email, size_t destSize) {
    if (dest == NULL || email == NULL || destSize < 4) {
        return;
    }
    
    const char* atSign = strchr(email, '@');
    if (atSign == NULL || atSign == email) {
        strncpy(dest, "***", destSize - 1);
        dest[destSize - 1] = '\\0';
        return;
    }
    
    /* Build masked email: first_char***@domain */
    snprintf(dest, destSize, "%c***@%s", email[0], atSign + 1);
}

/**
 * Masks SSN to show only last 4 digits.
 * SECURITY: Prevents full SSN exposure in responses.
 */
void maskSsn(char* dest, const char* ssn, size_t destSize) {
    if (dest == NULL || ssn == NULL || destSize < 4) {
        return;
    }
    
    size_t len = strlen(ssn);
    if (len < 4) {
        strncpy(dest, "***", destSize - 1);
        dest[destSize - 1] = '\\0';
        return;
    }
    
    /* Show only last 4 digits */
    snprintf(dest, destSize, "XXX-XX-%.4s", ssn + len - 4);
}

/**
 * Returns user information from session.
 * SECURITY: Never log sensitive data like SSN or email.
 * Returns only redacted/masked information to prevent exposure.
 */
char* info(const char* sessionId, char* result, size_t resultSize) {
    /* Validate input parameters - fail closed */
    if (sessionId == NULL || result == NULL || resultSize == 0) {
        if (result != NULL && resultSize > 0) {
            strncpy(result, "Error: Invalid input", resultSize - 1);
            result[resultSize - 1] = '\\0';
        }
        return result;
    }
    
    /* Validate session ID is not empty */
    if (strlen(sessionId) == 0) {
        strncpy(result, "Error: Invalid session", resultSize - 1);
        result[resultSize - 1] = '\\0';
        return result;
    }
    
    /* Find session - treat all input as untrusted */
    Session* session = NULL;
    for (int i = 0; i < sessionCount; i++) {
        if (sessionStore[i].active && strcmp(sessionStore[i].sessionId, sessionId) == 0) {
            session = &sessionStore[i];
            break;
        }
    }
    
    if (session == NULL) {
        /* Session not found - return generic error */
        strncpy(result, "Error: Session not found", resultSize - 1);
        result[resultSize - 1] = '\\0';
        return result;
    }
    
    /* SECURITY: Sanitize and mask all sensitive data */
    char sanitizedUsername[MAX_STRING_LEN];
    char maskedEmail[MAX_STRING_LEN];
    char maskedSsn[MAX_STRING_LEN];
    
    /* Initialize buffers to zero for safety */
    memset(sanitizedUsername, 0, sizeof(sanitizedUsername));
    memset(maskedEmail, 0, sizeof(maskedEmail));
    memset(maskedSsn, 0, sizeof(maskedSsn));
    
    sanitizeForOutput(sanitizedUsername, session->username, sizeof(sanitizedUsername));
    maskEmail(maskedEmail, session->email, sizeof(maskedEmail));
    maskSsn(maskedSsn, session->ssn, sizeof(maskedSsn));
    
    /* Build result string with bounds checking */
    snprintf(result, resultSize, "Username: %s, Email: %s, SSN: %s", 
             sanitizedUsername, maskedEmail, maskedSsn);
    
    return result;
}

/* Helper function to setup test sessions */
void setupTestSession(const char* sessionId, const char* username, 
                      const char* email, const char* ssn) {
    if (sessionCount >= MAX_SESSIONS) {
        return;
    }
    
    Session* s = &sessionStore[sessionCount];
    strncpy(s->sessionId, sessionId, MAX_STRING_LEN - 1);
    strncpy(s->username, username, MAX_STRING_LEN - 1);
    strncpy(s->email, email, MAX_STRING_LEN - 1);
    strncpy(s->ssn, ssn, MAX_STRING_LEN - 1);
    
    /* Ensure null termination */
    s->sessionId[MAX_STRING_LEN - 1] = '\\0';
    s->username[MAX_STRING_LEN - 1] = '\\0';
    s->email[MAX_STRING_LEN - 1] = '\\0';
    s->ssn[MAX_STRING_LEN - 1] = '\\0';
    
    s->active = 1;
    sessionCount++;
}

int main(void) {
    char result[512];
    
    printf("=== Test Cases ===\\n");
    
    /* Test case 1: Valid session */
    setupTestSession("session1", "john_doe", "john@example.com", "123456789");
    info("session1", result, sizeof(result));
    printf("Test 1: %s\\n", result);
    
    /* Test case 2: Another valid session */
    setupTestSession("session2", "jane_smith", "jane@test.org", "987654321");
    info("session2", result, sizeof(result));
    printf("Test 2: %s\\n", result);
    
    /* Test case 3: Invalid/non-existent session */
    info("invalid_session", result, sizeof(result));
    printf("Test 3: %s\\n", result);
    
    /* Test case 4: Empty session ID */
    info("", result, sizeof(result));
    printf("Test 4: %s\\n", result);
    
    /* Test case 5: Session with special characters in username */
    setupTestSession("session3", "<script>alert('xss')</script>", "user@domain.com", "111223333");
    info("session3", result, sizeof(result));
    printf("Test 5: %s\\n", result);
    
    printf("==================\\n");
    
    return 0;
}
