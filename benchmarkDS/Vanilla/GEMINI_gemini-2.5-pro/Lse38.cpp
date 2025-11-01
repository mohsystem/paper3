#include <iostream>
#include <string>
#include <map>

// Simulates a user database
std::map<std::string, std::string> userDatabase = {
    {"admin", "password123"},
    {"johndoe", "secret"},
    {"testuser", "test"}
};

/**
 * Checks user credentials against the database.
 * @param username The username to check.
 * @param password The password to check.
 * @return A string indicating the result of the login attempt.
 */
std::string login(const std::string& username, const std::string& password) {
    auto it = userDatabase.find(username);
    if (it != userDatabase.end()) { // Found username
        if (it->second == password) { // Check password
            return "Login Successful. Redirecting to the dashboard.";
        }
    }
    return "Invalid username or password. Redirecting to the login page.";
}

int main() {
    std::cout << "--- C++ Login System Test Cases ---" << std::endl;

    // Test Case 1: Correct credentials
    std::cout << "Test 1 (admin, password123): " << login("admin", "password123") << std::endl;

    // Test Case 2: Correct username, incorrect password
    std::cout << "Test 2 (admin, wrongpass): " << login("admin", "wrongpass") << std::endl;

    // Test Case 3: Incorrect username
    std::cout << "Test 3 (user, password123): " << login("user", "password123") << std::endl;

    // Test Case 4: Another set of correct credentials
    std::cout << "Test 4 (johndoe, secret): " << login("johndoe", "secret") << std::endl;

    // Test Case 5: Non-existent user
    std::cout << "Test 5 (guest, guest): " << login("guest", "guest") << std::endl;

    return 0;
}