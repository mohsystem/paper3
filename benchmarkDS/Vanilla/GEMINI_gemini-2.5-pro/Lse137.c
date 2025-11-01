#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/**
 * @brief Checks if a user is a moderator based on hardcoded credentials.
 * In a real application, this would involve database lookups, password hashing,
 * and proper session validation.
 * @param username The user's name.
 * @param password The user's password.
 * @param session The user's session token (unused in this simple example).
 * @return true if the user is a moderator, false otherwise.
 */
bool auth_mod(const char* username, const char* password, const char* session) {
    // Simple hardcoded check for demonstration purposes.
    // The session parameter is included to match the requirements but is not used in the logic.
    if (strcmp(username, "admin") == 0 && strcmp(password, "admin_pass") == 0) {
        return true;
    }
    return false;
}

/**
 * @brief Simulates a URL endpoint that checks for moderator privileges.
 * It receives user credentials and calls the authentication logic.
 * @param username The user's name from the URL.
 * @param password The user's password from the URL.
 * @param session The user's session from the URL.
 * @return The result from the auth_mod function.
 */
bool check_mod(const char* username, const char* password, const char* session) {
    return auth_mod(username, password, session);
}

int main() {
    // 5 test cases
    printf("--- Test Cases ---\n");

    // Test Case 1: Correct moderator credentials
    bool isMod1 = check_mod("admin", "admin_pass", "sess_abc123");
    printf("Test 1 (admin, admin_pass): Is Moderator? %s\n", isMod1 ? "true" : "false");

    // Test Case 2: Incorrect password
    bool isMod2 = check_mod("admin", "wrong_pass", "sess_def456");
    printf("Test 2 (admin, wrong_pass): Is Moderator? %s\n", isMod2 ? "true" : "false");

    // Test Case 3: Incorrect username
    bool isMod3 = check_mod("user", "admin_pass", "sess_ghi789");
    printf("Test 3 (user, admin_pass): Is Moderator? %s\n", isMod3 ? "true" : "false");

    // Test Case 4: Regular user credentials
    bool isMod4 = check_mod("user", "user_pass", "sess_jkl012");
    printf("Test 4 (user, user_pass): Is Moderator? %s\n", isMod4 ? "true" : "false");

    // Test Case 5: Empty credentials
    bool isMod5 = check_mod("", "", "sess_mno345");
    printf("Test 5 (empty credentials): Is Moderator? %s\n", isMod5 ? "true" : "false");

    return 0;
}