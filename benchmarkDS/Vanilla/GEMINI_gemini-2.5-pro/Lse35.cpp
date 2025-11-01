#include <iostream>
#include <string>
#include <map>

// This map simulates a database table of users
std::map<std::string, std::string> userDatabase = {
    {"admin", "admin123"},
    {"john.doe", "password123"},
    {"jane.doe", "securepass"}
};

/**
 * Simulates a login function that checks credentials against a database.
 * @param username The username provided by the user.
 * @param password The password provided by the user.
 * @return A string indicating the result of the login attempt.
 */
std::string login(const std::string& username, const std::string& password) {
    auto it = userDatabase.find(username);
    // Check if the username exists (iterator is not at the end) and the password matches
    if (it != userDatabase.end() && it->second == password) {
        return "Login successful. Redirecting to the home page.";
    } else {
        return "Invalid username or password. Redirecting to the login page.";
    }
}

class Lse35 {
public:
    static void runTests() {
        // 5 Test Cases
        std::cout << "Test Case 1: Valid credentials for 'admin'" << std::endl;
        std::cout << "Result: " << login("admin", "admin123") << std::endl;
        std::cout << "--------------------" << std::endl;

        std::cout << "Test Case 2: Valid credentials for 'john.doe'" << std::endl;
        std::cout << "Result: " << login("john.doe", "password123") << std::endl;
        std::cout << "--------------------" << std::endl;

        std::cout << "Test Case 3: Invalid password for 'admin'" << std::endl;
        std::cout << "Result: " << login("admin", "wrongpassword") << std::endl;
        std::cout << "--------------------" << std::endl;

        std::cout << "Test Case 4: Non-existent username" << std::endl;
        std::cout << "Result: " << login("unknown_user", "somepassword") << std::endl;
        std::cout << "--------------------" << std::endl;
        
        std::cout << "Test Case 5: Empty credentials" << std::endl;
        std::cout << "Result: " << login("", "") << std::endl;
        std::cout << "--------------------" << std::endl;
    }
};

int main() {
    Lse35::runTests();
    return 0;
}