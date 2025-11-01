#include <iostream>
#include <string>
#include <vector>

/**
 * @brief Checks if the username and password are correct.
 * The correct credentials are "admin" and "password123".
 * 
 * @param username The username to check.
 * @param password The password to check.
 * @return true if the credentials are correct, false otherwise.
 */
bool check_mod(const std::string& username, const std::string& password) {
    return username == "admin" && password == "password123";
}

int main() {
    // 5 Test Cases
    std::cout << std::boolalpha; // Print "true" or "false" instead of 1 or 0
    std::cout << "Test Case 1 (Correct): " << check_mod("admin", "password123") << std::endl;
    std::cout << "Test Case 2 (Wrong Password): " << check_mod("admin", "wrongpass") << std::endl;
    std::cout << "Test Case 3 (Wrong Username): " << check_mod("user", "password123") << std::endl;
    std::cout << "Test Case 4 (Both Wrong): " << check_mod("user", "wrongpass") << std::endl;
    std::cout << "Test Case 5 (Empty Strings): " << check_mod("", "") << std::endl;

    return 0;
}