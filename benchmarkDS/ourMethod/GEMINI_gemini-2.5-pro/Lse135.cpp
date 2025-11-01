#include <iostream>
#include <string>
#include <vector>

// WARNING: This is a simplified example for demonstration purposes only.
// DO NOT use hardcoded credentials or plaintext password comparison in production applications.
// In a real-world scenario, passwords should be securely stored as salted hashes
// using a strong algorithm like Argon2, scrypt, or PBKDF2, and credentials should
// be managed through a secure configuration or secrets management system.
const std::string CORRECT_USERNAME = "admin";
const std::string CORRECT_PASSWORD = "Pa$$w0rd123!";

/**
 * @brief Performs a constant-time comparison of two strings.
 * This is crucial for comparing secrets like passwords to mitigate timing attacks.
 * @param a The first string.
 * @param b The second string.
 * @return true if the strings are equal, false otherwise.
 */
bool constant_time_compare(const std::string& a, const std::string& b) {
    if (a.length() != b.length()) {
        return false;
    }

    int result = 0;
    for (size_t i = 0; i < a.length(); ++i) {
        result |= a[i] ^ b[i];
    }

    return result == 0;
}


/**
 * @brief Checks if the provided username and password are correct.
 * This function simulates checking credentials from a form.
 * @param username The username to check.
 * @param password The password to check.
 * @return true if the credentials are correct, false otherwise.
 */
bool check_mod(const std::string& username, const std::string& password) {
    // Rule#1: Input is validated by type (const std::string&).
    
    // Username can be compared directly.
    bool usernameMatch = (username == CORRECT_USERNAME);

    // Use a constant-time comparison for the password to mitigate timing attacks.
    bool passwordMatch = constant_time_compare(password, CORRECT_PASSWORD);

    return usernameMatch && passwordMatch;
}


int main() {
    // 5 Test Cases
    std::cout << std::boolalpha; // Print bools as "true" or "false"

    std::cout << "Test Case 1: Correct credentials" << std::endl;
    std::cout << "Input: user='admin', pass='Pa$$w0rd123!'" << std::endl;
    std::cout << "Result: " << check_mod("admin", "Pa$$w0rd123!") << std::endl; // Expected: true

    std::cout << "\nTest Case 2: Incorrect username" << std::endl;
    std::cout << "Input: user='user', pass='Pa$$w0rd123!'" << std::endl;
    std::cout << "Result: " << check_mod("user", "Pa$$w0rd123!") << std::endl; // Expected: false

    std::cout << "\nTest Case 3: Incorrect password" << std::endl;
    std::cout << "Input: user='admin', pass='wrongpassword'" << std::endl;
    std::cout << "Result: " << check_mod("admin", "wrongpassword") << std::endl; // Expected: false

    std::cout << "\nTest Case 4: Different length password" << std::endl;
    std::cout << "Input: user='admin', pass='short'" << std::endl;
    std::cout << "Result: " << check_mod("admin", "short") << std::endl; // Expected: false

    std::cout << "\nTest Case 5: Empty inputs" << std::endl;
    std::cout << "Input: user='', pass=''" << std::endl;
    std::cout << "Result: " << check_mod("", "") << std::endl; // Expected: false

    return 0;
}