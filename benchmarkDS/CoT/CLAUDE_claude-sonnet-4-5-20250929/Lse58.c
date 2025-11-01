
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <regex.h>
#include <stdbool.h>

#define MAX_EMAIL_LEN 256
#define MAX_HASH_LEN 65
#define MAX_SALT_LEN 33

typedef struct {
    char email[MAX_EMAIL_LEN];
    char passwordHash[MAX_HASH_LEN];
    char salt[MAX_SALT_LEN];
    bool isLoggedIn;
} User;

void generateSalt(char* salt) {
    unsigned char saltBytes[16];
    RAND_bytes(saltBytes, 16);
    
    for(int i = 0; i < 16; i++) {
        sprintf(salt + (i * 2), "%02x", saltBytes[i]);
    }
    salt[32] = '\\0';
}

void hashPassword(const char* password, const char* salt, char* output) {
    char combined[512];
    sprintf(combined, "%s%s", salt, password);
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)combined, strlen(combined), hash);
    
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\\0';
}

bool isValidEmail(const char* email) {
    regex_t regex;
    int ret = regcomp(&regex, "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$", REG_EXTENDED);
    if (ret != 0) return false;
    
    ret = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);
    return ret == 0;
}

User* createUser(const char* email, const char* password) {
    User* user = (User*)malloc(sizeof(User));
    if (!user) return NULL;
    
    strncpy(user->email, email, MAX_EMAIL_LEN - 1);
    user->email[MAX_EMAIL_LEN - 1] = '\\0';
    
    generateSalt(user->salt);
    hashPassword(password, user->salt, user->passwordHash);
    user->isLoggedIn = false;
    
    return user;
}

bool login(User* user, const char* email, const char* password) {
    if (!user) return false;
    
    char hash[MAX_HASH_LEN];
    hashPassword(password, user->salt, hash);
    
    if (strcmp(user->email, email) == 0 && strcmp(user->passwordHash, hash) == 0) {
        user->isLoggedIn = true;
        return true;
    }
    return false;
}

void logout(User* user) {
    if (user) {
        user->isLoggedIn = false;
    }
}

bool changeEmail(User* user, const char* oldEmail, const char* password, const char* newEmail) {
    if (!user) return false;
    
    if (!user->isLoggedIn) {
        printf("Error: User must be logged in to change email\\n");
        return false;
    }
    
    if (!newEmail || strlen(newEmail) == 0 || !isValidEmail(newEmail)) {
        printf("Error: Invalid new email format\\n");
        return false;
    }
    
    if (strcmp(user->email, oldEmail) != 0) {
        printf("Error: Old email does not match\\n");
        return false;
    }
    
    char hash[MAX_HASH_LEN];
    hashPassword(password, user->salt, hash);
    
    if (strcmp(user->passwordHash, hash) != 0) {
        printf("Error: Incorrect password\\n");
        return false;
    }
    
    strncpy(user->email, newEmail, MAX_EMAIL_LEN - 1);
    user->email[MAX_EMAIL_LEN - 1] = '\\0';
    printf("Email successfully changed to: %s\\n", newEmail);
    return true;
}

void freeUser(User* user) {
    if (user) {
        free(user);
    }
}

int main() {
    printf("=== Test Case 1: Successful email change ===\\n");
    User* user1 = createUser("user1@example.com", "password123");
    login(user1, "user1@example.com", "password123");
    changeEmail(user1, "user1@example.com", "password123", "newemail1@example.com");
    printf("Current email: %s\\n\\n", user1->email);
    freeUser(user1);
    
    printf("=== Test Case 2: Wrong old email ===\\n");
    User* user2 = createUser("user2@example.com", "password456");
    login(user2, "user2@example.com", "password456");
    changeEmail(user2, "wrong@example.com", "password456", "newemail2@example.com");
    printf("Current email: %s\\n\\n", user2->email);
    freeUser(user2);
    
    printf("=== Test Case 3: Wrong password ===\\n");
    User* user3 = createUser("user3@example.com", "password789");
    login(user3, "user3@example.com", "password789");
    changeEmail(user3, "user3@example.com", "wrongpassword", "newemail3@example.com");
    printf("Current email: %s\\n\\n", user3->email);
    freeUser(user3);
    
    printf("=== Test Case 4: Not logged in ===\\n");
    User* user4 = createUser("user4@example.com", "password000");
    changeEmail(user4, "user4@example.com", "password000", "newemail4@example.com");
    printf("Current email: %s\\n\\n", user4->email);
    freeUser(user4);
    
    printf("=== Test Case 5: Invalid new email format ===\\n");
    User* user5 = createUser("user5@example.com", "password111");
    login(user5, "user5@example.com", "password111");
    changeEmail(user5, "user5@example.com", "password111", "invalidemail");
    printf("Current email: %s\\n\\n", user5->email);
    freeUser(user5);
    
    return 0;
}
