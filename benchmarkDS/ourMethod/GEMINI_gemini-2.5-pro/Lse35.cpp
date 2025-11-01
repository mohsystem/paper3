#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <regex>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

// Note: This code requires linking with OpenSSL libraries.
// Compile with: g++ -std=c++17 your_file.cpp -o your_app -lssl -lcrypto

// Configuration for PBKDF2
const int SALT_LENGTH = 16;
const int HASH_LENGTH = 32; // Corresponds to SHA-256
const int ITERATIONS = 210000;
const EVP_MD* HASH_ALGORITHM = EVP_sha256();

// Rule #1: Basic input validation regex
// Username: 3-20 chars, alphanumeric
const std::regex USERNAME_REGEX("^[a-zA-Z0-9]{3,20}$");
// Password: 8-64 chars, at least one letter and one digit
const std::regex PASSWORD_REGEX("^(?=.*[A-Za-z])(?=.*\\d)[A-Za-z\\d]{8,64}$");

struct UserData {
    std::string username;
    std::vector<unsigned char> salt;
    std::vector<unsigned char> hashedPassword;
};

// Simulate a user database
static std::map<std::string, UserData> userDatabase;

// RAII wrapper to ensure OpenSSL error queue is printed on scope exit
struct OpenSSLErrorPrinter {
    ~OpenSSLErrorPrinter() {
        ERR_print_errors_fp(stderr);
    }
};

bool generateSalt(std::vector<unsigned char>& salt) {
    salt.resize(SALT_LENGTH);
    // Rule #10: Use cryptographically secure random generator
    if (RAND_bytes(salt.data(), SALT_LENGTH) != 1) {
        return false;
    }
    return true;
}

bool hashPassword(const std::string& password, const std::vector<unsigned char>& salt, std::vector<unsigned char>& hash) {
    hash.resize(HASH_LENGTH);
    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(),
        password.length(),
        salt.data(),
        salt.size(),
        ITERATIONS,
        HASH_ALGORITHM,
        hash.size(),
        hash.data()
    );
    return result == 1;
}

void registerUser(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        std::cerr << "Registration failed: Username or password cannot be empty." << std::endl;
        return;
    }

    UserData newUser;
    newUser.username = username;

    if (!generateSalt(newUser.salt)) {
        std::cerr << "Failed to generate salt for " << username << std::endl;
        OpenSSLErrorPrinter p;
        return;
    }

    if (!hashPassword(password, newUser.salt, newUser.hashedPassword)) {
        std::cerr << "Failed to hash password for " << username << std::endl;
        OpenSSLErrorPrinter p;
        return;
    }

    userDatabase[username] = newUser;
    std::cout << "User '" << username << "' registered." << std::endl;
}

std::string login(const std::string& username, const std::string& password) {
    // Rule #1: Input validation
    if (!std::regex_match(username, USERNAME_REGEX)) {
        std::cerr << "Error: Invalid username format." << std::endl;
        return "LOGIN_PAGE";
    }
    if (!std::regex_match(password, PASSWORD_REGEX)) {
        std::cerr << "Error: Invalid password format." << std::endl;
        return "LOGIN_PAGE";
    }

    auto it = userDatabase.find(username);
    if (it == userDatabase.end()) {
        // To prevent timing attacks, perform a dummy hash
        std::vector<unsigned char> dummySalt;
        std::vector<unsigned char> dummyHash;
        if (generateSalt(dummySalt)) {
            hashPassword("dummyPassword", dummySalt, dummyHash);
        }
        return "LOGIN_PAGE";
    }

    const UserData& user = it->second;
    std::vector<unsigned char> providedPasswordHash;

    if (!hashPassword(password, user.salt, providedPasswordHash)) {
        std::cerr << "Error hashing provided password during login." << std::endl;
        OpenSSLErrorPrinter p;
        return "LOGIN_PAGE";
    }

    // Rule #7 & #8: Use constant-time comparison from a vetted library
    if (user.hashedPassword.size() == providedPasswordHash.size() &&
        CRYPTO_memcmp(user.hashedPassword.data(), providedPasswordHash.data(), user.hashedPassword.size()) == 0) {
        return "HOME_PAGE";
    } else {
        return "LOGIN_PAGE";
    }
}

int main() {
    // Populate the mock database
    registerUser("admin", "AdminPass123");
    registerUser("user1", "UserPass456");

    std::cout << "\n--- Running Test Cases ---" << std::endl;

    // Test Case 1: Successful login
    std::cout << "\nTest Case 1: Valid credentials for 'admin'" << std::endl;
    std::string result1 = login("admin", "AdminPass123");
    std::cout << "Result: Login " << (result1 == "HOME_PAGE" ? "successful" : "failed") << ". Redirecting to " << result1 << std::endl;

    // Test Case 2: Incorrect password
    std::cout << "\nTest Case 2: Invalid password for 'admin'" << std::endl;
    std::string result2 = login("admin", "WrongPass789");
    std::cout << "Result: Login " << (result2 == "HOME_PAGE" ? "successful" : "failed") << ". Redirecting to " << result2 << std::endl;

    // Test Case 3: Non-existent user
    std::cout << "\nTest Case 3: Non-existent user 'nouser'" << std::endl;
    std::string result3 = login("nouser", "SomePass123");
    std::cout << "Result: Login " << (result3 == "HOME_PAGE" ? "successful" : "failed") << ". Redirecting to " << result3 << std::endl;

    // Test Case 4: Invalid username format
    std::cout << "\nTest Case 4: Invalid username format 'us'" << std::endl;
    std::string result4 = login("us", "SomePass123");
    std::cout << "Result: Login " << (result4 == "HOME_PAGE" ? "successful" : "failed") << ". Redirecting to " << result4 << std::endl;

    // Test Case 5: Invalid password format (too short)
    std::cout << "\nTest Case 5: Invalid password format for 'user1'" << std::endl;
    std::string result5 = login("user1", "pass");
    std::cout << "Result: Login " << (result5 == "HOME_PAGE" ? "successful" : "failed") << ". Redirecting to " << result5 << std::endl;

    return 0;
}