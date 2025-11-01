
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_USERS 10
#define MAX_SESSIONS 100
#define MAX_STRING 256

typedef struct {
    char username[MAX_STRING];
    char password[MAX_STRING];
} User;

typedef struct {
    char sessionId[MAX_STRING];
    char username[MAX_STRING];
} Session;

typedef struct {
    User users[MAX_USERS];
    int userCount;
    Session sessions[MAX_SESSIONS];
    int sessionCount;
} Lse52;

void initLse52(Lse52* app) {
    app->userCount = 5;
    app->sessionCount = 0;
    
    strcpy(app->users[0].username, "admin");
    strcpy(app->users[0].password, "admin123");
    
    strcpy(app->users[1].username, "user1");
    strcpy(app->users[1].password, "password1");
    
    strcpy(app->users[2].username, "user2");
    strcpy(app->users[2].password, "password2");
    
    strcpy(app->users[3].username, "testuser");
    strcpy(app->users[3].password, "testpass");
    
    strcpy(app->users[4].username, "demo");
    strcpy(app->users[4].password, "demo123");
}

void renderLoginPage(char* output, const char* error) {
    strcpy(output, "<html><body><h2>Login Page</h2>");
    if (error != NULL && strlen(error) > 0) {
        strcat(output, "<p style='color:red;'>");
        strcat(output, error);
        strcat(output, "</p>");
    }
    strcat(output, "<form method='post' action='/login'>");
    strcat(output, "Username: <input type='text' name='username' required><br>");
    strcat(output, "Password: <input type='password' name='password' required><br>");
    strcat(output, "<input type='submit' value='Login'>");
    strcat(output, "</form>");
    strcat(output, "<a href='/'>Back to Home</a>");
    strcat(output, "</body></html>");
}

void renderIndexPage(char* output) {
    strcpy(output, "<html><body><h2>Welcome to Index Page</h2>");
    strcat(output, "<a href='/login'>Go to Login</a></body></html>");
}

void renderSecretPage(char* output, const char* username) {
    strcpy(output, "<html><body><h2>Secret settings</h2>");
    strcat(output, "<p>Welcome, ");
    strcat(output, username);
    strcat(output, "!</p>");
    strcat(output, "<a href='/logout'>Logout</a></body></html>");
}

int doLogin(Lse52* app, const char* username, const char* password) {
    if (username == NULL || password == NULL) {
        return 0;
    }
    
    for (int i = 0; i < app->userCount; i++) {
        if (strcmp(app->users[i].username, username) == 0 &&
            strcmp(app->users[i].password, password) == 0) {
            return 1;
        }
    }
    return 0;
}

void generateSessionId(char* sessionId) {
    sprintf(sessionId, "session_%ld_%d", time(NULL), rand());
}

void createSession(Lse52* app, const char* username, char* sessionId) {
    if (app->sessionCount < MAX_SESSIONS) {
        generateSessionId(sessionId);
        strcpy(app->sessions[app->sessionCount].sessionId, sessionId);
        strcpy(app->sessions[app->sessionCount].username, username);
        app->sessionCount++;
    }
}

const char* getUserFromSession(Lse52* app, const char* sessionId) {
    for (int i = 0; i < app->sessionCount; i++) {
        if (strcmp(app->sessions[i].sessionId, sessionId) == 0) {
            return app->sessions[i].username;
        }
    }
    return NULL;
}

void logoutSession(Lse52* app, const char* sessionId) {
    for (int i = 0; i < app->sessionCount; i++) {
        if (strcmp(app->sessions[i].sessionId, sessionId) == 0) {
            for (int j = i; j < app->sessionCount - 1; j++) {
                app->sessions[j] = app->sessions[j + 1];
            }
            app->sessionCount--;
            break;
        }
    }
}

int main() {
    Lse52 app;
    initLse52(&app);
    srand(time(NULL));
    
    char buffer[2048];
    char sessionId[MAX_STRING];
    
    printf("=== Login System Test Cases ===\\n\\n");
    
    // Test Case 1: Valid login
    printf("Test Case 1: Valid login\\n");
    int result1 = doLogin(&app, "admin", "admin123");
    printf("Result: %s\\n", result1 ? "SUCCESS" : "FAILED");
    if (result1) {
        createSession(&app, "admin", sessionId);
        printf("Session created: %s\\n", sessionId);
        renderSecretPage(buffer, "admin");
        printf("%s\\n", buffer);
    }
    printf("\\n");
    
    // Test Case 2: Invalid password
    printf("Test Case 2: Invalid password\\n");
    int result2 = doLogin(&app, "admin", "wrongpass");
    printf("Result: %s\\n\\n", result2 ? "SUCCESS" : "FAILED");
    
    // Test Case 3: Non-existent user
    printf("Test Case 3: Non-existent user\\n");
    int result3 = doLogin(&app, "nonexistent", "password");
    printf("Result: %s\\n\\n", result3 ? "SUCCESS" : "FAILED");
    
    // Test Case 4: Valid login with different user
    printf("Test Case 4: Valid login with user1\\n");
    int result4 = doLogin(&app, "user1", "password1");
    printf("Result: %s\\n", result4 ? "SUCCESS" : "FAILED");
    if (result4) {
        createSession(&app, "user1", sessionId);
        printf("Session created: %s\\n", sessionId);
    }
    printf("\\n");
    
    // Test Case 5: NULL credentials
    printf("Test Case 5: NULL credentials\\n");
    int result5 = doLogin(&app, NULL, NULL);
    printf("Result: %s\\n\\n", result5 ? "SUCCESS" : "FAILED");
    
    // Display pages
    printf("=== Page Renders ===\\n\\n");
    printf("Index Page:\\n");
    renderIndexPage(buffer);
    printf("%s\\n\\n", buffer);
    
    printf("Login Page:\\n");
    renderLoginPage(buffer, NULL);
    printf("%s\\n", buffer);
    
    return 0;
}
