
#include <iostream>
#include <string>
#include <map>
#include <regex>
#include <cstring>
#include <openssl/evp.h>
#include <openssl/rand.h>

// Security constants
const int MIN_PASSWORD_LENGTH = 8;
const int SALT_LENGTH = 16;
const int PBKDF2_ITERATIONS = 210000;
const int SESSION_TOKEN_LENGTH = 32;

// User account structure
struct UserAccount {
    std::string email;
    unsigned char password_hash[32];
    unsigned char salt[SALT_LENGTH];
};

// In-memory stores
std::map<std::string, UserAccount> userStore;
std::map<std::string, std::string> sessionStore;

// Email validation pattern
const std::regex EMAIL_PATTERN("^[a-zA-Z0-9_+&*-]+(?:\\\\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\\\\.)+[a-zA-Z]{2,7}$");

// Securely clear memory
void secure_zero(void* ptr, size_t size) {
    if (ptr) {
        volatile unsigned char* p = static_cast<volatile unsigned char*>(ptr);
        while (size--) {
            *p++ = 0;
        }
    }
}

// Generate cryptographically secure random bytes
bool generate_random_bytes(unsigned char* buffer, int length) {
    if (!buffer || length <= 0) {
        return false;
    }
    return RAND_bytes(buffer, length) == 1;
}

// Hash password using PBKDF2-HMAC-SHA-256
bool hash_password(const std::string& password, const unsigned char* salt, 
                   unsigned char* output) {
    if (password.empty() || !salt || !output) {
        return false;
    }
    
    // Use PBKDF2-HMAC-SHA-256 with 210,000 iterations
    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(), password.length(),
        salt, SALT_LENGTH,
        PBKDF2_ITERATIONS,
        EVP_sha256(),
        32, output
    );
    
    return result == 1;
}

// Constant-time comparison to prevent timing attacks
bool constant_time_compare(const unsigned char* a, const unsigned char* b, size_t length) {
    if (!a || !b) {
        return false;
    }
    
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
    return std::regex_match(email, EMAIL_PATTERN);
}

// Validate password strength
bool is_valid_password(const std::string& password) {
    if (password.length() < MIN_PASSWORD_LENGTH) {
        return false;
    }
    
    bool has_digit = false;
    bool has_lower = false;
    bool has_upper = false;
    
    for (char c : password) {
        if (isdigit(c)) has_digit = true;
        if (islower(c)) has_lower = true;
        if (isupper(c)) has_upper = true;
    }
    
    return has_digit && has_lower && has_upper;
}

// Generate secure session token
std::string generate_session_token() {
    unsigned char token_bytes[SESSION_TOKEN_LENGTH];
    if (!generate_random_bytes(token_bytes, SESSION_TOKEN_LENGTH)) {
        return "";
    }
    
    // Convert to hex string
    char hex[SESSION_TOKEN_LENGTH * 2 + 1];
    for (int i = 0; i < SESSION_TOKEN_LENGTH; i++) {
        sprintf(hex + i * 2, "%02x", token_bytes[i]);
    }
    hex[SESSION_TOKEN_LENGTH * 2] = '\\0';
    
    return std::string(hex);
}

// Register a new user
std::string register_user(const std::string& email, const std::string& password) {
    // Validate input parameters
    if (email.empty() || password.empty()) {
        return "Error: Invalid input";
    }
    
    if (!is_valid_email(email)) {
        return "Error: Invalid email format";
    }
    
    if (!is_valid_password(password)) {
        return "Error: Password must be at least 8 characters and contain uppercase, lowercase, and digit";
    }
    
    // Check if user already exists
    if (userStore.find(email) != userStore.end()) {
        return "Error: User already exists";
    }
    
    // Create new user account
    UserAccount account;
    account.email = email;
    
    // Generate unique salt (mitigates CWE-759)
    if (!generate_random_bytes(account.salt, SALT_LENGTH)) {
        return "Error: Failed to generate salt";
    }
    
    // Hash password with salt
    if (!hash_password(password, account.salt, account.password_hash)) {
        return "Error: Failed to hash password";
    }
    
    // Store user account
    userStore[email] = account;
    
    return "Success: User registered";
}

// Login and create session
std::string login(const std::string& email, const std::string& password) {
    // Validate input
    if (email.empty() || password.empty()) {
        return "";
    }
    
    // Find user account
    auto it = userStore.find(email);
    if (it == userStore.end()) {
        return "";  // User not found - generic error
    }
    
    UserAccount& account = it->second;
    
    // Hash provided password with stored salt
    unsigned char provided_hash[32];
    if (!hash_password(password, account.salt, provided_hash)) {
        return "";
    }
    
    // Constant-time comparison to prevent timing attacks
    if (!constant_time_compare(provided_hash, account.password_hash, 32)) {
        secure_zero(provided_hash, sizeof(provided_hash));
        return "";  // Invalid password
    }
    
    secure_zero(provided_hash, sizeof(provided_hash));
    
    // Generate secure session token
    std::string session_token = generate_session_token();
    if (session_token.empty()) {
        return "";
    }
    
    // Store session
    sessionStore[session_token] = email;
    
    return session_token;
}

