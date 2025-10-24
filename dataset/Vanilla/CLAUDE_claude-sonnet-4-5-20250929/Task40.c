
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MAX_SESSIONS 100
#define MAX_TOKEN_LEN 65
#define MAX_STRING_LEN 256

typedef struct {
    char username[MAX_STRING_LEN];
    char email[MAX_STRING_LEN];
    char theme[20];
    bool notifications;
} UserSettings;

typedef struct {
    char sessionId[MAX_STRING_LEN];
    char token[MAX_TOKEN_LEN];
} CSRFToken;

typedef struct {
    char sessionId[MAX_STRING_LEN];
    UserSettings settings;
} UserData;

CSRFToken csrfTokens[MAX_SESSIONS];
UserData userDatabase[MAX_SESSIONS];
int tokenCount = 0;
int userCount = 0;

void generateRandomToken(char* token) {
    const char charset[] = "0123456789abcdef";
    for (int i = 0; i < 64; i++) {
        token[i] = charset[rand() % 16];
    }
    token[64] = '\\0';
}

char* generateCSRFToken(const char* sessionId) {
    static char token[MAX_TOKEN_LEN];
    generateRandomToken(token);
    
    // Store token
    if (tokenCount < MAX_SESSIONS) {
        strcpy(csrfTokens[tokenCount].sessionId, sessionId);
        strcpy(csrfTokens[tokenCount].token, token);
        tokenCount++;
    }
    
    return token;
}

bool validateCSRFToken(const char* sessionId, const char* token) {
    for (int i = 0; i < tokenCount; i++) {
        if (strcmp(csrfTokens[i].sessionId, sessionId) == 0 &&
            strcmp(csrfTokens[i].token, token) == 0) {
            return true;
        }
    }
    return false;
}

void getUserSettings(const char* sessionId, UserSettings* settings) {
    for (int i = 0; i < userCount; i++) {
        if (strcmp(userDatabase[i].sessionId, sessionId) == 0) {
            *settings = userDatabase[i].settings;
            return;
        }
    }
    // Default settings
    strcpy(settings->username, "john_doe");
    strcpy(settings->email, "john@example.com");
    strcpy(settings->theme, "light");
    settings->notifications = true;
}

char* generateSettingsForm(const char* sessionId, UserSettings* settings) {
    static char html[4096];
    char* token = generateCSRFToken(sessionId);
    
    snprintf(html, sizeof(html),
        "<!DOCTYPE html>\\n<html>\\n<head>\\n"
        "<title>User Settings</title>\\n"
        "<style>body{font-family:Arial;margin:20px;}"
        "form{max-width:400px;}input,select{width:100%%;padding:8px;margin:5px 0;}</style>\\n"
        "</head>\\n<body>\\n"
        "<h1>Update User Settings</h1>\\n"
        "<form method='POST' action='/update-settings'>\\n"
        "<input type='hidden' name='csrf_token' value='%s'>\\n"
        "<label>Username:</label>\\n"
        "<input type='text' name='username' value='%s' required><br>\\n"
        "<label>Email:</label>\\n"
        "<input type='email' name='email' value='%s' required><br>\\n"
        "<label>Theme:</label>\\n"
        "<select name='theme'>\\n"
        "<option value='light'%s>Light</option>\\n"
        "<option value='dark'%s>Dark</option>\\n"
        "</select><br>\\n"
        "<label><input type='checkbox' name='notifications' %s> Enable Notifications</label><br>\\n"
        "<input type='submit' value='Update Settings'>\\n"
        "</form>\\n</body>\\n</html>",
        token, settings->username, settings->email,
        strcmp(settings->theme, "light") == 0 ? " selected" : "",
        strcmp(settings->theme, "dark") == 0 ? " selected" : "",
        settings->notifications ? "checked" : "");
    
    return html;
}

