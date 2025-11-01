#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <iomanip>
#include <sstream>

// To compile: g++ your_file_name.cpp -o Lse95 -lssl -lcrypto
// You must have OpenSSL development libraries installed.

const int SALT_LENGTH = 16;
const int HASH_LENGTH = 32; // SHA-256 digest length

struct UserData {
    std::vector<unsigned char> salt;
    std::vector<unsigned char> hashedPassword;
};

// In-memory user database (for demonstration purposes)
static std::map<std::string, UserData> userDatabase;

/**
 * @brief Generates a cryptographically strong random salt.
 * @return A vector of unsigned char containing the salt.
 */
std::vector<unsigned char> generateSalt() {
    std::vector<unsigned char> salt(SALT_LENGTH);
    if (RAND_bytes(salt.data(), salt.size()) != 1) {
        throw std::runtime_error("Error generating random salt.");
    }
    return salt;
}

/**
 * @brief Hashes a password with a given salt using SHA-256.
 * @param password The password to hash.
 * @param salt The salt to use.
 * @return The hashed password as a vector of unsigned char.
 */
std::vector<unsigned char> hashPassword(const std::string& password, const std::vector<unsigned char>& salt) {
    std::vector<unsigned char> combined(salt.begin(), salt.end());
    combined.insert(combined.end(), password.begin(), password.end());

    std::vector<unsigned char> hash(EVP_MAX_MD_SIZE);
    unsigned int hash_len;

    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (mdctx == NULL) throw std::runtime_error("EVP_MD_CTX_new failed");

    try {
        if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1) throw std::runtime_error("EVP_DigestInit_ex failed");
        if (EVP_DigestUpdate(mdctx, combined.data(), combined.size()) != 1) throw std::runtime_error("EVP_DigestUpdate failed");
        if (EVP_DigestFinal_ex(mdctx, hash.data(), &hash_len) != 1) throw std::runtime_error("EVP_DigestFinal_ex failed");
        hash.resize(hash_len);
    } catch (...) {
        EVP_MD_CTX_free(mdctx);
        throw;
    }

    EVP_MD_CTX_free(mdctx);
    return hash;
}

/**
 * @brief Registers a new user.
 * @param username The username.
 * @param password The password.
 * @return true if registration is successful, false if user already exists.
 */
bool registerUser(const std::string& username, const std::string& password) {
    if (userDatabase.find(username) != userDatabase.end()) {
        std::cout << "Registration failed: Username '" << username << "' already exists." << std::endl;
        return false;
    }
    
    UserData newUser;
    newUser.salt = generateSalt();
    newUser.hashedPassword = hashPassword(password, newUser.salt);
    userDatabase[username] = newUser;
    
    std::cout << "User '" << username << "' registered successfully." << std::endl;
    return true;
}

/**
 * @brief Verifies a user's login credentials.
 * @param username The username.
 * @param password The password to verify.
 * @return true if credentials are correct, false otherwise.
 */
bool loginUser(const std::string& username, const std::string& password) {
    auto it = userDatabase.find(username);
    if (it == userDatabase.end()) {
        std::cout << "Login failed: User '" << username << "' not found." << std::endl;
        return false;
    }

    const UserData& userData = it->second;
    std::vector<unsigned char> providedPasswordHash = hashPassword(password, userData.salt);

    // Constant-time comparison to prevent timing attacks
    bool match = (providedPasswordHash.size() == userData.hashedPassword.size()) &&
                 (CRYPTO_memcmp(providedPasswordHash.data(), userData.hashedPassword.data(), providedPasswordHash.size()) == 0);

    if (match) {
        std::cout << "Login successful for user '" << username << "'." << std::endl;
    } else {
        std::cout << "Login failed: Incorrect password for user '" << username << "'." << std::endl;
    }
    return match;
}

class Lse95 {
public:
    static void runTests() {
        std::cout << "--- Running Test Cases ---" << std::endl;

        // Test Case 1: Register a new user
        std::cout << "\n[Test Case 1: Successful Registration]" << std::endl;
        registerUser("alice", "Password123!");

        // Test Case 2: Successful Login
        std::cout << "\n[Test Case 2: Successful Login]" << std::endl;
        loginUser("alice", "Password123!");

        // Test Case 3: Failed Login (Wrong Password)
        std::cout << "\n[Test Case 3: Failed Login - Wrong Password]" << std::endl;
        loginUser("alice", "WrongPassword");

        // Test Case 4: Failed Registration (Username already exists)
        std::cout << "\n[Test Case 4: Failed Registration - User Exists]" << std::endl;
        registerUser("alice", "AnotherPassword");

        // Test Case 5: Failed Login (User does not exist)
        std::cout << "\n[Test Case 5: Failed Login - User Not Found]" << std::endl;
        loginUser("bob", "Password123!");

        std::cout << "\n--- Test Cases Finished ---" << std::endl;
    }
};

int main() {
    Lse95::runTests();
    return 0;
}