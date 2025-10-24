
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define TOKEN_LENGTH 32
#define TOKEN_EXPIRY_MINUTES 15
#define MAX_USERS 100
#define MAX_TOKENS 100
#define MAX_EMAIL_LEN 100
#define MAX_PASSWORD_LEN 100

typedef struct {
    char email[MAX_EMAIL_LEN];
    char passwordHash[MAX_PASSWORD_LEN];
    int active;
} User;

typedef struct {
    char token[TOKEN_LENGTH + 1];
    char email[MAX_EMAIL_LEN];
    time_t expiryTime;
    int active;
} ResetToken;

User users[MAX_USERS];
ResetToken resetTokens[MAX_TOKENS];
int userCount = 0;
int tokenCount = 0;

int isValidEmail(const char* email) {
    if (email == NULL || strlen(email) == 0) return 0;
    
    int atCount = 0;
    int dotAfterAt = 0;
    int atPos = -1;
    
    for (int i = 0; email[i]; i++) {
        if (email[i] == '@') {
            atCount++;
            atPos = i;
        }
        if (atCount == 1 && email[i] == '.' && i > atPos) {
            dotAfterAt = 1;
        }
    }
    
    return atCount == 1 && dotAfterAt && atPos > 0;
}

int isValidPassword(const char* password) {
    if (password == NULL || strlen(password) < 8) return 0;
    
    int hasUpper = 0, hasLower = 0, hasDigit = 0, hasSpecial = 0;
    const char* specialChars = "@$!%*?&";
    
    for (int i = 0; password[i]; i++) {
        if (isupper(password[i])) hasUpper = 1;
        else if (islower(password[i])) hasLower = 1;
        else if (isdigit(password[i])) hasDigit = 1;
        else if (strchr(specialChars, password[i])) hasSpecial = 1;
    }
    
    return hasUpper && hasLower && hasDigit && hasSpecial;
}

void hashPassword(const char* password, char* output) {
    snprintf(output, MAX_PASSWORD_LEN, "HASHED_%s", password);
}

void generateRandomToken(char* output) {
    const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    int charsLen = strlen(chars);
    
    for (int i = 0; i < TOKEN_LENGTH; i++) {
        output[i] = chars[rand() % charsLen];
    }
    output[TOKEN_LENGTH] = '\\0';
}

char* registerUser(const char* email, const char* password, char* result) {
    if (!isValidEmail(email)) {
        strcpy(result, "Error: Invalid email format");
        return result;
    }
    
    for (int i = 0; i < userCount; i++) {
        if (users[i].active && strcmp(users[i].email, email) == 0) {
            strcpy(result, "Error: User already exists");
            return result;
        }
    }
    
    if (userCount >= MAX_USERS) {
        strcpy(result, "Error: Maximum users reached");
        return result;
    }
    
    strcpy(users[userCount].email, email);
    hashPassword(password, users[userCount].passwordHash);
    users[userCount].active = 1;
    userCount++;
    
    strcpy(result, "Success: User registered");
    return result;
}

char* generateResetToken(const char* email, char* result) {
    if (!isValidEmail(email)) {
        strcpy(result, "Error: Invalid email format");
        return result;
    }
    
    int userFound = 0;
    for (int i = 0; i < userCount; i++) {
        if (users[i].active && strcmp(users[i].email, email) == 0) {
            userFound = 1;
            break;
        }
    }
    
    if (!userFound) {
        strcpy(result, "Error: Email not found");
        return result;
    }
    
    if (tokenCount >= MAX_TOKENS) {
        strcpy(result, "Error: Maximum tokens reached");
        return result;
    }
    
    generateRandomToken(resetTokens[tokenCount].token);
    strcpy(resetTokens[tokenCount].email, email);
    resetTokens[tokenCount].expiryTime = time(NULL) + (TOKEN_EXPIRY_MINUTES * 60);
    resetTokens[tokenCount].active = 1;
    
    strcpy(result, resetTokens[tokenCount].token);
    tokenCount++;
    
    return result;
}

char* resetPassword(const char* token, const char* newPassword, char* result) {
    if (token == NULL || strlen(token) == 0) {
        strcpy(result, "Error: Token is required");
        return result;
    }
    
    if (!isValidPassword(newPassword)) {
        strcpy(result, "Error: Password must be at least 8 characters with uppercase, lowercase, digit, and special character");
        return result;
    }
    
    int tokenIndex = -1;
    for (int i = 0; i < tokenCount; i++) {
        if (resetTokens[i].active && strcmp(resetTokens[i].token, token) == 0) {
            tokenIndex = i;
            break;
        }
    }
    
    if (tokenIndex == -1) {
        strcpy(result, "Error: Invalid token");
        return result;
    }
    
    if (time(NULL) > resetTokens[tokenIndex].expiryTime) {
        resetTokens[tokenIndex].active = 0;
        strcpy(result, "Error: Token has expired");
        return result;
    }
    
    for (int i = 0; i < userCount; i++) {
        if (users[i].active && strcmp(users[i].email, resetTokens[tokenIndex].email) == 0) {
            hashPassword(newPassword, users[i].passwordHash);
            break;
        }
    }
    
    resetTokens[tokenIndex].active = 0;
    strcpy(result, "Success: Password reset successfully");
    return result;
}

int main() {
    srand(time(NULL));
    char result[200];
    
    printf("=== Password Reset Functionality Tests ===\\n\\n");
    
    // Test Case 1: Register users
    printf("Test 1: Register users\\n");
    registerUser("user1@example.com", "Password123!", result);
    printf("%s\\n", result);
    registerUser("user2@example.com", "Secure456@", result);
    printf("%s\\n\\n", result);
    
    // Test Case 2: Generate reset token for valid user
    printf("Test 2: Generate reset token for valid user\\n");
    generateResetToken("user1@example.com", result);
    printf("Token generated: %s\\n\\n", strstr(result, "Error") == result ? result : "Success (Token created)");
    
    // Test Case 3: Generate reset token for invalid email
    printf("Test 3: Generate reset token for invalid email\\n");
    generateResetToken("invalidemail", result);
    printf("%s\\n\\n", result);
    
    // Test Case 4: Reset password with valid token and valid password
    printf("Test 4: Reset password with valid token\\n");
    generateResetToken("user2@example.com", result);
    if (strstr(result, "Error") != result) {
        resetPassword(result, "NewPass789#", result);
        printf("%s\\n", result);
    }
    printf("\\n");
    
    // Test Case 5: Reset password with invalid token
    printf("Test 5: Reset password with invalid token\\n");
    resetPassword("invalidtoken123", "ValidPass123!", result);
    printf("%s\\n\\n", result);
    
    // Test Case 6: Reset password with weak password
    printf("Test 6: Reset password with weak password\\n");
    generateResetToken("user1@example.com", result);
    if (strstr(result, "Error") != result) {
        char token[TOKEN_LENGTH + 1];
        strcpy(token, result);
        resetPassword(token, "weak", result);
        printf("%s\\n", result);
    }
    
    return 0;
}
