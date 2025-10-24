
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

#define MAX_USERS 100
#define MAX_USERNAME 50
#define MAX_PASSWORD 100
#define SALT_LENGTH 16
#define HASH_LENGTH 65

typedef struct {
    char username[MAX_USERNAME];
    char passwordHash[HASH_LENGTH];
    char salt[SALT_LENGTH * 2 + 1];
    int active;
} UserCredentials;

typedef struct {
    UserCredentials users[MAX_USERS];
    int userCount;
} Task42;

void generateSalt(char* salt) {
    unsigned char randomBytes[SALT_LENGTH];
    RAND_bytes(randomBytes, SALT_LENGTH);
    
    for (int i = 0; i < SALT_LENGTH; i++) {
        sprintf(salt + (i * 2), "%02x", randomBytes[i]);
    }
    salt[SALT_LENGTH * 2] = '\\0';
}

void hashPassword(const char* password, const char* salt, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    char saltedPassword[MAX_PASSWORD + SALT_LENGTH * 2 + 1];
    
    strcpy(saltedPassword, salt);
    strcat(saltedPassword, password);
    
    SHA256((unsigned char*)saltedPassword, strlen(saltedPassword), hash);
    
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\\0';
}

Task42* createAuthSystem() {
    Task42* system = (Task42*)malloc(sizeof(Task42));
    system->userCount = 0;
    for (int i = 0; i < MAX_USERS; i++) {
        system->users[i].active = 0;
    }
    return system;
}

int registerUser(Task42* system, const char* username, const char* password) {
    if (!username || !password || strlen(username) == 0 || strlen(password) < 8) {
        return 0;
    }
    
    if (system->userCount >= MAX_USERS) {
        return 0;
    }
    
    for (int i = 0; i < system->userCount; i++) {
        if (system->users[i].active && strcmp(system->users[i].username, username) == 0) {
            return 0;
        }
    }
    
    UserCredentials* newUser = &system->users[system->userCount];
    strcpy(newUser->username, username);
    
    generateSalt(newUser->salt);
    hashPassword(password, newUser->salt, newUser->passwordHash);
    
    newUser->active = 1;
    system->userCount++;
    return 1;
}

int authenticateUser(Task42* system, const char* username, const char* password) {
    if (!username || !password) {
        return 0;
    }
    
    for (int i = 0; i < system->userCount; i++) {
        if (system->users[i].active && strcmp(system->users[i].username, username) == 0) {
            char hashedInput[HASH_LENGTH];
            hashPassword(password, system->users[i].salt, hashedInput);
            return strcmp(hashedInput, system->users[i].passwordHash) == 0;
        }
    }
    return 0;
}

int changePassword(Task42* system, const char* username, const char* oldPassword, 
                   const char* newPassword) {
    if (!authenticateUser(system, username, oldPassword)) {
        return 0;
    }
    
    if (!newPassword || strlen(newPassword) < 8) {
        return 0;
    }
    
    for (int i = 0; i < system->userCount; i++) {
        if (system->users[i].active && strcmp(system->users[i].username, username) == 0) {
            generateSalt(system->users[i].salt);
            hashPassword(newPassword, system->users[i].salt, system->users[i].passwordHash);
            return 1;
        }
    }
    return 0;
}

void freeAuthSystem(Task42* system) {
    free(system);
}

int main() {
    Task42* authSystem = createAuthSystem();
    
    // Test Case 1: Register new user
    printf("Test 1 - Register user 'alice': %s\\n", 
           registerUser(authSystem, "alice", "SecurePass123") ? "true" : "false");
    
    // Test Case 2: Authenticate valid user
    printf("Test 2 - Login alice with correct password: %s\\n", 
           authenticateUser(authSystem, "alice", "SecurePass123") ? "true" : "false");
    
    // Test Case 3: Authenticate with wrong password
    printf("Test 3 - Login alice with wrong password: %s\\n", 
           authenticateUser(authSystem, "alice", "WrongPass123") ? "true" : "false");
    
    // Test Case 4: Register duplicate username
    printf("Test 4 - Register duplicate user 'alice': %s\\n", 
           registerUser(authSystem, "alice", "AnotherPass123") ? "true" : "false");
    
    // Test Case 5: Change password
    registerUser(authSystem, "bob", "OldPassword123");
    printf("Test 5 - Change bob's password: %s\\n", 
           changePassword(authSystem, "bob", "OldPassword123", "NewPassword123") ? "true" : "false");
    printf("Test 5 - Login bob with new password: %s\\n", 
           authenticateUser(authSystem, "bob", "NewPassword123") ? "true" : "false");
    
    freeAuthSystem(authSystem);
    return 0;
}
