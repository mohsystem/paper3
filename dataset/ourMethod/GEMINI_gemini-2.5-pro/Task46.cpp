// To compile: g++ -Wall -Wextra -pedantic -std=c++17 your_file_name.cpp -o program -lssl -lcrypto
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <regex>
#include <iomanip>
#include <sstream>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdexcept>

// --- Constants ---
const int SALT_SIZE = 16;
const int HASH_SIZE = 32;
const int PBKDF2_ITERATIONS = 210000;
const char* DB_FILE = "users.db";

// --- Helper Functions ---

/**
 * @brief Converts a byte vector to a hexadecimal string.
 * @param bytes The vector of unsigned chars to convert.
 * @return A string representing the bytes in hexadecimal format.
 */
std::string bytes_to_hex(const std::vector<unsigned char>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned char byte : bytes) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

/**
 * @brief Validates an email address format.
 * @param email The email address string to validate.
 * @return True if the email format is valid, false otherwise.
 */
bool validate_email(const std::string& email) {
    // A simple regex for email validation. For production, a more comprehensive one is needed.
    const std::regex email_regex(R"((\w+)(\.{1}\w+)*@(\w+)(\.\w+)+)");
    return std::regex_match(email, email_regex);
}

/**
 * @brief Validates password complexity.
 * @param password The password string to validate.
 * @return True if the password meets complexity requirements, false otherwise.
 * Policy: min 12 chars, 1 uppercase, 1 lowercase, 1 digit, 1 special char.
 */
bool validate_password(const std::string& password) {
    if (password.length() < 12) return false;
    const std::regex complexity_regex(R"((?=.*[a-z])(?=.*[A-Z])(?=.*\d)(?=.*[@$!%*?&]).{12,})");
    return std::regex_match(password, complexity_regex);
}


/**
 * @brief Hashes a password using PBKDF2-HMAC-SHA256.
 * @param password The plaintext password.
 * @param salt The salt to use for hashing (output).
 * @param hash The resulting hash (output).
 * @return True on success, false on failure.
 */
bool hash_password(const std::string& password, std::vector<unsigned char>& salt, std::vector<unsigned char>& hash) {
    salt.assign(SALT_SIZE, 0);
    hash.assign(HASH_SIZE, 0);

    if (RAND_bytes(salt.data(), SALT_SIZE) != 1) {
        std::cerr << "Error: Failed to generate salt." << std::endl;
        return false;
    }

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

    if (result != 1) {
        std::cerr << "Error: PKCS5_PBKDF2_HMAC failed." << std::endl;
        return false;
    }

    return true;
}

/**
 * @brief Registers a new user by storing their details in a file.
 * @param name The user's name.
 * @param email The user's email.
 * @param password The user's plaintext password.
 * @return True if registration is successful, false otherwise.
 */
bool register_user(const std::string& name, const std::string& email, const std::string& password) {
    // 1. Validate inputs
    if (name.empty() || name.length() > 100) {
        std::cerr << "Registration failed: Invalid name." << std::endl;
        return false;
    }
    if (!validate_email(email)) {
        std::cerr << "Registration failed: Invalid email format." << std::endl;
        return false;
    }
    if (!validate_password(password)) {
        std::cerr << "Registration failed: Password does not meet complexity requirements." << std::endl;
        std::cerr << "Policy: min 12 chars, 1 uppercase, 1 lowercase, 1 digit, 1 special char (@$!%*?&)." << std::endl;
        return false;
    }

    // 2. Hash password
    std::vector<unsigned char> salt, hash;
    if (!hash_password(password, salt, hash)) {
        return false;
    }

    std::string salt_hex = bytes_to_hex(salt);
    std::string hash_hex = bytes_to_hex(hash);

    // 3. Store in "database" (append to a file)
    std::ofstream db_file(DB_FILE, std::ios_base::app);
    if (!db_file.is_open()) {
        std::cerr << "Error: Could not open database file for writing." << std::endl;
        return false;
    }

    // Format: name,email,salt,hash
    db_file << name << "," << email << "," << salt_hex << "," << hash_hex << std::endl;
    db_file.close();

    return true;
}

int main() {
    std::cout << "User Registration System" << std::endl;
    std::cout << "User data will be stored in '" << DB_FILE << "'" << std::endl;
    std::cout << "----------------------------------" << std::endl;

    // Test Case 1: Successful registration
    std::cout << "Test Case 1: Registering a valid user 'Alice'." << std::endl;
    if (register_user("Alice", "alice@example.com", "Password123!@#")) {
        std::cout << "SUCCESS: Alice registered." << std::endl;
    } else {
        std::cout << "FAILURE: Could not register Alice." << std::endl;
    }
    std::cout << "----------------------------------" << std::endl;
    
    // Test Case 2: Successful registration
    std::cout << "Test Case 2: Registering a valid user 'Bob'." << std::endl;
    if (register_user("Bob", "bob@example.com", "SecurePass$5678")) {
        std::cout << "SUCCESS: Bob registered." << std::endl;
    } else {
        std::cout << "FAILURE: Could not register Bob." << std::endl;
    }
    std::cout << "----------------------------------" << std::endl;

    // Test Case 3: Invalid email
    std::cout << "Test Case 3: Attempting to register with an invalid email." << std::endl;
    if (register_user("Charlie", "charlie@invalid", "Password123!@#")) {
        std::cout << "FAILURE: Charlie was registered (this should not happen)." << std::endl;
    } else {
        std::cout << "SUCCESS: Registration failed as expected for invalid email." << std::endl;
    }
    std::cout << "----------------------------------" << std::endl;

    // Test Case 4: Weak password
    std::cout << "Test Case 4: Attempting to register with a weak password." << std::endl;
    if (register_user("David", "david@example.com", "weak")) {
        std::cout << "FAILURE: David was registered (this should not happen)." << std::endl;
    } else {
        std::cout << "SUCCESS: Registration failed as expected for weak password." << std::endl;
    }
    std::cout << "----------------------------------" << std::endl;

    // Test Case 5: Empty name
    std::cout << "Test Case 5: Attempting to register with an empty name." << std::endl;
    if (register_user("", "eve@example.com", "AnotherGoodPass1!")) {
        std::cout << "FAILURE: User with empty name was registered (this should not happen)." << std::endl;
    } else {
        std::cout << "SUCCESS: Registration failed as expected for empty name." << std::endl;
    }
    std::cout << "----------------------------------" << std::endl;

    return 0;
}