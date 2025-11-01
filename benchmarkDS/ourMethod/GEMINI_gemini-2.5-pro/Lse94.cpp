#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <iomanip>
#include <sstream>

#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/err.h>

// Configuration constants
const int ITERATIONS = 310000;
const int KEY_LENGTH = 64; // in bytes (512 bits)
const int SALT_SIZE = 16;  // in bytes

// Helper to convert byte array to hex string
std::string bytes_to_hex(const unsigned char* data, size_t len) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) {
        ss << std::setw(2) << static_cast<unsigned int>(data[i]);
    }
    return ss.str();
}

// Hashes a password for a new user registration.
std::string register_user(const std::string& username, const std::string& password) {
    if (password.empty()) {
        throw std::invalid_argument("Password cannot be empty.");
    }
    if (username.empty()) {
        throw std::invalid_argument("Username cannot be empty.");
    }
    
    // 1. Generate a cryptographically secure random salt
    unsigned char salt[SALT_SIZE];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        throw std::runtime_error("Error generating random salt.");
    }

    // 2. Derive the key using PBKDF2
    unsigned char hash[KEY_LENGTH];
    
    std::vector<char> password_vec(password.begin(), password.end());
    
    int result = PKCS5_PBKDF2_HMAC(
        password_vec.data(),
        password_vec.size(),
        salt,
        sizeof(salt),
        ITERATIONS,
        EVP_sha256(),
        sizeof(hash),
        hash
    );

    // Securely clear the password from memory
    OPENSSL_cleanse(password_vec.data(), password_vec.size());
    
    if (result != 1) {
        throw std::runtime_error("Error in PKCS5_PBKDF2_HMAC.");
    }

    // 3. Format the output as "salt:hash" in hex
    return bytes_to_hex(salt, sizeof(salt)) + ":" + bytes_to_hex(hash, sizeof(hash));
}

// A mock database insertion function
void insert_into_database(const std::string& username, const std::string& stored_password) {
    std::cout << "[DB INSERT] Storing credentials for user: " << username << std::endl;
    std::cout << "  - Username: " << username << std::endl;
    std::cout << "  - Stored Password (Salt:Hash): " << stored_password << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
}

int main() {
    std::pair<std::string, std::string> test_users[] = {
        {"admin", "AdminPassword123!"},
        {"user01", "s#cureP@ssw0rd"},
        {"test_user", "password"},
        {"another_user", "12345678"},
        {"charlie.brown", "GoodGrief1!"}
    };
    
    for (const auto& user : test_users) {
        const std::string& username = user.first;
        const std::string& password = user.second;
        try {
            std::cout << "Processing user: " << username << std::endl;
            std::string hashed_password = register_user(username, password);
            insert_into_database(username, hashed_password);
        } catch (const std::exception& e) {
            std::cerr << "Error hashing password for user " << username << ": " << e.what() << std::endl;
            ERR_print_errors_fp(stderr);
        }
    }

    return 0;
}