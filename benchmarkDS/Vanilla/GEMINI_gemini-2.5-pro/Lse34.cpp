#include <iostream>
#include <string>
#include <map>

// Simulating a user database
std::map<std::string, std::string> userDatabase = {
    {"admin", "password123"},
    {"user1", "userpass"},
    {"john.doe", "johndoe@123"}
};

/**
 * @brief Checks user credentials against the database.
 * @param username The username entered by the user.
 * @param password The password entered by the user.
 * @return A string indicating the redirection path.
 */
std::string login(const std::string& username, const std::string& password) {
    auto it = userDatabase.find(username);
    
    // Check if the username exists and the password matches
    if (it != userDatabase.end() && it->second == password) {
        return "Login successful. Redirecting to dashboard page.";
    } else {
        return "Invalid credentials. Redirecting back to login page.";
    }
}

int main() {
    // Test Cases
    std::cout << "--- 5 Test Cases ---" << std::endl;

    // Test Case 1: Correct username and password
    std::cout << "Test 1 (admin, password123): " << login("admin", "password123") << std::endl;

    // Test Case 2: Correct username, incorrect password
    std::cout << "Test 2 (admin, wrongpassword): " << login("admin", "wrongpassword") << std::endl;

    // Test Case 3: Incorrect username
    std::cout << "Test 3 (unknownuser, password123): " << login("unknownuser", "password123") << std::endl;
    
    // Test Case 4: Another valid user
    std::cout << "Test 4 (user1, userpass): " << login("user1", "userpass") << std::endl;

    // Test Case 5: Case sensitivity check (username is case-sensitive)
    std::cout << "Test 5 (Admin, password123): " << login("Admin", "password123") << std::endl;

    return 0;
}