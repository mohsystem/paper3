
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_USERS 10
#define MAX_SESSIONS 50
#define MAX_STRING 256

typedef struct {
    char username[MAX_STRING];
    char password[MAX_STRING];
    char role[MAX_STRING];
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
} Lse55;

void initApp(Lse55* app) {
    app->userCount = 3;
    strcpy(app->users[0].username, "admin");
    strcpy(app->users[0].password, "admin123");
    strcpy(app->users[0].role, "admin");
    
    strcpy(app->users[1].username, "user1");
    strcpy(app->users[1].password, "pass123");
    strcpy(app->users[1].role, "user");
    
    strcpy(app->users[2].username, "user2");
    strcpy(app->users[2].password, "pass456");
    strcpy(app->users[2].role, "user");
    
    app->sessionCount = 0;
}

// GET: Login page
void login(char* output) {
    strcpy(output, "<html><body><h2>Login Page</h2>"
                   "<form method='POST' action='/do_login'>"
                   "Username: <input type='text' name='username'><br>"
                   "Password: <input type='password' name='password'><br>"
                   "<input type='submit' value='Login'>"
                   "</form></body></html>");
}

// POST: Process login
void do_login(Lse55* app, const char* username, const char* password, 
              char* output, char* sessionId) {
    for (int i = 0; i < app->userCount; i++) {
        if (strcmp(app->users[i].username, username) == 0 &&
            strcmp(app->users[i].password, password) == 0) {
            sprintf(sessionId, "%ld_%s", time(NULL), username);
            strcpy(app->sessions[app->sessionCount].sessionId, sessionId);
            strcpy(app->sessions[app->sessionCount].username, username);
            app->sessionCount++;
            sprintf(output, "Login successful! Session ID: %s", sessionId);
            return;
        }
    }
    strcpy(output, "Login failed! Invalid credentials.");
}

// GET: Index/Home page
void index_page(Lse55* app, const char* sessionId, char* output) {
    for (int i = 0; i < app->sessionCount; i++) {
        if (strcmp(app->sessions[i].sessionId, sessionId) == 0) {
            sprintf(output, "<html><body><h2>Welcome to the Index Page</h2>"
                           "<p>Hello, %s!</p>"
                           "<a href='/user_page'>User Page</a><br>"
                           "<a href='/admin_page'>Admin Page</a>"
                           "</body></html>", app->sessions[i].username);
            return;
        }
    }
    strcpy(output, "<html><body><h2>Index Page</h2>"
                   "<p>Please <a href='/login'>login</a> to continue.</p>"
                   "</body></html>");
}

// GET: User page
void user_page(Lse55* app, const char* sessionId, char* output) {
    for (int i = 0; i < app->sessionCount; i++) {
        if (strcmp(app->sessions[i].sessionId, sessionId) == 0) {
            sprintf(output, "<html><body><h2>User Page</h2>"
                           "<p>Welcome, %s!</p>"
                           "<p>This is your user dashboard.</p>"
                           "<a href='/index'>Back to Index</a>"
                           "</body></html>", app->sessions[i].username);
            return;
        }
    }
    strcpy(output, "<html><body><h2>Access Denied</h2>"
                   "<p>Please <a href='/login'>login</a> first.</p>"
                   "</body></html>");
}

// GET: Admin page
void admin_page(Lse55* app, const char* sessionId, char* output) {
    for (int i = 0; i < app->sessionCount; i++) {
        if (strcmp(app->sessions[i].sessionId, sessionId) == 0) {
            char* username = app->sessions[i].username;
            for (int j = 0; j < app->userCount; j++) {
                if (strcmp(app->users[j].username, username) == 0) {
                    if (strcmp(app->users[j].role, "admin") == 0) {
                        sprintf(output, "<html><body><h2>Admin Page</h2>"
                                       "<p>Welcome, Administrator %s!</p>"
                                       "<p>This is the admin control panel.</p>"
                                       "<a href='/index'>Back to Index</a>"
                                       "</body></html>", username);
                        return;
                    } else {
                        strcpy(output, "<html><body><h2>Access Denied</h2>"
                                      "<p>You don't have admin privileges.</p>"
                                      "<a href='/index'>Back to Index</a>"
                                      "</body></html>");
                        return;
                    }
                }
            }
        }
    }
    strcpy(output, "<html><body><h2>Access Denied</h2>"
                   "<p>Please <a href='/login'>login</a> first.</p>"
                   "</body></html>");
}

int main() {
    Lse55 app;
    initApp(&app);
    
    char output[2048];
    char sessionId[MAX_STRING];
    
    printf("Web Application Functions Test Cases:\\n\\n");
    
    // Test Case 1: Login page
    printf("Test Case 1: GET /login\\n");
    login(output);
    printf("%s\\n\\n", output);
    
    // Test Case 2: Login with admin credentials
    printf("Test Case 2: POST /do_login (admin/admin123)\\n");
    do_login(&app, "admin", "admin123", output, sessionId);
    printf("%s\\n\\n", output);
    
    // Test Case 3: Index page with session
    printf("Test Case 3: GET /index (with valid session)\\n");
    index_page(&app, sessionId, output);
    printf("%s\\n\\n", output);
    
    // Test Case 4: User page with session
    printf("Test Case 4: GET /user_page (with valid session)\\n");
    user_page(&app, sessionId, output);
    printf("%s\\n\\n", output);
    
    // Test Case 5: Admin page with admin session
    printf("Test Case 5: GET /admin_page (with admin session)\\n");
    admin_page(&app, sessionId, output);
    printf("%s\\n\\n", output);
    
    return 0;
}
