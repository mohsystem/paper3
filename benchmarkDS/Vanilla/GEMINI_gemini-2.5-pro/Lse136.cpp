#include <iostream>
#include <string>
#include <vector>

/**
 * @brief Checks if the provided username and password are correct.
 * For this example, the correct credentials are "admin" and "password123".
 *
 * @param username The username to check.
 * @param password The password to check.
 * @return The string "true" if credentials are correct, "false" otherwise.
 */
std::string check_mod(const std::string& username, const std::string& password) {
    const std::string correctUsername = "admin";
    const std::string correctPassword = "password123";

    if (username == correctUsername && password == correctPassword) {
        return "true";
    } else {
        return "false";
    }
}

int main() {
    // Test Case 1: Correct credentials
    std::cout << "Test 1 (Correct): " << check_mod("admin", "password123") << std::endl;

    // Test Case 2: Incorrect password
    std::cout << "Test 2 (Wrong Pass): " << check_mod("admin", "wrongpass") << std::endl;

    // Test Case 3: Incorrect username
    std::cout << "Test 3 (Wrong User): " << check_mod("user", "password123") << std::endl;

    // Test Case 4: Both incorrect
    std::cout << "Test 4 (Both Wrong): " << check_mod("user", "wrongpass") << std::endl;
    
    // Test Case 5: Empty inputs
    std::cout << "Test 5 (Empty Input): " << check_mod("", "") << std::endl;

    return 0;
}