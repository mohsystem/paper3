#include <iostream>
#include <string>
#include <vector>

// NOTE: Storing credentials in plaintext is a major security risk.
// In a real application, use a strong, salted hashing algorithm (e.g., Argon2, bcrypt)
// to store password hashes and compare against the hash of the user-provided password.
// These are hardcoded for demonstration purposes only.
const std::string CORRECT_USERNAME = "admin";
const std::string CORRECT_PASSWORD = "password123";

/**
 * Performs a constant-time string comparison to mitigate timing attacks.
 *
 * @param s1 The first string.
 * @param s2 The second string.
 * @return true if the strings are equal, false otherwise.
 */
bool secureEquals(const std::string& s1, const std::string& s2) {
    if (s1.length() != s2.length()) {
        return false;
    }

    int diff = 0;
    for (size_t i = 0; i < s1.length(); ++i) {
        diff |= s1[i] ^ s2[i];
    }
    return diff == 0;
}

/**
 * Simulates checking a username and password from a URL.
 * Returns "true" if credentials are correct, "false" otherwise.
 *
 * @param username The username to check.
 * @param password The password to check.
 * @return The string "true" on success, "false" on failure.
 */
std::string check_mod(const std::string& username, const std::string& password) {
    // Usernames are often not secret, so a standard comparison is acceptable.
    bool isUsernameCorrect = (CORRECT_USERNAME == username);

    // Passwords must be compared in constant time to prevent timing attacks.
    bool isPasswordCorrect = secureEquals(CORRECT_PASSWORD, password);

    if (isUsernameCorrect && isPasswordCorrect) {
        return "true";
    } else {
        return "false";
    }
}

int main() {
    // Test cases
    std::vector<std::pair<std::string, std::string>> testCases = {
        {"admin", "password123"},      // 1. Correct credentials
        {"admin", "wrongpassword"},     // 2. Correct user, wrong password
        {"user", "password123"},      // 3. Wrong user, correct password
        {"user", "wrongpassword"},      // 4. Wrong credentials
        {"", ""}                       // 5. Empty credentials
    };

    std::cout << "Running test cases..." << std::endl;
    int i = 1;
    for (const auto& testCase : testCases) {
        std::string username = testCase.first;
        std::string password = testCase.second;
        std::string result = check_mod(username, password);
        std::cout << "Test Case " << i++ << ":" << std::endl;
        std::cout << "  Username: \"" << username << "\"" << std::endl;
        std::cout << "  Password: \"" << password << "\"" << std::endl;
        std::cout << "  Result: " << result << std::endl << std::endl;
    }

    return 0;
}