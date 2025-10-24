/*
 * IMPORTANT: This code requires the OpenSSL library for secure password hashing.
 * The C++ standard library does not provide the necessary cryptographic functions.
 *
 * To compile this code, you need to have OpenSSL installed.
 *
 * Compilation command (on Linux/macOS):
 * g++ -o Task42 Task42.cpp -lssl -lcrypto
 */
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <sstream>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

const int SALT_BYTES = 16;
const int KEY_LENGTH = 32; // 256 bits
const int ITERATIONS = 65536;

struct UserData {
    std::string username;
    std::vector<unsigned char> salt;
    std::vector<unsigned char> hash;
};

class Task42 {
private:
    std::map<std::string, UserData> user_store;

    std::vector<unsigned char> hash_password(const std::string& password, const std::vector<unsigned char>& salt) {
        std::vector<unsigned char> hash(KEY_LENGTH);
        int result = PKCS5_PBKDF2_HMAC(
            password.c_str(), 
            password.length(),
            salt.data(), 
            salt.size(), 
            ITERATIONS, 
            EVP_sha256(), 
            hash.size(), 
            hash.data()
        );

        if (result != 1) {
            throw std::runtime_error("Error hashing password with PBKDF2.");
        }
        return hash;
    }

public:
    bool registerUser(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty()) {
            return false;
        }
        if (user_store.count(username)) {
            return false; // User already exists
        }

        std::vector<unsigned char> salt(SALT_BYTES);
        if (!RAND_bytes(salt.data(), salt.size())) {
            throw std::runtime_error("Error generating random salt.");
        }

        std::vector<unsigned char> hashedPassword = hash_password(password, salt);

        user_store[username] = {username, salt, hashedPassword};
        return true;
    }

    bool loginUser(const std::string& username, const std::string& password) {
        if (user_store.find(username) == user_store.end()) {
            return false; // User not found
        }

        const auto& userData = user_store.at(username);
        const auto& stored_salt = userData.salt;
        const auto& stored_hash = userData.hash;

        std::vector<unsigned char> attempted_hash = hash_password(password, stored_salt);

        // Constant-time comparison to prevent timing attacks
        return (stored_hash.size() == attempted_hash.size()) &&
               (CRYPTO_memcmp(stored_hash.data(), attempted_hash.data(), stored_hash.size()) == 0);
    }
};

int main() {
    Task42 authSystem;

    std::cout << "--- User Authentication System Test ---" << std::endl;

    // Test Case 1: Register a new user
    std::cout << "\nTest Case 1: Register a new user 'alice'" << std::endl;
    bool registered = authSystem.registerUser("alice", "Password123!");
    std::cout << "Registration successful: " << std::boolalpha << registered << std::endl;

    // Test Case 2: Log in with correct credentials
    std::cout << "\nTest Case 2: Log in as 'alice' with correct password" << std::endl;
    bool loginSuccess = authSystem.loginUser("alice", "Password123!");
    std::cout << "Login successful: " << std::boolalpha << loginSuccess << std::endl;

    // Test Case 3: Log in with incorrect credentials
    std::cout << "\nTest Case 3: Log in as 'alice' with incorrect password" << std::endl;
    bool loginFail = authSystem.loginUser("alice", "WrongPassword");
    std::cout << "Login successful: " << std::boolalpha << loginFail << std::endl;

    // Test Case 4: Attempt to register an existing user
    std::cout << "\nTest Case 4: Attempt to register existing user 'alice'" << std::endl;
    bool reRegistered = authSystem.registerUser("alice", "AnotherPassword");
    std::cout << "Registration successful: " << std::boolalpha << reRegistered << std::endl;

    // Test Case 5: Attempt to log in with a non-existent user
    std::cout << "\nTest Case 5: Attempt to log in as non-existent user 'bob'" << std::endl;
    bool nonExistentLogin = authSystem.loginUser("bob", "SomePassword");
    std::cout << "Login successful: " << std::boolalpha << nonExistentLogin << std::endl;

    return 0;
}