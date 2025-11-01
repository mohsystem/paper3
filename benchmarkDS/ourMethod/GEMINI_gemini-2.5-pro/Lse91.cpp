// To compile: g++ -std=c++17 your_file_name.cpp -o your_program -lssl -lcrypto
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include <sstream>
#include <iomanip>
#include <openssl/evp.h>
#include <openssl/rand.h>

// In-memory database simulation
static std::unordered_map<std::string, std::string> userDatabase;

// Constants for password hashing
const int SALT_LENGTH = 16;
const int HASH_LENGTH = 32;
// Iteration count should be high. OWASP recommends at least 310,000 for PBKDF2-SHA256.
const int ITERATIONS = 310000;
const EVP_MD* HASH_ALGORITHM = EVP_sha256();

// Regex for basic input validation
const std::regex USERNAME_REGEX("^[a-zA-Z0-9_]{3,20}$");
const std::regex PASSWORD_REGEX("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d)(?=.*[@$!%*?&])[A-Za-z\\d@$!%*?&]{8,100}$");


// Helper function to convert binary data to a hex string
std::string to_hex(const unsigned char* data, size_t len) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) {
        ss << std::setw(2) << static_cast<unsigned>(data[i]);
    }
    return ss.str();
}

std::string hash_password(const std::string& password) {
    // Rule #9 & #10: Generate a unique, unpredictable salt
    unsigned char salt[SALT_LENGTH];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        throw std::runtime_error("Failed to generate random salt.");
    }

    unsigned char hash[HASH_LENGTH];
    
    // Rule #8: Use a strong key derivation function
    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(),
        password.length(),
        salt,
        sizeof(salt),
        ITERATIONS,
        HASH_ALGORITHM,
        sizeof(hash),
        hash
    );

    if (result != 1) {
        throw std::runtime_error("Failed to hash password with PBKDF2.");
    }
    
    // Store parameters with the hash for future verification
    std::stringstream stored_password;
    stored_password << "pbkdf2_sha256:" << ITERATIONS << ":" << to_hex(salt, sizeof(salt)) << ":" << to_hex(hash, sizeof(hash));
    
    return stored_password.str();
}

bool registerUser(const std::string& username, std::string& password) {
    // Rule #1: Input validation
    if (!std::regex_match(username, USERNAME_REGEX)) {
        std::cerr << "Registration failed: Invalid username format." << std::endl;
        return false;
    }
    if (!std::regex_match(password, PASSWORD_REGEX)) {
        std::cerr << "Registration failed: Password does not meet policy requirements." << std::endl;
        return false;
    }
    if (userDatabase.count(username)) {
        std::cerr << "Registration failed: Username '" << username << "' already exists." << std::endl;
        return false;
    }

    try {
        std::string hashed_password = hash_password(password);
        userDatabase[username] = hashed_password;
        std::cout << "User '" << username << "' registered successfully." << std::endl;
        return true;
    } catch (const std::runtime_error& e) {
        std::cerr << "Critical error during password hashing: " << e.what() << std::endl;
        return false;
    } finally {
        // Securely clear password from memory
        // Note: std::string's internal buffer might not be overwritten immediately.
        // For higher security, use a custom allocator or a mutable char array.
        // This is a best-effort approach for std::string.
        password.assign(password.length(), '\0');
    }
}


int main() {
    std::cout << "--- User Registration Simulation ---" << std::endl;

    // Test Case 1: Successful registration
    std::string pass1 = "ValidPass1!";
    registerUser("alice_123", pass1);

    // Test Case 2: Successful registration with another user
    std::string pass2 = "Another_Good_P@ssw0rd";
    registerUser("bob_smith", pass2);

    // Test Case 3: Failed registration - duplicate username
    std::string pass3 = "SomeOtherPass1?";
    registerUser("alice_123", pass3);

    // Test Case 4: Failed registration - invalid username
    std::string pass4 = "ValidPass1!";
    registerUser("!!invalid-user!!", pass4);

    // Test Case 5: Failed registration - weak password
    std::string pass5 = "weak";
    registerUser("charlie", pass5);

    std::cout << "\n--- Simulated User Database ---" << std::endl;
    for (const auto& pair : userDatabase) {
        std::cout << "Username: " << pair.first << std::endl;
        std::cout << "Stored Hash: " << pair.second << std::endl << std::endl;
    }

    return 0;
}