#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <cstdio>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

// Constants
const int SALT_SIZE = 16;
const int HASH_SIZE = 32; // Corresponds to SHA-256
const int PBKDF2_ITERATIONS = 250000;
const std::string USER_DB_FILE = "users.db";
const size_t MAX_USERNAME_LEN = 50;
const size_t MAX_PASSWORD_LEN = 100;

// Function to convert binary data to a hex string
std::string to_hex(const std::vector<unsigned char>& data) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned char c : data) {
        ss << std::setw(2) << static_cast<int>(c);
    }
    return ss.str();
}

// Function to convert a hex string to binary data
std::vector<unsigned char> from_hex(const std::string& hex) {
    if (hex.length() % 2 != 0) {
        throw std::invalid_argument("Hex string must have an even number of digits.");
    }
    std::vector<unsigned char> bytes;
    bytes.reserve(hex.length() / 2);
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        unsigned char byte = static_cast<unsigned char>(std::stoul(byteString, nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}

// Generates a cryptographically secure random salt
bool generate_salt(std::vector<unsigned char>& salt) {
    salt.resize(SALT_SIZE);
    return RAND_bytes(salt.data(), SALT_SIZE) == 1;
}

// Hashes a password with a given salt using PBKDF2-HMAC-SHA256
bool hash_password(const std::string& password, const std::vector<unsigned char>& salt, std::vector<unsigned char>& hash) {
    hash.resize(HASH_SIZE);
    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(),
        password.length(),
        salt.data(),
        salt.size(),
        PBKDF2_ITERATIONS,
        EVP_sha256(),
        hash.size(),
        hash.data()
    );
    return result == 1;
}

// Checks if a user exists in the database
bool user_exists(const std::string& username) {
    std::ifstream user_file(USER_DB_FILE);
    if (!user_file.is_open()) {
        return false; // File doesn't exist, so user doesn't
    }
    std::string line;
    while (std::getline(user_file, line)) {
        std::size_t pos = line.find(':');
        if (pos != std::string::npos) {
            if (line.substr(0, pos) == username) {
                return true;
            }
        }
    }
    return false;
}

// Registers a new user
bool register_user(const std::string& username, std::string& password) {
    if (username.empty() || username.length() > MAX_USERNAME_LEN || password.empty() || password.length() > MAX_PASSWORD_LEN) {
        std::cerr << "Error: Invalid username or password length." << std::endl;
        return false;
    }
    if (username.find(':') != std::string::npos) {
        std::cerr << "Error: Username cannot contain ':'." << std::endl;
        return false;
    }

    if (user_exists(username)) {
        std::cerr << "Error: User '" << username << "' already exists." << std::endl;
        return false;
    }

    std::vector<unsigned char> salt;
    if (!generate_salt(salt)) {
        std::cerr << "Error: Failed to generate salt." << std::endl;
        return false;
    }

    std::vector<unsigned char> hash;
    if (!hash_password(password, salt, hash)) {
        std::cerr << "Error: Failed to hash password." << std::endl;
        OPENSSL_cleanse(&password[0], password.size());
        return false;
    }
    
    OPENSSL_cleanse(&password[0], password.size());

    std::ofstream user_file(USER_DB_FILE, std::ios_base::app);
    if (!user_file.is_open()) {
        std::cerr << "Error: Could not open user database for writing." << std::endl;
        return false;
    }

    user_file << username << ":" << to_hex(salt) << ":" << to_hex(hash) << std::endl;
    return true;
}

// Authenticates a user
bool authenticate_user(const std::string& username, std::string& password) {
    if (username.empty() || username.length() > MAX_USERNAME_LEN || password.empty() || password.length() > MAX_PASSWORD_LEN) {
        if (!password.empty()) OPENSSL_cleanse(&password[0], password.size());
        return false;
    }
    
    std::ifstream user_file(USER_DB_FILE);
    if (!user_file.is_open()) {
        OPENSSL_cleanse(&password[0], password.size());
        return false;
    }

    std::string line;
    bool user_found = false;
    bool authenticated = false;

    while (std::getline(user_file, line)) {
        std::stringstream ss(line);
        std::string stored_username, salt_hex, hash_hex;

        if (std::getline(ss, stored_username, ':') &&
            std::getline(ss, salt_hex, ':') &&
            std::getline(ss, hash_hex)) {
            
            if (stored_username == username) {
                user_found = true;
                try {
                    std::vector<unsigned char> salt = from_hex(salt_hex);
                    std::vector<unsigned char> stored_hash = from_hex(hash_hex);
                    std::vector<unsigned char> computed_hash;

                    if (!hash_password(password, salt, computed_hash)) {
                        break; 
                    }

                    if (stored_hash.size() == computed_hash.size() &&
                        CRYPTO_memcmp(stored_hash.data(), computed_hash.data(), stored_hash.size()) == 0) {
                        authenticated = true;
                    }

                } catch (const std::invalid_argument&) {
                    std::cerr << "Warning: Corrupted data for user '" << username << "'." << std::endl;
                }
                break;
            }
        }
    }
    
    OPENSSL_cleanse(&password[0], password.size());
    return user_found && authenticated;
}


void run_tests() {
    std::remove(USER_DB_FILE.c_str());

    std::cout << "--- Running C++ Authentication Tests ---" << std::endl;

    std::cout << "Test 1: Successful registration... ";
    std::string pass1 = "StrongP@ssw0rd123";
    if (register_user("testuser1", pass1)) std::cout << "PASSED" << std::endl;
    else std::cout << "FAILED" << std::endl;

    std::cout << "Test 2: Successful login... ";
    std::string pass2 = "StrongP@ssw0rd123";
    if (authenticate_user("testuser1", pass2)) std::cout << "PASSED" << std::endl;
    else std::cout << "FAILED" << std::endl;

    std::cout << "Test 3: Failed login (wrong password)... ";
    std::string pass3 = "WrongPassword";
    if (!authenticate_user("testuser1", pass3)) std::cout << "PASSED" << std::endl;
    else std::cout << "FAILED" << std::endl;

    std::cout << "Test 4: Failed login (user not found)... ";
    std::string pass4 = "any_password";
    if (!authenticate_user("nonexistent_user", pass4)) std::cout << "PASSED" << std::endl;
    else std::cout << "FAILED" << std::endl;

    std::cout << "Test 5: Failed registration (user already exists)... ";
    std::string pass5 = "AnotherPassword";
    if (!register_user("testuser1", pass5)) std::cout << "PASSED" << std::endl;
    else std::cout << "FAILED" << std::endl;
    
    std::remove(USER_DB_FILE.c_str());
    std::cout << "--- C++ Tests Finished ---" << std::endl << std::endl;
}

int main() {
    run_tests();
    return 0;
}