// Get email from session token
std::string get_email_from_session(const std::string& session_token) {
    if (session_token.empty()) {
        return "";
    }
    
    auto it = sessionStore.find(session_token);
    if (it == sessionStore.end()) {
        return "";
    }
    
    return it->second;
}

// Change email with authentication
std::string change_email(const std::string& session_token, const std::string& old_email,
                        const std::string& new_email, const std::string& password) {
    // Validate all input parameters
    if (session_token.empty() || old_email.empty() || 
        new_email.empty() || password.empty()) {
        return "Error: Invalid input";
    }
    
    // Verify session is valid
    std::string session_email = get_email_from_session(session_token);
    if (session_email.empty()) {
        return "Error: Not authenticated";
    }
    
    // Verify old email matches session email
    if (session_email != old_email) {
        return "Error: Email mismatch";
    }
    
    // Validate new email format
    if (!is_valid_email(new_email)) {
        return "Error: Invalid new email format";
    }
    
    // Check if new email is already taken
    if (userStore.find(new_email) != userStore.end() && new_email != old_email) {
        return "Error: Email already in use";
    }
    
    // Find user account
    auto it = userStore.find(old_email);
    if (it == userStore.end()) {
        return "Error: User not found";
    }
    
    UserAccount& account = it->second;
    
    // Verify password
    unsigned char provided_hash[32];
    if (!hash_password(password, account.salt, provided_hash)) {
        return "Error: Failed to verify password";
    }
    
    if (!constant_time_compare(provided_hash, account.password_hash, 32)) {
        secure_zero(provided_hash, sizeof(provided_hash));
        return "Error: Invalid password";
    }
    
    secure_zero(provided_hash, sizeof(provided_hash));
    
    // Update email
    UserAccount updated_account = account;
    updated_account.email = new_email;
    
    userStore.erase(old_email);
    userStore[new_email] = updated_account;
    
    // Update session
    sessionStore[session_token] = new_email;
    
    return "Success: Email changed";
}

int main() {
    std::cout << "=== Test Case 1: Register and change email successfully ===" << std::endl;
    std::string result1 = register_user("user1@example.com", "SecurePass123");
    std::cout << "Register: " << result1 << std::endl;
    std::string token1 = login("user1@example.com", "SecurePass123");
    std::cout << "Login: " << (!token1.empty() ? "Success" : "Failed") << std::endl;
    std::string change1 = change_email(token1, "user1@example.com", "newemail1@example.com", "SecurePass123");
    std::cout << "Change email: " << change1 << std::endl << std::endl;
    
    std::cout << "=== Test Case 2: Wrong old email ===" << std::endl;
    std::string result2 = register_user("user2@example.com", "StrongPass456");
    std::cout << "Register: " << result2 << std::endl;
    std::string token2 = login("user2@example.com", "StrongPass456");
    std::string change2 = change_email(token2, "wrong@example.com", "new2@example.com", "StrongPass456");
    std::cout << "Change email with wrong old email: " << change2 << std::endl << std::endl;
    
    std::cout << "=== Test Case 3: Wrong password ===" << std::endl;
    std::string result3 = register_user("user3@example.com", "MyPassword789");
    std::cout << "Register: " << result3 << std::endl;
    std::string token3 = login("user3@example.com", "MyPassword789");
    std::string change3 = change_email(token3, "user3@example.com", "new3@example.com", "WrongPass123");
    std::cout << "Change email with wrong password: " << change3 << std::endl << std::endl;
    
    std::cout << "=== Test Case 4: Not logged in ===" << std::endl;
    std::string change4 = change_email("invalid_token", "user4@example.com", "new4@example.com", "AnyPass123");
    std::cout << "Change email without login: " << change4 << std::endl << std::endl;
    
    std::cout << "=== Test Case 5: Invalid new email format ===" << std::endl;
    std::string result5 = register_user("user5@example.com", "ValidPass999");
    std::cout << "Register: " << result5 << std::endl;
    std::string token5 = login("user5@example.com", "ValidPass999");
    std::string change5 = change_email(token5, "user5@example.com", "invalid-email", "ValidPass999");
    std::cout << "Change to invalid email: " << change5 << std::endl;
    
    return 0;
}
