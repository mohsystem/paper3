
#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <ctime>
#include <random>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstring>
#include <openssl/sha.h>
#include <openssl/rand.h>

// Secure session timeout in seconds
const long SESSION_TIMEOUT_SECONDS = 30 * 60;

class User {
public:
    std::string username;
    std::string passwordHash;
    std::string salt;
    bool isAdmin;
    
    User(const std::string& username, const std::string& passwordHash, 
         const std::string& salt, bool isAdmin)
        : username(username), passwordHash(passwordHash), salt(salt), isAdmin(isAdmin) {}
};

class SessionData {
public:
    std::string username;
    bool isAdmin;
    time_t createdTime;
    
    SessionData(const std::string& username, bool isAdmin, time_t createdTime)
        : username(username), isAdmin(isAdmin), createdTime(createdTime) {}
    
    bool isExpired() const {
        return (time(nullptr) - createdTime) > SESSION_TIMEOUT_SECONDS;
    }
};

// User database and active sessions
std::map<std::string, std::unique_ptr<User>> userDatabase;
std::map<std::string, std::unique_ptr<SessionData>> activeSessions;

// Convert bytes to hex string
std::string bytesToHex(const unsigned char* data, size_t length) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < length; ++i) {
        ss << std::setw(2) << static_cast<int>(data[i]);
    }
    return ss.str();
}

// Generate cryptographically secure salt
std::string generateSalt() {
    unsigned char salt[32];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        throw std::runtime_error("Failed to generate secure random salt");
    }
    return bytesToHex(salt, sizeof(salt));
}

// Hash password with salt using SHA-256
std::string hashPassword(const std::string& password, const std::string& salt) {
    if (password.empty() || salt.empty()) {
        throw std::invalid_argument("Password and salt cannot be empty");
    }
    
    std::string saltedPassword = password + salt;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    
    SHA256(reinterpret_cast<const unsigned char*>(saltedPassword.c_str()), 
           saltedPassword.length(), hash);
    
    return bytesToHex(hash, SHA256_DIGEST_LENGTH);
}

// Timing-safe string comparison
bool timingSafeCompare(const std::string& a, const std::string& b) {
    if (a.length() != b.length()) {
        return false;
    }
    
    volatile unsigned char result = 0;
    for (size_t i = 0; i < a.length(); ++i) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

// Create user with secure password storage
void createUser(const std::string& username, const std::string& password, bool isAdmin) {
    if (username.empty() || password.empty()) {
        throw std::invalid_argument("Username and password cannot be empty");
    }
    
    std::string salt = generateSalt();
    std::string passwordHash = hashPassword(password, salt);
    userDatabase[username] = std::make_unique<User>(username, passwordHash, salt, isAdmin);
}

// Authenticate user
bool authenticateUser(const std::string& username, const std::string& password) {
    auto it = userDatabase.find(username);
    if (it == userDatabase.end()) {
        hashPassword(password, generateSalt());
        return false;
    }
    
    std::string providedHash = hashPassword(password, it->second->salt);
    return timingSafeCompare(providedHash, it->second->passwordHash);
}

// Generate secure session token
std::string generateSessionToken() {
    unsigned char tokenBytes[32];
    if (RAND_bytes(tokenBytes, sizeof(tokenBytes)) != 1) {
        throw std::runtime_error("Failed to generate secure session token");
    }
    return bytesToHex(tokenBytes, sizeof(tokenBytes));
}

// Create session
std::string createSession(const std::string& username, bool isAdmin) {
    std::string sessionToken = generateSessionToken();
    activeSessions[sessionToken] = std::make_unique<SessionData>(username, isAdmin, time(nullptr));
    return sessionToken;
}

// Validate session and return redirect URL
std::string redirectUser(const std::string& sessionToken) {
    if (sessionToken.empty()) {
        return "/login";
    }
    
    // Sanitize session token - only allow hex characters
    for (char c : sessionToken) {
        if (!std::isxdigit(static_cast<unsigned char>(c))) {
            return "/login";
        }
    }
    
    // Clean expired sessions
    auto it = activeSessions.begin();
    while (it != activeSessions.end()) {
        if (it->second->isExpired()) {
            it = activeSessions.erase(it);
        } else {
            ++it;
        }
    }
    
    auto sessionIt = activeSessions.find(sessionToken);
    if (sessionIt == activeSessions.end() || sessionIt->second->isExpired()) {
        return "/login";
    }
    
    if (sessionIt->second->isAdmin) {
        return "/admin";
    }
    
    return "/user";
}

// Login method
std::string login(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        return "";
    }
    
    // Sanitize username
    for (char c : username) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') {
            return "";
        }
    }
    
    if (username.length() > 50) {
        return "";
    }
    
    if (authenticateUser(username, password)) {
        auto userIt = userDatabase.find(username);
        return createSession(username, userIt->second->isAdmin);
    }
    
    return "";
}

int main() {
    // Initialize OpenSSL
    OPENSSL_init_crypto(OPENSSL_INIT_LOAD_CRYPTO_STRINGS, nullptr);
    
    // Create test users
    createUser("regularuser", "SecurePass123!", false);
    createUser("adminuser", "AdminPass456!", true);
    
    // Test case 1: No session token
    std::cout << "Test 1 - No session: " << redirectUser("") << std::endl;
    
    // Test case 2: Invalid session token
    std::cout << "Test 2 - Invalid token: " << redirectUser("invalid_token_123") << std::endl;
    
    // Test case 3: Regular user logged in
    std::string regularToken = login("regularuser", "SecurePass123!");
    if (!regularToken.empty()) {
        std::cout << "Test 3 - Regular user: " << redirectUser(regularToken) << std::endl;
    }
    
    // Test case 4: Admin user logged in
    std::string adminToken = login("adminuser", "AdminPass456!");
    if (!adminToken.empty()) {
        std::cout << "Test 4 - Admin user: " << redirectUser(adminToken) << std::endl;
    }
    
    // Test case 5: Expired session
    std::string expiredToken = login("regularuser", "SecurePass123!");
    if (!expiredToken.empty()) {
        activeSessions[expiredToken]->createdTime = time(nullptr) - (SESSION_TIMEOUT_SECONDS + 1);
        std::cout << "Test 5 - Expired session: " << redirectUser(expiredToken) << std::endl;
    }
    
    return 0;
}
