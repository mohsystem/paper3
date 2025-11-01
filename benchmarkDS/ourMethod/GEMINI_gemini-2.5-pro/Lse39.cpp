#include <iostream>
#include <string>
#include <vector>

// NOTE: Storing credentials directly in the code is highly insecure.
// In a real application, use a secure vault, environment variables,
// and store password hashes (e.g., using Argon2) instead of plaintext.
const std::string CORRECT_USERNAME = "admin";
const std::string CORRECT_PASSWORD = "password123!";
const size_t MAX_INPUT_LENGTH = 128;

/**
 * @brief Performs a constant-time string comparison to mitigate timing attacks.
 * @note This is an illustrative implementation. A production system should use a
 * well-vetted cryptographic library function. This simple version may leak length information.
 * @param user_input The first string.
 * @param stored_secret The second string.
 * @return True if the strings are equal, false otherwise.
 */
bool secure_compare(const std::string& user_input, const std::string& stored_secret) {
    // Length check must be done, but reveals length information.
    // A more robust solution might involve hashing both inputs before comparing.
    if (user_input.length() != stored_secret.length()) {
        return false;
    }

    int diff = 0;
    // Compare character by character without short-circuiting.
    for (size_t i = 0; i < user_input.length(); ++i) {
        diff |= user_input[i] ^ stored_secret[i];
    }

    return diff == 0;
}

/**
 * @brief Validates user credentials in a secure manner.
 * @param username The username provided by the user.
 * @param password The password provided by the user.
 * @return True if login is successful, false otherwise.
 */
bool login(const std::string& username, const std::string& password) {
    // Rule #1: Ensure all input is validated.
    if (username.empty() || username.length() > MAX_INPUT_LENGTH) {
        std::cerr << "Invalid username format." << std::endl;
        return false;
    }
    if (password.empty() || password.length() > MAX_INPUT_LENGTH) {
        std::cerr << "Invalid password format." << std::endl;
        return false;
    }

    // Standard comparison for username (not a secret).
    bool username_matches = (username == CORRECT_USERNAME);
    
    // Rule #7 & #8: Use constant-time comparison for secrets.
    bool password_matches = secure_compare(password, CORRECT_PASSWORD);

    if (username_matches && password_matches) {
        std::cout << "Login successful! Redirecting to session page..." << std::endl;
        return true;
    } else {
        std::cout << "Incorrect username or password." << std::endl;
        return false;
    }
}

int main() {
    std::cout << "--- Test Case 1: Correct credentials ---" << std::endl;
    login("admin", "password123!");

    std::cout << "\n--- Test Case 2: Incorrect password ---" << std::endl;
    login("admin", "wrongpassword");

    std::cout << "\n--- Test Case 3: Incorrect username ---" << std::endl;
    login("user", "password123!");

    std::cout << "\n--- Test Case 4: Empty input ---" << std::endl;
    login("", "");

    std::cout << "\n--- Test Case 5: Long input ---" << std::endl;
    std::string long_string(MAX_INPUT_LENGTH + 1, 'a');
    login(long_string, long_string);

    return 0;
}