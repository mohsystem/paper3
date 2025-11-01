/*
 * IMPORTANT: This code requires the OpenSSL library.
 * To compile, you need to link against libssl and libcrypto.
 * Example compilation command:
 * g++ -std=c++17 your_file_name.cpp -o your_executable -lssl -lcrypto
 */
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <iomanip>
#include <sstream>

// Constants for hashing
const int SALT_LENGTH = 16;
const int ITERATION_COUNT = 65536;
const int KEY_LENGTH = 32; // 32 bytes for SHA256

// Struct to hold user credentials
struct UserCredentials {
    std::vector<unsigned char> salt;
    std::vector<unsigned char> hashedPassword;
};

// In-memory map to simulate a database
static std::unordered_map<std::string, UserCredentials> userDatabase;

// Helper function to convert byte vector to hex string for printing
std::string to_hex(const std::vector<unsigned char>& data) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned char c : data) {
        ss << std::setw(2) << static_cast<int>(c);
    }
    return ss.str();
}

/**
 * Generates a random salt.
 * @return A vector of unsigned char containing the salt.
 */
std::vector<unsigned char> generateSalt() {
    std::vector<unsigned char> salt(SALT_LENGTH);
    if (RAND_bytes(salt.data(), SALT_LENGTH) != 1) {
        throw std::runtime_error("Error generating random salt.");
    }
    return salt;
}

/**
 * Hashes the password using PBKDF2.
 * @param password The password to hash.
 * @param salt The salt to use.
 * @return The hashed password as a vector of unsigned char.
 */
std::vector<unsigned char> hashPassword(const std::string& password, const std::vector<unsigned char>& salt) {
    std::vector<unsigned char> hash(KEY_LENGTH);
    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(),
        password.length(),
        salt.data(),
        salt.size(),
        ITERATION_COUNT,
        EVP_sha256(),
        KEY_LENGTH,
        hash.data()
    );

    if (result != 1) {
        throw std::runtime_error("Error hashing password with PBKDF2.");
    }
    return hash;
}

/**
 * Registers a new user.
 * @param username The username.
 * @param password The plaintext password.
 * @return true if registration is successful, false if user already exists.
 */
bool registerUser(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        std::cout << "Username and password cannot be empty." << std::endl;
        return false;
    }
    if (userDatabase.find(username) != userDatabase.end()) {
        return false; // User already exists
    }

    auto salt = generateSalt();
    auto hashedPassword = hashPassword(password, salt);

    userDatabase[username] = {salt, hashedPassword};
    return true;
}

/**
 * Verifies a user's password.
 * @param username The username.
 * @param password The plaintext password to check.
 * @return true if the password is correct, false otherwise.
 */
bool verifyPassword(const std::string& username, const std::string& password) {
    auto it = userDatabase.find(username);
    if (it == userDatabase.end()) {
        return false; // User not found
    }

    const auto& credentials = it->second;
    auto providedPasswordHash = hashPassword(password, credentials.salt);

    // Constant-time comparison to prevent timing attacks
    return CRYPTO_memcmp(providedPasswordHash.data(), credentials.hashedPassword.data(), KEY_LENGTH) == 0;
}

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;

    // Test Case 1: Successful registration
    bool reg1 = registerUser("alice", "Password123!");
    std::cout << "Test 1 - Alice registration: " << (reg1 ? "Success" : "Fail") << std::endl;

    // Test Case 2: Successful login
    bool login1 = verifyPassword("alice", "Password123!");
    std::cout << "Test 2 - Alice successful login: " << (login1 ? "Success" : "Fail") << std::endl;

    // Test Case 3: Failed login (wrong password)
    bool login2 = verifyPassword("alice", "WrongPassword");
    std::cout << "Test 3 - Alice failed login (wrong password): " << (!login2 ? "Success" : "Fail") << std::endl;

    // Test Case 4: Failed login (non-existent user)
    bool login3 = verifyPassword("bob", "Password123!");
    std::cout << "Test 4 - Bob failed login (user not found): " << (!login3 ? "Success" : "Fail") << std::endl;

    // Test Case 5: Failed registration (duplicate username)
    bool reg2 = registerUser("alice", "AnotherPassword");
    std::cout << "Test 5 - Alice duplicate registration: " << (!reg2 ? "Success" : "Fail") << std::endl;

    // Optional: Print stored data for verification
    auto it = userDatabase.find("alice");
    if (it != userDatabase.end()) {
        std::cout << "\nStored data for Alice:" << std::endl;
        std::cout << "Salt: " << to_hex(it->second.salt) << std::endl;
        std::cout << "Hashed Password: " << to_hex(it->second.hashedPassword) << std::endl;
    }

    return 0;
}