#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <random>
#include <algorithm>

// --- SECURITY WARNING ---
// This C++ code is for demonstration purposes ONLY. Standard C++ lacks a built-in,
// secure cryptographic library for password hashing. The "hashing" used here is
// a simple string manipulation and is COMPLETELY INSECURE.
//
// In a real-world application, you MUST use a proper, vetted cryptographic library
// like OpenSSL, Crypto++, or libsodium to implement a strong key derivation
// function such as Argon2, scrypt, or bcrypt. NEVER implement your own crypto
// and never store passwords in plaintext or with weak/fake hashing like this.

// A mock database storing: username -> salt:hashedPassword
static std::unordered_map<std::string, std::string> userDatabase;

/**
 * Generates a pseudo-random salt.
 * WARNING: This is NOT cryptographically secure. For demonstration only.
 */
std::string generate_salt(size_t length = 16) {
    const std::string CHARACTERS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<> distribution(0, CHARACTERS.size() - 1);
    std::string random_string;
    for (std::size_t i = 0; i < length; ++i) {
        random_string += CHARACTERS[distribution(generator)];
    }
    return random_string;
}

/**
 * "Hashes" a password using an INSECURE method for demonstration.
 * WARNING: This is NOT a real hash function. Do not use in production.
 */
std::string hash_password(const std::string& password, const std::string& salt) {
    // This is a placeholder for a real KDF like Argon2.
    // It is insecure and only serves to demonstrate the logic.
    return "hashed(" + password + salt + ")";
}

/**
 * Registers a new user by "hashing" their password and storing it.
 */
void registerUser(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) return;
    std::string salt = generate_salt();
    std::string hashedPassword = hash_password(password, salt);
    userDatabase[username] = salt + ":" + hashedPassword;
}

/**
 * Simulates a login attempt.
 * @param username The username provided by the user.
 * @param password The password provided by the user.
 * @return true if login is successful, false otherwise.
 */
bool login(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        return false;
    }

    auto it = userDatabase.find(username);
    if (it == userDatabase.end()) {
        // User not found. Perform a dummy hash to mitigate timing attacks.
        hash_password("dummy_password", generate_salt());
        return false;
    }

    std::string storedData = it->second;
    size_t delimiter_pos = storedData.find(':');
    if (delimiter_pos == std::string::npos) {
        return false; // Malformed data in DB
    }

    std::string salt = storedData.substr(0, delimiter_pos);
    std::string storedHash = storedData.substr(delimiter_pos + 1);

    // "Hash" the provided password with the stored salt
    std::string providedPasswordHash = hash_password(password, salt);

    // In a real application, this comparison must be constant-time.
    return storedHash == providedPasswordHash;
}

int main() {
    // Setup: Register some users in our mock database
    registerUser("admin", "S3cureP@ssword123!");
    registerUser("user1", "myWeakPassword");

    std::cout << "--- Running 5 Test Cases ---" << std::endl;

    // Test Case 1: Successful login for admin
    bool test1 = login("admin", "S3cureP@ssword123!");
    std::cout << "Test 1 (Correct admin password): " << (test1 ? "SUCCESS -> Redirect to home page" : "FAIL -> Redirect to login page") << std::endl;

    // Test Case 2: Failed login for admin (wrong password)
    bool test2 = login("admin", "wrongpassword");
    std::cout << "Test 2 (Incorrect admin password): " << (test2 ? "SUCCESS -> Redirect to home page" : "FAIL -> Redirect to login page") << std::endl;

    // Test Case 3: Failed login for non-existent user
    bool test3 = login("nonexistentuser", "somepassword");
    std::cout << "Test 3 (Non-existent user): " << (test3 ? "SUCCESS -> Redirect to home page" : "FAIL -> Redirect to login page") << std::endl;

    // Test Case 4: Successful login for another user
    bool test4 = login("user1", "myWeakPassword");
    std::cout << "Test 4 (Correct user1 password): " << (test4 ? "SUCCESS -> Redirect to home page" : "FAIL -> Redirect to login page") << std::endl;

    // Test Case 5: Failed login with empty credentials
    bool test5 = login("", "S3cureP@ssword123!");
    std::cout << "Test 5 (Empty username): " << (test5 ? "SUCCESS -> Redirect to home page" : "FAIL -> Redirect to login page") << std::endl;

    return 0;
}