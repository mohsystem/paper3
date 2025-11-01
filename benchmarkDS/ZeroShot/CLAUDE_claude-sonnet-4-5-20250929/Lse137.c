
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <openssl/sha.h>

#define MAX_USERS 100
#define MAX_SESSIONS 100
#define MAX_STRING 256

typedef struct {
    char username[MAX_STRING];
    char passwordHash[65];
    bool isModerator;
} User;

typedef struct {
    char sessionId[MAX_STRING];
    char username[MAX_STRING];
    time_t expiryTime;
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

User* findUser(const char* username) {
    for(int i = 0; i < userCount; i++) {
        if(strcmp(users[i].username, username) == 0) {
            return &users[i];
        }
    }
    return NULL;
}

Session* findSession(const char* sessionId) {
    for(int i = 0; i < sessionCount; i++) {
        if(strcmp(sessions[i].sessionId, sessionId) == 0) {
            return &sessions[i];
        }
    }
    return NULL;
}

bool auth_mod(const char* username, const char* password, const char* sessionId) {
    if(!username || !password || !sessionId) {
        return false;
    }
    
    Session* session = findSession(sessionId);
    if(!session || time(NULL) >= session->expiryTime || strcmp(session->username, username) != 0) {
        return false;
    }
    
    User* user = findUser(username);
    if(!user) {
        return false;
    }
    
    char hashedPassword[65];
    hashPassword(password, hashedPassword);
    
    if(strcmp(user->passwordHash, hashedPassword) != 0) {
        return false;
    }
    
    return user->isModerator;
}

bool check_mod(const char* username, const char* password, const char* sessionId) {
    return auth_mod(username, password, sessionId);
}

void addUser(const char* username, const char* password, bool isModerator) {
    strcpy(users[userCount].username, username);
    hashPassword(password, users[userCount].passwordHash);
    users[userCount].isModerator = isModerator;
    userCount++;
}

void addSession(const char* sessionId, const char* username, time_t expiry) {
    strcpy(sessions[sessionCount].sessionId, sessionId);
    strcpy(sessions[sessionCount].username, username);
    sessions[sessionCount].expiryTime = expiry;
    sessionCount++;
}

int main() {
    // Setup test data
    addUser("admin", "admin123", true);
    addUser("user1", "pass123", false);
    addUser("mod1", "modpass", true);
    
    addSession("session123", "admin", time(NULL) + 3600);
    addSession("session456", "user1", time(NULL) + 3600);
    addSession("session789", "mod1", time(NULL) + 3600);
    
    // Test cases
    printf("Test 1 (Valid moderator): %s\\n", check_mod("admin", "admin123", "session123") ? "true" : "false");
    printf("Test 2 (Valid non-moderator): %s\\n", check_mod("user1", "pass123", "session456") ? "true" : "false");
    printf("Test 3 (Invalid password): %s\\n", check_mod("admin", "wrongpass", "session123") ? "true" : "false");
    printf("Test 4 (Invalid session): %s\\n", check_mod("admin", "admin123", "invalidsession") ? "true" : "false");
    printf("Test 5 (Another valid moderator): %s\\n", check_mod("mod1", "modpass", "session789") ? "true" : "false");
    
    return 0;
}
