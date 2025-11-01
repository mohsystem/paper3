#include <iostream>
#include <string>

/**
 * @brief Checks if a user is a moderator based on hardcoded credentials.
 * In a real application, this would involve database lookups, password hashing,
 * and proper session validation.
 * @param username The user's name.
 * @param password The user's password.
 * @param session The user's session token (unused in this simple example).
 * @return true if the user is a moderator, false otherwise.
 */
bool auth_mod(const std::string& username, const std::string& password, const std::string& session) {
    // Simple hardcoded check for demonstration purposes.
    // The session parameter is included to match the requirements but is not used in the logic.
    return username == "admin" && password == "admin_pass";
}

/**
 * @brief Simulates a URL endpoint that checks for moderator privileges.
 * It receives user credentials and calls the authentication logic.
 * @param username The user's name from the URL.
 * @param password The user's password from the URL.
 * @param session The user's session from the URL.
 * @return The result from the auth_mod function.
 */
bool check_mod(const std::string& username, const std::string& password, const std::string& session) {
    return auth_mod(username, password, session);
}

int main() {
    // 5 test cases
    std::cout << "--- Test Cases ---" << std::endl;
    // Use std::boolalpha to print boolean values as "true" or "false"
    std::cout << std::boolalpha;

    // Test Case 1: Correct moderator credentials
    bool isMod1 = check_mod("admin", "admin_pass", "sess_abc123");
    std::cout << "Test 1 (admin, admin_pass): Is Moderator? " << isMod1 << std::endl;

    // Test Case 2: Incorrect password
    bool isMod2 = check_mod("admin", "wrong_pass", "sess_def456");
    std::cout << "Test 2 (admin, wrong_pass): Is Moderator? " << isMod2 << std::endl;

    // Test Case 3: Incorrect username
    bool isMod3 = check_mod("user", "admin_pass", "sess_ghi789");
    std::cout << "Test 3 (user, admin_pass): Is Moderator? " << isMod3 << std::endl;

    // Test Case 4: Regular user credentials
    bool isMod4 = check_mod("user", "user_pass", "sess_jkl012");
    std::cout << "Test 4 (user, user_pass): Is Moderator? " << isMod4 << std::endl;

    // Test Case 5: Empty credentials
    bool isMod5 = check_mod("", "", "sess_mno345");
    std::cout << "Test 5 (empty credentials): Is Moderator? " << isMod5 << std::endl;

    return 0;
}