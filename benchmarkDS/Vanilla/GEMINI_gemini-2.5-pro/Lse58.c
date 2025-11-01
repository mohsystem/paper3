#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Use a struct to represent the user
typedef struct {
    char* username;
    char* password;
    char* email;
    int isLoggedIn; // 0 for false, 1 for true
} User;

// Helper to create a user and allocate memory
User* createUser(const char* username, const char* password, const char* email) {
    User* user = (User*)malloc(sizeof(User));
    if (user == NULL) return NULL;

    user->username = (char*)malloc(strlen(username) + 1);
    strcpy(user->username, username);

    user->password = (char*)malloc(strlen(password) + 1);
    strcpy(user->password, password);
    
    user->email = (char*)malloc(strlen(email) + 1);
    strcpy(user->email, email);
    
    user->isLoggedIn = 0;
    return user;
}

// Helper to free all allocated memory for a user
void freeUser(User* user) {
    if (user != NULL) {
        free(user->username);
        free(user->password);
        free(user->email);
        free(user);
    }
}

void login(User* user, const char* username, const char* password) {
    if (strcmp(user->username, username) == 0 && strcmp(user->password, password) == 0) {
        user->isLoggedIn = 1;
        printf("Login successful.\n");
    } else {
        printf("Login failed: Invalid username or password.\n");
    }
}

void logout(User* user) {
    user->isLoggedIn = 0;
    printf("Logout successful.\n");
}

int changeEmail(User* user, const char* oldEmail, const char* password, const char* newEmail) {
    if (!user->isLoggedIn) {
        printf("Email change failed: User not logged in.\n");
        return 0;
    }
    if (strcmp(user->email, oldEmail) != 0) {
        printf("Email change failed: Old email does not match.\n");
        return 0;
    }
    if (strcmp(user->password, password) != 0) {
        printf("Email change failed: Incorrect password.\n");
        return 0;
    }

    // Free the old email string and allocate memory for the new one
    free(user->email);
    user->email = (char*)malloc(strlen(newEmail) + 1);
    if (user->email == NULL) {
        printf("Memory allocation failed.\n");
        return 0;
    }
    strcpy(user->email, newEmail);

    printf("Email changed successfully to: %s\n", user->email);
    return 1;
}

int main() {
    User* user = createUser("testuser", "password123", "user@example.com");

    // Test Case 1: Successful email change
    printf("--- Test Case 1: Successful Change ---\n");
    printf("Initial email: %s\n", user->email);
    login(user, "testuser", "password123");
    changeEmail(user, "user@example.com", "password123", "newuser@example.com");
    printf("Final email: %s\n", user->email);
    logout(user);
    printf("\n");

    // Test Case 2: Failed change (not logged in)
    printf("--- Test Case 2: Failed Change (Not Logged In) ---\n");
    printf("Initial email: %s\n", user->email);
    changeEmail(user, "newuser@example.com", "password123", "another@example.com");
    printf("Final email: %s\n", user->email);
    printf("\n");

    // Test Case 3: Failed change (wrong password)
    printf("--- Test Case 3: Failed Change (Wrong Password) ---\n");
    login(user, "testuser", "password123");
    printf("Initial email: %s\n", user->email);
    changeEmail(user, "newuser@example.com", "wrongpassword", "another@example.com");
    printf("Final email: %s\n", user->email);
    logout(user);
    printf("\n");

    // Test Case 4: Failed change (wrong old email)
    printf("--- Test Case 4: Failed Change (Wrong Old Email) ---\n");
    login(user, "testuser", "password123");
    printf("Initial email: %s\n", user->email);
    changeEmail(user, "wrong@example.com", "password123", "another@example.com");
    printf("Final email: %s\n", user->email);
    logout(user);
    printf("\n");

    // Test Case 5: Successful change after a failed attempt
    printf("--- Test Case 5: Success After Failure ---\n");
    login(user, "testuser", "password123");
    printf("Initial email: %s\n", user->email);
    printf("Attempting with wrong password...\n");
    changeEmail(user, "newuser@example.com", "wrongpass", "final@example.com");
    printf("Email after failed attempt: %s\n", user->email);
    printf("Attempting with correct details...\n");
    changeEmail(user, "newuser@example.com", "password123", "final@example.com");
    printf("Final email: %s\n", user->email);
    logout(user);

    freeUser(user);
    return 0;
}