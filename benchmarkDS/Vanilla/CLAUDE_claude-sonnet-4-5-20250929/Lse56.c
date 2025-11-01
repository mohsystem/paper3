
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_USERS 100
#define MAX_STRING 100

typedef struct {
    char username[MAX_STRING];
    char email[MAX_STRING];
    char password[MAX_STRING];
} User;

User users[MAX_USERS];
int userCount = 0;
char currentLoggedInUser[MAX_STRING] = "";

bool login(const char* username, const char* password) {
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, username) == 0) {
            if (strcmp(users[i].password, password) == 0) {
                strcpy(currentLoggedInUser, username);
                return true;
            }
        }
    }
    return false;
}

void logout() {
    strcpy(currentLoggedInUser, "");
}

bool registerUser(const char* username, const char* email, const char* password) {
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, username) == 0) {
            return false;
        }
    }
    
    strcpy(users[userCount].username, username);
    strcpy(users[userCount].email, email);
    strcpy(users[userCount].password, password);
    userCount++;
    return true;
}

const char* changeEmail(const char* oldEmail, const char* newEmail, const char* confirmPassword) {
    // Check if user is logged in
    if (strlen(currentLoggedInUser) == 0) {
        return "Error: User must be logged in to change email";
    }
    
    User* user = NULL;
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, currentLoggedInUser) == 0) {
            user = &users[i];
            break;
        }
    }
    
    if (user == NULL) {
        return "Error: User not found";
    }
    
    // Check if old email matches
    if (strcmp(user->email, oldEmail) != 0) {
        return "Error: Old email does not match";
    }
    
    // Check if password is correct
    if (strcmp(user->password, confirmPassword) != 0) {
        return "Error: Incorrect password";
    }
    
    // Check if new email is different from old email
    if (strcmp(oldEmail, newEmail) == 0) {
        return "Error: New email must be different from old email";
    }
    
    // Change email
    strcpy(user->email, newEmail);
    return "Success: Email changed successfully";
}

int main() {
    // Test Case 1: Change email successfully
    printf("Test Case 1:\\n");
    registerUser("user1", "old@example.com", "password123");
    login("user1", "password123");
    printf("%s\\n", changeEmail("old@example.com", "new@example.com", "password123"));
    logout();
    printf("\\n");
    
    // Test Case 2: Try to change email without logging in
    printf("Test Case 2:\\n");
    printf("%s\\n", changeEmail("old@example.com", "new2@example.com", "password123"));
    printf("\\n");
    
    // Test Case 3: Wrong old email
    printf("Test Case 3:\\n");
    registerUser("user2", "user2@example.com", "pass456");
    login("user2", "pass456");
    printf("%s\\n", changeEmail("wrong@example.com", "new@example.com", "pass456"));
    logout();
    printf("\\n");
    
    // Test Case 4: Wrong password
    printf("Test Case 4:\\n");
    registerUser("user3", "user3@example.com", "mypass");
    login("user3", "mypass");
    printf("%s\\n", changeEmail("user3@example.com", "newuser3@example.com", "wrongpass"));
    logout();
    printf("\\n");
    
    // Test Case 5: New email same as old email
    printf("Test Case 5:\\n");
    registerUser("user4", "user4@example.com", "secure123");
    login("user4", "secure123");
    printf("%s\\n", changeEmail("user4@example.com", "user4@example.com", "secure123"));
    logout();
    
    return 0;
}
