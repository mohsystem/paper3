#include <iostream>
#include <string>
#include <vector>

// NOTE: Storing credentials in plaintext is a major security risk.
// In a real application, use a secure method like hashed and salted passwords.
const std::string CORRECT_USERNAME = "admin";
const std::string CORRECT_PASSWORD = "password123";

/**
 * Performs a constant-time string comparison to mitigate timing attacks.
 *
 * @param a The first string.
 * @param b The second string.
 * @return true if strings are equal, false otherwise.
 */
bool secure_compare(const std::string& a, const std::string& b) {
    if (a.length() != b.length()) {
        return false;
    }

    // Use a bitwise OR operation that does not short-circuit.
    // This ensures the comparison takes the same amount of time regardless of where the first difference occurs.
    int result = 0;
    for (size_t i = 0; i < a.length(); ++i) {
        result |= a[i] ^ b[i];
    }

    return result == 0;
}

/**
 * Checks if the provided username and password are correct.
 *
 * @param username The username to check.
 * @param password The password to check.
 * @return true if credentials are correct, false otherwise.
 */
bool check_mod(const std::string& username, const std::string& password) {
    bool isUsernameCorrect = secure_compare(username, CORRECT_USERNAME);
    bool isPasswordCorrect = secure_compare(password, CORRECT_PASSWORD);

    return isUsernameCorrect && isPasswordCorrect;
}

int main() {
    // std::boolalpha makes cout print "true" or "false" instead of 1 or 0
    std::cout << std::boolalpha;

    // Test Case 1: Correct username and password
    std::cout << "Test Case 1 (Correct credentials): " << check_mod("admin", "password123") << std::endl;

    // Test Case 2: Correct username, incorrect password
    std::cout << "Test Case 2 (Incorrect password): " << check_mod("admin", "wrongpassword") << std::endl;

    // Test Case 3: Incorrect username, correct password
    std::cout << "Test Case 3 (Incorrect username): " << check_mod("user", "password123") << std::endl;

    // Test Case 4: Incorrect username and password
    std::cout << "Test Case 4 (Incorrect credentials): " << check_mod("user", "wrongpassword") << std::endl;
    
    // Test Case 5: Empty inputs
    std::cout << "Test Case 5 (Empty input): " << check_mod("admin", "") << std::endl;

    return 0;
}