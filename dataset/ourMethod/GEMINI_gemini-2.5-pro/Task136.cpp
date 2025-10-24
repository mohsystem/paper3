/*
 * Before compiling, ensure you have OpenSSL development libraries installed.
 * For example, on Debian/Ubuntu: sudo apt-get install libssl-dev
 *
 * Compile using g++:
 * g++ your_file_name.cpp -o auth_service -lssl -lcrypto
 */
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <string_view>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

// --- Constants ---
constexpr int SALT_LENGTH = 16;
constexpr int KEY_LENGTH = 32; // 256 bits
constexpr int ITERATIONS = 400000;
constexpr int MIN_PASSWORD_LENGTH = 8;

struct UserCredentials {
    std::vector<unsigned char> salt;
    std::vector<unsigned char> hash;
};

class AuthService {
private:
    std::map<std::string, UserCredentials> userDatabase;

    std::vector<unsigned char> hashPassword(std::string_view password, const std::vector<unsigned char>& salt) {
        std::vector<unsigned char> hash(KEY_LENGTH);
        int result = PKCS5_PBKDF2_HMAC(
            password.data(),
            password.length(),
            salt.data(),
            salt.size(),
            ITERATIONS,
            EVP_sha256(),
            hash.size(),
            hash.data()
        );
        if (result != 1) {
            throw std::runtime_error("Failed to hash password with PBKDF2.");
        }
        return hash;
    }

public:
    bool registerUser(std::string_view username, std::string_view password) {
        if (username.empty() || password.length() < MIN_PASSWORD_LENGTH) {
            std::cerr << "Error: Invalid username or password does not meet policy." << std::endl;
            return false;
        }
        if (userDatabase.count(std::string(username))) {
            std::cerr << "Error: Username '" << username << "' already exists." << std::endl;
            return false;
        }

        std::vector<unsigned char> salt(SALT_LENGTH);
        if (RAND_bytes(salt.data(), salt.size()) != 1) {
            std::cerr << "Error: Failed to generate random salt." << std::endl;
            return false;
        }

        try {
            std::vector<unsigned char> hash = hashPassword(password, salt);
            userDatabase[std::string(username)] = {salt, hash};
            return true;
        } catch (const std::runtime_error& e) {
            std::cerr << "Error during user registration: " << e.what() << std::endl;
            return false;
        }
    }

    bool authenticateUser(std::string_view username, std::string_view password) {
        if (username.empty() || password.empty()) {
            return false;
        }
        
        auto it = userDatabase.find(std::string(username));
        if (it == userDatabase.end()) {
            return false; // User not found
        }

        const UserCredentials& credentials = it->second;

        try {
            std::vector<unsigned char> calculatedHash = hashPassword(password, credentials.salt);
            // Constant-time comparison
            return CRYPTO_memcmp(credentials.hash.data(), calculatedHash.data(), KEY_LENGTH) == 0;
        } catch (const std::runtime_error& e) {
            std::cerr << "Error during user authentication: " << e.what() << std::endl;
            return false;
        }
    }
};

int main() {
    AuthService authService;

    // Test Case 1: Register a new user
    std::cout << "Test Case 1: Registering user 'alice'." << std::endl;
    bool registered = authService.registerUser("alice", "Password123!");
    std::cout << "Registration successful: " << std::boolalpha << registered << std::endl;
    std::cout << "--------------------" << std::endl;

    // Test Case 2: Authenticate user with correct password
    std::cout << "Test Case 2: Authenticating 'alice' with correct password." << std::endl;
    bool authenticated = authService.authenticateUser("alice", "Password123!");
    std::cout << "Authentication successful: " << std::boolalpha << authenticated << std::endl;
    std::cout << "--------------------" << std::endl;

    // Test Case 3: Authenticate user with incorrect password
    std::cout << "Test Case 3: Authenticating 'alice' with incorrect password." << std::endl;
    authenticated = authService.authenticateUser("alice", "WrongPassword!");
    std::cout << "Authentication successful: " << std::boolalpha << authenticated << std::endl;
    std::cout << "--------------------" << std::endl;

    // Test Case 4: Authenticate a non-existent user
    std::cout << "Test Case 4: Authenticating non-existent user 'bob'." << std::endl;
    authenticated = authService.authenticateUser("bob", "SomePassword");
    std::cout << "Authentication successful: " << std::boolalpha << authenticated << std::endl;
    std::cout << "--------------------" << std::endl;

    // Test Case 5: Register another user and authenticate
    std::cout << "Test Case 5: Registering and authenticating 'charlie'." << std::endl;
    registered = authService.registerUser("charlie", "SecurePass!@#");
    std::cout << "Registration of 'charlie' successful: " << std::boolalpha << registered << std::endl;
    authenticated = authService.authenticateUser("charlie", "SecurePass!@#");
    std::cout << "Authentication of 'charlie' successful: " << std::boolalpha << authenticated << std::endl;
    std::cout << "--------------------" << std::endl;

    // Test Case 6: Attempt to register with a short password
    std::cout << "Test Case 6: Attempt to register with a short password." << std::endl;
    registered = authService.registerUser("david", "short");
    std::cout << "Registration successful: " << std::boolalpha << registered << std::endl;
    std::cout << "--------------------" << std::endl;

    // Test Case 7: Attempt to register an existing user
    std::cout << "Test Case 7: Attempt to register 'alice' again." << std::endl;
    registered = authService.registerUser("alice", "AnotherPassword");
    std::cout << "Registration successful: " << std::boolalpha << registered << std::endl;
    std::cout << "--------------------" << std::endl;
    
    return 0;
}