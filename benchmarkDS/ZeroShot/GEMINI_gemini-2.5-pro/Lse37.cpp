#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>

// --- SECURITY WARNING ---
// C++ does not have a standard cryptography library.
// For a real-world, secure application, you MUST use a well-vetted,
// external library like OpenSSL, Crypto++, or libsodium for hashing.
// Using a simple, non-standard hash function as shown below is INSECURE
// and is only for demonstrating the logic flow (salting, constant-time compare).
// A real application should use an adaptive hashing function like Argon2 or BCrypt.

// --- INSECURE PLACEHOLDER HASH FUNCTION ---
// DO NOT USE THIS IN PRODUCTION. It is a simple demonstration.
std::string insecure_placeholder_hash(const std::string& password, const std::string& salt) {
    std::string to_hash = salt + password;
    unsigned long hash = 5381;
    for (char c : to_hash) {
        hash = ((hash << 5) + hash) + c; // djb2 algorithm
    }
    return std::to_string(hash);
}

// Simulates a database table.
struct UserCredentials {
    std::string salt;
    std::string hashedPassword;
};
static std::map<std::string, UserCredentials> userDatabase;

/**
 * Performs a constant-time comparison of two strings to prevent timing attacks.
 * It compares all characters and returns true only if they are all identical.
 * The function's execution time does not depend on where the first difference occurs.
 * @param a First string.
 * @param b Second string.
 * @return True if strings are equal, false otherwise.
 */
bool constant_time_compare(const std::string& a, const std::string& b) {
    if (a.length() != b.length()) {
        return false;
    }

    int result = 0;
    for (size_t i = 0; i < a.length(); ++i) {
        result |= a[i] ^ b[i];
    }

    return result == 0;
}


/**
 * Securely checks a user's credentials.
 * @param username The username to check.
 * @param password The password to check.
 * @return A string indicating the result: "success", "cant_login", or "internal_server_error".
 */
std::string login(const std::string& username, const std::string& password) {
    try {
        if (username.empty() || password.empty()) {
            return "cant_login";
        }

        auto it = userDatabase.find(username);
        if (it == userDatabase.end()) {
            // To prevent timing attacks, perform a dummy hash and comparison
            // even if the user does not exist.
            std::string dummy_salt = "dummy_salt_value";
            std::string dummy_hash = insecure_placeholder_hash("some_dummy_password", dummy_salt);
            constant_time_compare(dummy_hash, "a");
            return "cant_login";
        }

        const UserCredentials& credentials = it->second;
        std::string attemptedHash = insecure_placeholder_hash(password, credentials.salt);

        if (constant_time_compare(attemptedHash, credentials.hashedPassword)) {
            return "success";
        } else {
            return "cant_login";
        }

    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return "internal_server_error";
    }
}

void setupTestDatabase() {
    std::string testUser = "alice";
    std::string testPass = "password123";
    // In a real app, use a cryptographically secure random number generator for the salt.
    std::string salt = "random_salt_for_alice_123"; 
    
    std::string hashedPassword = insecure_placeholder_hash(testPass, salt);
    
    userDatabase[testUser] = {salt, hashedPassword};

    std::cout << "Test database setup for user: " << testUser << std::endl;
    std::cout << "Salt: " << salt << std::endl;
    std::cout << "Hashed Password: " << hashedPassword << std::endl;
    std::cout << "------------------------------------" << std::endl;
}


int main() {
    setupTestDatabase();

    // Test Cases
    std::cout << "Test Case 1: Correct credentials" << std::endl;
    std::cout << "Login for 'alice' with 'password123': " << login("alice", "password123") << std::endl << std::endl;

    std::cout << "Test Case 2: Correct username, incorrect password" << std::endl;
    std::cout << "Login for 'alice' with 'wrongpassword': " << login("alice", "wrongpassword") << std::endl << std::endl;
    
    std::cout << "Test Case 3: Incorrect username" << std::endl;
    std::cout << "Login for 'bob' with 'password123': " << login("bob", "password123") << std::endl << std::endl;

    std::cout << "Test Case 4: Empty password" << std::endl;
    std::cout << "Login for 'alice' with '': " << login("alice", "") << std::endl << std::endl;

    std::cout << "Test Case 5: Non-existent user" << std::endl;
    std::cout << "Login for 'charlie' with 'somepass': " << login("charlie", "somepass") << std::endl << std::endl;

    return 0;
}