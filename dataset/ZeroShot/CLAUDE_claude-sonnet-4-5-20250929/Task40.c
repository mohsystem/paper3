
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <regex.h>

#define MAX_SESSIONS 100
#define MAX_TOKEN_LEN 65
#define MAX_STRING_LEN 256
#define MAX_HTML_LEN 4096

typedef struct {
    char username[MAX_STRING_LEN];
    char email[MAX_STRING_LEN];
    char theme[MAX_STRING_LEN];
    int notifications;
} UserSettings;

typedef struct {
    char sessionId[MAX_STRING_LEN];
    char token[MAX_TOKEN_LEN];
    UserSettings settings;
    int active;
} SessionData;

SessionData sessions[MAX_SESSIONS];
int sessionCount = 0;

void generateRandomToken(char* token) {
    const char charset[] = "0123456789abcdef";
    for (int i = 0; i < 64; i++) {
        token[i] = charset[rand() % 16];
    }
    token[64] = '\\0';
}

char* generateCSRFToken(const char* sessionId) {
    for (int i = 0; i < sessionCount; i++) {
        if (strcmp(sessions[i].sessionId, sessionId) == 0) {
            generateRandomToken(sessions[i].token);
            return sessions[i].token;
        }
    }
    
    if (sessionCount < MAX_SESSIONS) {
        strcpy(sessions[sessionCount].sessionId, sessionId);
        generateRandomToken(sessions[sessionCount].token);
        sessions[sessionCount].active = 1;
        strcpy(sessions[sessionCount].settings.username, "");
        strcpy(sessions[sessionCount].settings.email, "");
        strcpy(sessions[sessionCount].settings.theme, "light");
        sessions[sessionCount].settings.notifications = 1;
        return sessions[sessionCount++].token;
    }
    
    return NULL;
}

int validateCSRFToken(const char* sessionId, const char* token) {
    if (sessionId == NULL || token == NULL || strlen(token) == 0) {
        return 0;
    }
    
    for (int i = 0; i < sessionCount; i++) {
        if (strcmp(sessions[i].sessionId, sessionId) == 0 && sessions[i].active) {
            return strcmp(sessions[i].token, token) == 0;
        }
    }
    return 0;
}

int validateEmail(const char* email) {
    regex_t regex;
    int ret;
    
    ret = regcomp(&regex, "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+$", REG_EXTENDED);
    if (ret) return 0;
    
    ret = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);
    
    return !ret;
}

char* updateUserSettings(const char* sessionId, const char* csrfToken,
                        const char* username, const char* email,
                        const char* theme, int notifications, char* result) {
    // Validate CSRF token
    if (!validateCSRFToken(sessionId, csrfToken)) {
        strcpy(result, "ERROR: Invalid CSRF token. Request rejected.");
        return result;
    }
    
    // Validate input
    if (username == NULL || strlen(username) == 0) {
        strcpy(result, "ERROR: Username cannot be empty.");
        return result;
    }
    
    if (!validateEmail(email)) {
        strcpy(result, "ERROR: Invalid email format.");
        return result;
    }
    
    // Update user settings
    for (int i = 0; i < sessionCount; i++) {
        if (strcmp(sessions[i].sessionId, sessionId) == 0) {
            strcpy(sessions[i].settings.username, username);
            strcpy(sessions[i].settings.email, email);
            strcpy(sessions[i].settings.theme, theme);
            sessions[i].settings.notifications = notifications;
            
            // Generate new CSRF token
            generateCSRFToken(sessionId);
            
            sprintf(result, "SUCCESS: User settings updated successfully. UserSettings{username='%s', email='%s', theme='%s', notifications=%d}",
                    username, email, theme, notifications);
            return result;
        }
    }
    
    strcpy(result, "ERROR: Session not found.");
    return result;
}

