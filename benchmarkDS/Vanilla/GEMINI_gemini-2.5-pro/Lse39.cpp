#include <iostream>
#include <string>
#include <vector>
#include <utility>

/**
 * @brief Checks if the provided username and password are correct.
 * @param username The username to check.
 * @param password The password to check.
 * @return true if credentials are correct, false otherwise.
 */
bool login(const std::string& username, const std::string& password) {
    // Hardcoded correct credentials
    const std::string correctUsername = "admin";
    const std::string correctPassword = "password123";

    // Check if both username and password match
    return (username == correctUsername && password == correctPassword);
}

int main() {
    // 5 test cases
    std::vector<std::pair<std::string, std::string>> testCases = {
        {"admin", "password123"},    // 1. Correct credentials
        {"admin", "wrongpassword"},  // 2. Correct username, wrong password
        {"user", "password123"},     // 3. Wrong username, correct password
        {"user", "wrongpassword"},   // 4. Both wrong
        {"", ""}                     // 5. Empty credentials
    };

    for (const auto& testCase : testCases) {
        const std::string& username = testCase.first;
        const std::string& password = testCase.second;

        std::cout << "Testing with Username: '" << username << "', Password: '" << password << "'" << std::endl;
        if (login(username, password)) {
            std::cout << "Login successful. Redirecting to session page." << std::endl;
        } else {
            std::cout << "Incorrect username or password." << std::endl;
        }
        std::cout << "--------------------" << std::endl;
    }

    return 0;
}