#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm> // For std::transform
#include <cctype>    // For ::tolower

// ============================= SECURITY WARNING =================================
// The C++ standard library does NOT include cryptographic functions.
// The hashing function (`pseudo_hash`) below is for DEMONSTRATION PURPOSES ONLY.
// It illustrates the concepts of salting and iteration but is NOT
// CRYPTOGRAPHICALLY SECURE and MUST NOT be used in a real application.
//
// In a real-world C++ application, you MUST use a well-vetted, reputable
// cryptography library like OpenSSL, Crypto++, or libsodium to handle password
// hashing (e.g., with Argon2, scrypt, or PBKDF2).
// ==============================================================================

struct UserData {
    std::string username;
    std::string salt;
    std::string hashedPassword;
};

// In-memory user store to simulate a database.
std::unordered_map<std::string, UserData> userDatabase;

/**
 * A NON-SECURE pseudo-hashing function for demonstration purposes.
 * It combines password and salt and iterates to make it slower, mimicking
 * a key-derivation function's structure.
 * DO NOT USE THIS IN PRODUCTION.
 */
std::string pseudo_hash(const std::string& password, const std::string& salt) {
    std::string combined = password + salt;
    unsigned long long hash = 5381; // A simple starting prime for djb2
    for (int i = 0; i < 10000; ++i) { // Fake "iterations"
        for (char c : combined) {
            hash = ((hash << 5) + hash) + c; // hash * 33 + c
        }
        combined = std::to_string(hash);
    }
    return std::to_string(hash);
}

/**
 * Compares two strings in a way that takes a constant amount of time
 * to help prevent timing attacks. It compares all characters regardless
 * of when the first difference is found.
 */
bool secure_compare(const std::string& s1, const std::string& s2) {
    if (s1.length() != s2.length()) {
        return false;
    }

    int result = 0;
    for (size_t i = 0; i < s1.length(); ++i) {
        result |= s1[i] ^ s2[i];
    }
    return result == 0;
}

/**
 * Simulates user registration.
 */
void registerUser(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        std::cout << "Registration failed: Username and password cannot be empty." << std::endl;
        return;
    }
    // Simple pseudo-random salt (in a real app, use a crypto-secure RNG)
    std::string salt = "salt_for_" + username; 
    std::string hashedPassword = pseudo_hash(password, salt);
    
    std::string lower_username = username;
    std::transform(lower_username.begin(), lower_username.end(), lower_username.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    userDatabase[lower_username] = {username, salt, hashedPassword};
}


/**
 * Simulates a login attempt.
 * @return A string indicating the result.
 */
std::string login(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        return "Login failed: Username or password cannot be empty. Redirecting to login page.";
    }
    
    std::string lower_username = username;
    std::transform(lower_username.begin(), lower_username.end(), lower_username.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    auto it = userDatabase.find(lower_username);

    if (it != userDatabase.end()) {
        UserData& user = it->second;
        std::string providedHash = pseudo_hash(password, user.salt);
        
        if (secure_compare(providedHash, user.hashedPassword)) {
            return "Login successful for " + user.username + "! Redirecting to dashboard.";
        }
    }

    return "Login failed: Invalid username or password. Redirecting to login page.";
}

int main() {
    // --- Setup: Simulate user registration ---
    registerUser("admin", "AdminPassword123!");
    registerUser("jdoe", "P@ssw0rd_jdoe");
    std::cout << "--- User database initialized ---" << std::endl;

    // --- Test Cases ---
    std::cout << "\n--- Running Test Cases ---" << std::endl;

    // Test Case 1: Correct username and password
    std::cout << "1. " << login("admin", "AdminPassword123!") << std::endl;

    // Test Case 2: Correct username, incorrect password
    std::cout << "2. " << login("admin", "WrongPassword") << std::endl;

    // Test Case 3: Incorrect username
    std::cout << "3. " << login("unknownUser", "anyPassword") << std::endl;

    // Test Case 4: Correct username (different case), correct password
    std::cout << "4. " << login("Admin", "AdminPassword123!") << std::endl;

    // Test Case 5: Empty password
    std::cout << "5. " << login("jdoe", "") << std::endl;

    // Test Case 6: Empty username
    std::cout << "6. " << login("", "P@ssw0rd_jdoe") << std::endl;
    
    return 0;
}