char* generateSettingsForm(const char* sessionId, char* html) {
    char* token = NULL;
    UserSettings* settings = NULL;
    
    for (int i = 0; i < sessionCount; i++) {
        if (strcmp(sessions[i].sessionId, sessionId) == 0) {
            token = sessions[i].token;
            settings = &sessions[i].settings;
            break;
        }
    }
    
    if (token == NULL) {
        token = generateCSRFToken(sessionId);
        for (int i = 0; i < sessionCount; i++) {
            if (strcmp(sessions[i].sessionId, sessionId) == 0) {
                settings = &sessions[i].settings;
                break;
            }
        }
    }
    
    sprintf(html, "<!DOCTYPE html>\\n<html>\\n<head>\\n<title>User Settings</title>\\n"
            "<style>body{font-family:Arial;padding:20px;}</style>\\n</head>\\n<body>\\n"
            "<h2>Update User Settings</h2>\\n<form method='POST' action='/update-settings'>\\n"
            "  <input type='hidden' name='csrf_token' value='%s'>\\n"
            "  <label>Username:</label><br>\\n"
            "  <input type='text' name='username' value='%s' required><br><br>\\n"
            "  <label>Email:</label><br>\\n"
            "  <input type='email' name='email' value='%s' required><br><br>\\n"
            "  <label>Theme:</label><br>\\n  <select name='theme'>\\n"
            "    <option value='light'%s>Light</option>\\n"
            "    <option value='dark'%s>Dark</option>\\n"
            "  </select><br><br>\\n"
            "  <label><input type='checkbox' name='notifications'%s> Enable Notifications</label><br><br>\\n"
            "  <button type='submit'>Update Settings</button>\\n</form>\\n</body>\\n</html>",
            token,
            settings ? settings->username : "",
            settings ? settings->email : "",
            settings && strcmp(settings->theme, "light") == 0 ? " selected" : "",
            settings && strcmp(settings->theme, "dark") == 0 ? " selected" : "",
            settings && settings->notifications ? " checked" : "");
    
    return html;
}

int main() {
    srand(time(NULL));
    char result[MAX_STRING_LEN * 2];
    
    printf("=== Web Application CSRF Protection Test Cases ===\\n\\n");
    
    // Test Case 1: Valid CSRF token - should succeed
    printf("Test Case 1: Valid CSRF token\\n");
    const char* session1 = "session_user1";
    char* token1 = generateCSRFToken(session1);
    updateUserSettings(session1, token1, "john_doe", "john@example.com", "dark", 1, result);
    printf("%s\\n\\n", result);
    
    // Test Case 2: Invalid CSRF token - should fail
    printf("Test Case 2: Invalid CSRF token\\n");
    const char* session2 = "session_user2";
    generateCSRFToken(session2);
    updateUserSettings(session2, "invalid_token_12345", "jane_doe", "jane@example.com", "light", 0, result);
    printf("%s\\n\\n", result);
    
    // Test Case 3: Missing CSRF token - should fail
    printf("Test Case 3: Missing CSRF token\\n");
    const char* session3 = "session_user3";
    generateCSRFToken(session3);
    updateUserSettings(session3, "", "alice", "alice@example.com", "dark", 1, result);
    printf("%s\\n\\n", result);
    
    // Test Case 4: Invalid email format - should fail
    printf("Test Case 4: Invalid email format\\n");
    const char* session4 = "session_user4";
    char* token4 = generateCSRFToken(session4);
    updateUserSettings(session4, token4, "bob", "invalid-email", "light", 0, result);
    printf("%s\\n\\n", result);
    
    // Test Case 5: Valid update with all fields
    printf("Test Case 5: Valid update with all fields\\n");
    const char* session5 = "session_user5";
    char* token5 = generateCSRFToken(session5);
    updateUserSettings(session5, token5, "charlie_brown", "charlie@example.com", "light", 1, result);
    printf("%s\\n\\n", result);
    
    // Demonstrate form generation
    printf("Test Case 6: Generate Settings Form HTML\\n");
    char html[MAX_HTML_LEN];
    generateSettingsForm(session1, html);
    printf("Form generated (first 200 chars): %.200s...\\n", html);
    
    return 0;
}
