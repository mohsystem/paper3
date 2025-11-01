#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_USERS 10
#define MAX_STRING_LENGTH 50

// User struct to hold user data.
typedef struct {
    char username[MAX_STRING_LENGTH];
    char email[MAX_STRING_LENGTH];
    char password[MAX_STRING_LENGTH]; // In a real app, this should be a salted password hash.
} User;

// In-memory database and state
static User userDatabase[MAX_USERS];
static int userCount = 0;
static char loggedInUser[MAX_STRING_LENGTH] = ""; // Empty string means no one is logged in

/**
 * Adds a new user to the database. Helper function for setup.
 */
void addUser(const char* username, const char* email, const char* password) {
    if (userCount < MAX_USERS) {
        strncpy(userDatabase[userCount].username, username, MAX_STRING_LENGTH - 1);
        strncpy(userDatabase[userCount].email, email, MAX_STRING_LENGTH - 1);
        strncpy(userDatabase[userCount].password, password, MAX_STRING_LENGTH - 1);
        userDatabase[userCount].username[MAX_STRING_LENGTH - 1] = '\0';
        userDatabase[userCount].email[MAX_STRING_LENGTH - 1] = '\0';
        userDatabase[userCount].password[MAX_STRING_LENGTH - 1] = '\0';
        userCount++;
    }
}

/**
 * Simulates a user logging in.
 */
bool login(const char* username, const char* password) {
    for (int i = 0; i < userCount; i++) {
        if (strcmp(userDatabase[i].username, username) == 0 && strcmp(userDatabase[i].password, password) == 0) {
            strncpy(loggedInUser, username, MAX_STRING_LENGTH - 1);
            loggedInUser[MAX_STRING_LENGTH - 1] = '\0';
            printf("Login successful for: %s\n", username);
            return true;
        }
    }
    printf("Login failed for: %s\n", username);
    return false;
}

/**
 * Simulates a user logging out.
 */
void logout() {
    if (strlen(loggedInUser) > 0) {
        printf("User %s logged out.\n", loggedInUser);
        loggedInUser[0] = '\0'; // Set to empty string
    }
}

/**
 * Changes the email for the currently logged-in user.
 * Returns a constant string literal indicating the result.
 */
const char* changeEmail(const char* oldEmail, const char* newEmail, const char* confirmPassword) {
    // 1. Authorization: Check if a user is logged in.
    if (strlen(loggedInUser) == 0) {
        return "Error: You must be logged in to change your email.";
    }

    // Find the current user's data in the database
    User* currentUser = NULL;
    for (int i = 0; i < userCount; i++) {
        if (strcmp(userDatabase[i].username, loggedInUser) == 0) {
            currentUser = &userDatabase[i];
            break;
        }
    }
    
    if (currentUser == NULL) {
         return "Error: Logged-in user not found in database.";
    }

    // 2. Validation: Check if the provided old email matches the user's record.
    if (strcmp(currentUser->email, oldEmail) != 0) {
        return "Error: The old email address provided is incorrect.";
    }

    // 3. Authentication: Verify the user's password.
    if (strcmp(currentUser->password, confirmPassword) != 0) {
        return "Error: Incorrect password.";
    }
    
    // 4. Input Validation: Check if new email is valid
    if (newEmail == NULL || strlen(newEmail) == 0 || strchr(newEmail, '@') == NULL) {
        return "Error: New email is not valid.";
    }

    // 5. Uniqueness Check: Ensure the new email is not already in use by another user.
    for (int i = 0; i < userCount; i++) {
        if (strcmp(userDatabase[i].email, newEmail) == 0 && strcmp(userDatabase[i].username, loggedInUser) != 0) {
            return "Error: The new email address is already in use by another account.";
        }
    }

    // 6. Update the email
    strncpy(currentUser->email, newEmail, MAX_STRING_LENGTH - 1);
    currentUser->email[MAX_STRING_LENGTH - 1] = '\0';
    return "Success: Email has been changed."; // Can't easily concatenate strings in return
}

int main() {
    // Setup initial user data
    addUser("john.doe", "john.d@example.com", "password123");
    addUser("jane.doe", "jane.d@example.com", "password456");

    printf("--- Test Case 1: Successful Email Change ---\n");
    login("john.doe", "password123");
    const char* result1 = changeEmail("john.d@example.com", "john.new@example.com", "password123");
    printf("Result: %s\n", result1);
    printf("Current user info: email = %s\n", userDatabase[0].email);
    logout();
    printf("\n--------------------------------------------\n\n");

    printf("--- Test Case 2: Attempt to Change Email While Logged Out ---\n");
    const char* result2 = changeEmail("jane.d@example.com", "jane.new@example.com", "password456");
    printf("Result: %s\n", result2);
    printf("\n--------------------------------------------\n\n");

    printf("--- Test Case 3: Incorrect Password ---\n");
    login("jane.doe", "password456");
    const char* result3 = changeEmail("jane.d@example.com", "jane.new@example.com", "wrongpassword");
    printf("Result: %s\n", result3);
    logout();
    printf("\n--------------------------------------------\n\n");
    
    printf("--- Test Case 4: Incorrect Old Email ---\n");
    login("john.doe", "password123");
    const char* result4 = changeEmail("wrong.email@example.com", "john.another@example.com", "password123");
    printf("Result: %s\n", result4);
    logout();
    printf("\n--------------------------------------------\n\n");

    printf("--- Test Case 5: New Email Already in Use ---\n");
    login("john.doe", "password123");
    const char* result5 = changeEmail("john.new@example.com", "jane.d@example.com", "password123");
    printf("Result: %s\n", result5);
    logout();
    printf("\n--------------------------------------------\n\n");

    return 0;
}