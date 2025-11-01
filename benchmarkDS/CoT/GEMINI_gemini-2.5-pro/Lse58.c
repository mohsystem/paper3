#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LEN 50

typedef struct {
    char username[MAX_LEN];
    char password[MAX_LEN]; // In a real app, this should be a hashed password
    char email[MAX_LEN];
    bool isLoggedIn;
} User;

void init_user(User* user, const char* username, const char* password, const char* email) {
    strncpy(user->username, username, MAX_LEN - 1);
    user->username[MAX_LEN - 1] = '\0';
    strncpy(user->password, password, MAX_LEN - 1);
    user->password[MAX_LEN - 1] = '\0';
    strncpy(user->email, email, MAX_LEN - 1);
    user->email[MAX_LEN - 1] = '\0';
    user->isLoggedIn = false;
}

bool login_user(User* user, const char* username, const char* password) {
    if (strcmp(user->username, username) == 0 && strcmp(user->password, password) == 0) {
        user->isLoggedIn = true;
        printf("Login successful for user: %s\n", user->username);
        return true;
    }
    printf("Login failed for user: %s\n", username);
    return false;
}

void logout_user(User* user) {
    user->isLoggedIn = false;
    printf("%s has been logged out.\n", user->username);
}

bool change_email(User* user, const char* oldEmail, const char* confirmPassword, const char* newEmail) {
    if (!user->isLoggedIn) {
        printf("Error: You must be logged in to change your email.\n");
        return false;
    }

    if (strcmp(user->email, oldEmail) == 0 && strcmp(user->password, confirmPassword) == 0) {
        strncpy(user->email, newEmail, MAX_LEN - 1);
        user->email[MAX_LEN - 1] = '\0';
        printf("Success: Email has been changed to %s\n", newEmail);
        return true;
    } else {
        printf("Error: Incorrect old email or password. Email not changed.\n");
        return false;
    }
}

int main() {
    User user;
    init_user(&user, "testuser", "password123", "user@example.com");

    printf("--- Test Case 1: Successful Email Change ---\n");
    login_user(&user, "testuser", "password123");
    change_email(&user, "user@example.com", "password123", "newuser@example.com");
    printf("Current email: %s\n", user.email);
    logout_user(&user);
    printf("\n");

    // Reset user for next tests
    init_user(&user, "testuser", "password123", "user@example.com");

    printf("--- Test Case 2: Failed Change (Not Logged In) ---\n");
    change_email(&user, "user@example.com", "password123", "another@example.com");
    printf("Current email: %s\n", user.email);
    printf("\n");

    printf("--- Test Case 3: Failed Change (Wrong Password) ---\n");
    login_user(&user, "testuser", "password123");
    change_email(&user, "user@example.com", "wrongpassword", "another@example.com");
    printf("Current email: %s\n", user.email);
    logout_user(&user);
    printf("\n");
    
    // Reset user
    init_user(&user, "testuser", "password123", "user@example.com");

    printf("--- Test Case 4: Failed Change (Wrong Old Email) ---\n");
    login_user(&user, "testuser", "password123");
    change_email(&user, "wrong@example.com", "password123", "another@example.com");
    printf("Current email: %s\n", user.email);
    printf("\n");

    printf("--- Test Case 5: Failed Change (Logged out then attempt) ---\n");
    logout_user(&user); // Ensuring user is logged out
    change_email(&user, "user@example.com", "password123", "final@example.com");
    printf("Current email: %s\n", user.email);
    
    return 0;
}