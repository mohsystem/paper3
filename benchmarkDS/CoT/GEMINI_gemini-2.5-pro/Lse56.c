#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// In a real application, never store passwords in plain text.
// Always use a strong, salted hashing algorithm like Argon2, scrypt, or bcrypt.
typedef struct {
    char* email;
    char* password; // Plain text for demonstration only.
    int isLoggedIn; // 0 for false, 1 for true
} User;

// Helper function to create a user and allocate memory
User* createUser(const char* email, const char* password) {
    User* user = (User*)malloc(sizeof(User));
    if (user == NULL) return NULL;

    user->email = (char*)malloc(strlen(email) + 1);
    strcpy(user->email, email);

    user->password = (char*)malloc(strlen(password) + 1);
    strcpy(user->password, password);
    
    user->isLoggedIn = 0;
    return user;
}

// Helper function to free user memory
void destroyUser(User* user) {
    if (user) {
        free(user->email);
        free(user->password);
        free(user);
    }
}

void loginUser(User* user, const char* password) {
    if (strcmp(user->password, password) == 0) {
        user->isLoggedIn = 1;
        printf("User %s logged in successfully.\n", user->email);
    } else {
        printf("Login failed for user %s\n", user->email);
    }
}

void logoutUser(User* user) {
    user->isLoggedIn = 0;
    printf("User %s logged out.\n", user->email);
}

/**
 * Attempts to change a user's email address after performing security checks.
 *
 * @param user A pointer to the user struct attempting the change.
 * @param oldEmail The user's claimed old email address.
 * @param newEmail The desired new email address.
 * @param password The user's password for confirmation.
 * @return A string literal indicating success or the specific reason for failure.
 */
const char* changeEmail(User* user, const char* oldEmail, const char* newEmail, const char* password) {
    // 1. Check if the user is logged in
    if (!user->isLoggedIn) {
        return "Error: User is not logged in.";
    }

    // 2. Check if the provided old email matches the current one
    if (strcmp(user->email, oldEmail) != 0) {
        return "Error: Old email does not match the current email.";
    }

    // 3. Check if the provided password is correct
    if (strcmp(user->password, password) != 0) {
        return "Error: Incorrect password.";
    }

    // 4. Check if the new email is different from the old email
    if (strcmp(user->email, newEmail) == 0) {
        return "Error: New email cannot be the same as the old email.";
    }
    
    // In a real application, you would also validate the new email format.

    // All checks passed, update the email
    char* newEmailBuffer = (char*)malloc(strlen(newEmail) + 1);
    if (newEmailBuffer == NULL) {
        return "Error: Memory allocation failed.";
    }
    strcpy(newEmailBuffer, newEmail);

    // Free the old email memory and assign the new one
    free(user->email);
    user->email = newEmailBuffer;

    return "Success: Email has been changed.";
}

// In C, a class structure is not native. We use functions and a main entry point.
// Lse56 is represented by the main function and related functions.
void main_Lse56() {
    printf("--- C Test Cases ---\n");

    // Test Case 1: Successful Email Change
    printf("\n--- Test Case 1: Successful Change ---\n");
    User* user1 = createUser("test1@example.com", "password123");
    loginUser(user1, "password123");
    const char* result1 = changeEmail(user1, "test1@example.com", "new1@example.com", "password123");
    printf("Result: %s\n", result1);
    printf("User's current email: %s\n", user1->email);
    logoutUser(user1);
    destroyUser(user1);

    // Test Case 2: User Not Logged In
    printf("\n--- Test Case 2: User Not Logged In ---\n");
    User* user2 = createUser("test2@example.com", "password456");
    // user2 does not log in
    const char* result2 = changeEmail(user2, "test2@example.com", "new2@example.com", "password456");
    printf("Result: %s\n", result2);
    printf("User's current email: %s\n", user2->email);
    destroyUser(user2);

    // Test Case 3: Incorrect Old Email
    printf("\n--- Test Case 3: Incorrect Old Email ---\n");
    User* user3 = createUser("test3@example.com", "password789");
    loginUser(user3, "password789");
    const char* result3 = changeEmail(user3, "wrong-email@example.com", "new3@example.com", "password789");
    printf("Result: %s\n", result3);
    printf("User's current email: %s\n", user3->email);
    logoutUser(user3);
    destroyUser(user3);

    // Test Case 4: Incorrect Password
    printf("\n--- Test Case 4: Incorrect Password ---\n");
    User* user4 = createUser("test4@example.com", "securepass");
    loginUser(user4, "securepass");
    const char* result4 = changeEmail(user4, "test4@example.com", "new4@example.com", "wrongpass");
    printf("Result: %s\n", result4);
    printf("User's current email: %s\n", user4->email);
    logoutUser(user4);
    destroyUser(user4);

    // Test Case 5: New Email is Same as Old Email
    printf("\n--- Test Case 5: New Email is Same as Old Email ---\n");
    User* user5 = createUser("test5@example.com", "pass12345");
    loginUser(user5, "pass12345");
    const char* result5 = changeEmail(user5, "test5@example.com", "test5@example.com", "pass12345");
    printf("Result: %s\n", result5);
    printf("User's current email: %s\n", user5->email);
    logoutUser(user5);
    destroyUser(user5);
}

int main() {
    main_Lse56();
    return 0;
}