#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <regex>
#include <stdexcept>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <cstring> // For memcmp

// To compile: g++ -std=c++17 your_file_name.cpp -o your_program -lssl -lcrypto
// This code requires OpenSSL development libraries to be installed.

const int SALT_SIZE = 16;
const int PBKDF2_ITERATIONS = 260000;
const int HASH_SIZE = 32;

struct User {
    std::string username;
    std::string email;
    std::vector<unsigned char> passwordHash;
    std::vector<unsigned char> salt;
    bool isLoggedIn = false;
};

// In a real application, use a proper database. This is for demonstration.
std::map<std::string, User> userDatabase;

std::vector<unsigned char> generateSalt() {
    std::vector<unsigned char> salt(SALT_SIZE);
    if (RAND_bytes(salt.data(), SALT_SIZE) != 1) {
        throw std::runtime_error("Failed to generate random salt.");
    }
    return salt;
}

std::vector<unsigned char> hashPassword(const std::string& password, const std::vector<unsigned char>& salt) {
    if (password.empty()) {
        throw std::invalid_argument("Password cannot be empty.");
    }
    std::vector<unsigned char> hash(HASH_SIZE);
    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(), password.length(),
        salt.data(), salt.size(),
        PBKDF2_ITERATIONS,
        EVP_sha256(),
        hash.size(),
        hash.data()
    );
    if (result != 1) {
        throw std::runtime_error("Failed to hash password with PBKDF2.");
    }
    return hash;
}

bool verifyPassword(const std::string& password, const User& user) {
    if (password.empty()) {
        return false;
    }
    try {
        std::vector<unsigned char> newHash = hashPassword(password, user.salt);
        // Constant-time comparison
        return CRYPTO_memcmp(newHash.data(), user.passwordHash.data(), HASH_SIZE) == 0;
    } catch (const std::exception&) {
        return false;
    }
}

// Function to create a user for testing purposes
void createUser(const std::string& username, const std::string& email, const std::string& password) {
    User newUser;
    newUser.username = username;
    newUser.email = email;
    newUser.salt = generateSalt();
    newUser.passwordHash = hashPassword(password, newUser.salt);
    newUser.isLoggedIn = false;
    userDatabase[username] = newUser;
}

bool changeEmail(const std::string& username, const std::string& oldEmail, const std::string& newEmail, const std::string& password) {
    // Rule #1: Input Validation
    if (username.empty() || oldEmail.empty() || newEmail.empty() || password.empty()) {
        std::cout << "Error: All fields are required." << std::endl;
        return false;
    }

    // Basic email validation
    const std::regex emailPattern("^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\\.[a-zA-Z0-9-.]+$");
    if (!std::regex_match(newEmail, emailPattern)) {
        std::cout << "Error: New email format is invalid." << std::endl;
        return false;
    }

    auto it = userDatabase.find(username);
    if (it == userDatabase.end()) {
        // Generic error to prevent user enumeration
        std::cout << "Error: Email change failed." << std::endl;
        return false;
    }

    User& user = it->second;

    // Security checks
    if (!user.isLoggedIn) {
        std::cout << "Error: User must be logged in to change email." << std::endl;
        return false;
    }

    if (newEmail == oldEmail) {
        std::cout << "Error: New email must be different from the old email." << std::endl;
        return false;
    }

    if (user.email != oldEmail) {
        std::cout << "Error: Email change failed." << std::endl; // Generic error
        return false;
    }

    if (!verifyPassword(password, user)) {
        std::cout << "Error: Email change failed." << std::endl; // Generic error
        return false;
    }

    // All checks passed, update the email
    user.email = newEmail;
    std::cout << "Email for user '" << username << "' has been successfully updated to " << newEmail << std::endl;
    return true;
}

int main() {
    // Setup: Create a user
    std::string testUsername = "testUser";
    std::string initialEmail = "initial@example.com";
    std::string userPassword = "Password123!";

    try {
        createUser(testUsername, initialEmail, userPassword);
    } catch (const std::exception& e) {
        std::cerr << "Setup failed: " << e.what() << std::endl;
        return 1;
    }
    
    User& testUser = userDatabase.at(testUsername);

    std::cout << "Initial state: User '" << testUser.username << "' with email '" << testUser.email << "' is logged out." << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;

    // --- Test Cases ---
    std::cout << "Running Test Cases...\n" << std::endl;

    // Test Case 1: Successful email change
    std::cout << "--- Test Case 1: Successful Change ---" << std::endl;
    testUser.isLoggedIn = true; // Simulate login
    std::cout << "User is now logged in." << std::endl;
    bool result1 = changeEmail(testUsername, initialEmail, "new.email@example.com", userPassword);
    std::cout << "Result: " << (result1 ? "SUCCESS" : "FAILURE") << std::endl;
    std::cout << "Current email: " << testUser.email << "\n" << std::endl;
    std::string updatedEmail = testUser.email; // Save for next tests

    // Test Case 2: Failure - User not logged in
    std::cout << "--- Test Case 2: Failure (Not Logged In) ---" << std::endl;
    testUser.isLoggedIn = false; // Simulate logout
    std::cout << "User is now logged out." << std::endl;
    bool result2 = changeEmail(testUsername, updatedEmail, "another.email@example.com", userPassword);
    std::cout << "Result: " << (result2 ? "SUCCESS" : "FAILURE") << std::endl;
    std::cout << "Current email: " << testUser.email << "\n" << std::endl;
    testUser.isLoggedIn = true; // Log back in for subsequent tests

    // Test Case 3: Failure - Incorrect password
    std::cout << "--- Test Case 3: Failure (Incorrect Password) ---" << std::endl;
    bool result3 = changeEmail(testUsername, updatedEmail, "another.email@example.com", "WrongPassword!");
    std::cout << "Result: " << (result3 ? "SUCCESS" : "FAILURE") << std::endl;
    std::cout << "Current email: " << testUser.email << "\n" << std::endl;

    // Test Case 4: Failure - Incorrect old email
    std::cout << "--- Test Case 4: Failure (Incorrect Old Email) ---" << std::endl;
    bool result4 = changeEmail(testUsername, "wrong.old.email@example.com", "another.email@example.com", userPassword);
    std::cout << "Result: " << (result4 ? "SUCCESS" : "FAILURE") << std::endl;
    std::cout << "Current email: " << testUser.email << "\n" << std::endl;

    // Test Case 5: Failure - New email is same as old
    std::cout << "--- Test Case 5: Failure (New Email is Same as Old) ---" << std::endl;
    bool result5 = changeEmail(testUsername, updatedEmail, updatedEmail, userPassword);
    std::cout << "Result: " << (result5 ? "SUCCESS" : "FAILURE") << std::endl;
    std::cout << "Current email: " << testUser.email << "\n" << std::endl;

    return 0;
}