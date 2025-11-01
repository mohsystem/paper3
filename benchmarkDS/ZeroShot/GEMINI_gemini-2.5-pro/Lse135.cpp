#include <iostream>
#include <string>
#include <vector>

// In a real application, these would not be hardcoded.
// The password would be a salted hash, not plaintext.
const std::string CORRECT_USERNAME = "admin";
const std::string CORRECT_PASSWORD = "P@ssw0rd!123";

/**
 * @brief Performs a constant-time string comparison to mitigate timing attacks.
 * 
 * @param a The first string.
 * @param b The second string.
 * @return true if the strings are equal, false otherwise.
 */
bool secure_equals(const std::string& a, const std::string& b) {
    // If lengths are different, they can't be equal.
    // This check is the first step.
    if (a.length() != b.length()) {
        return false;
    }

    // Use a variable to accumulate differences without short-circuiting.
    // The XOR result will be 0 only if all characters are the same.
    int diff = 0;
    for (size_t i = 0; i < a.length(); ++i) {
        diff |= a[i] ^ b[i];
    }

    return diff == 0;
}

/**
 * @brief Securely checks if the provided username and password are correct.
 *
 * @param username The username provided by the user.
 * @param password The password provided by the user.
 * @return true if credentials are correct, false otherwise.
 */
bool check_mod(const std::string& username, const std::string& password) {
    bool userMatch = secure_equals(CORRECT_USERNAME, username);
    bool passMatch = secure_equals(CORRECT_PASSWORD, password);
    return userMatch && passMatch;
}

int main() {
    std::cout << "Test Case 1: Correct credentials" << std::endl;
    std::cout << "Input: admin, P@ssw0rd!123" << std::endl;
    std::cout << "Output: " << std::boolalpha << check_mod("admin", "P@ssw0rd!123") << std::endl; // Expected: true
    std::cout << "--------------------" << std::endl;

    std::cout << "Test Case 2: Correct username, incorrect password" << std::endl;
    std::cout << "Input: admin, wrongpassword" << std::endl;
    std::cout << "Output: " << std::boolalpha << check_mod("admin", "wrongpassword") << std::endl; // Expected: false
    std::cout << "--------------------" << std::endl;

    std::cout << "Test Case 3: Incorrect username, correct password" << std::endl;
    std::cout << "Input: user, P@ssw0rd!123" << std::endl;
    std::cout << "Output: " << std::boolalpha << check_mod("user", "P@ssw0rd!123") << std::endl; // Expected: false
    std::cout << "--------------------" << std::endl;

    std::cout << "Test Case 4: Incorrect credentials" << std::endl;
    std::cout << "Input: user, password" << std::endl;
    std::cout << "Output: " << std::boolalpha << check_mod("user", "password") << std::endl; // Expected: false
    std::cout << "--------------------" << std::endl;

    std::cout << "Test Case 5: Correct username, password with different length" << std::endl;
    std::cout << "Input: admin, P@ss" << std::endl;
    std::cout << "Output: " << std::boolalpha << check_mod("admin", "P@ss") << std::endl; // Expected: false
    std::cout << "--------------------" << std::endl;

    return 0;
}