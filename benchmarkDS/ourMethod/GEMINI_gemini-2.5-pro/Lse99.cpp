/*
 * COMPILE with: g++ -o your_program_name your_source_file.cpp -lssl -lcrypto
 */
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <map>
#include <openssl/evp.h>
#include <openssl/rand.h>

// In a real application, this would be a database.
struct UserRecord {
    std::string salt_hex;
    std::string hash_hex;
};
std::map<std::string, UserRecord> userDatabase;

// Constants for PBKDF2
const int SALT_LENGTH = 16;
const int KEY_LENGTH = 32; // 256 bits for SHA256
const int ITERATION_COUNT = 310000;

// Helper function to convert binary data to a hex string
std::string to_hex(const std::vector<unsigned char>& data) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned char byte : data) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

// Hashes a password using PBKDF2-HMAC-SHA256
bool hashPassword(const std::string& password, std::vector<unsigned char>& salt, std::vector<unsigned char>& hash) {
    salt.resize(SALT_LENGTH);
    if (RAND_bytes(salt.data(), SALT_LENGTH) != 1) {
        std::cerr << "Error: Failed to generate random salt." << std::endl;
        return false;
    }

    hash.resize(KEY_LENGTH);
    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(),
        password.length(),
        salt.data(),
        salt.size(),
        ITERATION_COUNT,
        EVP_sha256(),
        hash.size(),
        hash.data()
    );

    return result == 1;
}

// Simulates inserting user credentials into a database
void insertIntoDatabase(const std::string& username, const std::string& salt_hex, const std::string& hash_hex) {
    std::cout << "Simulating database insert for user: " << username << std::endl;
    userDatabase[username] = {salt_hex, hash_hex};
    std::cout << " -> Stored Salt: " << salt_hex << std::endl;
    std::cout << " -> Stored Hash: " << hash_hex << std::endl;
    std::cout << "User '" << username << "' successfully registered." << std::endl;
}

// Registers a new user
bool registerUser(const std::string& username, std::string& password) {
    if (username.empty() || username.find_first_not_of(' ') == std::string::npos) {
        std::cerr << "Error: Username cannot be empty." << std::endl;
        return false;
    }
    if (password.length() < 8) {
        std::cerr << "Error: Password must be at least 8 characters long." << std::endl;
        // Securely clear the password from memory
        std::fill(password.begin(), password.end(), '\0');
        return false;
    }
    if (userDatabase.count(username)) {
        std::cerr << "Error: Username '" << username << "' already exists." << std::endl;
        // Securely clear the password from memory
        std::fill(password.begin(), password.end(), '\0');
        return false;
    }

    std::vector<unsigned char> salt, hash;
    if (!hashPassword(password, salt, hash)) {
        std::cerr << "Error: Password hashing failed." << std::endl;
        // Securely clear the password from memory
        std::fill(password.begin(), password.end(), '\0');
        return false;
    }
    
    // Securely clear the password from memory as soon as it's no longer needed
    std::fill(password.begin(), password.end(), '\0');
    
    std::string salt_hex = to_hex(salt);
    std::string hash_hex = to_hex(hash);

    insertIntoDatabase(username, salt_hex, hash_hex);
    return true;
}

int main() {
    std::cout << "--- Running 5 Test Cases ---" << std::endl;
    std::string password;

    std::cout << "\n--- Test Case 1: Valid user ---" << std::endl;
    password = "Password123!";
    registerUser("alice", password);

    std::cout << "\n--- Test Case 2: Another valid user ---" << std::endl;
    password = "SecurePass!@#$";
    registerUser("bob", password);

    std::cout << "\n--- Test Case 3: User with short password ---" << std::endl;
    password = "short";
    registerUser("charlie", password);

    std::cout << "\n--- Test Case 4: Duplicate username ---" << std::endl;
    password = "AnotherPassword456";
    registerUser("alice", password);
    
    std::cout << "\n--- Test Case 5: User with empty username ---" << std::endl;
    password = "ValidPassword123";
    registerUser(" ", password);

    std::cout << "\n--- Test Cases Finished ---" << std::endl;
    return 0;
}