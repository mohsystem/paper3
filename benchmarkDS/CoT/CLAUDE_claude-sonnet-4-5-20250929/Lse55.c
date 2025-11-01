
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_USERS 100
#define MAX_SESSIONS 100
#define MAX_STRING 256
#define SESSION_TIMEOUT 3600

typedef struct {
    char username[MAX_STRING];
    char passwordHash[MAX_STRING];
    char role[MAX_STRING];
    int active;
} User;

typedef struct {
    char token[MAX_STRING];
    char username[MAX_STRING];
    char role[MAX_STRING];
    time_t expiryTime;
    int active;
} Session;

static User users[MAX_USERS];
static Session sessions[MAX_SESSIONS];
static int userCount = 0;
static int sessionCount = 0;

void simpleHash(const char* input, char* output) {
    unsigned long hash = 5381;
    int c;
    while ((c = *input++))
        hash = ((hash << 5) + hash) + c;
    sprintf(output, "%lx", hash);
}

void generateToken(char* token) {
    sprintf(token, "token_%ld_%d", time(NULL), rand());
}

char* login() {
    return "LOGIN_PAGE: Please provide username and password";
}

void doLogin(const char* username, const char* password, char* status, char* message, char* token) {
    strcpy(status, "error");
    strcpy(message, "Invalid credentials");
    token[0] = '\\0';
    
    if (!username || !password || strlen(username) == 0 || strlen(password) == 0) {
        return;
    }
    
    char passHash[MAX_STRING];
    simpleHash(password, passHash);
    
    for (int i = 0; i < userCount; i++) {
        if (users[i].active && strcmp(users[i].username, username) == 0 && 
            strcmp(users[i].passwordHash, passHash) == 0) {
            
            if (sessionCount < MAX_SESSIONS) {
                generateToken(sessions[sessionCount].token);
                strcpy(sessions[sessionCount].username, users[i].username);
                strcpy(sessions[sessionCount].role, users[i].role);
                sessions[sessionCount].expiryTime = time(NULL) + SESSION_TIMEOUT;
                sessions[sessionCount].active = 1;
                
                strcpy(status, "success");
                strcpy(message, "Login successful");
                strcpy(token, sessions[sessionCount].token);
                sessionCount++;
            }
            return;
        }
    }
}

char* index_page() {
    return "INDEX_PAGE: Welcome to the home page";
}

void userPage(const char* sessionToken, char* result) {
    strcpy(result, "ERROR: Authentication required");
    
    if (!sessionToken || strlen(sessionToken) == 0) {
        return;
    }
    
    for (int i = 0; i < sessionCount; i++) {
        if (sessions[i].active && strcmp(sessions[i].token, sessionToken) == 0) {
            if (time(NULL) < sessions[i].expiryTime) {
                sprintf(result, "USER_PAGE: Welcome %s!", sessions[i].username);
                return;
            } else {
                strcpy(result, "ERROR: Invalid or expired session");
                return;
            }
        }
    }
    strcpy(result, "ERROR: Invalid or expired session");
}

void adminPage(const char* sessionToken, char* result) {
    strcpy(result, "ERROR: Authentication required");
    
    if (!sessionToken || strlen(sessionToken) == 0) {
        return;
    }
    
    for (int i = 0; i < sessionCount; i++) {
        if (sessions[i].active && strcmp(sessions[i].token, sessionToken) == 0) {
            if (time(NULL) >= sessions[i].expiryTime) {
                strcpy(result, "ERROR: Invalid or expired session");
                return;
            }
            if (strcmp(sessions[i].role, "admin") != 0) {
                strcpy(result, "ERROR: Unauthorized - Admin access required");
                return;
            }
            sprintf(result, "ADMIN_PAGE: Welcome Admin %s!", sessions[i].username);
            return;
        }
    }
    strcpy(result, "ERROR: Invalid or expired session");
}

void initTestUsers() {
    char hash[MAX_STRING];
    
    simpleHash("password123", hash);
    strcpy(users[0].username, "user1");
    strcpy(users[0].passwordHash, hash);
    strcpy(users[0].role, "user");
    users[0].active = 1;
    
    simpleHash("admin123", hash);
    strcpy(users[1].username, "admin1");
    strcpy(users[1].passwordHash, hash);
    strcpy(users[1].role, "admin");
    users[1].active = 1;
    
    userCount = 2;
}

int main() {
    srand(time(NULL));
    initTestUsers();
    
    char status[MAX_STRING], message[MAX_STRING], token[MAX_STRING], result[MAX_STRING];
    
    printf("=== Test Case 1: Login page ===\\n");
    printf("%s\\n\\n", login());
    
    printf("=== Test Case 2: Failed login ===\\n");
    doLogin("user1", "wrongpassword", status, message, token);
    printf("Status: %s, Message: %s\\n\\n", status, message);
    
    printf("=== Test Case 3: Successful user login ===\\n");
    doLogin("user1", "password123", status, message, token);
    printf("Status: %s, Message: %s\\n\\n", status, message);
    
    printf("=== Test Case 4: User accessing user page ===\\n");
    userPage(token, result);
    printf("%s\\n\\n", result);
    
    printf("=== Test Case 5: User accessing admin page (should fail) ===\\n");
    adminPage(token, result);
    printf("%s\\n\\n", result);
    
    printf("=== Test Case 6: Admin login and access ===\\n");
    char adminToken[MAX_STRING];
    doLogin("admin1", "admin123", status, message, adminToken);
    adminPage(adminToken, result);
    printf("%s\\n\\n", result);
    
    printf("=== Test Case 7: Index page ===\\n");
    printf("%s\\n", index_page());
    
    return 0;
}
