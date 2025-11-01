
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_USERS 100
#define MAX_LENGTH 100

typedef struct {
    char username[MAX_LENGTH];
    char email[MAX_LENGTH];
    char password[MAX_LENGTH];
} User;

User users[MAX_USERS];
int userCount = 0;
int loggedInUserIndex = -1;

bool registerUser(const char* username, const char* email, const char* password) {
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, username) == 0) {
            return false;
        }
    }
    if (userCount < MAX_USERS) {
        strcpy(users[userCount].username, username);
        strcpy(users[userCount].email, email);
        strcpy(users[userCount].password, password);
        userCount++;
        return true;
    }
    return false;
}

bool login(const char* username, const char* password) {
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, username) == 0 && 
            strcmp(users[i].password, password) == 0) {
            loggedInUserIndex = i;
            return true;
        }
    }
    return false;
}

void logout() {
    loggedInUserIndex = -1;
}

bool changeEmail(const char* oldEmail, const char* newEmail, const char* password) {
    if (loggedInUserIndex == -1) {
        return false;
    }
    
    if (strcmp(users[loggedInUserIndex].email, oldEmail) == 0 && 
        strcmp(users[loggedInUserIndex].password, password) == 0) {
        strcpy(users[loggedInUserIndex].email, newEmail);
        return true;
    }
    return false;
}

const char* getCurrentEmail() {
    if (loggedInUserIndex != -1) {
        return users[loggedInUserIndex].email;
    }
    return "";
}

int main() {
    // Test Case 1: Register and login, then change email successfully
    printf("Test Case 1:\\n");
    registerUser("user1", "user1@example.com", "password123");
    login("user1", "password123");
    bool result1 = changeEmail("user1@example.com", "newemail1@example.com", "password123");
    printf("Change email result: %d, New email: %s\\n", result1, getCurrentEmail());
    logout();
    
    // Test Case 2: Try to change email without logging in
    printf("\\nTest Case 2:\\n");
    bool result2 = changeEmail("user1@example.com", "newemail2@example.com", "password123");
    printf("Change email without login: %d\\n", result2);
    
    // Test Case 3: Login but provide wrong old email
    printf("\\nTest Case 3:\\n");
    login("user1", "password123");
    bool result3 = changeEmail("wrong@example.com", "newemail3@example.com", "password123");
    printf("Change email with wrong old email: %d, Current email: %s\\n", result3, getCurrentEmail());
    logout();
    
    // Test Case 4: Login but provide wrong password
    printf("\\nTest Case 4:\\n");
    login("user1", "password123");
    bool result4 = changeEmail("newemail1@example.com", "newemail4@example.com", "wrongpassword");
    printf("Change email with wrong password: %d, Current email: %s\\n", result4, getCurrentEmail());
    logout();
    
    // Test Case 5: Successful email change with correct credentials
    printf("\\nTest Case 5:\\n");
    registerUser("user2", "user2@example.com", "pass456");
    login("user2", "pass456");
    bool result5 = changeEmail("user2@example.com", "user2new@example.com", "pass456");
    printf("Change email result: %d, New email: %s\\n", result5, getCurrentEmail());
    logout();
    
    return 0;
}
