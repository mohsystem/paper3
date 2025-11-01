#include <stdio.h>
#include <string.h>

// Define a structure for user credentials
typedef struct {
    char username[50];
    char password[50];
} User;

// Simulates a user database using an array of structs
User userDatabase[] = {
    {"admin", "password123"},
    {"johndoe", "secret"},
    {"testuser", "test"}
};
const int DB_SIZE = sizeof(userDatabase) / sizeof(userDatabase[0]);

/**
 * Checks user credentials against the database.
 * @param username The username to check.
 * @param password The password to check.
 * @return A string indicating the result of the login attempt.
 */
const char* login(const char* username, const char* password) {
    for (int i = 0; i < DB_SIZE; i++) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            if (strcmp(userDatabase[i].password, password) == 0) {
                return "Login Successful. Redirecting to the dashboard.";
            }
            // Username found, but password incorrect. Stop checking.
            return "Invalid username or password. Redirecting to the login page.";
        }
    }
    // Username not found after checking all entries
    return "Invalid username or password. Redirecting to the login page.";
}

int main() {
    printf("--- C Login System Test Cases ---\n");

    // Test Case 1: Correct credentials
    printf("Test 1 (admin, password123): %s\n", login("admin", "password123"));

    // Test Case 2: Correct username, incorrect password
    printf("Test 2 (admin, wrongpass): %s\n", login("admin", "wrongpass"));

    // Test Case 3: Incorrect username
    printf("Test 3 (user, password123): %s\n", login("user", "password123"));

    // Test Case 4: Another set of correct credentials
    printf("Test 4 (johndoe, secret): %s\n", login("johndoe", "secret"));

    // Test Case 5: Non-existent user
    printf("Test 5 (guest, guest): %s\n", login("guest", "guest"));

    return 0;
}