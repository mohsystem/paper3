#include <iostream>
#include <string>
#include <vector>

// WARNING: Storing credentials in plaintext is highly insecure.
// In a real application, use a secure password hashing mechanism (e.g., Argon2, bcrypt)
// and store the hashes in a secure database. This is for demonstration only.
const std::string CORRECT_USERNAME = "admin";
const std::string CORRECT_PASSWORD = "supersecretpassword123!";

/**
 * Performs a constant-time comparison of two strings to prevent timing attacks.
 * It compares every byte and only reveals the result at the very end.
 *
 * @param a First string.
 * @param b Second string.
 * @return True if strings are equal, false otherwise.
 */
bool secure_compare(const std::string& a, const std::string& b) {
    // Mismatched lengths are an immediate fail, but we must not return early.
    // We XOR the lengths and use this as the initial difference.
    size_t len_a = a.length();
    size_t len_b = b.length();
    unsigned char diff = static_cast<unsigned char>(len_a ^ len_b);

    // We iterate up to the length of the shorter string.
    size_t min_len = (len_a < len_b) ? len_a : len_b;
    for (size_t i = 0; i < min_len; ++i) {
        // Accumulate differences using bitwise OR. This ensures the loop
        // is not short-circuited by the compiler.
        diff |= static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i]);
    }
    
    // The result is true only if the accumulated difference is 0.
    return diff == 0;
}

/**
 * Checks if the provided username and password are correct.
 * This function is designed to be called when a user visits a specific URL.
 * It uses a constant-time comparison to mitigate timing attacks.
 *
 * @param username The username obtained from the URL.
 * @param password The password obtained from the URL.
 * @return The string "true" if the credentials are correct, "false" otherwise.
 */
std::string check_mod(const std::string& username, const std::string& password) {
    // Use the secure comparison for both username and password.
    bool is_username_match = secure_compare(CORRECT_USERNAME, username);
    bool is_password_match = secure_compare(CORRECT_PASSWORD, password);

    // Use a bitwise AND (&) to ensure both comparisons are executed before the check.
    if (is_username_match & is_password_match) {
        return "true";
    } else {
        return "false";
    }
}

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;

    // Test Case 1: Correct credentials
    std::cout << "Test 1 (Correct): " << check_mod("admin", "supersecretpassword123!") << std::endl;

    // Test Case 2: Correct username, incorrect password
    std::cout << "Test 2 (Wrong Pass): " << check_mod("admin", "wrongpassword") << std::endl;

    // Test Case 3: Incorrect username, correct password
    std::cout << "Test 3 (Wrong User): " << check_mod("user", "supersecretpassword123!") << std::endl;

    // Test Case 4: Incorrect credentials
    std::cout << "Test 4 (All Wrong): " << check_mod("user", "wrongpassword") << std::endl;

    // Test Case 5: Empty input
    std::cout << "Test 5 (Empty Input): " << check_mod("", "") << std::endl;

    return 0;
}