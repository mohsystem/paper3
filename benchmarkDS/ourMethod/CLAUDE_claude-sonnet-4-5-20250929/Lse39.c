
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 128
#define SALT_LENGTH 16
#define MAX_USERS 100
#define MAX_LOGIN_ATTEMPTS 3
#define LOCKOUT_TIME_SECONDS 300

typedef struct {
    char username[MAX_USERNAME_LEN + 1];
    unsigned char passwordHash[SHA256_DIGEST_LENGTH];
    unsigned char salt[SALT_LENGTH];
    int failedAttempts;
    time_t lockoutUntil;
    int active;
} UserCredential;

static UserCredential userDatabase[MAX_USERS];
static int userCount = 0;

/* Generate cryptographically secure random salt */
int generateSalt(unsigned char* salt, size_t length) {
    if (RAND_bytes(salt, (int)length) != 1) {
        return 0;
    }
    return 1;
}

/* Hash password with unique salt using SHA-256 and multiple iterations */
void hashPassword(const char* password, const unsigned char* salt, unsigned char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    size_t pwdLen;
    int i;
    
    if (password == NULL || salt == NULL || output == NULL) {
        return;
    }
    
    pwdLen = strlen(password);
    
    /* Initial hash with salt */
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, salt, SALT_LENGTH);
    SHA256_Update(&sha256, password, pwdLen);
    SHA256_Final(hash, &sha256);
    
    /* Multiple iterations to slow down brute force attacks */
    for (i = 0; i < 10000; i++) {
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, hash, SHA256_DIGEST_LENGTH);
        SHA256_Final(hash, &sha256);
    }
    
    memcpy(output, hash, SHA256_DIGEST_LENGTH);
}

/* Constant-time comparison to prevent timing attacks */
int constantTimeCompare(const unsigned char* a, const unsigned char* b, size_t length) {
    unsigned char result = 0;
    size_t i;
    
    for (i = 0; i < length; i++) {
        result |= a[i] ^ b[i];
    }
    
    return result == 0;
}

/* Validate username format */
int isValidUsername(const char* username) {
    size_t len;
    size_t i;
    
    if (username == NULL || username[0] == '\\0') {
        return 0;
    }
    
    len = strlen(username);
    if (len > MAX_USERNAME_LEN) {
        return 0;
    }
    
    for (i = 0; i < len; i++) {
        if (!isalnum((unsigned char)username[i])) {
            return 0;
        }
    }
    
    return 1;
}

/* Add user to database with hashed password */
int addUser(const char* username, const char* password) {
    UserCredential* user;
    
    /* Input validation */
    if (username == NULL || password == NULL || username[0] == '\\0' || password[0] == '\\0') {
        return 0;
    }
    
    if (!isValidUsername(username)) {
        return 0;
    }
    
    if (strlen(password) > MAX_PASSWORD_LEN) {
        return 0;
    }
    
    if (userCount >= MAX_USERS) {
        return 0;
    }
    
    user = &userDatabase[userCount];
    
    /* Copy username */
    strncpy(user->username, username, MAX_USERNAME_LEN);
    user->username[MAX_USERNAME_LEN] = '\\0';
    
    /* Generate unique salt and hash password */
    if (!generateSalt(user->salt, SALT_LENGTH)) {
        return 0;
    }
    
    hashPassword(password, user->salt, user->passwordHash);
    
    user->failedAttempts = 0;
    user->lockoutUntil = 0;
    user->active = 1;
    
    userCount++;
    return 1;
}

/* Find user in database */
UserCredential* findUser(const char* username) {
    int i;
    
    if (username == NULL) {
        return NULL;
    }
    
    for (i = 0; i < userCount; i++) {
        if (userDatabase[i].active && strcmp(userDatabase[i].username, username) == 0) {
            return &userDatabase[i];
        }
    }
    
    return NULL;
}

