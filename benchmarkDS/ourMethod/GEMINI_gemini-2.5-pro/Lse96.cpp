#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <map>
#include <sstream>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/crypto.h> // For CRYPTO_memcmp

// Configuration for PBKDF2
const int ITERATIONS = 310000;
const int SALT_SIZE = 16;
const int KEY_LENGTH = 32; // 256 bits

// A simple in-memory map to simulate a user database
static std::map<std::string, std::string> userDatabase;

// Helper to handle OpenSSL errors
void handle_openssl_error() {
    // In a real application, this should be logged to a secure location
    // ERR_print_errors_fp(stderr);
    throw std::runtime_error("An OpenSSL error occurred. Check logs for details.");
}

// Base64 encode helper using OpenSSL
std::string base64_encode(const std::vector<unsigned char>& data) {
    size_t encoded_len_alloc = 4 * ((data.size() + 2) / 3) + 1;
    std::vector<unsigned char> encoded_data(encoded_len_alloc, 0);
    int encoded_len = EVP_EncodeBlock(encoded_data.data(), data.data(), data.size());
    if (encoded_len <= 0) {
        handle_openssl_error();
    }
    return std::string(reinterpret_cast<char*>(encoded_data.data()), encoded_len);
}

// Base64 decode helper using OpenSSL
std::vector<unsigned char> base64_decode(const std::string& input) {
    std::vector<unsigned char> decoded_data(input.length(), 0); // Max possible size
    int decoded_len = EVP_DecodeBlock(decoded_data.data(), reinterpret_cast<const unsigned char*>(input.c_str()), input.length());
    if (decoded_len < 0) {
        handle_openssl_error();
    }
    decoded_data.resize(decoded_len);
    return decoded_data;
}

std::vector<unsigned char> generate_salt() {
    std::vector<unsigned char> salt(SALT_SIZE);
    if (RAND_bytes(salt.data(), SALT_SIZE) != 1) {
        handle_openssl_error();
    }
    return salt;
}

std::vector<unsigned char> hash_password(const std::string& password, const std::vector<unsigned char>& salt) {
    std::vector<unsigned char> hash(KEY_LENGTH);
    if (PKCS5_PBKDF2_HMAC(
            password.c_str(),
            password.length(),
            salt.data(),
            salt.size(),
            ITERATIONS,
            EVP_sha256(),
            KEY_LENGTH,
            hash.data()) == 0) {
        handle_openssl_error();
    }
    return hash;
}

std::string registerUser(const std::string& username, const std::string& password) {
    try {
        std::vector<unsigned char> salt = generate_salt();
        std::vector<unsigned char> hashedPassword = hash_password(password, salt);

        std::string salt_b64 = base64_encode(salt);
        std::string hash_b64 = base64_encode(hashedPassword);

        std::string storedPassword = salt_b64 + ":" + hash_b64;
        userDatabase[username] = storedPassword;
        return storedPassword;
    } catch (const std::exception& e) {
        std::cerr << "Error during user registration: " << e.what() << std::endl;
        return "";
    }
}

bool verifyPassword(const std::string& username, const std::string& providedPassword) {
    if (userDatabase.find(username) == userDatabase.end()) {
        return false; // User not found
    }

    std::string storedPassword = userDatabase.at(username);
    
    try {
        std::stringstream ss(storedPassword);
        std::string salt_b64, hash_b64;
        if (!std::getline(ss, salt_b64, ':') || !std::getline(ss, hash_b64)) {
            return false; // Invalid format
        }

        std::vector<unsigned char> salt = base64_decode(salt_b64);
        std::vector<unsigned char> originalHash = base64_decode(hash_b64);
        
        std::vector<unsigned char> newHash = hash_password(providedPassword, salt);
        
        if (originalHash.size() != newHash.size()) {
            return false;
        }

        // Use constant-time comparison to prevent timing attacks
        return CRYPTO_memcmp(originalHash.data(), newHash.data(), originalHash.size()) == 0;
    } catch (const std::exception& e) {
        std::cerr << "Error during password verification: " << e.what() << std::endl;
        return false;
    }
}

void run_test_case(const std::string& username, const std::string& password) {
    std::cout << "\n--- Testing for user: " << username << " ---" << std::endl;

    // 1. Registration
    std::string storedPassword = registerUser(username, password);
    std::cout << "Registering user '" << username << "'." << std::endl;
    std::cout << "Stored format (Base64(salt):Base64(hash)): " << storedPassword << std::endl;

    // 2. Verification
    std::cout << "Verifying with correct password..." << std::endl;
    bool isCorrect = verifyPassword(username, password);
    std::cout << "Verification successful: " << std::boolalpha << isCorrect << std::endl;

    std::cout << "Verifying with incorrect password ('wrongpassword')..." << std::endl;
    bool isIncorrect = verifyPassword(username, "wrongpassword");
    std::cout << "Verification successful: " << std::boolalpha << isIncorrect << std::endl;
}

int main() {
    std::cout << "--- User Registration and Verification Simulation ---" << std::endl;
    std::cout << "Note: This program must be compiled and linked with OpenSSL." << std::endl;
    std::cout << "Example: g++ -std=c++11 your_file.cpp -o your_program -lssl -lcrypto" << std::endl;
    
    run_test_case("alice", "Password123!");
    run_test_case("bob", "my$ecretP@ss");
    run_test_case("charlie", "aVeryLongAndComplexPassword-12345");
    run_test_case("dave", "short");
    run_test_case("eve", "another_password");
    
    return 0;
}