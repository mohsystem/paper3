#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

// --- WARNING ---
// This example simulates a login system. It does NOT use a cryptographically
// secure hash function because that would require an external library like OpenSSL
// or Crypto++.
// DO NOT use this hashing function in a real application. It is for
// demonstration purposes only. In a real system, use a library to perform
// strong, salted hashing with an algorithm like Argon2 or BCrypt.

// A simple, non-secure hash function for demonstration.
std::string simple_hash(const std::string& password) {
    unsigned long hash = 5381;
    for (char c : password) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return std::to_string(hash);
}

// Constant-time string comparison to mitigate timing attacks.
// Compares every character regardless of when a mismatch is found.
bool secure_compare(const std::string& s1, const std::string& s2) {
    if (s1.length() != s2.length()) {
        return false;
    }
    int result = 0;
    for (size_t i = 0; i < s1.length(); ++i) {
        result |= (s1[i] ^ s2[i]);
    }
    return result == 0;
}

class Lse34 {
private:
    // Simulates a database table of users.
    std::unordered_map<std::string, std::string> userDatabase;

public:
    Lse34() {
        // Pre-populate the database with some users and their "hashed" passwords.
        userDatabase["alice"] = simple_hash("password123");
        userDatabase["bob"] = simple_hash("adminPass");
        userDatabase["charlie"] = simple_hash("securePwd!");
    }

    /**
     * Attempts to log a user in.
     * @param username The username entered by the user.
     * @param password The password entered by the user.
     * @return A string indicating the result.
     */
    std::string login(const std::string& username, const std::string& password) {
        // 1. Basic input validation
        if (username.empty() || password.empty()) {
            return "Invalid username or password. Redirecting to login page...";
        }

        // 2. Hash the provided password
        std::string providedHashedPassword = simple_hash(password);

        // 3. Find the user in the database
        auto it = userDatabase.find(username);

        // 4. If user does not exist, return a generic error message
        // to prevent username enumeration attacks.
        if (it == userDatabase.end()) {
            // We still compare against a dummy hash to make the timing similar
            // to a failed password attempt, which can help mitigate timing attacks.
            secure_compare(providedHashedPassword, "dummy_hash_for_timing");
            return "Invalid username or password. Redirecting to login page...";
        }

        const std::string& storedHashedPassword = it->second;

        // 5. Compare the hashes in a way that is resistant to timing attacks.
        if (secure_compare(storedHashedPassword, providedHashedPassword)) {
            return "Login successful! Redirecting to dashboard page...";
        } else {
            return "Invalid username or password. Redirecting to login page...";
        }
    }
};

int main() {
    Lse34 authSystem;

    std::cout << "--- 5 Test Cases ---" << std::endl;

    // Test Case 1: Correct username and password
    std::cout << "1. Testing with correct credentials (alice):" << std::endl;
    std::cout << "   Result: " << authSystem.login("alice", "password123") << std::endl << std::endl;

    // Test Case 2: Correct username, incorrect password
    std::cout << "2. Testing with incorrect password (alice):" << std::endl;
    std::cout << "   Result: " << authSystem.login("alice", "wrongpassword") << std::endl << std::endl;

    // Test Case 3: Incorrect username
    std::cout << "3. Testing with non-existent user (david):" << std::endl;
    std::cout << "   Result: " << authSystem.login("david", "somepassword") << std::endl << std::endl;

    // Test Case 4: Empty input for password
    std::cout << "4. Testing with empty password (bob):" << std::endl;
    std::cout << "   Result: " << authSystem.login("bob", "") << std::endl << std::endl;

    // Test Case 5: Correct credentials for another user
    std::cout << "5. Testing with correct credentials (charlie):" << std::endl;
    std::cout << "   Result: " << authSystem.login("charlie", "securePwd!") << std::endl << std::endl;

    return 0;
}