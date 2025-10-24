
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>

#define MAX_USERS 100
#define MAX_TOKENS 100
#define MAX_STRING 256

typedef struct {
    char username[MAX_STRING];
    char email[MAX_STRING];
    char passwordHash[65];
} User;

typedef struct {
    char token[65];
    char username[MAX_STRING];
    long long expiryTime;
} ResetToken;

typedef struct {
    User users[MAX_USERS];
    int userCount;
    ResetToken resetTokens[MAX_TOKENS];
    int tokenCount;
} Task133;

void hashPassword(const char* password, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\\0';
}

void generateToken(const char* username, char* output) {
    char data[MAX_STRING * 2];
    sprintf(data, "%s%ld%d", username, time(NULL), rand());
    hashPassword(data, output);
    output[32] = '\\0';
}

int findUser(Task133* system, const char* username) {
    for (int i = 0; i < system->userCount; i++) {
        if (strcmp(system->users[i].username, username) == 0) {
            return i;
        }
    }
    return -1;
}

int findToken(Task133* system, const char* token) {
    for (int i = 0; i < system->tokenCount; i++) {
        if (strcmp(system->resetTokens[i].token, token) == 0) {
            return i;
        }
    }
    return -1;
}

int registerUser(Task133* system, const char* username, const char* email, const char* password) {
    if (system->userCount >= MAX_USERS || findUser(system, username) != -1) {
        return 0;
    }
    
    User* user = &system->users[system->userCount];
    strcpy(user->username, username);
    strcpy(user->email, email);
    hashPassword(password, user->passwordHash);
    system->userCount++;
    return 1;
}

int requestPasswordReset(Task133* system, const char* username, char* tokenOut) {
    if (findUser(system, username) == -1 || system->tokenCount >= MAX_TOKENS) {
        return 0;
    }
    
    ResetToken* token = &system->resetTokens[system->tokenCount];
    generateToken(username, token->token);
    strcpy(token->username, username);
    token->expiryTime = time(NULL) * 1000 + 3600000;
    
    strcpy(tokenOut, token->token);
    system->tokenCount++;
    return 1;
}

int resetPassword(Task133* system, const char* token, const char* newPassword) {
    int tokenIdx = findToken(system, token);
    if (tokenIdx == -1) {
        return 0;
    }
    
    ResetToken* resetToken = &system->resetTokens[tokenIdx];
    if (time(NULL) * 1000 >= resetToken->expiryTime) {
        // Remove expired token
        for (int i = tokenIdx; i < system->tokenCount - 1; i++) {
            system->resetTokens[i] = system->resetTokens[i + 1];
        }
        system->tokenCount--;
        return 0;
    }
    
    int userIdx = findUser(system, resetToken->username);
    if (userIdx == -1) {
        return 0;
    }
    
    hashPassword(newPassword, system->users[userIdx].passwordHash);
    
    // Remove used token
    for (int i = tokenIdx; i < system->tokenCount - 1; i++) {
        system->resetTokens[i] = system->resetTokens[i + 1];
    }
    system->tokenCount--;
    return 1;
}

int verifyPassword(Task133* system, const char* username, const char* password) {
    int userIdx = findUser(system, username);
    if (userIdx == -1) {
        return 0;
    }
    
    char hash[65];
    hashPassword(password, hash);
    return strcmp(system->users[userIdx].passwordHash, hash) == 0;
}

int main() {
    Task133 system = {0};
    srand(time(NULL));
    
    // Test Case 1: Register a user and verify password
    printf("Test Case 1: Register user\\n");
    int registered = registerUser(&system, "john_doe", "john@example.com", "password123");
    printf("User registered: %s\\n", registered ? "true" : "false");
    printf("Password verified: %s\\n", verifyPassword(&system, "john_doe", "password123") ? "true" : "false");
    printf("\\n");
    
    // Test Case 2: Request password reset
    printf("Test Case 2: Request password reset\\n");
    char token[65];
    int tokenGenerated = requestPasswordReset(&system, "john_doe", token);
    printf("Reset token generated: %s\\n", tokenGenerated ? "true" : "false");
    printf("\\n");
    
    // Test Case 3: Reset password with valid token
    printf("Test Case 3: Reset password with valid token\\n");
    int resetSuccess = resetPassword(&system, token, "newPassword456");
    printf("Password reset successful: %s\\n", resetSuccess ? "true" : "false");
    printf("Old password verified: %s\\n", verifyPassword(&system, "john_doe", "password123") ? "true" : "false");
    printf("New password verified: %s\\n", verifyPassword(&system, "john_doe", "newPassword456") ? "true" : "false");
    printf("\\n");
    
    // Test Case 4: Try to reset with invalid token
    printf("Test Case 4: Reset with invalid token\\n");
    int invalidReset = resetPassword(&system, "invalidtoken123", "anotherPassword");
    printf("Reset with invalid token: %s\\n", invalidReset ? "true" : "false");
    printf("\\n");
    
    // Test Case 5: Request reset for non-existent user
    printf("Test Case 5: Request reset for non-existent user\\n");
    char invalidToken[65];
    int invalidTokenGenerated = requestPasswordReset(&system, "non_existent_user", invalidToken);
    printf("Token for non-existent user: %s\\n", invalidTokenGenerated ? "true" : "false");
    
    return 0;
}
