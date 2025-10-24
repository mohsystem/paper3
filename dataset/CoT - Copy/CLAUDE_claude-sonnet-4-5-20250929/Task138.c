
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

#define MAX_USERNAME_LENGTH 50
#define MIN_PASSWORD_LENGTH 8
#define MAX_USERS 100
#define MAX_LOGIN_ATTEMPTS 3
#define SALT_LENGTH 32
#define HASH_LENGTH 64

typedef enum {
    PERM_READ = 1,
    PERM_WRITE = 2,
    PERM_DELETE = 4,
    PERM_ADMIN = 8
} Permission;

typedef struct {
    char username[MAX_USERNAME_LENGTH + 1];
    char passwordHash[HASH_LENGTH + 1];
    char salt[SALT_LENGTH + 1];
    int permissions;
} User;

typedef struct {
    User users[MAX_USERS];
    int userCount;
    int loginAttempts[MAX_USERS];
} PermissionManager;

int isValidUsername(const char* username) {
    if (username == NULL || strlen(username) < 3 || strlen(username) > MAX_USERNAME_LENGTH) {
        return 0;
    }
    
    for (size_t i = 0; i < strlen(username); i++) {
        if (!isalnum(username[i]) && username[i] != '_') {
            return 0;
        }
    }
    return 1;
}

int isValidPassword(const char* password) {
    return password != NULL && strlen(password) >= MIN_PASSWORD_LENGTH;
}

void generateSalt(char* salt) {
    unsigned char random[16];
    RAND_bytes(random, 16);
    
    for (int i = 0; i < 16; i++) {
        snprintf(salt + (i * 2), 3, "%02x", random[i]);
    }
    salt[SALT_LENGTH] = '\\0';
}

void hashPassword(const char* password, const char* salt, char* output) {
    char combined[256];
    snprintf(combined, sizeof(combined), "%s%s", salt, password);
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)combined, strlen(combined), hash);
    
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        snprintf(output + (i * 2), 3, "%02x", hash[i]);
    }
    output[HASH_LENGTH] = '\\0';
}

void initPermissionManager(PermissionManager* manager) {
    if (manager == NULL) return;
    manager->userCount = 0;
    memset(manager->loginAttempts, 0, sizeof(manager->loginAttempts));
}

int findUserIndex(PermissionManager* manager, const char* username) {
    if (manager == NULL || username == NULL) return -1;
    
    for (int i = 0; i < manager->userCount; i++) {
        if (strcmp(manager->users[i].username, username) == 0) {
            return i;
        }
    }
    return -1;
}

int addUser(PermissionManager* manager, const char* username, 
            const char* password, int permissions) {
    if (manager == NULL || username == NULL || password == NULL) {
        return 0;
    }
    
    if (!isValidUsername(username) || !isValidPassword(password)) {
        return 0;
    }
    
    if (manager->userCount >= MAX_USERS) {
        return 0;
    }
    
    if (findUserIndex(manager, username) != -1) {
        return 0;
    }
    
    User* user = &manager->users[manager->userCount];
    strncpy(user->username, username, MAX_USERNAME_LENGTH);
    user->username[MAX_USERNAME_LENGTH] = '\\0';
    
    generateSalt(user->salt);
    hashPassword(password, user->salt, user->passwordHash);
    user->permissions = permissions;
    
    manager->userCount++;
    return 1;
}

int authenticate(PermissionManager* manager, const char* username, const char* password) {
    if (manager == NULL || username == NULL || password == NULL) {
        return 0;
    }
    
    int userIndex = findUserIndex(manager, username);
    if (userIndex == -1) {
        return 0;
    }
    
    if (manager->loginAttempts[userIndex] >= MAX_LOGIN_ATTEMPTS) {
        return 0;
    }
    
    char hash[HASH_LENGTH + 1];
    hashPassword(password, manager->users[userIndex].salt, hash);
    
    if (strcmp(hash, manager->users[userIndex].passwordHash) == 0) {
        manager->loginAttempts[userIndex] = 0;
        return 1;
    } else {
        manager->loginAttempts[userIndex]++;
        return 0;
    }
}

int hasPermission(PermissionManager* manager, const char* username, Permission permission) {
    if (manager == NULL || username == NULL) {
        return 0;
    }
    
    int userIndex = findUserIndex(manager, username);
    if (userIndex == -1) {
        return 0;
    }
    
    int userPerms = manager->users[userIndex].permissions;
    return (userPerms & permission) || (userPerms & PERM_ADMIN);
}

int grantPermission(PermissionManager* manager, const char* adminUsername,
                   const char* targetUsername, Permission permission) {
    if (!hasPermission(manager, adminUsername, PERM_ADMIN)) {
        return 0;
    }
    
    int targetIndex = findUserIndex(manager, targetUsername);
    if (targetIndex == -1) {
        return 0;
    }
    
    manager->users[targetIndex].permissions |= permission;
    return 1;
}

int revokePermission(PermissionManager* manager, const char* adminUsername,
                    const char* targetUsername, Permission permission) {
    if (!hasPermission(manager, adminUsername, PERM_ADMIN)) {
        return 0;
    }
    
    int targetIndex = findUserIndex(manager, targetUsername);
    if (targetIndex == -1) {
        return 0;
    }
    
    manager->users[targetIndex].permissions &= ~permission;
    return 1;
}

int main() {
    PermissionManager manager;
    initPermissionManager(&manager);
    
    // Test Case 1: Add admin user
    int result1 = addUser(&manager, "admin", "Admin@1234", PERM_ADMIN);
    printf("Test 1 - Add admin user: %s\\n", result1 ? "PASSED" : "FAILED");
    
    // Test Case 2: Add regular user with READ permission
    int result2 = addUser(&manager, "user1", "User@12345", PERM_READ);
    printf("Test 2 - Add regular user: %s\\n", result2 ? "PASSED" : "FAILED");
    
    // Test Case 3: Authenticate user
    int result3 = authenticate(&manager, "admin", "Admin@1234");
    printf("Test 3 - Authenticate admin: %s\\n", result3 ? "PASSED" : "FAILED");
    
    // Test Case 4: Check permission
    int result4 = hasPermission(&manager, "user1", PERM_READ);
    printf("Test 4 - User1 has READ permission: %s\\n", result4 ? "PASSED" : "FAILED");
    
    // Test Case 5: Grant permission
    int result5 = grantPermission(&manager, "admin", "user1", PERM_WRITE);
    int hasWrite = hasPermission(&manager, "user1", PERM_WRITE);
    printf("Test 5 - Admin grants WRITE to user1: %s\\n", 
           (result5 && hasWrite) ? "PASSED" : "FAILED");
    
    return 0;
}