char* processSettingsUpdate(const char* sessionId, const char* csrfToken,
                            const char* username, const char* email,
                            const char* theme, bool notifications) {
    static char result[512];
    
    if (!validateCSRFToken(sessionId, csrfToken)) {
        strcpy(result, "ERROR: Invalid CSRF token. Possible CSRF attack detected!");
        return result;
    }
    
    // Update or add user settings
    int idx = -1;
    for (int i = 0; i < userCount; i++) {
        if (strcmp(userDatabase[i].sessionId, sessionId) == 0) {
            idx = i;
            break;
        }
    }
    
    if (idx == -1 && userCount < MAX_SESSIONS) {
        idx = userCount++;
        strcpy(userDatabase[idx].sessionId, sessionId);
    }
    
    if (idx >= 0) {
        strcpy(userDatabase[idx].settings.username, username);
        strcpy(userDatabase[idx].settings.email, email);
        strcpy(userDatabase[idx].settings.theme, theme);
        userDatabase[idx].settings.notifications = notifications;
        
        snprintf(result, sizeof(result),
                "SUCCESS: Settings updated successfully - UserSettings{username='%s', email='%s', theme='%s', notifications=%s}",
                username, email, theme, notifications ? "true" : "false");
    }
    
    // Invalidate token
    for (int i = 0; i < tokenCount; i++) {
        if (strcmp(csrfTokens[i].sessionId, sessionId) == 0) {
            memmove(&csrfTokens[i], &csrfTokens[i+1], (tokenCount-i-1) * sizeof(CSRFToken));
            tokenCount--;
            break;
        }
    }
    
    return result;
}

int main() {
    srand(time(NULL));
    printf("=== CSRF Protection Demo - Web Application ===\\n\\n");
    
    // Test Case 1: Generate form for new session
    printf("Test Case 1: Generate form with CSRF token\\n");
    char session1[MAX_STRING_LEN];
    snprintf(session1, sizeof(session1), "session_%ld", time(NULL));
    UserSettings settings1;
    getUserSettings(session1, &settings1);
    char* form = generateSettingsForm(session1, &settings1);
    printf("Form generated with CSRF token (showing first 200 chars):\\n");
    printf("%.200s...\\n\\n", form);
    
    // Test Case 2: Valid update with correct CSRF token
    printf("Test Case 2: Valid update with correct CSRF token\\n");
    char* token2 = generateCSRFToken("session_user2");
    char* result2 = processSettingsUpdate("session_user2", token2, "alice",
                                         "alice@example.com", "dark", false);
    printf("%s\\n\\n", result2);
    
    // Test Case 3: Invalid update with wrong CSRF token
    printf("Test Case 3: Invalid update with wrong CSRF token\\n");
    generateCSRFToken("session_user3");
    char* result3 = processSettingsUpdate("session_user3", "invalid_token", "bob",
                                         "bob@example.com", "light", true);
    printf("%s\\n\\n", result3);
    
    // Test Case 4: Token reuse attempt
    printf("Test Case 4: Token reuse attempt\\n");
    char* token4 = generateCSRFToken("session_user4");
    processSettingsUpdate("session_user4", token4, "charlie", "charlie@example.com", "dark", true);
    char* result4 = processSettingsUpdate("session_user4", token4, "charlie_changed",
                                         "charlie2@example.com", "light", false);
    printf("%s\\n\\n", result4);
    
    // Test Case 5: Multiple users with different tokens
    printf("Test Case 5: Multiple concurrent sessions\\n");
    char* token5a = generateCSRFToken("session_user5a");
    char* token5b = generateCSRFToken("session_user5b");
    char* result5a = processSettingsUpdate("session_user5a", token5a, "david",
                                          "david@example.com", "light", true);
    char* result5b = processSettingsUpdate("session_user5b", token5b, "eve",
                                          "eve@example.com", "dark", false);
    printf("User A: %s\\n", result5a);
    printf("User B: %s\\n", result5b);
    
    return 0;
}
