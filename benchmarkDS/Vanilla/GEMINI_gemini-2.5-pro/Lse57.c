#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_USERS 10
#define MAX_EMAIL_LEN 50
#define MAX_PASSWORD_LEN 30

typedef struct {
    char email[MAX_EMAIL_LEN];
    char password[MAX_PASSWORD_LEN];
    bool isLoggedIn;
} User;

User userDatabase[MAX_USERS];
int userCount = 0;

User* findUser(const char* email) {
    for (int i = 0; i < userCount; ++i) {
        if (strcmp(userDatabase[i].email, email) == 0) {
            return &userDatabase[i];
        }
    }
    return NULL;
}

void addUser(const char* email, const char* password) {
    if (userCount < MAX_USERS) {
        User* existing = findUser(email);
        if (existing == NULL) {
            strcpy(userDatabase[userCount].email, email);
            strcpy(userDatabase[userCount].password, password);
            userDatabase[userCount].isLoggedIn = false;
            userCount++;
        }
    }
}

bool login(const char* email, const char* password) {
    User* user = findUser(email);
    if (user != NULL && strcmp(user->password, password) == 0) {
        user->isLoggedIn = true;
        return true;
    }
    return false;
}

void logout(const char* email) {
    User* user = findUser(email);
    if (user != NULL) {
        user->isLoggedIn = false;
    }
}

const char* changeEmail(const char* oldEmail, const char* newEmail, const char* confirmPassword) {
    User* user = findUser(oldEmail);

    if (user == NULL) {
        return "Error: Old email does not exist.";
    }

    if (!user->isLoggedIn) {
        return "Error: User must be logged in to change email.";
    }

    if (strcmp(user->password, confirmPassword) != 0) {
        return "Error: Incorrect password.";
    }

    if (findUser(newEmail) != NULL) {
        return "Error: New email is already taken.";
    }

    // In C, since the email is a field, we just change it.
    strcpy(user->email, newEmail);
    // Returning a static string. In a real app, memory should be managed.
    return "Success: Email changed.";
}


int main() {
    // Setup: Add some users to our database
    addUser("user1@example.com", "password123");
    addUser("user2@example.com", "securepass456");

    printf("--- Test Case 1: Successful Email Change ---\n");
    login("user1@example.com", "password123");
    User* user1 = findUser("user1@example.com");
    printf("Login status for user1: %s\n", user1 ? (user1->isLoggedIn ? "true" : "false") : "Not Found");
    const char* result1 = changeEmail("user1@example.com", "new.user1@example.com", "password123");
    printf("%s\n", result1);
    printf("Database contains new.user1@example.com: %s\n", findUser("new.user1@example.com") ? "true" : "false");
    printf("Database contains user1@example.com: %s\n", findUser("user1@example.com") ? "true" : "false");
    printf("\n");

    printf("--- Test Case 2: Failed Change (Incorrect Password) ---\n");
    login("user2@example.com", "securepass456");
    const char* result2 = changeEmail("user2@example.com", "another.user@example.com", "wrongpassword");
    printf("%s\n", result2);
    printf("\n");

    printf("--- Test Case 3: Failed Change (User Not Logged In) ---\n");
    logout("user2@example.com"); // Make sure user is logged out
    const char* result3 = changeEmail("user2@example.com", "new.user2@example.com", "securepass456");
    printf("%s\n", result3);
    printf("\n");

    printf("--- Test Case 4: Failed Change (Old Email Not Found) ---\n");
    const char* result4 = changeEmail("nonexistent@example.com", "anything@example.com", "anypassword");
    printf("%s\n", result4);
    printf("\n");

    printf("--- Test Case 5: Logout and then Fail Change ---\n");
    printf("User 'user2@example.com' attempts another change after logging out.\n");
    logout("user2@example.com");
    User* user2 = findUser("user2@example.com");
    printf("Login status for user2: %s\n", user2->isLoggedIn ? "true" : "false");
    const char* result5 = changeEmail("user2@example.com", "final.user2@example.com", "securepass456");
    printf("%s\n", result5);
    printf("\n");

    return 0;
}