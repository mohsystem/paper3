
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

#define MAX_USERNAME_LEN 51
#define MAX_PASSWORD_LEN 128
#define MAX_HASH_LEN 65
#define MAX_SALT_LEN 65
#define MAX_TOKEN_LEN 65
#define MAX_USERS 100
#define MAX_SESSIONS 100
#define SESSION_TIMEOUT_SECONDS 1800

typedef struct {
    char username[MAX_USERNAME_LEN];
    char passwordHash[MAX_HASH_LEN];
    char salt[MAX_SALT_LEN];
    int isAdmin;
    int active;
} User;

typedef struct {
    char sessionToken[MAX_TOKEN_LEN];
    char username[MAX_USERNAME_LEN];
    int isAdmin;
    time_t createdTime;
    int active;
} Session;

static User userDatabase[MAX_USERS];
static Session activeSessions[MAX_SESSIONS];
static int userCount = 0;
static int sessionCount = 0;

void bytesToHex(const unsigned char* bytes, size_t length, char* output, size_t outputSize) {
    if (output == NULL || outputSize < (length * 2 + 1)) {
        return;
    }
    
    for (size_t i = 0; i < length && i * 2 < outputSize - 1; i++) {
        snprintf(output + (i * 2), 3, "%02x", bytes[i]);
    }
    output[length * 2] = '\\0';
}

int generateSalt(char* salt, size_t saltSize) {
    if (salt == NULL || saltSize < MAX_SALT_LEN) {
        return 0;
    }
    
    unsigned char randomBytes[32];
    if (RAND_bytes(randomBytes, sizeof(randomBytes)) != 1) {
        return 0;
    }
    
    bytesToHex(randomBytes, sizeof(randomBytes), salt, saltSize);
    return 1;
}

int hashPassword(const char* password, const char* salt, char* output, size_t outputSize) {
    if (password == NULL || salt == NULL || output == NULL || outputSize < MAX_HASH_LEN) {
        return 0;
    }
    
    size_t passwordLen = strnlen(password, MAX_PASSWORD_LEN);
    size_t saltLen = strnlen(salt, MAX_SALT_LEN);
    size_t combinedLen = passwordLen + saltLen;
    
    if (combinedLen >= MAX_PASSWORD_LEN + MAX_SALT_LEN) {
        return 0;
    }
    
    char* combined = (char*)malloc(combinedLen + 1);
    if (combined == NULL) {
        return 0;
    }
    
    snprintf(combined, combinedLen + 1, "%s%s", password, salt);
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)combined, combinedLen, hash);
    
    volatile char* ptr = combined;
    for (size_t i = 0; i < combinedLen; i++) {
        ptr[i] = 0;
    }
    free(combined);
    
    bytesToHex(hash, SHA256_DIGEST_LENGTH, output, outputSize);
    return 1;
}

int timingSafeCompare(const char* a, const char* b) {
    if (a == NULL || b == NULL) {
        return 0;
    }
    
    size_t lenA = strnlen(a, MAX_HASH_LEN);
    size_t lenB = strnlen(b, MAX_HASH_LEN);
    
    if (lenA != lenB) {
        return 0;
    }
    
    volatile unsigned char result = 0;
    for (size_t i = 0; i < lenA; i++) {
        result |= (unsigned char)a[i] ^ (unsigned char)b[i];
    }
    
    return result == 0;
}

int createUser(const char* username, const char* password, int isAdmin) {
    if (username == NULL || password == NULL || userCount >= MAX_USERS) {
        return 0;
    }
    
    if (strnlen(username, MAX_USERNAME_LEN) == 0 || strnlen(password, MAX_PASSWORD_LEN) == 0) {
        return 0;
    }
    
    char salt[MAX_SALT_LEN];
    if (!generateSalt(salt, sizeof(salt))) {
        return 0;
    }
    
    char passwordHash[MAX_HASH_LEN];
    if (!hashPassword(password, salt, passwordHash, sizeof(passwordHash))) {
        return 0;
    }
    
    strncpy(userDatabase[userCount].username, username, MAX_USERNAME_LEN - 1);
    userDatabase[userCount].username[MAX_USERNAME_LEN - 1] = '\\0';
    
    strncpy(userDatabase[userCount].passwordHash, passwordHash, MAX_HASH_LEN - 1);
    userDatabase[userCount].passwordHash[MAX_HASH_LEN - 1] = '\\0';
    
    strncpy(userDatabase[userCount].salt, salt, MAX_SALT_LEN - 1);
    userDatabase[userCount].salt[MAX_SALT_LEN - 1] = '\\0';
    
    userDatabase[userCount].isAdmin = isAdmin;
    userDatabase[userCount].active = 1;
    userCount++;
    
    return 1;
}

int authenticateUser(const char* username, const char* password) {
    if (username == NULL || password == NULL) {
        return 0;
    }
    
    User* user = NULL;
    for (int i = 0; i < userCount; i++) {
        if (userDatabase[i].active && strcmp(userDatabase[i].username, username) == 0) {
            user = &userDatabase[i];
            break;
        }
    }
    
    if (user == NULL) {
        char dummySalt[MAX_SALT_LEN];
        char dummyHash[MAX_HASH_LEN];
        generateSalt(dummySalt, sizeof(dummySalt));
        hashPassword(password, dummySalt, dummyHash, sizeof(dummyHash));
        return 0;
    }
    
    char providedHash[MAX_HASH_LEN];
    if (!hashPassword(password, user->salt, providedHash, sizeof(providedHash))) {
        return 0;
    }
    
    return timingSafeCompare(providedHash, user->passwordHash);
}

