
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <openssl/sha.h>

#define MAX_USERS 100
#define MAX_SESSIONS 100
#define MAX_STRING 256

typedef struct {
    char username[MAX_STRING];
    char passwordHash[65];
    char role[MAX_STRING];
} User;

typedef struct {
    char sessionId[MAX_STRING];
    char username[MAX_STRING];
} Session;

User users[MAX_USERS];
int userCount = 0;

Session sessions[MAX_SESSIONS];
int sessionCount = 0;

void hashPassword(const char* password, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\\0';
}

void generateSessionId(char* output) {
    sprintf(output, "session_%ld_%d", time(NULL), rand());
}

void initializeUsers() {
    char hash[65];
    
    hashPassword("admin123", hash);
    strcpy(users[userCount].username, "admin");
    strcpy(users[userCount].passwordHash, hash);
    strcpy(users[userCount].role, "admin");
    userCount++;
    
    hashPassword("user123", hash);
    strcpy(users[userCount].username, "user1");
    strcpy(users[userCount].passwordHash, hash);
    strcpy(users[userCount].role, "user");
    userCount++;
    
    hashPassword("pass123", hash);
    strcpy(users[userCount].username, "user2");
    strcpy(users[userCount].passwordHash, hash);
    strcpy(users[userCount].role, "user");
    userCount++;
    
    hashPassword("test123", hash);
    strcpy(users[userCount].username, "testuser");
    strcpy(users[userCount].passwordHash, hash);
    strcpy(users[userCount].role, "user");
    userCount++;
    
    hashPassword("john123", hash);
    strcpy(users[userCount].username, "john");
    strcpy(users[userCount].passwordHash, hash);
    strcpy(users[userCount].role, "user");
    userCount++;
}

char* login() {
    static char result[MAX_STRING];
    strcpy(result, "LOGIN_PAGE: Please enter username and password");
    return result;
}

char* doLogin(const char* username, const char* password, char* sessionIdOut) {
    static char result[MAX_STRING];
    
    if (!username || !password || strlen(username) == 0 || strlen(password) == 0) {
        strcpy(result, "ERROR: Username and password required");
        return result;
    }
    
    User* user = NULL;
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, username) == 0) {
            user = &users[i];
            break;
        }
    }
    
    if (!user) {
        strcpy(result, "ERROR: Invalid credentials");
        return result;
    }
    
    char hashedPassword[65];
    hashPassword(password, hashedPassword);
    
    if (strcmp(user->passwordHash, hashedPassword) != 0) {
        strcpy(result, "ERROR: Invalid credentials");
        return result;
    }
    
    char sessionId[MAX_STRING];
    generateSessionId(sessionId);
    strcpy(sessions[sessionCount].sessionId, sessionId);
    strcpy(sessions[sessionCount].username, username);
    sessionCount++;
    
    strcpy(sessionIdOut, sessionId);
    sprintf(result, "SUCCESS: Logged in as %s | Session: %s", username, sessionId);
    return result;
}

char* index_page(const char* sessionId) {
    static char result[MAX_STRING];
    
    if (!sessionId || strlen(sessionId) == 0) {
        strcpy(result, "INDEX_PAGE: Welcome! Please login to continue.");
        return result;
    }
    
    char* username = NULL;
    for (int i = 0; i < sessionCount; i++) {
        if (strcmp(sessions[i].sessionId, sessionId) == 0) {
            username = sessions[i].username;
            break;
        }
    }
    
    if (!username) {
        strcpy(result, "INDEX_PAGE: Welcome! Please login to continue.");
        return result;
    }
    
    sprintf(result, "INDEX_PAGE: Welcome %s! You are logged in.", username);
    return result;
}

char* userPage(const char* sessionId) {
    static char result[MAX_STRING];
    
    if (!sessionId || strlen(sessionId) == 0) {
        strcpy(result, "ERROR: Unauthorized. Please login first.");
        return result;
    }
    
    char* username = NULL;
    for (int i = 0; i < sessionCount; i++) {
        if (strcmp(sessions[i].sessionId, sessionId) == 0) {
            username = sessions[i].username;
            break;
        }
    }
    
    if (!username) {
        strcpy(result, "ERROR: Unauthorized. Please login first.");
        return result;
    }
    
    sprintf(result, "USER_PAGE: Hello %s! This is your user dashboard.", username);
    return result;
}

char* adminPage(const char* sessionId) {
    static char result[MAX_STRING];
    
    if (!sessionId || strlen(sessionId) == 0) {
        strcpy(result, "ERROR: Unauthorized. Please login first.");
        return result;
    }
    
    char* username = NULL;
    for (int i = 0; i < sessionCount; i++) {
        if (strcmp(sessions[i].sessionId, sessionId) == 0) {
            username = sessions[i].username;
            break;
        }
    }
    
    if (!username) {
        strcpy(result, "ERROR: Unauthorized. Please login first.");
        return result;
    }
    
    User* user = NULL;
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, username) == 0) {
            user = &users[i];
            break;
        }
    }
    
    if (!user || strcmp(user->role, "admin") != 0) {
        strcpy(result, "ERROR: Forbidden. Admin access required.");
        return result;
    }
    
    sprintf(result, "ADMIN_PAGE: Welcome Admin %s! You have full access.", username);
    return result;
}

int main() {
    srand(time(NULL));
    initializeUsers();
    
    printf("=== Test Case 1: Access login page ===\\n");
    printf("%s\\n", login());
    
    printf("\\n=== Test Case 2: Successful user login ===\\n");
    char sessionId1[MAX_STRING];
    printf("%s\\n", doLogin("user1", "user123", sessionId1));
    
    printf("\\n=== Test Case 3: Failed login with wrong password ===\\n");
    char dummySession[MAX_STRING];
    printf("%s\\n", doLogin("user1", "wrongpass", dummySession));
    
    printf("\\n=== Test Case 4: Access index and user page ===\\n");
    printf("%s\\n", index_page(sessionId1));
    printf("%s\\n", userPage(sessionId1));
    
    printf("\\n=== Test Case 5: Admin login and access admin page ===\\n");
    char sessionId2[MAX_STRING];
    printf("%s\\n", doLogin("admin", "admin123", sessionId2));
    printf("%s\\n", adminPage(sessionId2));
    printf("User trying to access admin page:\\n");
    printf("%s\\n", adminPage(sessionId1));
    
    return 0;
}
