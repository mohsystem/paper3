
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>

#define MAX_USERS 100
#define SALT_LENGTH 16
#define HASH_LENGTH 65

typedef struct {
    char username[50];
    char salt[SALT_LENGTH * 2 + 1];
    char hashedPassword[HASH_LENGTH];
} UserCredentials;

static UserCredentials userDatabase[MAX_USERS];
static int userCount = 0;

void generateSalt(char* salt) {
    srand(time(NULL) + userCount);
    for (int i = 0; i < SALT_LENGTH * 2; i++) {
        sprintf(&salt[i * 2], "%02x", rand() % 256);
    }
    salt[SALT_LENGTH * 2] = '\\0';
}

void hashPassword(const char* password, const char* salt, char* output) {
    char combined[256];
    snprintf(combined, sizeof(combined), "%s%s", salt, password);
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)combined, strlen(combined), hash);
    
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&output[i * 2], "%02x", hash[i]);
    }
    output[SHA256_DIGEST_LENGTH * 2] = '\\0';
}

int signup(const char* username, const char* password) {
    if (username == NULL || strlen(username) == 0 || 
        password == NULL || strlen(password) < 8 || userCount >= MAX_USERS) {
        return 0;
    }
    
    for (int i = 0; i < userCount; i++) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            return 0;
        }
    }
    
    strcpy(userDatabase[userCount].username, username);
    generateSalt(userDatabase[userCount].salt);
    hashPassword(password, userDatabase[userCount].salt, userDatabase[userCount].hashedPassword);
    userCount++;
    return 1;
}

int verifyLogin(const char* username, const char* password) {
    for (int i = 0; i < userCount; i++) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            char hashedAttempt[HASH_LENGTH];
            hashPassword(password, userDatabase[i].salt, hashedAttempt);
            return strcmp(hashedAttempt, userDatabase[i].hashedPassword) == 0;
        }
    }
    return 0;
}

int main() {
    printf("Test Case 1: Valid signup\\n");
    int result1 = signup("user1", "SecurePass123");
    printf("Signup successful: %d\\n", result1);
    printf("Login verification: %d\\n\\n", verifyLogin("user1", "SecurePass123"));
    
    printf("Test Case 2: Duplicate username\\n");
    int result2 = signup("user1", "AnotherPass456");
    printf("Signup successful: %d\\n\\n", result2);
    
    printf("Test Case 3: Weak password (too short)\\n");
    int result3 = signup("user2", "short");
    printf("Signup successful: %d\\n\\n", result3);
    
    printf("Test Case 4: Multiple valid users\\n");
    int result4a = signup("alice", "AlicePass2024");
    int result4b = signup("bob", "BobSecure789");
    printf("Alice signup: %d, Bob signup: %d\\n", result4a, result4b);
    printf("Alice login: %d\\n", verifyLogin("alice", "AlicePass2024"));
    printf("Bob login: %d\\n\\n", verifyLogin("bob", "BobSecure789"));
    
    printf("Test Case 5: Invalid login attempt\\n");
    signup("user3", "ValidPassword123");
    int validLogin = verifyLogin("user3", "ValidPassword123");
    int invalidLogin = verifyLogin("user3", "WrongPassword");
    printf("Valid login: %d\\n", validLogin);
    printf("Invalid login: %d\\n", invalidLogin);
    
    return 0;
}
