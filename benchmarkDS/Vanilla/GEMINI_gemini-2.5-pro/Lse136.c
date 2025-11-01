#include <stdio.h>
#include <string.h>

/**
 * @brief Checks if the provided username and password are correct.
 * For this example, the correct credentials are "admin" and "password123".
 *
 * @param username The username to check.
 * @param password The password to check.
 * @return The string "true" if credentials are correct, "false" otherwise.
 */
const char* check_mod(const char* username, const char* password) {
    const char* correctUsername = "admin";
    const char* correctPassword = "password123";

    if (username != NULL && password != NULL &&
        strcmp(username, correctUsername) == 0 &&
        strcmp(password, correctPassword) == 0) {
        return "true";
    } else {
        return "false";
    }
}

int main() {
    // Test Case 1: Correct credentials
    printf("Test 1 (Correct): %s\n", check_mod("admin", "password123"));

    // Test Case 2: Incorrect password
    printf("Test 2 (Wrong Pass): %s\n", check_mod("admin", "wrongpass"));

    // Test Case 3: Incorrect username
    printf("Test 3 (Wrong User): %s\n", check_mod("user", "password123"));

    // Test Case 4: Both incorrect
    printf("Test 4 (Both Wrong): %s\n", check_mod("user", "wrongpass"));

    // Test Case 5: Null inputs
    printf("Test 5 (Null Input): %s\n", check_mod(NULL, NULL));

    return 0;
}