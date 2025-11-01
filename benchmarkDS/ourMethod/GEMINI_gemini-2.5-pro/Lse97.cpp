#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <regex>
#include <stdexcept>
#include <iomanip>
#include <sstream>

// This code requires OpenSSL.
// Compile with: g++ -std=c++17 your_file.cpp -o your_app -lssl -lcrypto
#include <openssl/rand.h>
#include <openssl/evp.h>

// Using an in-memory map to simulate a database.
// In a real application, this would be a database connection.
struct User {
    std::string salt; // Stored as hex string
    std::string hashedPassword; // Stored as hex string
    std::string firstName;
    std::string lastName;
    std::string email;
};

static std::map<std::string, User> userDatabase;

// Constants for password hashing
const int HASH_ITERATIONS = 210000;
const int SALT_SIZE = 16; // 128 bits
const int HASH_KEY_LENGTH = 32; // 256 bits

// Function to convert byte array to hex string
std::string bytes_to_hex(const unsigned char* bytes, size_t len) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) {
        ss << std::setw(2) << static_cast<unsigned>(bytes[i]);
    }
    return ss.str();
}

// Securely wipe memory
void secure_wipe(void* data, size_t len) {
    // Using volatile pointer to prevent compiler optimization
    volatile unsigned char* p = static_cast<volatile unsigned char*>(data);
    while (len--) {
        *p++ = 0;
    }
}

std::string hash_password(const std::string& password, const std::vector<unsigned char>& salt) {
    std::vector<unsigned char> hash(HASH_KEY_LENGTH);
    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(), 
        password.length(), 
        salt.data(), 
        salt.size(), 
        HASH_ITERATIONS, 
        EVP_sha256(), 
        hash.size(), 
        hash.data()
    );

    if (result != 1) {
        throw std::runtime_error("Password hashing failed.");
    }
    return bytes_to_hex(hash.data(), hash.size());
}

std::string registerUser(std::string username, std::string password, std::string firstName, std::string lastName, std::string email) {
    // Rule #1: Input validation
    const std::regex username_regex("^[a-zA-Z0-9_]{3,20}$");
    if (!std::regex_match(username, username_regex)) {
        return "Registration failed: Invalid username format. Must be 3-20 alphanumeric characters or underscores.";
    }

    const std::regex password_regex("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d)(?=.*[@$!%*?&])[A-Za-z\\d@$!%*?&]{12,}$");
    if (!std::regex_match(password, password_regex)) {
        return "Registration failed: Password does not meet policy. Must be at least 12 characters and contain an uppercase letter, a lowercase letter, a digit, and a special character.";
    }

    const std::regex email_regex("^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\\.[a-zA-Z0-9-.]+$");
    if (firstName.empty() || firstName.length() > 50 || lastName.empty() || lastName.length() > 50 || !std::regex_match(email, email_regex)) {
        return "Registration failed: Invalid name or email format.";
    }

    if (userDatabase.count(username)) {
        return "Registration failed: Username already exists.";
    }
    
    std::vector<unsigned char> salt(SALT_SIZE);
    if (RAND_bytes(salt.data(), salt.size()) != 1) {
        return "Registration failed: Could not generate secure salt.";
    }

    try {
        std::string hashedPassword = hash_password(password, salt);
        // Securely clear the plaintext password
        secure_wipe(&password[0], password.size());

        User newUser;
        newUser.salt = bytes_to_hex(salt.data(), salt.size());
        newUser.hashedPassword = hashedPassword;
        newUser.firstName = firstName;
        newUser.lastName = lastName;
        newUser.email = email;
        
        userDatabase[username] = newUser;
        return "Registration succeeded for username: " + username;
    } catch (const std::exception& e) {
        // Securely clear the plaintext password in case of an error
        secure_wipe(&password[0], password.size());
        return std::string("Registration failed: ") + e.what();
    }
}

int main() {
    std::cout << "--- Running Test Cases ---" << std::endl;

    // Test Case 1: Successful Registration
    std::cout << "Test 1: " << registerUser("john_doe", "StrongPass@123", "John", "Doe", "john.doe@example.com") << std::endl;

    // Test Case 2: Username already exists
    std::cout << "Test 2: " << registerUser("john_doe", "AnotherPass@456", "John", "Doe", "john.d@example.com") << std::endl;

    // Test Case 3: Weak password
    std::cout << "Test 3: " << registerUser("jane_doe", "weak", "Jane", "Doe", "jane.doe@example.com") << std::endl;
    
    // Test Case 4: Invalid username
    std::cout << "Test 4: " << registerUser("jd", "StrongPass@123", "Jane", "Doe", "jane.doe@example.com") << std::endl;

    // Test Case 5: Invalid email
    std::cout << "Test 5: " << registerUser("peter_jones", "StrongPass@123", "Peter", "Jones", "peterjones@invalid") << std::endl;

    std::cout << "\n--- Database Content (for verification) ---" << std::endl;
    for (const auto& pair : userDatabase) {
        std::cout << "Username: " << pair.first << ", Hashed Password: " << pair.second.hashedPassword.substr(0, 10) << "..." << std::endl;
    }

    return 0;
}