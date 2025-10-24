#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// For secure random number generation
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <wincrypt.h>
#pragma comment(lib, "advapi32.lib")
#else
#include <fcntl.h>
#include <unistd.h>
#endif

// --- Configuration and Data Structures ---
#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 50
#define TOKEN_LEN 64       // 256 bits represented as 64 hex characters
#define MAX_USERS 10

typedef struct {
    char username[MAX_USERNAME_LEN];
    char passwordHash[MAX_PASSWORD_LEN + 8]; // Space for "hashed_" prefix
    char resetToken[TOKEN_LEN + 1];
} User;

// In a real application, this would be a database.
User userStore[MAX_USERS];
int userCount = 0;

/**
 * SECURITY: This is a placeholder for a real password hashing function.
 * NEVER store passwords in plaintext or with a simple scheme like this.
 * Use a standard, well-vetted C library like libsodium or Argon2.
 */
void hashPassword(const char* password, char* hashBuffer) {
    snprintf(hashBuffer, MAX_PASSWORD_LEN + 8, "hashed_%s", password);
}

/**
 * Validates if a new password meets the security policy.
 * (No regex in standard C, so we check manually).
 */
bool isPasswordStrong(const char* password) {
    int len = strlen(password);
    if (len < 12) return false;

    bool hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
    const char* specialChars = "@#$%^&+=!";

    for (int i = 0; i < len; ++i) {
        if (islower(password[i])) hasLower = true;
        else if (isupper(password[i])) hasUpper = true;
        else if (isdigit(password[i])) hasDigit = true;
        else if (strchr(specialChars, password[i])) hasSpecial = true;
    }
    return hasLower && hasUpper && hasDigit && hasSpecial;
}

/**
 * Generates a cryptographically secure random token.
 * Reads from OS-specific entropy sources.
 */
void generateResetToken(char* tokenBuffer) {
    unsigned char randomBytes[TOKEN_LEN / 2];
    
#if defined(_WIN32) || defined(_WIN64)
    HCRYPTPROV hCryptProv;
    if (!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        // Handle error, for this example we'll fail silently
        strcpy(tokenBuffer, "token_generation_failed");
        return;
    }
    if (!CryptGenRandom(hCryptProv, sizeof(randomBytes), randomBytes)) {
        // Handle error
        CryptReleaseContext(hCryptProv, 0);
        strcpy(tokenBuffer, "token_generation_failed");
        return;
    }
    CryptReleaseContext(hCryptProv, 0);
#else
    int urandom = open("/dev/urandom", O_RDONLY);
    if (urandom == -1) {
        strcpy(tokenBuffer, "token_generation_failed");
        return;
    }
    read(urandom, randomBytes, sizeof(randomBytes));
    close(urandom);
#endif

    for (size_t i = 0; i < sizeof(randomBytes); ++i) {
        sprintf(tokenBuffer + (i * 2), "%02x", randomBytes[i]);
    }
    tokenBuffer[TOKEN_LEN] = '\0';
}

User* findUser(const char* username) {
    for (int i = 0; i < userCount; ++i) {
        if (strcmp(userStore[i].username, username) == 0) {
            return &userStore[i];
        }
    }
    return NULL;
}

/**
 * Initiates a password reset request.
 * SECURITY: To prevent user enumeration, this function's observable behavior
 * should be the same whether the user exists or not.
 */
void requestPasswordReset(const char* username, char* tokenBuffer) {
    generateResetToken(tokenBuffer);
    User* user = findUser(username);
    if (user != NULL) {
        strncpy(user->resetToken, tokenBuffer, TOKEN_LEN);
        user->resetToken[TOKEN_LEN] = '\0';
    }
}

/**
 * Resets the user's password if the token is valid and the new password is strong.
 */
bool resetPassword(const char* username, const char* token, const char* newPassword) {
    if (!isPasswordStrong(newPassword)) {
        printf("Password reset failed for %s: New password is too weak.\n", username);
        return false;
    }

    User* user = findUser(username);
    if (user == NULL || strlen(user->resetToken) == 0 || strcmp(user->resetToken, token) != 0) {
        printf("Password reset failed for %s: Invalid username or token.\n", username);
        return false;
    }

    hashPassword(newPassword, user->passwordHash);
    user->resetToken[0] = '\0'; // Invalidate token

    printf("Password for %s has been reset successfully.\n", username);
    return true;
}

void addUser(const char* username, const char* password) {
    if (userCount >= MAX_USERS) return;
    strncpy(userStore[userCount].username, username, MAX_USERNAME_LEN - 1);
    userStore[userCount].username[MAX_USERNAME_LEN - 1] = '\0';
    hashPassword(password, userStore[userCount].passwordHash);
    userStore[userCount].resetToken[0] = '\0';
    userCount++;
}

int main() {
    addUser("alice", "Password123!");
    addUser("bob", "SecurePass456@");

    printf("--- Running 5 Test Cases ---\n");
    char tokenBuffer[TOKEN_LEN + 1];

    // Test Case 1: Successful Password Reset
    printf("\n--- Test Case 1: Successful Reset ---\n");
    requestPasswordReset("alice", tokenBuffer);
    printf("Alice requested a reset token: %s\n", tokenBuffer);
    bool success1 = resetPassword("alice", tokenBuffer, "NewStrongP@ssw0rd123");
    printf("Result: %s\n", success1 ? "SUCCESS" : "FAILURE");

    // Test Case 2: Failed Reset - Invalid Token
    printf("\n--- Test Case 2: Invalid Token ---\n");
    requestPasswordReset("bob", tokenBuffer);
    printf("Bob requested a reset token: %s\n", tokenBuffer);
    bool success2 = resetPassword("bob", "thisIsAWrongToken12345", "AnotherP@ssword1");
    printf("Result: %s\n", success2 ? "SUCCESS" : "FAILURE");

    // Test Case 3: Failed Reset - Weak Password
    printf("\n--- Test Case 3: Weak Password ---\n");
    bool success3 = resetPassword("bob", tokenBuffer, "weak");
    printf("Result: %s\n", success3 ? "SUCCESS" : "FAILURE");

    // Test Case 4: Failed Reset - Non-existent User
    printf("\n--- Test Case 4: Non-existent User ---\n");
    char charlieToken[TOKEN_LEN + 1];
    requestPasswordReset("charlie", charlieToken);
    printf("A token was generated for a potential user 'charlie': %s\n", charlieToken);
    bool success4 = resetPassword("charlie", charlieToken, "StrongP@ssword123");
    printf("Result: %s\n", success4 ? "SUCCESS" : "FAILURE");

    // Test Case 5: Failed Reset - Token Reuse
    printf("\n--- Test Case 5: Token Reuse ---\n");
    bool firstReset = resetPassword("bob", tokenBuffer, "BobNewP@ssw0rd987");
    printf("Bob's first reset attempt with valid token: %s\n", firstReset ? "SUCCESS" : "FAILURE");
    printf("Attempting to reuse Bob's token...\n");
    bool success5 = resetPassword("bob", tokenBuffer, "SomeOtherP@ssw0rd");
    printf("Result: %s\n", success5 ? "SUCCESS" : "FAILURE");

    return 0;
}