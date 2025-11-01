#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

// In a real application, this would be a secure database connection.
// Passwords must be stored as salted hashes using a strong algorithm like Argon2, scrypt, or bcrypt.
// This map is a simplified simulation for demonstration purposes.
std::unordered_map<std::string, std::string> userDatabase = {
    // Storing "hashed" passwords. Real hashes are much longer.
    {"admin", "hashed_password_for_admin"},
    {"testuser", "hashed_password_for_user"}
};

/**
 * Performs a constant-time string comparison to mitigate timing attacks.
 * It compares every character regardless of when a mismatch is found.
 *
 * @param a The first string.
 * @param b The second string.
 * @return true if the strings are equal, false otherwise.
 */
bool secureEquals(const std::string& a, const std::string& b) {
    if (a.length() != b.length()) {
        return false;
    }

    // Use a variable to accumulate differences without short-circuiting.
    int diff = 0;
    for (size_t i = 0; i < a.length(); ++i) {
        diff |= a[i] ^ b[i];
    }
    return diff == 0;
}


/**
 * Simulates a user login process.
 *
 * @param username The username provided by the user.
 * @param password The password provided by the user.
 * @return A string indicating the result: "success", "cant_login", or "internal_server_error".
 */
std::string login(const std::string& username, const std::string& password) {
    try {
        // 1. Input Validation: Ensure username and password are not empty.
        if (username.empty() || password.empty()) {
            return "cant_login";
        }

        // 2. Database Lookup: Find the user in our simulated database.
        auto it = userDatabase.find(username);
        
        // 3. Password Verification
        // In a real app, you would hash the input password with the user's salt
        // and then compare the result with the stored hash.
        // The comparison MUST be done in constant time.
        bool is_match = false;
        if (it != userDatabase.end()) {
            // User found, compare passwords securely.
            std::string storedHashedPassword = it->second;
            is_match = secureEquals(storedHashedPassword, password);
        } else {
            // User not found, perform a dummy comparison to prevent timing leaks.
            secureEquals("dummy_hash_to_prevent_timing_attack", password);
        }

        if (is_match) {
            return "success";
        } else {
            // Generic error for both non-existent user and wrong password to prevent user enumeration.
            return "cant_login";
        }

    } catch (const std::exception& e) {
        // 4. Error Handling: Catch any unexpected exceptions.
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return "internal_server_error";
    }
}

int main() {
    std::cout << "--- Running Test Cases ---" << std::endl;

    // Test Case 1: Successful login
    std::string result1 = login("admin", "hashed_password_for_admin");
    std::cout << "Test Case 1 (Correct Credentials): " << result1 << std::endl;

    // Test Case 2: Incorrect password
    std::string result2 = login("admin", "wrong_password");
    std::cout << "Test Case 2 (Incorrect Password): " << result2 << std::endl;

    // Test Case 3: Non-existent user
    std::string result3 = login("unknown_user", "some_password");
    std::cout << "Test Case 3 (Non-existent User): " << result3 << std::endl;
    
    // Test Case 4: Empty username
    std::string result4 = login("", "some_password");
    std::cout << "Test Case 4 (Empty Username): " << result4 << std::endl;

    // Test Case 5: Empty password
    std::string result5 = login("testuser", "");
    std::cout << "Test Case 5 (Empty Password): " << result5 << std::endl;

    std::cout << "--- Test Cases Finished ---" << std::endl;

    return 0;
}