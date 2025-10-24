#include <iostream>
#include <string>
#include <map>

// In a real application, passwords should be hashed and stored securely.
// For this example, we use a simple map with plain text for demonstration.
std::map<std::string, std::string> userDatabase = {
    {"admin", "password123"},
    {"user1", "securepass"},
    {"john.doe", "johndoe_pass"}
};

/**
 * @brief Authenticates a user based on username and password.
 * 
 * @param username The username to check.
 * @param password The password to verify.
 * @return true if authentication is successful, false otherwise.
 */
bool authenticate(const std::string& username, const std::string& password) {
    auto it = userDatabase.find(username);
    
    // Check if user exists
    if (it != userDatabase.end()) {
        // User found, check if password matches
        if (it->second == password) {
            return true;
        }
    }
    
    // User not found or password incorrect
    return false;
}

int main() {
    std::cout << "--- User Authentication Test Cases ---" << std::endl;

    // Test Case 1: Successful login
    bool result1 = authenticate("admin", "password123");
    std::cout << "1. admin / password123 -> " << (result1 ? "Success" : "Failure") << std::endl;

    // Test Case 2: Wrong password
    bool result2 = authenticate("user1", "wrongpass");
    std::cout << "2. user1 / wrongpass -> " << (result2 ? "Success" : "Failure") << std::endl;

    // Test Case 3: Non-existent user
    bool result3 = authenticate("unknown_user", "password123");
    std::cout << "3. unknown_user / password123 -> " << (result3 ? "Success" : "Failure") << std::endl;

    // Test Case 4: Case-sensitive username
    bool result4 = authenticate("Admin", "password123");
    std::cout << "4. Admin / password123 -> " << (result4 ? "Success" : "Failure") << std::endl;

    // Test Case 5: Empty password
    bool result5 = authenticate("admin", "");
    std::cout << "5. admin / (empty) -> " << (result5 ? "Success" : "Failure") << std::endl;

    return 0;
}