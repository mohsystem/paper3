#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <iomanip>

// NOTE: C++ does not have a standard library for cryptographic hashing.
// The hashPassword function below is a VERY SIMPLE, NON-SECURE hash for demonstration purposes only.
// DO NOT USE THIS IN A REAL APPLICATION.
// For production code, use a dedicated library like OpenSSL, Crypto++, or Botan
// to implement a strong, salted hashing algorithm (e.g., bcrypt, Argon2).

/**
 * A simple, non-secure string hashing function for demonstration.
 * WARNING: NOT FOR PRODUCTION USE.
 * @param password The string to hash.
 * @return A hex string representation of the simple hash.
 */
std::string hashPassword(const std::string& password) {
    unsigned long hash = 5381;
    for (char c : password) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << hash;
    return ss.str();
}

/**
 * Updates the user's password in the simulated database.
 * @param username The username of the user to update.
 * @param newPassword The new plain text password.
 * @param db The database map to update.
 */
void updateUserPassword(const std::string& username, const std::string& newPassword, std::map<std::string, std::string>& db) {
    std::string hashedPassword = hashPassword(newPassword);
    db[username] = hashedPassword;
    std::cout << "Password for user '" << username << "' has been updated." << std::endl;
}

void printDatabase(const std::map<std::string, std::string>& db) {
    for (const auto& pair : db) {
        std::cout << "User: " << pair.first << ", Hashed Pass: " << pair.second << std::endl;
    }
}

int main() {
    // Simulates a database table of users. Maps username -> hashedPassword
    std::map<std::string, std::string> database;
    
    // Initialize database with some users
    database["alice"] = hashPassword("initialPass1");
    database["bob"] = hashPassword("initialPass2");

    std::cout << "--- Initial Database State ---" << std::endl;
    printDatabase(database);

    std::cout << "\n--- Running 5 Test Cases ---" << std::endl;
    
    // Test Case 1: Update existing user 'alice'
    updateUserPassword("alice", "newSecurePassword123", database);

    // Test Case 2: Update existing user 'bob'
    updateUserPassword("bob", "anotherPassword!@#", database);

    // Test Case 3: Add a new user 'charlie'
    updateUserPassword("charlie", "charliePass2024", database);
    
    // Test Case 4: Update user 'alice' again
    updateUserPassword("alice", "evenNewerPassword$", database);

    // Test Case 5: Add a new user 'david'
    updateUserPassword("david", "d@v1d$P@$$", database);

    std::cout << "\n--- Final Database State ---" << std::endl;
    printDatabase(database);

    return 0;
}