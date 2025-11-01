
#include <iostream>
#include <string>
#include <map>
#include <cstring>
#include <stdexcept>
#include <regex>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

// User record structure
struct UserRecord {
    std::string email;
    std::string password_hash;
    unsigned char salt[16];
    
    UserRecord() { memset(salt, 0, sizeof(salt)); }
    UserRecord(const std::string& e, const std::string& h, const unsigned char* s) 
        : email(e), password_hash(h) {
        memcpy(salt, s, 16);
    }
    ~UserRecord() {
        // Clear sensitive data - Rule#1
        OPENSSL_cleanse(salt, sizeof(salt));
    }
};

// Simulated user database
std::map<std::string, UserRecord> user_database;

// Generate cryptographically secure salt - Rule#8, Rule#9
void generate_salt(unsigned char* salt, size_t len) {
    if (RAND_bytes(salt, len) != 1) {
        throw std::runtime_error("Salt generation failed");
    }
}

// Hash password with PBKDF2-HMAC-SHA256 - Rule#6, Rule#15
std::string hash_password(const std::string& password, const unsigned char* salt) {
    unsigned char hash[32];
    // Use PBKDF2 with at least 210000 iterations
    if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                          salt, 16, 210000,
                          EVP_sha256(), 32, hash) != 1) {
        throw std::runtime_error("Password hashing failed");
    }
    
    // Convert to hex string
    char hex[65];
    for (int i = 0; i < 32; i++) {
        snprintf(hex + (i * 2), 3, "%02x", hash[i]);
    }
    hex[64] = '\\0';
    
    // Clear sensitive data - Rule#1
    OPENSSL_cleanse(hash, sizeof(hash));
    
    return std::string(hex);
}

// Constant-time comparison to prevent timing attacks - Rule#6
bool constant_time_equals(const std::string& a, const std::string& b) {
    if (a.empty() || b.empty() || a.length() != b.length()) {
        return false;
    }
    return CRYPTO_memcmp(a.c_str(), b.c_str(), a.length()) == 0;
}

// Validate email format - Rule#1, Rule#13
bool is_valid_email(const std::string& email) {
    if (email.empty() || email.length() > 254) return false;
    
    // Basic email validation
    std::regex pattern("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$");
    return std::regex_match(email, pattern);
}

// Verify password meets policy - Rule#15
bool meets_password_policy(const std::string& password) {
    if (password.length() < 12) return false;
    
    bool has_upper = false, has_lower = false, has_digit = false, has_special = false;
    const std::string special_chars = "!@#$%^&*()_+-=[]{}|;:,.<>?";
    
    for (char c : password) {
        if (std::isupper(c)) has_upper = true;
        if (std::islower(c)) has_lower = true;
        if (std::isdigit(c)) has_digit = true;
        if (special_chars.find(c) != std::string::npos) has_special = true;
    }
    
    return has_upper && has_lower && has_digit && has_special;
}

// Change email securely - Rule#1, Rule#2, Rule#16
std::string change_email(const std::string& session_user_id, 
                        const std::string& old_email,
                        const std::string& new_email, 
                        const std::string& confirm_password) {
    try {
        // Input validation - Rule#13
        if (session_user_id.empty()) {
            return "Error: Not authenticated";
        }
        
        if (old_email.empty() || new_email.empty() || confirm_password.empty()) {
            return "Error: All fields required";
        }
        
        // Validate email formats - Rule#13
        if (!is_valid_email(old_email) || !is_valid_email(new_email)) {
            return "Error: Invalid email format";
        }
        
        // Check email length limits - Rule#14
        if (old_email.length() > 254 || new_email.length() > 254) {
            return "Error: Email too long";
        }
        
        // Prevent same old and new email
        if (old_email == new_email) {
            return "Error: New email must be different";
        }
        
        // Retrieve user record
        auto it = user_database.find(session_user_id);
        if (it == user_database.end()) {
            return "Error: User not found";
        }
        
        UserRecord& user = it->second;
        
        // Verify old email matches current email - Rule#1
        if (!constant_time_equals(user.email, old_email)) {
            return "Error: Old email incorrect";
        }
        
        // Verify password - Rule#6, Rule#15
        std::string provided_hash = hash_password(confirm_password, user.salt);
        if (!constant_time_equals(user.password_hash, provided_hash)) {
            // Clear sensitive data
            OPENSSL_cleanse(&provided_hash[0], provided_hash.size());
            return "Error: Password incorrect";
        }
        
        // Clear password hash from memory
        OPENSSL_cleanse(&provided_hash[0], provided_hash.size());
        
        // Check if new email already exists
        for (const auto& pair : user_database) {
            if (constant_time_equals(pair.second.email, new_email)) {
                return "Error: Email already in use";
            }
        }
        
        // Update email
        user.email = new_email;
        
        return "Success: Email changed";
        
    } catch (const std::exception& e) {
        // Rule#1, Rule#16 - Generic error, log details securely
        std::cerr << "Email change error: " << typeid(e).name() << std::endl;
        return "Error: Operation failed";
    }
}

// Helper to create test user
void create_user(const std::string& user_id, const std::string& email, const std::string& password) {
    unsigned char salt[16];
    generate_salt(salt, 16);
    std::string hash = hash_password(password, salt);
    user_database[user_id] = UserRecord(email, hash, salt);
}

int main() {
    std::cout << "=== Email Change System Test Cases ===\\n\\n";
    
    // Setup test users
    create_user("user1", "alice@example.com", "SecurePass123!");
    create_user("user2", "bob@example.com", "AnotherPass456#");
    
    // Test 1: Successful email change
    std::cout << "Test 1: Valid email change\\n";
    std::string result1 = change_email("user1", "alice@example.com", "alice.new@example.com", "SecurePass123!");
    std::cout << result1 << "\\n\\n";
    
    // Test 2: Wrong old email
    std::cout << "Test 2: Wrong old email\\n";
    std::string result2 = change_email("user2", "wrong@example.com", "bob.new@example.com", "AnotherPass456#");
    std::cout << result2 << "\\n\\n";
    
    // Test 3: Wrong password
    std::cout << "Test 3: Wrong password\\n";
    std::string result3 = change_email("user2", "bob@example.com", "bob.new@example.com", "WrongPassword!");
    std::cout << result3 << "\\n\\n";
    
    // Test 4: Invalid email format
    std::cout << "Test 4: Invalid email format\\n";
    std::string result4 = change_email("user1", "alice.new@example.com", "invalid-email", "SecurePass123!");
    std::cout << result4 << "\\n\\n";
    
    // Test 5: Empty fields
    std::cout << "Test 5: Empty fields\\n";
    std::string result5 = change_email("user2", "", "bob.final@example.com", "AnotherPass456#");
    std::cout << result5 << "\\n\\n";
    
    return 0;
}
