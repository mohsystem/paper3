#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/**
 * @brief Checks if the provided username and password are correct.
 * @param username The username to check.
 * @param password The password to check.
 * @return true if credentials are correct, false otherwise.
 */
bool login(const char* username, const char* password) {
    // Hardcoded correct credentials
    const char* correctUsername = "admin";
    const char* correctPassword = "password123";

    // strcmp returns 0 if strings are equal
    if (strcmp(username, correctUsername) == 0 && strcmp(password, correctPassword) == 0) {
        return true;
    }
    return false;
}

int main() {
    // A structure to hold test case data
    typedef struct {
        const char* username;
        const char* password;
    } TestCase;

    // 5 test cases
    TestCase testCases[] = {
        {"admin", "password123"},    // 1. Correct credentials
        {"admin", "wrongpassword"},  // 2. Correct username, wrong password
        {"user", "password123"},     // 3. Wrong username, correct password
        {"user", "wrongpassword"},   // 4. Both wrong
        {"", ""}                     // 5. Empty credentials
    };

    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; i++) {
        const char* username = testCases[i].username;
        const char* password = testCases[i].password;
        
        printf("Testing with Username: '%s', Password: '%s'\n", username, password);
        if (login(username, password)) {
            printf("Login successful. Redirecting to session page.\n");
        } else {
            printf("Incorrect username or password.\n");
        }
        printf("--------------------\n");
    }

    return 0;
}