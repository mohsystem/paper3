
#include <iostream>
#include <string>
#include <map>
#include <cstring>
#include <regex>
#include <random>
#include <openssl/evp.h>
#include <openssl/rand.h>

// User structure with secure password storage
struct User {
    std::string email;
    unsigned char password_hash[32];
    unsigned char salt[16];
    bool logged_in;
    
    User() : logged_in(false) {
        memset(password_hash, 0, sizeof(password_hash));
        memset(salt, 0, sizeof(salt));
    }
};

// Simulated user database
std::map<std::string, User> users;

// Generate cryptographically secure random salt
void generate_salt(unsigned char* salt, size_t length) {
    if (RAND_bytes(salt, length) != 1) {
        throw std::runtime_error("Failed to generate random salt");
    }
}

// Hash password using PBKDF2-HMAC-SHA256 with 210000 iterations
void hash_password(const std::string& password, const unsigned char* salt, 
                   unsigned char* hash) {
    PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                      salt, 16, 210000, EVP_sha256(), 32, hash);
}

// Constant-time comparison to prevent timing attacks
bool constant_time_equals(const unsigned char* a, const unsigned char* b, size_t length) {
    unsigned char result = 0;
    for (size_t i = 0; i < length; i++) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

// Validate email format
bool is_valid_email(const std::string& email) {
    if (email.empty() || email.length() > 254) {
        return false;
    }
    std::regex pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\\\.[a-zA-Z]{2,}$");
    return std::regex_match(email, pattern);
}

// Sanitize input to prevent injection attacks
std::string sanitize_input(const std::string& input) {
    if (input.empty()) {
        return "";
    }
    std::string sanitized;
    for (char c : input) {
        if (c >= 0x20 && c != 0x7F) {
            sanitized += c;
        }
    }
    return sanitized.substr(0, std::min(sanitized.length(), size_t(254)));
}

// Register a new user
std::string register_user(const std::string& email, const std::string& password) {
    std::string clean_email = sanitize_input(email);
    if (!is_valid_email(clean_email)) {
        return "Error: Invalid email format";
    }
    
    if (password.length() < 8) {
        return "Error: Password must be at least 8 characters";
    }
    
    if (users.find(clean_email) != users.end()) {
        return "Error: User already exists";
    }
    
    User user;
    user.email = clean_email;
    generate_salt(user.salt, 16);
    hash_password(password, user.salt, user.password_hash);
    user.logged_in = false;
    
    users[clean_email] = user;
    return "Success: User registered";
}

// Login user
std::string login(const std::string& email, const std::string& password) {
    std::string clean_email = sanitize_input(email);
    if (!is_valid_email(clean_email)) {
        return "Error: Invalid credentials";
    }
    
    if (password.empty()) {
        return "Error: Invalid credentials";
    }
    
    auto it = users.find(clean_email);
    if (it == users.end()) {
        return "Error: Invalid credentials";
    }
    
    unsigned char provided_hash[32];
    hash_password(password, it->second.salt, provided_hash);
    
    if (!constant_time_equals(it->second.password_hash, provided_hash, 32)) {
        return "Error: Invalid credentials";
    }
    
    it->second.logged_in = true;
    return "Success: Logged in";
}

// Change email - requires authentication
std::string change_email(const std::string& old_email, 
                        const std::string& new_email,
                        const std::string& password) {
    std::string clean_old_email = sanitize_input(old_email);
    std::string clean_new_email = sanitize_input(new_email);
    
    if (!is_valid_email(clean_old_email) || !is_valid_email(clean_new_email)) {
        return "Error: Invalid email format";
    }
    
    if (password.empty()) {
        return "Error: Password required";
    }
    
    auto it = users.find(clean_old_email);
    if (it == users.end()) {
        return "Error: User not found";
    }
    
    if (!it->second.logged_in) {
        return "Error: User must be logged in";
    }
    
    unsigned char provided_hash[32];
    hash_password(password, it->second.salt, provided_hash);
    
    if (!constant_time_equals(it->second.password_hash, provided_hash, 32)) {
        return "Error: Incorrect password";
    }
    
    if (clean_old_email == clean_new_email) {
        return "Error: New email must be different from old email";
    }
    
    if (users.find(clean_new_email) != users.end()) {
        return "Error: New email already in use";
    }
    
    User user = it->second;
    user.email = clean_new_email;
    users.erase(it);
    users[clean_new_email] = user;
    
    return "Success: Email changed";
}

int main() {
    // Test Case 1: Register and login
    std::cout << "Test 1: Register and Login" << std::endl;
    std::cout << register_user("user1@example.com", "SecurePass123!") << std::endl;
    std::cout << login("user1@example.com", "SecurePass123!") << std::endl;
    
    // Test Case 2: Change email with valid credentials
    std::cout << "\\nTest 2: Change email successfully" << std::endl;
    std::cout << change_email("user1@example.com", "newuser1@example.com", "SecurePass123!") << std::endl;
    
    // Test Case 3: Try to change email without being logged in
    std::cout << "\\nTest 3: Change email without login" << std::endl;
    std::cout << register_user("user2@example.com", "Password456!") << std::endl;
    std::cout << change_email("user2@example.com", "newuser2@example.com", "Password456!") << std::endl;
    
    // Test Case 4: Try to change email with wrong password
    std::cout << "\\nTest 4: Change email with wrong password" << std::endl;
    std::cout << register_user("user3@example.com", "MyPass789!") << std::endl;
    std::cout << login("user3@example.com", "MyPass789!") << std::endl;
    std::cout << change_email("user3@example.com", "newuser3@example.com", "WrongPassword") << std::endl;
    
    // Test Case 5: Try to change to same email
    std::cout << "\\nTest 5: Change to same email" << std::endl;
    std::cout << register_user("user4@example.com", "TestPass000!") << std::endl;
    std::cout << login("user4@example.com", "TestPass000!") << std::endl;
    std::cout << change_email("user4@example.com", "user4@example.com", "TestPass000!") << std::endl;
    
    return 0;
}
