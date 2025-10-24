
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <stdexcept>

// Constants for security parameters
constexpr size_t SALT_SIZE = 16;
constexpr size_t HASH_SIZE = 32;
constexpr int PBKDF2_ITERATIONS = 210000;

// Structure to hold user credentials
struct UserCredentials {
    std::string username;
    std::vector<uint8_t> salt;
    std::vector<uint8_t> password_hash;
};

// Securely clear memory containing sensitive data
void secure_clear(void* ptr, size_t len) {
    if (ptr == nullptr || len == 0) return;
    volatile unsigned char* p = static_cast<volatile unsigned char*>(ptr);
    while (len--) *p++ = 0;
}

// RAII wrapper for secure memory
class SecureBuffer {
private:
    std::vector<uint8_t> buffer;
public:
    explicit SecureBuffer(size_t size) : buffer(size, 0) {}
    ~SecureBuffer() {
        secure_clear(buffer.data(), buffer.size());
    }
    uint8_t* data() { return buffer.data(); }
    size_t size() const { return buffer.size(); }
};

// Generate cryptographically secure random salt
bool generate_salt(std::vector<uint8_t>& salt) {
    // CWE-338 mitigation: Use cryptographically secure RNG
    salt.resize(SALT_SIZE);
    if (RAND_bytes(salt.data(), SALT_SIZE) != 1) {
        return false;
    }
    return true;
}

// Hash password with PBKDF2-HMAC-SHA256
bool hash_password(const std::string& password, const std::vector<uint8_t>& salt, 
                   std::vector<uint8_t>& hash) {
    // Input validation - CWE-20 mitigation
    if (password.empty() || password.length() > 128) {
        return false;
    }
    if (salt.size() != SALT_SIZE) {
        return false;
    }

    hash.resize(HASH_SIZE);
    
    // CWE-327, CWE-759 mitigation: Use PBKDF2 with SHA-256, unique salt, and sufficient iterations
    if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.length()),
                          salt.data(), static_cast<int>(salt.size()),
                          PBKDF2_ITERATIONS, EVP_sha256(),
                          static_cast<int>(hash.size()), hash.data()) != 1) {
        return false;
    }
    
    return true;
}

// Constant-time comparison to prevent timing attacks
bool constant_time_compare(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
    if (a.size() != b.size()) {
        return false;
    }
    
    // CWE-208 mitigation: Constant-time comparison
    int result = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

// Register a new user
bool register_user(const std::string& username, const std::string& password,
                   std::vector<UserCredentials>& users) {
    // Input validation - CWE-20 mitigation
    if (username.empty() || username.length() > 64 || password.empty() || password.length() > 128) {
        return false;
    }
    
    // Check for alphanumeric username
    for (char c : username) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-') {
            return false;
        }
    }
    
    // Check if user already exists
    for (const auto& user : users) {
        if (user.username == username) {
            return false; // User already exists
        }
    }
    
    UserCredentials creds;
    creds.username = username;
    
    // Generate unique salt for this user - CWE-759 mitigation
    if (!generate_salt(creds.salt)) {
        return false;
    }
    
    // Hash password with salt
    if (!hash_password(password, creds.salt, creds.password_hash)) {
        return false;
    }
    
    users.push_back(std::move(creds));
    return true;
}

// Authenticate a user
bool authenticate_user(const std::string& username, const std::string& password,
                       const std::vector<UserCredentials>& users) {
    // Input validation - CWE-20 mitigation
    if (username.empty() || username.length() > 64 || password.empty() || password.length() > 128) {
        return false;
    }
    
    // Find user
    const UserCredentials* user_creds = nullptr;
    for (const auto& user : users) {
        if (user.username == username) {
            user_creds = &user;
            break;
        }
    }
    
    if (user_creds == nullptr) {
        // User not found - perform dummy hash to prevent timing attacks
        std::vector<uint8_t> dummy_salt(SALT_SIZE, 0);
        std::vector<uint8_t> dummy_hash;
        hash_password(password, dummy_salt, dummy_hash);
        return false;
    }
    
    // Hash provided password with stored salt
    std::vector<uint8_t> computed_hash;
    if (!hash_password(password, user_creds->salt, computed_hash)) {
        return false;
    }
    
    // Constant-time comparison - CWE-208 mitigation
    return constant_time_compare(computed_hash, user_creds->password_hash);
}

int main() {
    std::vector<UserCredentials> users;
    
    std::cout << "User Authentication System" << std::endl;
    std::cout << "==========================" << std::endl << std::endl;
    
    // Test case 1: Register valid user
    std::cout << "Test 1: Register user 'alice' with password 'SecurePass123!': ";
    if (register_user("alice", "SecurePass123!", users)) {
        std::cout << "SUCCESS" << std::endl;
    } else {
        std::cout << "FAILED" << std::endl;
    }
    
    // Test case 2: Authenticate with correct credentials
    std::cout << "Test 2: Authenticate 'alice' with correct password: ";
    if (authenticate_user("alice", "SecurePass123!", users)) {
        std::cout << "SUCCESS" << std::endl;
    } else {
        std::cout << "FAILED" << std::endl;
    }
    
    // Test case 3: Authenticate with incorrect password
    std::cout << "Test 3: Authenticate 'alice' with wrong password: ";
    if (!authenticate_user("alice", "WrongPassword", users)) {
        std::cout << "SUCCESS (correctly rejected)" << std::endl;
    } else {
        std::cout << "FAILED" << std::endl;
    }
    
    // Test case 4: Register another user
    std::cout << "Test 4: Register user 'bob' with password 'AnotherSecure456!': ";
    if (register_user("bob", "AnotherSecure456!", users)) {
        std::cout << "SUCCESS" << std::endl;
    } else {
        std::cout << "FAILED" << std::endl;
    }
    
    // Test case 5: Attempt to register duplicate username
    std::cout << "Test 5: Attempt to register duplicate user 'alice': ";
    if (!register_user("alice", "DifferentPass789!", users)) {
        std::cout << "SUCCESS (correctly rejected)" << std::endl;
    } else {
        std::cout << "FAILED" << std::endl;
    }
    
    return 0;
}
