#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

// User struct to hold user data.
struct User {
    std::string username;
    std::string email;
    std::string password; // In a real app, this should be a salted password hash.
};

// In-memory database and logged-in user state.
// In a real application, this would be managed differently (e.g., session management).
static std::unordered_map<std::string, User> userDatabase;
static std::string loggedInUser; // Empty string means no user is logged in.

/**
 * Simulates a user logging in.
 * @param username The username of the user.
 * @param password The password of the user.
 * @return true if login is successful, false otherwise.
 */
bool login(const std::string& username, const std::string& password) {
    auto it = userDatabase.find(username);
    if (it != userDatabase.end() && it->second.password == password) {
        loggedInUser = username;
        std::cout << "Login successful for: " << username << std::endl;
        return true;
    }
    std::cout << "Login failed for: " << username << std::endl;
    return false;
}

/**
 * Simulates a user logging out.
 */
void logout() {
    if (!loggedInUser.empty()) {
        std::cout << "User " << loggedInUser << " logged out." << std::endl;
        loggedInUser.clear();
    }
}

/**
 * Changes the email for the currently logged-in user.
 * @param oldEmail The user's current email address.
 * @param newEmail The desired new email address.
 * @param confirmPassword The user's password for verification.
 * @return A status message indicating success or failure.
 */
std::string changeEmail(const std::string& oldEmail, const std::string& newEmail, const std::string& confirmPassword) {
    // 1. Authorization: Check if a user is logged in.
    if (loggedInUser.empty()) {
        return "Error: You must be logged in to change your email.";
    }

    auto it = userDatabase.find(loggedInUser);
    if (it == userDatabase.end()) {
        // This case should ideally not happen if login logic is sound.
        return "Error: Logged-in user not found in database.";
    }

    // 2. Validation: Check if the provided old email matches the logged-in user's record.
    if (it->second.email != oldEmail) {
        return "Error: The old email address provided is incorrect.";
    }

    // 3. Authentication: Verify the user's password.
    // NOTE: In a real system, use a constant-time comparison for security.
    if (it->second.password != confirmPassword) {
        return "Error: Incorrect password.";
    }
    
    // 4. Input Validation: Check if new email is valid.
    if (newEmail.empty() || newEmail.find('@') == std::string::npos) {
        return "Error: New email is not valid.";
    }

    // 5. Uniqueness Check: Ensure the new email is not already in use by another user.
    for (const auto& pair : userDatabase) {
        if (pair.second.email == newEmail && pair.first != loggedInUser) {
            return "Error: The new email address is already in use by another account.";
        }
    }

    // 6. Update the email
    it->second.email = newEmail;
    return "Success: Email has been changed to " + newEmail;
}


int main() {
    // Setup initial user data
    userDatabase["john.doe"] = {"john.doe", "john.d@example.com", "password123"};
    userDatabase["jane.doe"] = {"jane.doe", "jane.d@example.com", "password456"};

    std::cout << "--- Test Case 1: Successful Email Change ---" << std::endl;
    login("john.doe", "password123");
    std::string result1 = changeEmail("john.d@example.com", "john.new@example.com", "password123");
    std::cout << "Result: " << result1 << std::endl;
    std::cout << "Current user info: email = " << userDatabase["john.doe"].email << std::endl;
    logout();
    std::cout << "\n--------------------------------------------\n" << std::endl;

    std::cout << "--- Test Case 2: Attempt to Change Email While Logged Out ---" << std::endl;
    std::string result2 = changeEmail("jane.d@example.com", "jane.new@example.com", "password456");
    std::cout << "Result: " << result2 << std::endl;
    std::cout << "\n--------------------------------------------\n" << std::endl;
    
    std::cout << "--- Test Case 3: Incorrect Password ---" << std::endl;
    login("jane.doe", "password456");
    std::string result3 = changeEmail("jane.d@example.com", "jane.new@example.com", "wrongpassword");
    std::cout << "Result: " << result3 << std::endl;
    logout();
    std::cout << "\n--------------------------------------------\n" << std::endl;
    
    std::cout << "--- Test Case 4: Incorrect Old Email ---" << std::endl;
    login("john.doe", "password123");
    std::string result4 = changeEmail("wrong.email@example.com", "john.another@example.com", "password123");
    std::cout << "Result: " << result4 << std::endl;
    logout();
    std::cout << "\n--------------------------------------------\n" << std::endl;

    std::cout << "--- Test Case 5: New Email Already in Use ---" << std::endl;
    login("john.doe", "password123");
    std::string result5 = changeEmail("john.new@example.com", "jane.d@example.com", "password123");
    std::cout << "Result: " << result5 << std::endl;
    logout();
    std::cout << "\n--------------------------------------------\n" << std::endl;

    return 0;
}