/* Verify password */
int verifyPassword(const char* username, const char* password) {
    UserCredential* user;
    unsigned char providedHash[SHA256_DIGEST_LENGTH];
    unsigned char dummySalt[SALT_LENGTH];
    unsigned char dummyHash[SHA256_DIGEST_LENGTH];
    int matches;
    
    /* Input validation */
    if (username == NULL || password == NULL || username[0] == '\\0' || password[0] == '\\0') {
        return 0;
    }
    
    user = findUser(username);
    
    if (user == NULL) {
        /* Perform dummy hash to prevent timing attacks */
        if (generateSalt(dummySalt, SALT_LENGTH)) {
            hashPassword(password, dummySalt, dummyHash);
        }
        return 0;
    }
    
    /* Check if account is locked */
    if (user->lockoutUntil > time(NULL)) {
        return 0;
    }
    
    /* Hash provided password with stored salt */
    hashPassword(password, user->salt, providedHash);
    
    /* Constant-time comparison */
    matches = constantTimeCompare(providedHash, user->passwordHash, SHA256_DIGEST_LENGTH);
    
    if (matches) {
        /* Reset failed attempts on success */
        user->failedAttempts = 0;
        user->lockoutUntil = 0;
    } else {
        /* Increment failed attempts */
        user->failedAttempts++;
        if (user->failedAttempts >= MAX_LOGIN_ATTEMPTS) {
            user->lockoutUntil = time(NULL) + LOCKOUT_TIME_SECONDS;
        }
    }
    
    return matches;
}

/* Simulate session page redirect */
void redirectToSessionPage(const char* username) {
    printf("Login successful! Redirecting to session page...\\n");
    printf("Welcome, %s!\\n", username);
}

/* Main login function */
const char* login(const char* username, const char* password) {
    char sanitizedUsername[MAX_USERNAME_LEN + 1];
    size_t i, j;
    size_t len;
    UserCredential* user;
    
    /* Input validation */
    if (username == NULL || password == NULL || username[0] == '\\0' || password[0] == '\\0') {
        return "Invalid username or password";
    }
    
    /* Length validation */
    len = strlen(username);
    if (len > MAX_USERNAME_LEN || strlen(password) > MAX_PASSWORD_LEN) {
        return "Invalid username or password";
    }
    
    /* Sanitize username - allow only alphanumeric */
    j = 0;
    for (i = 0; i < len && j < MAX_USERNAME_LEN; i++) {
        if (isalnum((unsigned char)username[i])) {
            sanitizedUsername[j++] = username[i];
        }
    }
    sanitizedUsername[j] = '\\0';
    
    if (j == 0 || strcmp(sanitizedUsername, username) != 0) {
        return "Invalid username or password";
    }
    
    /* Check if account is locked */
    user = findUser(sanitizedUsername);
    if (user != NULL && user->lockoutUntil > time(NULL)) {
        return "Account temporarily locked due to multiple failed attempts";
    }
    
    /* Verify credentials */
    if (verifyPassword(sanitizedUsername, password)) {
        redirectToSessionPage(sanitizedUsername);
        return "SUCCESS";
    } else {
        return "Invalid username or password";
    }
}

int main(void) {
    const char* result;
    
    /* Initialize OpenSSL */
    OpenSSL_add_all_algorithms();
    
    /* Initialize with test users */
    if (!addUser("admin", "SecurePass123!")) {
        fprintf(stderr, "Failed to add admin user\\n");
        return 1;
    }
    
    if (!addUser("user1", "MyPassword456@")) {
        fprintf(stderr, "Failed to add user1\\n");
        return 1;
    }
    
    if (!addUser("testuser", "Test789#Pass")) {
        fprintf(stderr, "Failed to add testuser\\n");
        return 1;
    }
    
    printf("=== Secure Login System Test Cases ===\\n\\n");
    
    /* Test case 1: Valid login */
    printf("Test 1: Valid credentials (admin/SecurePass123!)\\n");
    result = login("admin", "SecurePass123!");
    printf("Result: %s\\n\\n", result);
    
    /* Test case 2: Invalid password */
    printf("Test 2: Invalid password\\n");
    result = login("admin", "WrongPassword");
    printf("Result: %s\\n\\n", result);
    
    /* Test case 3: Non-existent user */
    printf("Test 3: Non-existent user\\n");
    result = login("nonexistent", "password");
    printf("Result: %s\\n\\n", result);
    
    /* Test case 4: Empty credentials */
    printf("Test 4: Empty username\\n");
    result = login("", "password");
    printf("Result: %s\\n\\n", result);
    
    /* Test case 5: Valid login for another user */
    printf("Test 5: Valid credentials (user1/MyPassword456@)\\n");
    result = login("user1", "MyPassword456@");
    printf("Result: %s\\n\\n", result);
    
    return 0;
}
