#include <iostream>
#include <string>
#include <map>

/**
 * Simulates a login function.
 * @param username The username entered by the user.
 * @param password The password entered by the user.
 * @param userDatabase A map simulating the database.
 * @return A string indicating the login status.
 */
std::string login(const std::string& username, const std::string& password, const std::map<std::string, std::string>& userDatabase) {
    // Check if the username exists in the database
    auto it = userDatabase.find(username);
    if (it != userDatabase.end()) {
        // Username found, check if the password is correct
        if (it->second == password) {
            return "Login Successful. Redirecting to home page.";
        } else {
            return "Incorrect Password.";
        }
    } else {
        // Username not found
        return "User Not Found.";
    }
}

int main() {
    // Simulating a user database
    std::map<std::string, std::string> userDatabase = {
        {"admin", "admin123"},
        {"user_test", "password456"},
        {"john_doe", "jd_pass"}
    };

    std::cout << "--- Running 5 Test Cases ---" << std::endl;

    // Test Case 1: Successful login
    std::cout << "Test 1 (admin, admin123):" << std::endl;
    std::string result1 = login("admin", "admin123", userDatabase);
    std::cout << "Result: " << result1 << std::endl << std::endl;

    // Test Case 2: Incorrect password
    std::cout << "Test 2 (user_test, wrongpassword):" << std::endl;
    std::string result2 = login("user_test", "wrongpassword", userDatabase);
    std::cout << "Result: " << result2 << std::endl << std::endl;

    // Test Case 3: User does not exist
    std::cout << "Test 3 (unknown_user, password123):" << std::endl;
    std::string result3 = login("unknown_user", "password123", userDatabase);
    std::cout << "Result: " << result3 << std::endl << std::endl;

    // Test Case 4: Another successful login
    std::cout << "Test 4 (john_doe, jd_pass):" << std::endl;
    std::string result4 = login("john_doe", "jd_pass", userDatabase);
    std::cout << "Result: " << result4 << std::endl << std::endl;

    // Test Case 5: Existing user, incorrect password
    std::cout << "Test 5 (admin, password456):" << std::endl;
    std::string result5 = login("admin", "password456", userDatabase);
    std::cout << "Result: " << result5 << std::endl << std::endl;

    return 0;
}