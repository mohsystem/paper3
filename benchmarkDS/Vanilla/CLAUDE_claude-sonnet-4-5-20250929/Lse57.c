
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_USERS 100
#define MAX_EMAIL_LEN 100
#define MAX_PASS_LEN 100

typedef struct {
    char email[MAX_EMAIL_LEN];
    char passwordHash[MAX_PASS_LEN];
    bool active;
} User;

typedef struct {
    User users[MAX_USERS];
    int userCount;
    char loggedInUser[MAX_EMAIL_LEN];
} Lse57;

void hashPassword(const char* password, char* output) {
    // Simple hash simulation (not secure, just for demonstration)
    strcpy(output, password);
}

void initSystem(Lse57* system) {
    system->userCount = 0;
    system->loggedInUser[0] = '\\0';
    for (int i = 0; i < MAX_USERS; i++) {
        system->users[i].active = false;
    }
}

bool registerUser(Lse57* system, const char* email, const char* password) {
    for (int i = 0; i < system->userCount; i++) {
        if (system->users[i].active && strcmp(system->users[i].email, email) == 0) {
            return false;
        }
    }
    
    if (system->userCount >= MAX_USERS) {
        return false;
    }
    
    strcpy(system->users[system->userCount].email, email);
    hashPassword(password, system->users[system->userCount].passwordHash);
    system->users[system->userCount].active = true;
    system->userCount++;
    return true;
}

bool login(Lse57* system, const char* email, const char* password) {
    char hashedPass[MAX_PASS_LEN];
    hashPassword(password, hashedPass);
    
    for (int i = 0; i < system->userCount; i++) {
        if (system->users[i].active && 
            strcmp(system->users[i].email, email) == 0 &&
            strcmp(system->users[i].passwordHash, hashedPass) == 0) {
            strcpy(system->loggedInUser, email);
            return true;
        }
    }
    return false;
}

bool changeEmail(Lse57* system, const char* oldEmail, const char* newEmail, 
                 const char* confirmPassword) {
    if (strlen(system->loggedInUser) == 0) {
        return false;
    }
    
    if (strcmp(system->loggedInUser, oldEmail) != 0) {
        return false;
    }
    
    int userIndex = -1;
    for (int i = 0; i < system->userCount; i++) {
        if (system->users[i].active && strcmp(system->users[i].email, oldEmail) == 0) {
            userIndex = i;
            break;
        }
    }
    
    if (userIndex == -1) {
        return false;
    }
    
    char hashedPass[MAX_PASS_LEN];
    hashPassword(confirmPassword, hashedPass);
    if (strcmp(system->users[userIndex].passwordHash, hashedPass) != 0) {
        return false;
    }
    
    for (int i = 0; i < system->userCount; i++) {
        if (system->users[i].active && strcmp(system->users[i].email, newEmail) == 0) {
            return false;
        }
    }
    
    strcpy(system->users[userIndex].email, newEmail);
    strcpy(system->loggedInUser, newEmail);
    
    return true;
}

void logoutUser(Lse57* system) {
    system->loggedInUser[0] = '\\0';
}

int main() {
    printf("Test Case 1: Successful email change\\n");
    Lse57 system1;
    initSystem(&system1);
    registerUser(&system1, "user1@example.com", "password123");
    login(&system1, "user1@example.com", "password123");
    bool result1 = changeEmail(&system1, "user1@example.com", "newuser1@example.com", "password123");
    printf("Result: %s (Expected: true)\\n", result1 ? "true" : "false");
    logoutUser(&system1);
    
    printf("\\nTest Case 2: Not logged in\\n");
    Lse57 system2;
    initSystem(&system2);
    registerUser(&system2, "user2@example.com", "password456");
    bool result2 = changeEmail(&system2, "user2@example.com", "newuser2@example.com", "password456");
    printf("Result: %s (Expected: false)\\n", result2 ? "true" : "false");
    
    printf("\\nTest Case 3: Wrong confirm password\\n");
    Lse57 system3;
    initSystem(&system3);
    registerUser(&system3, "user3@example.com", "password789");
    login(&system3, "user3@example.com", "password789");
    bool result3 = changeEmail(&system3, "user3@example.com", "newuser3@example.com", "wrongpassword");
    printf("Result: %s (Expected: false)\\n", result3 ? "true" : "false");
    logoutUser(&system3);
    
    printf("\\nTest Case 4: Old email doesn't match logged in user\\n");
    Lse57 system4;
    initSystem(&system4);
    registerUser(&system4, "user4@example.com", "password111");
    registerUser(&system4, "user5@example.com", "password222");
    login(&system4, "user4@example.com", "password111");
    bool result4 = changeEmail(&system4, "user5@example.com", "newuser5@example.com", "password222");
    printf("Result: %s (Expected: false)\\n", result4 ? "true" : "false");
    logoutUser(&system4);
    
    printf("\\nTest Case 5: New email already exists\\n");
    Lse57 system5;
    initSystem(&system5);
    registerUser(&system5, "user6@example.com", "password333");
    registerUser(&system5, "user7@example.com", "password444");
    login(&system5, "user6@example.com", "password333");
    bool result5 = changeEmail(&system5, "user6@example.com", "user7@example.com", "password333");
    printf("Result: %s (Expected: false)\\n", result5 ? "true" : "false");
    logoutUser(&system5);
    
    return 0;
}
