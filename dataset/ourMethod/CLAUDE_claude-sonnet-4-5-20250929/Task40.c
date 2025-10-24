
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>

#define MAX_USERNAME_LENGTH 50
#define MAX_EMAIL_LENGTH 100
#define MAX_THEME_LENGTH 20
#define MAX_TOKEN_LENGTH 256
#define MAX_SESSIONS 100
#define SECRET_KEY_SIZE 32

typedef struct {
    char sessionId[MAX_TOKEN_LENGTH];
    char token[MAX_TOKEN_LENGTH];
} SessionToken;

typedef struct {
    char sessionId[MAX_TOKEN_LENGTH];
    char username[MAX_USERNAME_LENGTH + 1];
    char email[MAX_EMAIL_LENGTH + 1];
    char theme[MAX_THEME_LENGTH + 1];
} UserSettings;

static unsigned char SECRET_KEY[SECRET_KEY_SIZE];
static SessionToken sessionTokens[MAX_SESSIONS];
static UserSettings userSettings[MAX_SESSIONS];
static int sessionCount = 0;
static int settingsCount = 0;

void base64_encode(const unsigned char* input, size_t length, char* output, size_t outLen) {
    static const char base64_chars[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    size_t i = 0, j = 0;
    
    if (outLen < ((length + 2) / 3) * 4 + 1) {
        output[0] = '\\0';
        return;
    }
    
    while (i < length) {
        unsigned char b1 = input[i++];
        unsigned char b2 = (i < length) ? input[i++] : 0;
        unsigned char b3 = (i < length) ? input[i++] : 0;
        
        output[j++] = base64_chars[b1 >> 2];
        output[j++] = base64_chars[((b1 & 0x03) << 4) | (b2 >> 4)];
        output[j++] = base64_chars[((b2 & 0x0F) << 2) | (b3 >> 6)];
        output[j++] = base64_chars[b3 & 0x3F];
    }
    output[j] = '\\0';
}

void generateRandomToken(char* output, size_t outLen) {
    unsigned char buffer[32];
    if (RAND_bytes(buffer, sizeof(buffer)) != 1) {
        output[0] = '\\0';
        return;
    }
    base64_encode(buffer, sizeof(buffer), output, outLen);
}

int generateCSRFToken(const char* sessionId, char* outputToken, size_t outLen) {
    if (!sessionId || !outputToken || outLen < MAX_TOKEN_LENGTH) {
        return 0;
    }
    
    char token[MAX_TOKEN_LENGTH];
    generateRandomToken(token, sizeof(token));
    
    char data[MAX_TOKEN_LENGTH * 2];
    int dataLen = snprintf(data, sizeof(data), "%s:%s", sessionId, token);
    if (dataLen < 0 || dataLen >= (int)sizeof(data)) {
        return 0;
    }
    
    unsigned char hmacResult[EVP_MAX_MD_SIZE];
    unsigned int hmacLen = 0;
    
    if (!HMAC(EVP_sha256(), SECRET_KEY, SECRET_KEY_SIZE,
              (unsigned char*)data, dataLen, hmacResult, &hmacLen)) {
        return 0;
    }
    
    char signature[MAX_TOKEN_LENGTH];
    base64_encode(hmacResult, hmacLen, signature, sizeof(signature));
    
    int len = snprintf(outputToken, outLen, "%s.%s", token, signature);
    if (len < 0 || len >= (int)outLen) {
        return 0;
    }
    
    if (sessionCount < MAX_SESSIONS) {
        strncpy(sessionTokens[sessionCount].sessionId, sessionId, MAX_TOKEN_LENGTH - 1);
        sessionTokens[sessionCount].sessionId[MAX_TOKEN_LENGTH - 1] = '\\0';
        strncpy(sessionTokens[sessionCount].token, outputToken, MAX_TOKEN_LENGTH - 1);
        sessionTokens[sessionCount].token[MAX_TOKEN_LENGTH - 1] = '\\0';
        sessionCount++;
    }
    
    return 1;
}

int constantTimeEquals(const char* a, const char* b) {
    if (!a || !b) return 0;
    
    size_t lenA = strlen(a);
    size_t lenB = strlen(b);
    
    if (lenA != lenB) return 0;
    
    int result = 0;
    for (size_t i = 0; i < lenA; i++) {
        result |= (unsigned char)a[i] ^ (unsigned char)b[i];
    }
    return result == 0;
}

int validateCSRFToken(const char* sessionId, const char* token) {
    if (!sessionId || !token || sessionId[0] == '\\0' || token[0] == '\\0') {
        return 0;
    }
    
    for (int i = 0; i < sessionCount; i++) {
        if (strcmp(sessionTokens[i].sessionId, sessionId) == 0) {
            return constantTimeEquals(sessionTokens[i].token, token);
        }
    }
    return 0;
}

void sanitizeInput(const char* input, char* output, size_t maxLength) {
    if (!input || !output) {
        if (output) output[0] = '\\0';
        return;
    }
    
    size_t j = 0;
    size_t len = strlen(input);
    
    for (size_t i = 0; i < len && j < maxLength; i++) {
        char c = input[i];
        if (c != '<' && c != '>' && c != '"' && c != '\\'' && c != '&' && !isspace((unsigned char)c)) {
            output[j++] = c;
        }
    }
    output[j] = '\\0';
}

int isValidEmail(const char* email) {
    if (!email || strlen(email) == 0 || strlen(email) > MAX_EMAIL_LENGTH) {
        return 0;
    }
    
    int atCount = 0;
    int dotAfterAt = 0;
    const char* atPos = NULL;
    
    for (const char* p = email; *p; p++) {
        if (*p == '@') {
            atCount++;
            atPos = p;
        } else if (atCount == 1 && *p == '.' && atPos && p > atPos) {
            dotAfterAt = 1;
        }
    }
    
    return (atCount == 1 && dotAfterAt);
}

const char* updateUserSettings(const char* sessionId, const char* csrfToken,
                               const char* username, const char* email, const char* theme) {
    if (!validateCSRFToken(sessionId, csrfToken)) {
        return "Error: Invalid CSRF token";
    }
    
    char cleanUsername[MAX_USERNAME_LENGTH + 1];
    char cleanEmail[MAX_EMAIL_LENGTH + 1];
    char cleanTheme[MAX_THEME_LENGTH + 1];
    
    sanitizeInput(username, cleanUsername, MAX_USERNAME_LENGTH);
    sanitizeInput(email, cleanEmail, MAX_EMAIL_LENGTH);
    sanitizeInput(theme, cleanTheme, MAX_THEME_LENGTH);
    
    if (strlen(cleanUsername) == 0) {
        return "Error: Username cannot be empty";
    }
    
    if (!isValidEmail(cleanEmail)) {
        return "Error: Invalid email format";
    }
    
    if (strcmp(cleanTheme, "light") != 0 && strcmp(cleanTheme, "dark") != 0 && 
        strcmp(cleanTheme, "auto") != 0) {
        return "Error: Invalid theme selection";
    }
    
    if (settingsCount < MAX_SESSIONS) {
        strncpy(userSettings[settingsCount].sessionId, sessionId, MAX_TOKEN_LENGTH - 1);
        userSettings[settingsCount].sessionId[MAX_TOKEN_LENGTH - 1] = '\\0';
        strncpy(userSettings[settingsCount].username, cleanUsername, MAX_USERNAME_LENGTH);
        userSettings[settingsCount].username[MAX_USERNAME_LENGTH] = '\\0';
        strncpy(userSettings[settingsCount].email, cleanEmail, MAX_EMAIL_LENGTH);
        userSettings[settingsCount].email[MAX_EMAIL_LENGTH] = '\\0';
        strncpy(userSettings[settingsCount].theme, cleanTheme, MAX_THEME_LENGTH);
        userSettings[settingsCount].theme[MAX_THEME_LENGTH] = '\\0';
        settingsCount++;
    }
    
    return "Success: Settings updated successfully";
}

int main(void) {
    if (RAND_bytes(SECRET_KEY, SECRET_KEY_SIZE) != 1) {
        fprintf(stderr, "Failed to initialize secret key\\n");
        return 1;
    }
    
    printf("Test Case 1: Valid CSRF token\\n");
    char token1[MAX_TOKEN_LENGTH];
    generateCSRFToken("session1", token1, sizeof(token1));
    const char* result1 = updateUserSettings("session1", token1, "john_doe", "john@example.com", "dark");
    printf("%s\\n", result1);
    
    printf("\\nTest Case 2: Invalid CSRF token\\n");
    char token2[MAX_TOKEN_LENGTH];
    generateCSRFToken("session2", token2, sizeof(token2));
    const char* result2 = updateUserSettings("session2", "invalid_token", "jane_doe", "jane@example.com", "light");
    printf("%s\\n", result2);
    
    printf("\\nTest Case 3: Invalid email format\\n");
    char token3[MAX_TOKEN_LENGTH];
    generateCSRFToken("session3", token3, sizeof(token3));
    const char* result3 = updateUserSettings("session3", token3, "bob", "invalid-email", "auto");
    printf("%s\\n", result3);
    
    printf("\\nTest Case 4: Empty username\\n");
    char token4[MAX_TOKEN_LENGTH];
    generateCSRFToken("session4", token4, sizeof(token4));
    const char* result4 = updateUserSettings("session4", token4, "", "alice@example.com", "light");
    printf("%s\\n", result4);
    
    printf("\\nTest Case 5: Invalid theme\\n");
    char token5[MAX_TOKEN_LENGTH];
    generateCSRFToken("session5", token5, sizeof(token5));
    const char* result5 = updateUserSettings("session5", token5, "charlie", "charlie@example.com", "invalid_theme");
    printf("%s\\n", result5);
    
    return 0;
}
