
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/sha.h>

#define MAX_USERS 100
#define MAX_EMAIL_LEN 100
#define MAX_PASS_LEN 65

typedef struct {
    char email[MAX_EMAIL_LEN];
    char passwordHash[MAX_PASS_LEN];
    int active;
} UserAccount;

typedef struct {
    UserAccount users[MAX_USERS];
    int userCount;
    char currentLoggedInUser[MAX_EMAIL_LEN];
} Lse56;

void hashPassword(const char* password, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\\0';
}

void initLse56(Lse56* system) {
    system->userCount = 0;
    strcpy(system->currentLoggedInUser, "");
    for(int i = 0; i < MAX_USERS; i++) {
        system->users[i].active = 0;
    }
}

int registerUser(Lse56* system, const char* email, const char* password) {
    for(int i = 0; i < system->userCount; i++) {
        if(system->users[i].active && strcmp(system->users[i].email, email) == 0) {
            return 0;
        }
    }
    
    if(system->userCount < MAX_USERS) {
        strcpy(system->users[system->userCount].email, email);
        hashPassword(password, system->users[system->userCount].passwordHash);
        system->users[system->userCount].active = 1;
        system->userCount++;
        return 1;
    }
    return 0;
}

int login(Lse56* system, const char* email, const char* password) {
    char passHash[MAX_PASS_LEN];
    hashPassword(password, passHash);
    
    for(int i = 0; i < system->userCount; i++) {
        if(system->users[i].active && 
           strcmp(system->users[i].email, email) == 0 &&
           strcmp(system->users[i].passwordHash, passHash) == 0) {
            strcpy(system->currentLoggedInUser, email);
            return 1;
        }
    }
    return 0;
}

void logout(Lse56* system) {
    strcpy(system->currentLoggedInUser, "");
}

int changeEmail(Lse56* system, const char* oldEmail, const char* newEmail, 
                const char* confirmPassword) {
    if(strlen(system->currentLoggedInUser) == 0) {
        printf("Error: User must be logged in to change email\\n");
        return 0;
    }
    
    int userIndex = -1;
    for(int i = 0; i < system->userCount; i++) {
        if(system->users[i].active && 
           strcmp(system->users[i].email, system->currentLoggedInUser) == 0) {
            userIndex = i;
            break;
        }
    }
    
    if(userIndex == -1 || strcmp(system->users[userIndex].email, oldEmail) != 0) {
        printf("Error: Old email does not match current email\\n");
        return 0;
    }
    
    if(strcmp(oldEmail, newEmail) == 0) {
        printf("Error: New email must be different from old email\\n");
        return 0;
    }
    
    char passHash[MAX_PASS_LEN];
    hashPassword(confirmPassword, passHash);
    if(strcmp(system->users[userIndex].passwordHash, passHash) != 0) {
        printf("Error: Incorrect password\\n");
        return 0;
    }
    
    for(int i = 0; i < system->userCount; i++) {
        if(system->users[i].active && strcmp(system->users[i].email, newEmail) == 0) {
            printf("Error: Email already exists\\n");
            return 0;
        }
    }
    
    strcpy(system->users[userIndex].email, newEmail);
    strcpy(system->currentLoggedInUser, newEmail);
    
    printf("Email changed successfully from %s to %s\\n", oldEmail, newEmail);
    return 1;
}

int main() {
    Lse56 system;
    initLse56(&system);
    
    // Test Case 1
    registerUser(&system, "user1@example.com", "password123");
    login(&system, "user1@example.com", "password123");
    printf("Test 1: %d\\n", changeEmail(&system, "user1@example.com", "newuser1@example.com", "password123"));
    logout(&system);
    
    // Test Case 2
    printf("\\nTest 2: %d\\n", changeEmail(&system, "newuser1@example.com", "another@example.com", "password123"));
    
    // Test Case 3
    registerUser(&system, "user2@example.com", "pass456");
    login(&system, "user2@example.com", "pass456");
    printf("\\nTest 3: %d\\n", changeEmail(&system, "user2@example.com", "newuser2@example.com", "wrongpassword"));
    logout(&system);
    
    // Test Case 4
    login(&system, "user2@example.com", "pass456");
    printf("\\nTest 4: %d\\n", changeEmail(&system, "wrongemail@example.com", "newuser2@example.com", "pass456"));
    logout(&system);
    
    // Test Case 5
    login(&system, "user2@example.com", "pass456");
    printf("\\nTest 5: %d\\n", changeEmail(&system, "user2@example.com", "user2@example.com", "pass456"));
    logout(&system);
    
    return 0;
}
