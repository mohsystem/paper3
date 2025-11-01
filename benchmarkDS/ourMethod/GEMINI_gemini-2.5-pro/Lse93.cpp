#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <map>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

// To compile: g++ your_file.cpp -o your_app -lssl -lcrypto

// Use a high iteration count for PBKDF2 as recommended by security standards.
const int ITERATIONS = 210000;
const int KEY_LENGTH_BYTES = 32; // 256 bits
const int SALT_LENGTH_BYTES = 16;

// In a real application, this would be a proper database.
// This map simulates a user table.
static std::map<std::string, std::string> userDatabase;

/**
 * Converts a byte array to a hex string.
 */
std::string bytes_to_hex(const unsigned char* bytes, int len) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < len; ++i) {
        ss << std::setw(2) << static_cast<unsigned>(bytes[i]);
    }
    return ss.str();
}

/**
 * Hashes a password using PBKDF2 with HMAC-SHA256.
 * Returns an empty vector on failure.
 */
std::vector<unsigned char> hash_password(const std::string& password, const std::vector<unsigned char>& salt) {
    std::vector<unsigned char> hash(KEY_LENGTH_BYTES);
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
        // Handle error, for example by logging ERR_error_string(ERR_get_error(), NULL)
        return {};
    }
    return hash;
}

/**
 * Generates a cryptographically strong random salt.
 * Returns an empty vector on failure.
 */
std::vector<unsigned char> generate_salt() {
    std::vector<unsigned char> salt(SALT_LENGTH_BYTES);
    if (RAND_bytes(salt.data(), salt.size()) != 1) {
        // Handle error
        return {};
    }
    return salt;
}

/**
 * Simulates inserting user data into a database.
 */
void insertIntoDatabase(const std::string& username, const std::string& storedPassword) {
    userDatabase[username] = storedPassword;
    std::cout << "User '" << username << "' registered successfully." << std::endl;
    // For demonstration, we print the stored value. In production, never log this.
    std::cout << "Stored format (salt:hash): " << storedPassword << std::endl;
    std::cout << "----------------------------------------" << std::endl;
}

/**
 * Registers a new user by hashing their password and storing the result.
 */
void registerUser(const std::string& username, std::string password) {
    if (username.empty() || password.empty()) {
        std::cerr << "Error: Username and password cannot be empty." << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        return;
    }

    std::vector<unsigned char> salt = generate_salt();
    if (salt.empty()) {
        std::cerr << "Error: Failed to generate salt." << std::endl;
        return;
    }

    std::vector<unsigned char> hashedPassword = hash_password(password, salt);
    if (hashedPassword.empty()) {
        std::cerr << "Error: Failed to hash password." << std::endl;
        return;
    }

    // Securely clear the password from memory
    OPENSSL_cleanse(&password[0], password.size());

    // Store salt and hash together. A common format is salt:hash.
    std::string salt_hex = bytes_to_hex(salt.data(), salt.size());
    std::string hash_hex = bytes_to_hex(hashedPassword.data(), hashedPassword.size());
    std::string storedPassword = salt_hex + ":" + hash_hex;

    insertIntoDatabase(username, storedPassword);
}

int main() {
    std::cout << "--- Running User Registration Test Cases ---" << std::endl;

    // Test Case 1
    registerUser("alice", "Password123!");

    // Test Case 2
    registerUser("bob", "S3cureP@ssw0rd");

    // Test Case 3
    registerUser("charlie", "my-weak-password");

    // Test Case 4: Empty password
    registerUser("david", "");

    // Test Case 5: Empty username
    registerUser("", "anotherPassword");

    return 0;
}