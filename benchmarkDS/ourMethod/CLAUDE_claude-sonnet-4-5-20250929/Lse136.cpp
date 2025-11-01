
#include <iostream>
#include <string>
#include <map>
#include <cstring>
#include <cstdlib>
#include <regex>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

// Simulated secure credential store
// In production, use environment variables or secure vault
std::map<std::string, std::string> userStore;
std::map<std::string, std::string> saltStore;

// Secure memory cleanup for sensitive data
void secure_zero(void* ptr, size_t len) {
    if (ptr == NULL || len == 0) return;
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (len--) {
        *p++ = 0;
    }
}

// Generate cryptographically secure random bytes
bool generate_random_bytes(unsigned char* buffer, size_t length) {
    if (buffer == NULL || length == 0) return false;
    return RAND_bytes(buffer, (int)length) == 1;
}

// Hash password using PBKDF2-HMAC-SHA-256 with unique salt
bool hash_password(const std::string& password, const unsigned char* salt, 
                   size_t salt_len, unsigned char* out_hash, size_t hash_len) {
    if (salt == NULL || out_hash == NULL) return false;
    
    // Use PBKDF2 with high iteration count (210000)
    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(), (int)password.length(),
        salt, (int)salt_len,
        210000,
        EVP_sha256(),
        (int)hash_len,
        out_hash
    );
    
    return result == 1;
}

// Constant-time comparison to prevent timing attacks
bool constant_time_equals(const unsigned char* a, const unsigned char* b, size_t len) {
    if (a == NULL || b == NULL) return false;
    
    unsigned char result = 0;
    for (size_t i = 0; i < len; i++) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

// Store user with hashed password and unique salt
bool store_user(const std::string& username, const std::string& password) {
    const size_t SALT_LEN = 16;
    const size_t HASH_LEN = 32;
    
    unsigned char salt[SALT_LEN];
    unsigned char hash[HASH_LEN];
    
    // Generate unique cryptographically secure salt
    if (!generate_random_bytes(salt, SALT_LEN)) {
        return false;
    }
    
    // Hash password with salt
    if (!hash_password(password, salt, SALT_LEN, hash, HASH_LEN)) {
        secure_zero(salt, SALT_LEN);
        secure_zero(hash, HASH_LEN);
        return false;
    }
    
    // Store salt and hash as base64 or hex strings
    std::string salt_str((char*)salt, SALT_LEN);
    std::string hash_str((char*)hash, HASH_LEN);
    
    userStore[username] = hash_str;
    saltStore[username] = salt_str;
    
    // Clear sensitive data
    secure_zero(salt, SALT_LEN);
    secure_zero(hash, HASH_LEN);
    
    return true;
}

// Validate input to prevent injection attacks
bool is_valid_input(const std::string& input) {
    if (input.empty() || input.length() > 100) {
        return false;
    }
    // Allow only safe characters
    std::regex pattern("^[a-zA-Z0-9!@#$%^&*()_+\\\\-=\\\\[\\\\]{};':\\"\\\\\\\\|,.<>/?]+$");
    return std::regex_match(input, pattern);
}

// Check credentials securely
bool check_credentials(const std::string& username, const std::string& password) {
    const size_t HASH_LEN = 32;
    
    // Validate inputs before processing
    if (!is_valid_input(username) || !is_valid_input(password)) {
        return false;
    }
    
    // Check if user exists
    if (userStore.find(username) == userStore.end() || 
        saltStore.find(username) == saltStore.end()) {
        // Perform dummy computation to prevent timing attacks
        unsigned char dummy_salt[16];
        unsigned char dummy_hash[HASH_LEN];
        generate_random_bytes(dummy_salt, 16);
        hash_password(password, dummy_salt, 16, dummy_hash, HASH_LEN);
        secure_zero(dummy_salt, 16);
        secure_zero(dummy_hash, HASH_LEN);
        return false;
    }
    
    // Retrieve stored salt and hash
    std::string salt_str = saltStore[username];
    std::string stored_hash_str = userStore[username];
    
    unsigned char provided_hash[HASH_LEN];
    
    // Hash provided password with stored salt
    if (!hash_password(password, (unsigned char*)salt_str.data(), 
                      salt_str.length(), provided_hash, HASH_LEN)) {
        secure_zero(provided_hash, HASH_LEN);
        return false;
    }
    
    // Use constant-time comparison
    bool result = constant_time_equals(
        (unsigned char*)stored_hash_str.data(),
        provided_hash,
        HASH_LEN
    );
    
    // Clear sensitive data
    secure_zero(provided_hash, HASH_LEN);
    
    return result;
}

// Simple URL parameter parser
std::map<std::string, std::string> parse_query(const std::string& query) {
    std::map<std::string, std::string> params;
    if (query.empty()) return params;
    
    size_t pos = 0;
    while (pos < query.length()) {
        size_t amp_pos = query.find('&', pos);
        if (amp_pos == std::string::npos) amp_pos = query.length();
        
        std::string param = query.substr(pos, amp_pos - pos);
        size_t eq_pos = param.find('=');
        
        if (eq_pos != std::string::npos) {
            std::string key = param.substr(0, eq_pos);
            std::string value = param.substr(eq_pos + 1);
            params[key] = value;
        }
        
        pos = amp_pos + 1;
    }
    
    return params;
}

// Simulate check_mod function
std::string check_mod(const std::string& query) {
    try {
        // Parse query parameters
        std::map<std::string, std::string> params = parse_query(query);
        
        // Check if username and password are present
        if (params.find("username") == params.end() || 
            params.find("password") == params.end()) {
            return "false";
        }
        
        std::string username = params["username"];
        std::string password = params["password"];
        
        // Check credentials
        if (check_credentials(username, password)) {
            return "true";
        } else {
            return "false";
        }
    } catch (...) {
        // Generic error handling
        return "false";
    }
}

int main() {
    // Initialize OpenSSL
    OpenSSL_add_all_algorithms();
    
    // Initialize test users with secure passwords
    store_user("admin", "SecurePass123!");
    store_user("user1", "MyPassword456!");
    store_user("testuser", "TestPass789!");
    store_user("moderator", "ModPass321!");
    store_user("developer", "DevPass654!");
    
    std::cout << "Test cases:\\n";
    
    // Test case 1: Valid credentials
    std::string result1 = check_mod("username=admin&password=SecurePass123!");
    std::cout << "1. Valid admin: " << result1 << std::endl;
    
    // Test case 2: Valid credentials
    std::string result2 = check_mod("username=user1&password=MyPassword456!");
    std::cout << "2. Valid user1: " << result2 << std::endl;
    
    // Test case 3: Invalid password
    std::string result3 = check_mod("username=admin&password=wrong");
    std::cout << "3. Invalid password: " << result3 << std::endl;
    
    // Test case 4: Invalid user
    std::string result4 = check_mod("username=invalid&password=test");
    std::cout << "4. Invalid user: " << result4 << std::endl;
    
    // Test case 5: Missing parameters
    std::string result5 = check_mod("username=admin");
    std::cout << "5. Missing params: " << result5 << std::endl;
    
    // Cleanup OpenSSL
    EVP_cleanup();
    
    return 0;
}
