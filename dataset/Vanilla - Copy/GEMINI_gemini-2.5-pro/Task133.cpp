#include <iostream>
#include <string>
#include <map>

// Type alias for our user database for clarity
using UserDatabase = std::map<std::string, std::string>;

/**
 * @brief Resets the password for a given user if the user exists.
 *
 * @param db The user database (map of username to password). Passed by reference to allow modification.
 * @param username The username of the user.
 * @param newPassword The new password to set.
 * @return true if the password was successfully reset, false otherwise.
 */
bool resetPassword(UserDatabase& db, const std::string& username, const std::string& newPassword) {
    if (username.empty()) {
        return false;
    }

    auto it = db.find(username);
    if (it != db.end()) {
        it->second = newPassword; // Update the password
        return true;
    }
    return false; // User not found
}

void printDatabase(const UserDatabase& db) {
    std::cout << "{";
    for (auto const& [key, val] : db) {
        std::cout << key << ": " << val << ", ";
    }
    std::cout << "}" << std::endl;
}

int main() {
    // Initialize the database with some users
    UserDatabase userDatabase = {
        {"alice", "password123"},
        {"bob", "bobspass"},
        {"charlie", "securepass"}
    };

    std::cout << "Initial Database: ";
    printDatabase(userDatabase);
    std::cout << "--- Running Test Cases ---" << std::endl;

    // Test Case 1: Successful password reset for an existing user
    bool result1 = resetPassword(userDatabase, "alice", "newAlicePass!@#");
    std::cout << "1. Resetting 'alice' password. Success: " << std::boolalpha << result1 << std::endl;
    std::cout << "   Database after: ";
    printDatabase(userDatabase);

    // Test Case 2: Attempt to reset password for a non-existent user
    bool result2 = resetPassword(userDatabase, "dave", "davespass");
    std::cout << "\n2. Resetting 'dave' (non-existent) password. Success: " << std::boolalpha << result2 << std::endl;
    std::cout << "   Database after: ";
    printDatabase(userDatabase);

    // Test Case 3: Successful reset for another existing user
    bool result3 = resetPassword(userDatabase, "bob", "betterP@ssw0rd");
    std::cout << "\n3. Resetting 'bob' password. Success: " << std::boolalpha << result3 << std::endl;
    std::cout << "   Database after: ";
    printDatabase(userDatabase);
    
    // Test Case 4: Resetting the same user's password again
    bool result4 = resetPassword(userDatabase, "alice", "evenNewerPass456");
    std::cout << "\n4. Resetting 'alice' password again. Success: " << std::boolalpha << result4 << std::endl;
    std::cout << "   Database after: ";
    printDatabase(userDatabase);

    // Test Case 5: Attempt to reset password with an empty username
    bool result5 = resetPassword(userDatabase, "", "somepassword");
    std::cout << "\n5. Resetting password for empty username. Success: " << std::boolalpha << result5 << std::endl;
    std::cout << "   Database after: ";
    printDatabase(userDatabase);

    return 0;
}