int generateSessionToken(char* token, size_t tokenSize) {
    if (token == NULL || tokenSize < MAX_TOKEN_LEN) {
        return 0;
    }
    
    unsigned char randomBytes[32];
    if (RAND_bytes(randomBytes, sizeof(randomBytes)) != 1) {
        return 0;
    }
    
    bytesToHex(randomBytes, sizeof(randomBytes), token, tokenSize);
    return 1;
}

int createSession(const char* username, int isAdmin, char* sessionToken, size_t tokenSize) {
    if (username == NULL || sessionToken == NULL || sessionCount >= MAX_SESSIONS) {
        return 0;
    }
    
    if (!generateSessionToken(sessionToken, tokenSize)) {
        return 0;
    }
    
    strncpy(activeSessions[sessionCount].sessionToken, sessionToken, MAX_TOKEN_LEN - 1);
    activeSessions[sessionCount].sessionToken[MAX_TOKEN_LEN - 1] = '\\0';
    
    strncpy(activeSessions[sessionCount].username, username, MAX_USERNAME_LEN - 1);
    activeSessions[sessionCount].username[MAX_USERNAME_LEN - 1] = '\\0';
    
    activeSessions[sessionCount].isAdmin = isAdmin;
    activeSessions[sessionCount].createdTime = time(NULL);
    activeSessions[sessionCount].active = 1;
    sessionCount++;
    
    return 1;
}

const char* redirectUser(const char* sessionToken) {
    if (sessionToken == NULL || strnlen(sessionToken, MAX_TOKEN_LEN) == 0) {
        return "/login";
    }
    
    size_t tokenLen = strnlen(sessionToken, MAX_TOKEN_LEN);
    for (size_t i = 0; i < tokenLen; i++) {
        if (!isxdigit((unsigned char)sessionToken[i])) {
            return "/login";
        }
    }
    
    time_t currentTime = time(NULL);
    for (int i = 0; i < sessionCount; i++) {
        if (activeSessions[i].active && 
            (currentTime - activeSessions[i].createdTime) > SESSION_TIMEOUT_SECONDS) {
            activeSessions[i].active = 0;
        }
    }
    
    Session* session = NULL;
    for (int i = 0; i < sessionCount; i++) {
        if (activeSessions[i].active && 
            strcmp(activeSessions[i].sessionToken, sessionToken) == 0) {
            session = &activeSessions[i];
            break;
        }
    }
    
    if (session == NULL || (currentTime - session->createdTime) > SESSION_TIMEOUT_SECONDS) {
        return "/login";
    }
    
    if (session->isAdmin) {
        return "/admin";
    }
    
    return "/user";
}

int login(const char* username, const char* password, char* sessionToken, size_t tokenSize) {
    if (username == NULL || password == NULL || sessionToken == NULL) {
        return 0;
    }
    
    size_t usernameLen = strnlen(username, MAX_USERNAME_LEN);
    if (usernameLen == 0 || usernameLen > 50) {
        return 0;
    }
    
    for (size_t i = 0; i < usernameLen; i++) {
        if (!isalnum((unsigned char)username[i]) && username[i] != '_') {
            return 0;
        }
    }
    
    if (!authenticateUser(username, password)) {
        return 0;
    }
    
    User* user = NULL;
    for (int i = 0; i < userCount; i++) {
        if (userDatabase[i].active && strcmp(userDatabase[i].username, username) == 0) {
            user = &userDatabase[i];
            break;
        }
    }
    
    if (user == NULL) {
        return 0;
    }
    
    return createSession(username, user->isAdmin, sessionToken, tokenSize);
}

int main(void) {
    OPENSSL_init_crypto(OPENSSL_INIT_LOAD_CRYPTO_STRINGS, NULL);
    
    createUser("regularuser", "SecurePass123!", 0);
    createUser("adminuser", "AdminPass456!", 1);
    
    printf("Test 1 - No session: %s\\n", redirectUser(NULL));
    
    printf("Test 2 - Invalid token: %s\\n", redirectUser("invalid_token_123"));
    
    char regularToken[MAX_TOKEN_LEN] = {0};
    if (login("regularuser", "SecurePass123!", regularToken, sizeof(regularToken))) {
        printf("Test 3 - Regular user: %s\\n", redirectUser(regularToken));
    }
    
    char adminToken[MAX_TOKEN_LEN] = {0};
    if (login("adminuser", "AdminPass456!", adminToken, sizeof(adminToken))) {
        printf("Test 4 - Admin user: %s\\n", redirectUser(adminToken));
    }
    
    char expiredToken[MAX_TOKEN_LEN] = {0};
    if (login("regularuser", "SecurePass123!", expiredToken, sizeof(expiredToken))) {
        for (int i = 0; i < sessionCount; i++) {
            if (strcmp(activeSessions[i].sessionToken, expiredToken) == 0) {
                activeSessions[i].createdTime = time(NULL) - (SESSION_TIMEOUT_SECONDS + 1);
                break;
            }
        }
        printf("Test 5 - Expired session: %s\\n", redirectUser(expiredToken));
    }
    
    return 0;
}
