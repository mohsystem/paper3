
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>

#define MAX_USERS 100
#define MAX_USERNAME 50
#define MAX_PASSWORD 100
#define SALT_LENGTH 16

typedef struct {
    char username[MAX_USERNAME];
    unsigned char salt[SALT_LENGTH];
    unsigned char hashedPassword[SHA256_DIGEST_LENGTH];
    int active;
} UserCredentials;

UserCredentials userDatabase[MAX_USERS];
int userCount = 0;

void generateSalt(unsigned char* salt) {
    srand(time(NULL) + rand());
    for (int i = 0; i < SALT_LENGTH; i++) {
        salt[i] = rand() % 256;
    }
}

void hashPassword(const char* password, const unsigned char* salt, unsigned char* output) {
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, salt, SALT_LENGTH);
    SHA256_Update(&ctx, password, strlen(password));
    SHA256_Final(output, &ctx);
}

int registerUser(const char* username, const char* password) {
    if (username == NULL || password == NULL || strlen(username) == 0 || strlen(password) < 8) {
        return 0;
    }
    
    for (int i = 0; i < userCount; i++) {
        if (userDatabase[i].active && strcmp(userDatabase[i].username, username) == 0) {
            return 0;
        }
    }
    
    if (userCount >= MAX_USERS) {
        return 0;
    }
    
    strncpy(userDatabase[userCount].username, username, MAX_USERNAME - 1);
    userDatabase[userCount].username[MAX_USERNAME - 1] = '\\0';
    
    generateSalt(userDatabase[userCount].salt);
    hashPassword(password, userDatabase[userCount].salt, userDatabase[userCount].hashedPassword);
    userDatabase[userCount].active = 1;
    userCount++;
    
    return 1;
}

int authenticateUser(const char* username, const char* password) {
    if (username == NULL || password == NULL) {
        return 0;
    }
    
    for (int i = 0; i < userCount; i++) {
        if (userDatabase[i].active && strcmp(userDatabase[i].username, username) == 0) {
            unsigned char hashedPassword[SHA256_DIGEST_LENGTH];
            hashPassword(password, userDatabase[i].salt, hashedPassword);
            
            if (memcmp(hashedPassword, userDatabase[i].hashedPassword, SHA256_DIGEST_LENGTH) == 0) {
                return 1;
            }
            return 0;
        }
    }
    return 0;
}

int main() {
    // Test Case 1: Successful registration and authentication
    printf("Test 1 - Register and authenticate user:\\n");
    int registered = registerUser("alice", "SecurePass123");
    printf("Registration: %s\\n", registered ? "true" : "false");
    int authenticated = authenticateUser("alice", "SecurePass123");
    printf("Authentication: %s\\n", authenticated ? "true" : "false");
    printf("\\n");
    
    // Test Case 2: Failed authentication with wrong password
    printf("Test 2 - Wrong password:\\n");
    int wrongPass = authenticateUser("alice", "WrongPassword");
    printf("Authentication: %s\\n", wrongPass ? "true" : "false");
    printf("\\n");
    
    // Test Case 3: Registration with weak password
    printf("Test 3 - Weak password:\\n");
    int weakPass = registerUser("bob", "weak");
    printf("Registration: %s\\n", weakPass ? "true" : "false");
    printf("\\n");
    
    // Test Case 4: Duplicate username registration
    printf("Test 4 - Duplicate username:\\n");
    int duplicate = registerUser("alice", "AnotherPass456");
    printf("Registration: %s\\n", duplicate ? "true" : "false");
    printf("\\n");
    
    // Test Case 5: Non-existent user authentication
    printf("Test 5 - Non-existent user:\\n");
    int nonExistent = authenticateUser("charlie", "SomePassword789");
    printf("Authentication: %s\\n", nonExistent ? "true" : "false");
    
    return 0;
}
