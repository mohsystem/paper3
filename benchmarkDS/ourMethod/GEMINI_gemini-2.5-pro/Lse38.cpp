#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <regex>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <iomanip>
#include <sstream>

// In a real application, this would be a database.
struct UserData {
    std::vector<unsigned char> salt;
    std::vector<unsigned char> hash;
};
std::map<std::string, UserData> userDatabase;

// Constants for PBKDF2
const int SALT_LENGTH = 16;
const int ITERATIONS = 310000; // OWASP recommendation as of 2023 for PBKDF2-HMAC-SHA256
const int KEY_LENGTH = 32; // 256 bits

// Strong password policy: at least 8 chars, one uppercase, one lowercase, one digit, one special char.
const std::regex PASSWORD_REGEX("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d)(?=.*[@$!%*?&])[A-Za-z\\d@$!%*?&]{8,}$");
// Username policy: 5-20 alphanumeric characters
const std::regex USERNAME_REGEX("^[a-zA-Z0-9]{5,20}$");

void handle_openssl_errors() {
    // In a real application, log errors securely.
    // ERR_print_errors_fp(stderr);
    // For this example, we just throw an exception.
    throw std::runtime_error("An OpenSSL error occurred.");
}

std::vector<unsigned char> hashPassword(const std::string& password, const std::vector<unsigned char>& salt) {
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
        handle_openssl_errors();
    }
    return hash;
}

bool registerUser(const std::string& username, std::string password) {
    if (!std::regex_match(username, USERNAME_REGEX) || !std::regex_match(password, PASSWORD_REGEX)) {
        std::cout << "Registration failed: Username or password does not meet complexity requirements." << std::endl;
        return false;
    }

    if (userDatabase.count(username)) {
        std::cout << "Registration failed: Username '" << username << "' already exists." << std::endl;
        return false;
    }

    std::vector<unsigned char> salt(SALT_LENGTH);
    if (RAND_bytes(salt.data(), salt.size()) != 1) {
        handle_openssl_errors();
    }

    UserData newUser;
    newUser.salt = salt;
    newUser.hash = hashPassword(password, salt);
    
    // Securely clear password from memory
    #ifdef _WIN32
        SecureZeroMemory(&password[0], password.size());
    #else
        // Using volatile to prevent compiler optimization
        volatile char *p = &password[0];
        for(size_t i = 0; i < password.size(); ++i) {
            p[i] = 0;
        }
    #endif

    userDatabase[username] = newUser;
    std::cout << "User '" << username << "' registered successfully." << std::endl;
    return true;
}

bool login(const std::string& username, std::string password) {
    if (username.empty() || password.empty()) {
        std::cout << "Login failed: Username or password cannot be empty." << std::endl;
        // Simulating redirect to login page
        return false;
    }

    auto it = userDatabase.find(username);
    if (it == userDatabase.end()) {
        std::cout << "Login failed: Incorrect username or password." << std::endl; // Generic message
        return false;
    }

    const UserData& userData = it->second;
    std::vector<unsigned char> attemptedHash = hashPassword(password, userData.salt);
    
    // Securely clear password from memory
    #ifdef _WIN32
        SecureZeroMemory(&password[0], password.size());
    #else
        volatile char *p = &password[0];
        for(size_t i = 0; i < password.size(); ++i) {
            p[i] = 0;
        }
    #endif


    // Constant-time comparison to prevent timing attacks
    if (CRYPTO_memcmp(attemptedHash.data(), userData.hash.data(), KEY_LENGTH) == 0) {
        std::cout << "Login successful for user '" << username << "'. Redirecting to dashboard." << std::endl;
        // Simulating redirect to dashboard
        return true;
    } else {
        std::cout << "Login failed: Incorrect username or password." << std::endl;
        // Simulating redirect to login page
        return false;
    }
}

int main() {
    std::cout << "--- Setting up user ---" << std::endl;
    registerUser("cppuser1", "SecurePass#2024");
    std::cout << "\n--- Running Test Cases ---" << std::endl;

    // Test Case 1: Correct username and password
    std::cout << "1. Testing correct credentials..." << std::endl;
    login("cppuser1", "SecurePass#2024");

    // Test Case 2: Correct username, incorrect password
    std::cout << "\n2. Testing incorrect password..." << std::endl;
    login("cppuser1", "WrongPassword!2");

    // Test Case 3: Incorrect username
    std::cout << "\n3. Testing incorrect username..." << std::endl;
    login("nonexistentuser", "SecurePass#2024");
    
    // Test Case 4: Empty password
    std::cout << "\n4. Testing empty password..." << std::endl;
    login("cppuser1", "");

    // Test Case 5: Registration with weak password
    std::cout << "\n5. Testing registration with weak password..." << std::endl;
    registerUser("weakusercpp", "pass");

    return 0;
}