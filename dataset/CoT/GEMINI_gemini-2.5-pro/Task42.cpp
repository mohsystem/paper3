/*
 * IMPORTANT: This code requires the OpenSSL library.
 * To compile, you need to link against it. For example:
 * g++ Task42.cpp -o task42 -lssl -lcrypto
 */
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h> // For CRYPTO_memcmp

const int SALT_LENGTH = 16;
const int HASH_LENGTH = 32; // SHA-256 produces a 32-byte hash

struct UserCredentials {
    std::vector<unsigned char> salt;
    std::vector<unsigned char> hashedPassword;
};

// In-memory storage. In a real application, use a database.
std::map<std::string, UserCredentials> userStore;

// Hashes the password with a given salt using SHA-256.
std::vector<unsigned char> hashPassword(const std::string& password, const std::vector<unsigned char>& salt) {
    std::vector<unsigned char> hash(HASH_LENGTH);
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    const EVP_MD* md = EVP_sha256();

    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, salt.data(), salt.size());
    EVP_DigestUpdate(mdctx, password.c_str(), password.length());
    unsigned int hash_len;
    EVP_DigestFinal_ex(mdctx, hash.data(), &hash_len);
    EVP_MD_CTX_free(mdctx);
    
    hash.resize(hash_len);
    return hash;
}

// Registers a new user.
bool registerUser(const std::string& username, const std::string& password) {
    if (userStore.count(username)) {
        return false; // User already exists
    }

    std::vector<unsigned char> salt(SALT_LENGTH);
    if (RAND_bytes(salt.data(), SALT_LENGTH) != 1) {
        // Error generating random bytes
        return false;
    }

    UserCredentials creds;
    creds.salt = salt;
    creds.hashedPassword = hashPassword(password, salt);
    userStore[username] = creds;
    
    return true;
}

// Authenticates a user.
bool loginUser(const std::string& username, const std::string& password) {
    if (!userStore.count(username)) {
        return false; // User not found
    }

    const UserCredentials& creds = userStore.at(username);
    std::vector<unsigned char> providedPasswordHash = hashPassword(password, creds.salt);

    // Use constant-time comparison to prevent timing attacks
    return (creds.hashedPassword.size() == providedPasswordHash.size()) &&
           (CRYPTO_memcmp(creds.hashedPassword.data(), providedPasswordHash.data(), creds.hashedPassword.size()) == 0);
}

int main() {
    std::cout << "--- C++ User Authentication System Test ---" << std::endl;

    // Test Case 1: Successful registration
    std::cout << "Test 1: Registering a new user 'alice'." << std::endl;
    bool regSuccess = registerUser("alice", "Password123!");
    std::cout << "Registration successful: " << std::boolalpha << regSuccess << std::endl << std::endl;

    // Test Case 2: Successful login
    std::cout << "Test 2: 'alice' attempts to log in with correct password." << std::endl;
    bool loginSuccess = loginUser("alice", "Password123!");
    std::cout << "Login successful: " << std::boolalpha << loginSuccess << std::endl << std::endl;

    // Test Case 3: Failed login (wrong password)
    std::cout << "Test 3: 'alice' attempts to log in with incorrect password." << std::endl;
    bool loginFailWrongPass = loginUser("alice", "WrongPassword!");
    std::cout << "Login successful: " << std::boolalpha << loginFailWrongPass << std::endl << std::endl;

    // Test Case 4: Failed login (non-existent user)
    std::cout << "Test 4: Non-existent user 'bob' attempts to log in." << std::endl;
    bool loginFailNoUser = loginUser("bob", "SomePassword");
    std::cout << "Login successful: " << std::boolalpha << loginFailNoUser << std::endl << std::endl;

    // Test Case 5: Failed registration (duplicate username)
    std::cout << "Test 5: Attempting to register 'alice' again." << std::endl;
    bool regFailDuplicate = registerUser("alice", "AnotherPassword");
    std::cout << "Registration successful: " << std::boolalpha << regFailDuplicate << std::endl << std::endl;

    return 0;
}