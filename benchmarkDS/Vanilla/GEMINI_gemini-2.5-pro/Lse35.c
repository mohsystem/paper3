#include <stdio.h>
#include <string.h>

// Struct to hold user credentials
typedef struct {
    const char* username;
    const char* password;
} User;

// This array of structs simulates a database table of users
User userDatabase[] = {
    {"admin", "admin123"},
    {"john.doe", "password123"},
    {"jane.doe", "securepass"}
};

// Calculate the number of users in the database
const int dbSize = sizeof(userDatabase) / sizeof(userDatabase[0]);

/**
 * Simulates a login function that checks credentials against a database.
 * @param username The username provided by the user.
 * @param password The password provided by the user.
 * @return A string literal indicating the result of the login attempt.
 */
const char* login(const char* username, const char* password) {
    for (int i = 0; i < dbSize; ++i) {
        // Check if both username and password match
        if (strcmp(userDatabase[i].username, username) == 0 && 
            strcmp(userDatabase[i].password, password) == 0) {
            return "Login successful. Redirecting to the home page.";
        }
    }
    // If no match is found after checking all users
    return "Invalid username or password. Redirecting to the login page.";
}

void runTests() {
    // 5 Test Cases
    printf("Test Case 1: Valid credentials for 'admin'\n");
    printf("Result: %s\n", login("admin", "admin123"));
    printf("--------------------\n");

    printf("Test Case 2: Valid credentials for 'john.doe'\n");
    printf("Result: %s\n", login("john.doe", "password123"));
    printf("--------------------\n");

    printf("Test Case 3: Invalid password for 'admin'\n");
    printf("Result: %s\n", login("admin", "wrongpassword"));
    printf("--------------------\n");

    printf("Test Case 4: Non-existent username\n");
    printf("Result: %s\n", login("unknown_user", "somepassword"));
    printf("--------------------\n");

    printf("Test Case 5: Empty credentials\n");
    printf("Result: %s\n", login("", ""));
    printf("--------------------\n");
}

int main() {
    // In C, a main class name is not applicable. 
    // The main function serves as the entry point.
    // The class name Lse35 is emulated through function naming if needed,
    // but here we just run the tests directly.
    runTests();
    return 0;
}