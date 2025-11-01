
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <regex.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

#define MAX_EMAIL_LEN 256
#define MAX_PASSWORD_LEN 256
#define MAX_HASH_LEN 65
#define MAX_SALT_LEN 33
#define MAX_TOKEN_LEN 128
#define MAX_USERS 100

typedef struct {
    char email[MAX_EMAIL_LEN];
    char passwordHash[MAX_HASH_LEN];
    char salt[MAX_SALT_LEN];
} User;

typedef struct {
    char token[MAX_TOKEN_LEN];
    char email[MAX_EMAIL_LEN];
} Session;

static User userDatabase[MAX_USERS];
static int userCount = 0;
static Session activeSessions[MAX_USERS];
static int sessionCount = 0;

void hashPassword(const char* password, const char* salt, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    char combined[MAX_PASSWORD_LEN + MAX_SALT_LEN];
    snprintf(combined, sizeof(combined), "%s%s", password, salt);
    
    SHA256((unsigned char*)combined, strlen(combined), hash);
    
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\\0';
}

void generateSalt(char* salt) {
    unsigned char bytes[16];
    RAND_bytes(bytes, 16);
    
    for(int i = 0; i < 16; i++) {
        sprintf(salt + (i * 2), "%02x", bytes[i]);
    }
    salt[32] = '\\0';
}

int isValidEmail(const char* email) {
    regex_t regex;
    int ret = regcomp(&regex, "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$", REG_EXTENDED);
    if (ret) return 0;
    
    ret = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);
    return ret == 0;
}

char* registerUser(const char* email, const char* password) {
    if (!isValidEmail(email)) {
        return "Invalid email format";
    }
    if (strlen(password) < 8) {
        return "Password must be at least 8 characters";
    }
    
    for(int i = 0; i < userCount; i++) {
        if (strcmp(userDatabase[i].email, email) == 0) {
            return "Email already exists";
        }
    }
    
    if (userCount >= MAX_USERS) {
        return "User limit reached";
    }
    
    strcpy(userDatabase[userCount].email, email);
    generateSalt(userDatabase[userCount].salt);
    hashPassword(password, userDatabase[userCount].salt, userDatabase[userCount].passwordHash);
    userCount++;
    
    return "Registration successful";
}

char* login(const char* email, const char* password, char* tokenOutput) {
    int userIndex = -1;
    for(int i = 0; i < userCount; i++) {
        if (strcmp(userDatabase[i].email, email) == 0) {
            userIndex = i;
            break;
        }
    }
    
    if (userIndex == -1) {
        tokenOutput[0] = '\\0';
        return NULL;
    }
    
    char passwordHash[MAX_HASH_LEN];
    hashPassword(password, userDatabase[userIndex].salt, passwordHash);
    
    if (strcmp(passwordHash, userDatabase[userIndex].passwordHash) == 0) {
        char temp[MAX_EMAIL_LEN + 20];
        snprintf(temp, sizeof(temp), "%s%ld", email, time(NULL));
        char salt[MAX_SALT_LEN];
        generateSalt(salt);
        hashPassword(temp, salt, tokenOutput);
        
        if (sessionCount < MAX_USERS) {
            strcpy(activeSessions[sessionCount].token, tokenOutput);
            strcpy(activeSessions[sessionCount].email, email);
            sessionCount++;
        }
        return tokenOutput;
    }
    
    tokenOutput[0] = '\\0';
    return NULL;
}

char* changeEmail(const char* sessionToken, const char* oldEmail,
                 const char* newEmail, const char* confirmPassword) {
    int sessionIndex = -1;
    for(int i = 0; i < sessionCount; i++) {
        if (strcmp(activeSessions[i].token, sessionToken) == 0) {
            sessionIndex = i;
            break;
        }
    }
    
    if (sessionIndex == -1) {
        return "User not logged in";
    }
    
    if (strcmp(activeSessions[sessionIndex].email, oldEmail) != 0) {
        return "Old email does not match logged in user";
    }
    
    int userIndex = -1;
    for(int i = 0; i < userCount; i++) {
        if (strcmp(userDatabase[i].email, oldEmail) == 0) {
            userIndex = i;
            break;
        }
    }
    
    if (userIndex == -1) {
        return "Old email does not exist";
    }
    
    if (!isValidEmail(newEmail)) {
        return "Invalid new email format";
    }
    
    for(int i = 0; i < userCount; i++) {
        if (strcmp(userDatabase[i].email, newEmail) == 0) {
            return "New email already exists";
        }
    }
    
    char passwordHash[MAX_HASH_LEN];
    hashPassword(confirmPassword, userDatabase[userIndex].salt, passwordHash);
    
    if (strcmp(passwordHash, userDatabase[userIndex].passwordHash) != 0) {
        return "Incorrect password";
    }
    
    strcpy(userDatabase[userIndex].email, newEmail);
    strcpy(activeSessions[sessionIndex].email, newEmail);
    
    return "Email changed successfully";
}

int main() {
    char token[MAX_TOKEN_LEN];
    
    printf("=== Test Case 1: Successful email change ===\\n");
    registerUser("user1@example.com", "password123");
    login("user1@example.com", "password123", token);
    printf("%s\\n", changeEmail(token, "user1@example.com", 
                              "newuser1@example.com", "password123"));
    
    printf("\\n=== Test Case 2: Invalid session token ===\\n");
    printf("%s\\n", changeEmail("invalid_token", "user2@example.com", 
                              "newuser2@example.com", "password123"));
    
    printf("\\n=== Test Case 3: Incorrect password ===\\n");
    registerUser("user3@example.com", "password123");
    login("user3@example.com", "password123", token);
    printf("%s\\n", changeEmail(token, "user3@example.com", 
                              "newuser3@example.com", "wrongpassword"));
    
    printf("\\n=== Test Case 4: New email already exists ===\\n");
    registerUser("user4@example.com", "password123");
    registerUser("existing@example.com", "password123");
    login("user4@example.com", "password123", token);
    printf("%s\\n", changeEmail(token, "user4@example.com", 
                              "existing@example.com", "password123"));
    
    printf("\\n=== Test Case 5: Invalid new email format ===\\n");
    registerUser("user5@example.com", "password123");
    login("user5@example.com", "password123", token);
    printf("%s\\n", changeEmail(token, "user5@example.com", 
                              "invalid-email", "password123"));
    
    return 0;
}
