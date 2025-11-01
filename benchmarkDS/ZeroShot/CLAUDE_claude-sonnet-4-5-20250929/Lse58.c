
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <regex.h>

#define MAX_STRING_LEN 256

typedef struct {
    char username[MAX_STRING_LEN];
    char email[MAX_STRING_LEN];
    char passwordHash[65];
    int loggedIn;
} User;

void hashPassword(const char* password, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\\0';
}

int isValidEmail(const char* email) {
    regex_t regex;
    int result;
    
    result = regcomp(&regex, "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$", 
                     REG_EXTENDED);
    if (result) return 0;
    
    result = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);
    
    return !result;
}

User* createUser(const char* username, const char* email, const char* password) {
    User* user = (User*)malloc(sizeof(User));
    strncpy(user->username, username, MAX_STRING_LEN - 1);
    strncpy(user->email, email, MAX_STRING_LEN - 1);
    hashPassword(password, user->passwordHash);
    user->loggedIn = 0;
    return user;
}

int login(User* user, const char* password) {
    char hash[65];
    hashPassword(password, hash);
    
    if (strcmp(hash, user->passwordHash) == 0) {
        user->loggedIn = 1;
        return 1;
    }
    return 0;
}

void logoutUser(User* user) {
    user->loggedIn = 0;
}

int changeEmail(User* user, const char* oldEmail, const char* password, 
                const char* newEmail) {
    char hash[65];
    
    if (!user->loggedIn) {
        return 0;
    }
    if (strcmp(user->email, oldEmail) != 0) {
        return 0;
    }
    
    hashPassword(password, hash);
    if (strcmp(hash, user->passwordHash) != 0) {
        return 0;
    }
    
    if (!newEmail || strlen(newEmail) == 0 || !isValidEmail(newEmail)) {
        return 0;
    }
    
    strncpy(user->email, newEmail, MAX_STRING_LEN - 1);
    return 1;
}

void freeUser(User* user) {
    free(user);
}

int main() {
    // Test Case 1: Successful email change
    User* user1 = createUser("john_doe", "john@example.com", "password123");
    login(user1, "password123");
    int result1 = changeEmail(user1, "john@example.com", "password123", 
                             "newemail@example.com");
    printf("Test 1 - Successful change: %d, New email: %s\\n", result1, user1->email);
    freeUser(user1);

    // Test Case 2: Wrong old email
    User* user2 = createUser("jane_doe", "jane@example.com", "securepass");
    login(user2, "securepass");
    int result2 = changeEmail(user2, "wrong@example.com", "securepass", 
                             "newemail@example.com");
    printf("Test 2 - Wrong old email: %d, Email unchanged: %s\\n", result2, user2->email);
    freeUser(user2);

    // Test Case 3: Wrong password
    User* user3 = createUser("bob_smith", "bob@example.com", "mypassword");
    login(user3, "mypassword");
    int result3 = changeEmail(user3, "bob@example.com", "wrongpassword", 
                             "newemail@example.com");
    printf("Test 3 - Wrong password: %d, Email unchanged: %s\\n", result3, user3->email);
    freeUser(user3);

    // Test Case 4: Not logged in
    User* user4 = createUser("alice_wonder", "alice@example.com", "alicepass");
    int result4 = changeEmail(user4, "alice@example.com", "alicepass", 
                             "newemail@example.com");
    printf("Test 4 - Not logged in: %d, Email unchanged: %s\\n", result4, user4->email);
    freeUser(user4);

    // Test Case 5: Invalid new email format
    User* user5 = createUser("charlie_brown", "charlie@example.com", "charpass");
    login(user5, "charpass");
    int result5 = changeEmail(user5, "charlie@example.com", "charpass", "invalidemail");
    printf("Test 5 - Invalid email format: %d, Email unchanged: %s\\n", result5, user5->email);
    freeUser(user5);

    return 0;
}
