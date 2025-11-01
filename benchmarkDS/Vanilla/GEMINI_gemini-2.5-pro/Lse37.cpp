#include <iostream>
#include <string>
#include <map>

// In-memory map to simulate a user database
static std::map<std::string, std::string> database = {
    {"admin", "password123"},
    {"user", "userpass"},
    {"testuser", "test"}
};

/**
 * Checks user credentials against the database.
 * @param username The username to check.
 * @param password The password to check.
 * @return "success" on match, "cant_login" on mismatch, "internal_server_error" on unexpected issues.
 */
std::string login(const std::string& username, const std::string& password) {
    try {
        // Find the username in the database
        auto it = database.find(username);
        
        // Check if username exists and the password matches
        if (it != database.end() && it->second == password) {
            return "success";
        } else {
            return "cant_login";
        }
    } catch (...) {
        // Catch any unexpected exceptions
        return "internal_server_error";
    }
}

int main() {
    // Test Case 1: Correct username and password
    std::cout << "Test 1: login(\"admin\", \"password123\") -> " << login("admin", "password123") << std::endl;

    // Test Case 2: Correct username, incorrect password
    std::cout << "Test 2: login(\"user\", \"wrongpassword\") -> " << login("user", "wrongpassword") << std::endl;

    // Test Case 3: Incorrect username
    std::cout << "Test 3: login(\"unknown\", \"userpass\") -> " << login("unknown", "userpass") << std::endl;

    // Test Case 4: Empty credentials
    std::cout << "Test 4: login(\"\", \"\") -> " << login("", "") << std::endl;

    // Test Case 5: Another correct username and password
    std::cout << "Test 5: login(\"testuser\", \"test\") -> " << login("testuser", "test") << std::endl;

    return 0;
}