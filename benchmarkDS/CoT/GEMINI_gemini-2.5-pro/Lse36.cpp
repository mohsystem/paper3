#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm> // For std::reverse

// Simulates a database. In a real application, passwords would be hashed
// with a strong algorithm like Argon2 or bcrypt.
const std::unordered_map<std::string, std::string> userDatabase = {
    {"admin", "321drowssap_dehsah"}, // Simulated hash of "password123"
    {"jdoe", "#@!ssaPeruces_dehsah"}, // Simulated hash of "securePass!@#"
    {"testuser", "tset_dehsah"}      // Simulated hash of "test"
};

/**
 * Simulates hashing a password.
 * In a real system, use a dedicated crypto library.
 * This is NOT a secure hash. For demonstration only.
 */
std::string hashPassword(std::string password) {
    std::reverse(password.begin(), password.end());
    return password + "_dehsah";
}

/**
 * Performs a constant-time comparison of two strings to prevent timing attacks.
 * Note: A simple string '==' is often optimized in ways that are not constant time.
 * A real implementation should use a verified crypto library function.
 */
bool secure_equals(const std::string& a, const std::string& b) {
    if (a.length() != b.length()) {
        return false;
    }
    // Use a bitwise XOR to compare characters without early exit.
    int diff = 0;
    for (size_t i = 0; i < a.length(); ++i) {
        diff |= a[i] ^ b[i];
    }
    return diff == 0;
}

/**
 * Simulates a user login process.
 *
 * @param username The username entered by the user.
 * @param password The password entered by the user.
 * @return A string indicating the result of the login attempt.
 */
std::string login(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        return "Username or password cannot be empty.";
    }

    auto it = userDatabase.find(username);
    std::string inputHashedPassword = hashPassword(password);
    bool login_success = false;

    // To prevent timing attacks, we proceed with a comparison even if the user is not found.
    // In that case, we compare against a dummy hash or the input hash itself.
    if (it != userDatabase.end()) {
        // User found, compare with the stored hash
        if (secure_equals(it->second, inputHashedPassword)) {
            login_success = true;
        }
    } else {
        // User not found, perform a dummy comparison to take a similar amount of time.
        secure_equals(inputHashedPassword, "dummy_hash_value_for_timing_mitigation");
    }

    if (login_success) {
        return "Login successful! Redirecting to home page...";
    } else {
        // Generic error message to prevent username enumeration.
        return "Invalid username or password.";
    }
}

int main() {
    std::cout << "--- Login Test Cases ---" << std::endl;

    // Test Case 1: Successful login
    std::cout << "1. admin / password123 -> " << login("admin", "password123") << std::endl;

    // Test Case 2: Incorrect password
    std::cout << "2. admin / wrongpassword -> " << login("admin", "wrongpassword") << std::endl;

    // Test Case 3: Non-existent user
    std::cout << "3. unknownuser / password123 -> " << login("unknownuser", "password123") << std::endl;

    // Test Case 4: Another successful login
    std::cout << "4. jdoe / securePass!@# -> " << login("jdoe", "securePass!@#") << std::endl;

    // Test Case 5: Empty credentials
    std::cout << "5. \"\" / \"\" -> " << login("", "") << std::endl;

    return 0;
}