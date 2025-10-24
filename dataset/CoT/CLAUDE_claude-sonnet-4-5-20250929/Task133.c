
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

#define MAX_EMAIL_LENGTH 256
#define MAX_PASSWORD_LENGTH 256
#define MAX_TOKEN_LENGTH 128
#define MAX_HASH_LENGTH 128
#define MIN_PASSWORD_LENGTH 8
#define TOKEN_EXPIRY_MINUTES 15
#define MAX_USERS 100
#define MAX_TOKENS 100
#define SALT_LENGTH 16

typedef struct {
    char email[MAX_EMAIL_LENGTH];
    char passwordHash[MAX_HASH_LENGTH];
    char salt[MAX_HASH_LENGTH];
    int active;
} UserAccount;

typedef struct {
    char token[MAX_TOKEN_LENGTH];
    char tokenHash[MAX_HASH_LENGTH];
    char email[MAX_EMAIL_LENGTH];
    time_t expiryTime;
    int used;
    int active;
} ResetToken;

typedef struct {
    UserAccount users[MAX_USERS];
    ResetToken tokens[MAX_TOKENS];
    int userCount;
    int tokenCount;
} Task133;

void base64Encode(const unsigned char* buffer, size_t length, char* output) {
    static const char base64_chars[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t i = 0, j = 0, out_pos = 0;
    unsigned char array_3[3], array_4[4];
    
    while (length--) {
        array_3[i++] = *(buffer++);
        if (i == 3) {
            array_4[0] = (array_3[0] & 0xfc) >> 2;
            array_4[1] = ((array_3[0] & 0x03) << 4) + ((array_3[1] & 0xf0) >> 4);
            array_4[2] = ((array_3[1] & 0x0f) << 2) + ((array_3[2] & 0xc0) >> 6);
            array_4[3] = array_3[2] & 0x3f;
            
            for(i = 0; i < 4; i++)
                output[out_pos++] = base64_chars[array_4[i]];
            i = 0;
        }
    }
    
    if (i) {
        for(j = i; j < 3; j++)
            array_3[j] = '\\0';
        
        array_4[0] = (array_3[0] & 0xfc) >> 2;
        array_4[1] = ((array_3[0] & 0x03) << 4) + ((array_3[1] & 0xf0) >> 4);
        array_4[2] = ((array_3[1] & 0x0f) << 2) + ((array_3[2] & 0xc0) >> 6);
        
        for (j = 0; j < i + 1; j++)
            output[out_pos++] = base64_chars[array_4[j]];
        
        while(i++ < 3)
            output[out_pos++] = '=';
    }
    
    output[out_pos] = '\\0';
}

void generateSalt(char* salt) {
    unsigned char saltBytes[SALT_LENGTH];
    RAND_bytes(saltBytes, SALT_LENGTH);
    base64Encode(saltBytes, SALT_LENGTH, salt);
}

void hashPassword(const char* password, const char* salt, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    char combined[MAX_PASSWORD_LENGTH + MAX_HASH_LENGTH];
    
    snprintf(combined, sizeof(combined), "%s%s", salt, password);
    
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, combined, strlen(combined));
    SHA256_Final(hash, &sha256);
    
    base64Encode(hash, SHA256_DIGEST_LENGTH, output);
}

void generateSecureToken(char* token) {
    unsigned char tokenBytes[32];
    RAND_bytes(tokenBytes, 32);
    base64Encode(tokenBytes, 32, token);
    
    size_t len = strlen(token);
    while (len > 0 && token[len - 1] == '=') {
        token[len - 1] = '\\0';
        len--;
    }
}

void toLowerCase(const char* str, char* output) {
    size_t i;
    for (i = 0; str[i]; i++) {
        output[i] = tolower((unsigned char)str[i]);
    }
    output[i] = '\\0';
}

int isPasswordValid(const char* password) {
    if (password == NULL || strlen(password) < MIN_PASSWORD_LENGTH) {
        return 0;
    }
    
    int hasLower = 0, hasUpper = 0, hasDigit = 0, hasSpecial = 0;
    size_t i;
    
    for (i = 0; password[i]; i++) {
        if (islower((unsigned char)password[i])) hasLower = 1;
        else if (isupper((unsigned char)password[i])) hasUpper = 1;
        else if (isdigit((unsigned char)password[i])) hasDigit = 1;
        else if (strchr("@$!%*?&", password[i])) hasSpecial = 1;
        else if (!isalnum((unsigned char)password[i]) && !strchr("@$!%*?&", password[i])) {
            return 0;
        }
    }
    
    return hasLower && hasUpper && hasDigit && hasSpecial;
}

void initTask133(Task133* task) {
    task->userCount = 0;
    task->tokenCount = 0;
    memset(task->users, 0, sizeof(task->users));
    memset(task->tokens, 0, sizeof(task->tokens));
}

int registerUser(Task133* task, const char* email, const char* password) {
    char normalizedEmail[MAX_EMAIL_LENGTH];
    int i;
    
    if (email == NULL || strlen(email) == 0) {
        return 0;
    }
    
    if (!isPasswordValid(password)) {
        return 0;
    }
    
    toLowerCase(email, normalizedEmail);
    
    for (i = 0; i < task->userCount; i++) {
        if (task->users[i].active && strcmp(task->users[i].email, normalizedEmail) == 0) {
            return 0;
        }
    }
    
    if (task->userCount >= MAX_USERS) {
        return 0;
    }
    
    UserAccount* user = &task->users[task->userCount];
    strncpy(user->email, normalizedEmail, MAX_EMAIL_LENGTH - 1);
    user->email[MAX_EMAIL_LENGTH - 1] = '\\0';
    
    generateSalt(user->salt);
    hashPassword(password, user->salt, user->passwordHash);
    user->active = 1;
    
    task->userCount++;
    return 1;
}

