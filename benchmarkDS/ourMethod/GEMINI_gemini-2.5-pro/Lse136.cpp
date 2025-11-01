#include <iostream>
#include <string>
#include <vector>
#include <cstdlib> // For getenv, setenv
#include <cstring> // For strcmp
#include <iomanip>
#include <sstream>

// This code requires linking with OpenSSL: g++ your_file.cpp -o your_app -lssl -lcrypto
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/crypto.h>

const int MIN_USERNAME_LENGTH = 4;
const int MAX_USERNAME_LENGTH = 64;
const int MIN_PASSWORD_LENGTH = 8;
const int MAX_PASSWORD_LENGTH = 128;

const int ITERATIONS = 600000;
const int KEY_LENGTH = 32; // 256 bits
const int SALT_LENGTH = 16; // 128 bits

// Helper to convert byte array to hex string
std::string bytes_to_hex(const unsigned char* data, size_t len) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) {
        ss << std::setw(2) << static_cast<unsigned>(data[i]);
    }
    return ss.str();
}

// Helper to convert hex string to byte vector
std::vector<unsigned char> hex_to_bytes(const std::string& hex) {
    std::vector<unsigned char> bytes;
    if (hex.length() % 2 != 0) return bytes; // Invalid hex string
    for (unsigned int i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        unsigned char byte = static_cast<unsigned char>(strtol(byteString.c_str(), NULL, 16));
        bytes.push_back(byte);
    }
    return bytes;
}

// Rule #9: Use a cryptographically secure random number generator.
std::vector<unsigned char> generate_salt() {
    std::vector<unsigned char> salt(SALT_LENGTH);
    if (RAND_bytes(salt.data(), SALT_LENGTH) != 1) {
        // Handle error: OpenSSL's CSPRNG failed
        throw std::runtime_error("Failed to generate random salt.");
    }
    return salt;
}

// Rule #5, #8: Use strong hashing with a unique salt per password.
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
        throw std::runtime_error("Failed to hash password.");
    }
    return hash;
}

/**
 * Checks if the username and password are correct.
 * @param username The username from the URL.
 * @param password The password from the URL.
 * @return "true" if credentials are valid, "false" otherwise.
 */
std::string check_mod(const std::string& username, const std::string& password) {
    // Rule #1: Ensure all input is validated.
    if (username.length() < MIN_USERNAME_LENGTH || username.length() > MAX_USERNAME_LENGTH) {
        return "false";
    }
    if (password.length() < MIN_PASSWORD_LENGTH || password.length() > MAX_PASSWORD_LENGTH) {
        return "false";
    }

    // Rule #7: Avoid hardcoding credentials. Fetch from a secure source.
    const char* stored_username_cstr = std::getenv("CORRECT_USERNAME");
    const char* stored_hash_hex_cstr = std::getenv("PASSWORD_HASH");
    const char* stored_salt_hex_cstr = std::getenv("PASSWORD_SALT");

    if (!stored_username_cstr || !stored_hash_hex_cstr || !stored_salt_hex_cstr) {
        return "false"; // Configuration error
    }

    if (username != stored_username_cstr) {
        return "false";
    }

    try {
        std::vector<unsigned char> stored_salt = hex_to_bytes(stored_salt_hex_cstr);
        std::vector<unsigned char> stored_hash = hex_to_bytes(stored_hash_hex_cstr);
        
        if(stored_salt.empty() || stored_hash.empty()) return "false";

        std::vector<unsigned char> generated_hash = hash_password(password, stored_salt);

        // Rule #10: Use constant-time comparison to prevent timing attacks.
        if (stored_hash.size() == generated_hash.size() &&
            CRYPTO_memcmp(stored_hash.data(), generated_hash.data(), stored_hash.size()) == 0) {
            return "true";
        }
    } catch (const std::exception& e) {
        // Log error in a real application
        std::cerr << "Authentication error: " << e.what() << std::endl;
        return "false";
    }

    return "false";
}

int main() {
    std::cout << "--- Running 5 Test Cases ---" << std::endl;

    // --- Test Setup ---
    try {
        std::string correct_username = "testuser";
        std::string correct_password = "S3cur3P@ssw0rd!";

        std::vector<unsigned char> salt = generate_salt();
        std::vector<unsigned char> hash = hash_password(correct_password, salt);

        std::string salt_hex = bytes_to_hex(salt.data(), salt.size());
        std::string hash_hex = bytes_to_hex(hash.data(), hash.size());
        
        // Store credentials in environment variables for the test
        setenv("CORRECT_USERNAME", correct_username.c_str(), 1);
        setenv("PASSWORD_SALT", salt_hex.c_str(), 1);
        setenv("PASSWORD_HASH", hash_hex.c_str(), 1);

        // --- Test Cases ---
        // Test Case 1: Correct credentials
        std::string result1 = check_mod(correct_username, correct_password);
        std::cout << "Test 1 (Correct credentials): " << result1 << " (Expected: true)" << std::endl;
        
        // Test Case 2: Incorrect password
        std::string result2 = check_mod(correct_username, "wrongpassword");
        std::cout << "Test 2 (Incorrect password): " << result2 << " (Expected: false)" << std::endl;

        // Test Case 3: Incorrect username
        std::string result3 = check_mod("wronguser", correct_password);
        std::cout << "Test 3 (Incorrect username): " << result3 << " (Expected: false)" << std::endl;

        // Test Case 4: Empty password
        std::string result4 = check_mod(correct_username, "");
        std::cout << "Test 4 (Empty password): " << result4 << " (Expected: false)" << std::endl;
        
        // Test Case 5: Password too short
        std::string result5 = check_mod(correct_username, "short");
        std::cout << "Test 5 (Short password): " << result5 << " (Expected: false)" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "An error occurred during test setup: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}