int requestPasswordReset(Task133* task, const char* email, char* tokenOutput) {
    char normalizedEmail[MAX_EMAIL_LENGTH];
    int i, userFound = 0;
    
    if (email == NULL || strlen(email) == 0) {
        return 0;
    }
    
    toLowerCase(email, normalizedEmail);
    
    for (i = 0; i < task->userCount; i++) {
        if (task->users[i].active && strcmp(task->users[i].email, normalizedEmail) == 0) {
            userFound = 1;
            break;
        }
    }
    
    if (!userFound) {
        return 0;
    }
    
    if (task->tokenCount >= MAX_TOKENS) {
        return 0;
    }
    
    ResetToken* resetToken = &task->tokens[task->tokenCount];
    generateSecureToken(resetToken->token);
    hashPassword(resetToken->token, "", resetToken->tokenHash);
    strncpy(resetToken->email, normalizedEmail, MAX_EMAIL_LENGTH - 1);
    resetToken->email[MAX_EMAIL_LENGTH - 1] = '\\0';
    resetToken->expiryTime = time(NULL) + (TOKEN_EXPIRY_MINUTES * 60);
    resetToken->used = 0;
    resetToken->active = 1;
    
    strncpy(tokenOutput, resetToken->token, MAX_TOKEN_LENGTH - 1);
    tokenOutput[MAX_TOKEN_LENGTH - 1] = '\\0';
    
    task->tokenCount++;
    return 1;
}

int resetPassword(Task133* task, const char* token, const char* newPassword) {
    int i, j, tokenFound = 0, userIndex = -1;
    ResetToken* resetToken = NULL;
    
    if (token == NULL || strlen(token) == 0) {
        return 0;
    }
    
    if (!isPasswordValid(newPassword)) {
        return 0;
    }
    
    for (i = 0; i < task->tokenCount; i++) {
        if (task->tokens[i].active && strcmp(task->tokens[i].token, token) == 0) {
            resetToken = &task->tokens[i];
            tokenFound = 1;
            break;
        }
    }
    
    if (!tokenFound || resetToken->used) {
        return 0;
    }
    
    if (time(NULL) > resetToken->expiryTime) {
        resetToken->active = 0;
        return 0;
    }
    
    for (j = 0; j < task->userCount; j++) {
        if (task->users[j].active && strcmp(task->users[j].email, resetToken->email) == 0) {
            userIndex = j;
            break;
        }
    }
    
    if (userIndex == -1) {
        return 0;
    }
    
    generateSalt(task->users[userIndex].salt);
    hashPassword(newPassword, task->users[userIndex].salt, task->users[userIndex].passwordHash);
    
    resetToken->used = 1;
    resetToken->active = 0;
    
    return 1;
}

int verifyPassword(Task133* task, const char* email, const char* password) {
    char normalizedEmail[MAX_EMAIL_LENGTH];
    char hashedPassword[MAX_HASH_LENGTH];
    int i;
    
    if (email == NULL || password == NULL) {
        return 0;
    }
    
    toLowerCase(email, normalizedEmail);
    
    for (i = 0; i < task->userCount; i++) {
        if (task->users[i].active && strcmp(task->users[i].email, normalizedEmail) == 0) {
            hashPassword(password, task->users[i].salt, hashedPassword);
            return strcmp(hashedPassword, task->users[i].passwordHash) == 0;
        }
    }
    
    return 0;
}

int main() {
    Task133 passwordReset;
    char token1[MAX_TOKEN_LENGTH];
    char token2[MAX_TOKEN_LENGTH];
    
    initTask133(&passwordReset);
    
    printf("=== Test Case 1: Register users ===\\n");
    int result1 = registerUser(&passwordReset, "user1@example.com", "SecurePass123!");
    printf("Register user1: %s\\n", result1 ? "true" : "false");
    int result2 = registerUser(&passwordReset, "user2@example.com", "AnotherPass456@");
    printf("Register user2: %s\\n", result2 ? "true" : "false");
    
    printf("\\n=== Test Case 2: Request password reset ===\\n");
    int tokenResult1 = requestPasswordReset(&passwordReset, "user1@example.com", token1);
    printf("Reset token generated: %s\\n", tokenResult1 ? "true" : "false");
    
    printf("\\n=== Test Case 3: Reset password with valid token ===\\n");
    int reset1 = resetPassword(&passwordReset, token1, "NewSecurePass789!");
    printf("Password reset successful: %s\\n", reset1 ? "true" : "false");
    int verify1 = verifyPassword(&passwordReset, "user1@example.com", "NewSecurePass789!");
    printf("New password verified: %s\\n", verify1 ? "true" : "false");
    
    printf("\\n=== Test Case 4: Try to reuse token ===\\n");
    int reset2 = resetPassword(&passwordReset, token1, "AnotherNewPass000!");
    printf("Reuse token (should fail): %s\\n", reset2 ? "true" : "false");
    
    printf("\\n=== Test Case 5: Invalid password reset ===\\n");
    int tokenResult2 = requestPasswordReset(&passwordReset, "user2@example.com", token2);
    int reset3 = resetPassword(&passwordReset, token2, "weak");
    printf("Reset with weak password (should fail): %s\\n", reset3 ? "true" : "false");
    int reset4 = resetPassword(&passwordReset, "invalid_token", "ValidPass123!");
    printf("Reset with invalid token (should fail): %s\\n", reset4 ? "true" : "false");
    
    return 0;
}
