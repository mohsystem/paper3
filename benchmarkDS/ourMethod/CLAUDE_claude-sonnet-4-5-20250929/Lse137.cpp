
#include <iostream>
#include <string>
#include <map>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <random>
#include <iomanip>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

// Constants for input validation
constexpr size_t MAX_USERNAME_LENGTH = 50;
constexpr size_t MAX_PASSWORD_LENGTH = 128;
constexpr size_t MAX_SESSION_LENGTH = 256;
constexpr size_t MAX_QUERY_LENGTH = 1024;
constexpr size_t SALT_SIZE = 16;
constexpr size_t SESSION_TOKEN_SIZE = 32;

// User data structure
struct UserData {
    std::string passwordHash;
    std::string salt;
    bool isModerator;
    
    UserData() : isModerator(false) {}
    UserData(const std::string& hash, const std::string& s, bool mod)
        : passwordHash(hash), salt(s), isModerator(mod) {}
};

// Simulated database and session store
static std::map<std::string, UserData> userDatabase;
static std::map<std::string, std::string> sessionStore;

// Base64 encoding helper
std::string base64_encode(const unsigned char* buffer, size_t length) {
    static const char base64_chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    std::string encoded;
    encoded.reserve(((length + 2) / 3) * 4);
    
    for (size_t i = 0; i < length; i += 3) {
        unsigned int val = buffer[i] << 16;
        if (i + 1 < length) val |= buffer[i + 1] << 8;
        if (i + 2 < length) val |= buffer[i + 2];
        
        encoded.push_back(base64_chars[(val >> 18) & 0x3F]);
        encoded.push_back(base64_chars[(val >> 12) & 0x3F]);
        encoded.push_back(i + 1 < length ? base64_chars[(val >> 6) & 0x3F] : '=');
        encoded.push_back(i + 2 < length ? base64_chars[val & 0x3F] : '=');
    }
    
    return encoded;
}

// Base64 decoding helper
std::string base64_decode(const std::string& encoded) {
    static const unsigned char base64_table[256] = {
        ['A'] = 0, ['B'] = 1, ['C'] = 2, ['D'] = 3, ['E'] = 4, ['F'] = 5,
        ['G'] = 6, ['H'] = 7, ['I'] = 8, ['J'] = 9, ['K'] = 10, ['L'] = 11,
        ['M'] = 12, ['N'] = 13, ['O'] = 14, ['P'] = 15, ['Q'] = 16, ['R'] = 17,
        ['S'] = 18, ['T'] = 19, ['U'] = 20, ['V'] = 21, ['W'] = 22, ['X'] = 23,
        ['Y'] = 24, ['Z'] = 25, ['a'] = 26, ['b'] = 27, ['c'] = 28, ['d'] = 29,
        ['e'] = 30, ['f'] = 31, ['g'] = 32, ['h'] = 33, ['i'] = 34, ['j'] = 35,
        ['k'] = 36, ['l'] = 37, ['m'] = 38, ['n'] = 39, ['o'] = 40, ['p'] = 41,
        ['q'] = 42, ['r'] = 43, ['s'] = 44, ['t'] = 45, ['u'] = 46, ['v'] = 47,
        ['w'] = 48, ['x'] = 49, ['y'] = 50, ['z'] = 51, ['0'] = 52, ['1'] = 53,
        ['2'] = 54, ['3'] = 55, ['4'] = 56, ['5'] = 57, ['6'] = 58, ['7'] = 59,
        ['8'] = 60, ['9'] = 61, ['+'] = 62, ['/'] = 63
    };
    
    std::string decoded;
    size_t len = encoded.length();
    if (len == 0 || len % 4 != 0) return decoded;
    
    for (size_t i = 0; i < len; i += 4) {
        unsigned int val = (base64_table[static_cast<unsigned char>(encoded[i])] << 18) |
                          (base64_table[static_cast<unsigned char>(encoded[i + 1])] << 12) |
                          (base64_table[static_cast<unsigned char>(encoded[i + 2])] << 6) |
                          base64_table[static_cast<unsigned char>(encoded[i + 3])];
        
        decoded.push_back((val >> 16) & 0xFF);
        if (encoded[i + 2] != '=') decoded.push_back((val >> 8) & 0xFF);
        if (encoded[i + 3] != '=') decoded.push_back(val & 0xFF);
    }
    
    return decoded;
}

// Generate cryptographically secure random bytes using OpenSSL
std::string generateSecureRandom(size_t length) {
    unsigned char buffer[length];
    
    // Use OpenSSL's CSPRNG\n    if (RAND_bytes(buffer, length) != 1) {\n        std::cerr << "Failed to generate secure random bytes" << std::endl;\n        return "";\n    }\n    \n    return base64_encode(buffer, length);\n}\n\n// Secure password hashing with SHA-256 and salt\nstd::string hashPassword(const std::string& password, const std::string& salt) {\n    // Decode salt from base64\n    std::string saltBytes = base64_decode(salt);\n    \n    // Combine salt and password\n    std::string combined = saltBytes + password;\n    \n    // Hash with SHA-256 using OpenSSL EVP\n    unsigned char hash[SHA256_DIGEST_LENGTH];\n    SHA256(reinterpret_cast<const unsigned char*>(combined.c_str()), \n           combined.length(), hash);\n    \n    return base64_encode(hash, SHA256_DIGEST_LENGTH);\n}\n\n// Constant-time comparison to prevent timing attacks\nbool constantTimeCompare(const std::string& a, const std::string& b) {\n    if (a.length() != b.length()) {\n        return false;\n    }\n    \n    volatile unsigned char result = 0;\n    for (size_t i = 0; i < a.length(); ++i) {\n        result |= a[i] ^ b[i];\n    }\n    \n    return result == 0;\n}\n\n// Create secure session token\nstd::string createSession(const std::string& username) {\n    std::string sessionToken = generateSecureRandom(SESSION_TOKEN_SIZE);\n    sessionStore[username] = sessionToken;\n    return sessionToken;\n}\n\n// Validate session token\nbool validateSession(const std::string& username, const std::string& sessionToken) {\n    if (username.empty() || sessionToken.empty()) {\n        return false;\n    }\n    \n    auto it = sessionStore.find(username);\n    if (it == sessionStore.end()) {\n        return false;\n    }\n    \n    // Constant-time comparison\n    return constantTimeCompare(it->second, sessionToken);\n}\n\n// Add user with secure password hashing\nvoid addUser(const std::string& username, const std::string& password, bool isModerator) {\n    // Generate cryptographically secure salt\n    std::string salt = generateSecureRandom(SALT_SIZE);\n    std::string passwordHash = hashPassword(password, salt);\n    \n    userDatabase[username] = UserData(passwordHash, salt, isModerator);\n}\n\n// Validate username (alphanumeric + underscore only)\nbool isValidUsername(const std::string& username) {\n    if (username.empty() || username.length() > MAX_USERNAME_LENGTH) {\n        return false;\n    }\n    \n    for (char c : username) {\n        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') {\n            return false;\n        }\n    }\n    \n    return true;\n}\n\n// URL decode helper\nstd::string urlDecode(const std::string& encoded) {\n    std::string decoded;\n    decoded.reserve(encoded.length());\n    \n    for (size_t i = 0; i < encoded.length(); ++i) {\n        if (encoded[i] == '%' && i + 2 < encoded.length()) {\n            int value;\n            std::istringstream is(encoded.substr(i + 1, 2));\n            if (is >> std::hex >> value) {\n                decoded += static_cast<char>(value);\n                i += 2;\n            } else {\n                decoded += encoded[i];\n            }\n        } else if (encoded[i] == '+') {\n            decoded += ' ';\n        } else {\n            decoded += encoded[i];\n        }\n    }\n    \n    return decoded;\n}\n\n// Parse query string safely\nstd::map<std::string, std::string> parseQueryString(const std::string& query) {\n    std::map<std::string, std::string> params;\n    \n    if (query.empty()) {\n        return params;\n    }\n    \n    std::istringstream stream(query);\n    std::string pair;\n    size_t paramCount = 0;\n    \n    while (std::getline(stream, pair, '&')) {\n        // Limit number of parameters to prevent DoS\n        if (++paramCount > 10) {\n            break;\n        }\n        \n        size_t pos = pair.find('=');
        if (pos != std::string::npos && pos > 0 && pos < pair.length() - 1) {
            std::string key = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);
            
            // Only accept expected parameter names
            if (key == "username" || key == "password" || key == "session") {
                params[key] = value;
            }
        }
    }
    
    return params;
}

/**
 * Authenticates user and checks if they are a moderator
 * @param username - validated username (max 50 chars, alphanumeric + underscore)
 * @param password - password for authentication
 * @param session - session token for validation
 * @return true if user is authenticated and is a moderator, false otherwise
 */
bool auth_mod(const std::string& username, const std::string& password, 
              const std::string& session) {
    // Input validation - reject empty inputs
    if (username.empty() || password.empty() || session.empty()) {
        return false;
    }
    
    // Input validation - length checks
    if (username.length() > MAX_USERNAME_LENGTH || 
        password.length() > MAX_PASSWORD_LENGTH || 
        session.length() > MAX_SESSION_LENGTH) {
        return false;
    }
    
    // Input validation - sanitize username
    if (!isValidUsername(username)) {
        return false;
    }
    
    // Check if user exists
    auto it = userDatabase.find(username);
    if (it == userDatabase.end()) {
        return false;
    }
    
    const UserData& userData = it->second;
    
    // Validate session token
    if (!validateSession(username, session)) {
        return false;
    }
    
    // Verify password with constant-time comparison
    try {
        std::string providedHash = hashPassword(password, userData.salt);
        
        bool passwordValid = constantTimeCompare(userData.passwordHash, providedHash);
        
        if (!passwordValid) {
            return false;
        }
        
        // Return moderator status
        return userData.isModerator;
        
    } catch (...) {
        // Log error securely without exposing details
        std::cerr << "Authentication error occurred" << std::endl;
        return false;
    }
}

/**
 * Handles /check_mod endpoint
 * Extracts username, password, and session from URL parameters
 * Calls auth_mod() to verify moderator status
 */
bool check_mod(const std::string& queryString) {
    // Input validation - reject empty or excessively long query strings
    if (queryString.empty() || queryString.length() > MAX_QUERY_LENGTH) {
        return false;
    }
    
    try {
        // Parse query parameters safely
        std::map<std::string, std::string> params = parseQueryString(queryString);
        
        // Extract parameters with validation
        auto usernameIt = params.find("username");
        auto passwordIt = params.find("password");
        auto sessionIt = params.find("session");
        
        // All parameters are required
        if (usernameIt == params.end() || 
            passwordIt == params.end() || 
            sessionIt == params.end()) {
            return false;
        }
        
        // URL decode parameters
        std::string username = urlDecode(usernameIt->second);
        std::string password = urlDecode(passwordIt->second);
        std::string session = urlDecode(sessionIt->second);
        
        // Call authentication function
        return auth_mod(username, password, session);
        
    } catch (...) {
        // Generic error handling - do not leak internal details
        std::cerr << "Request processing error" << std::endl;
        return false;
    }
}

int main() {
    // Initialize OpenSSL
    OpenSSL_add_all_algorithms();
    
    // Initialize test database with secure password hashing
    addUser("admin", "AdminPass123!", true);
    addUser("moderator", "ModPass456!", true);
    addUser("user1", "UserPass789!", false);
    addUser("user2", "Pass2024Secure!", false);
    addUser("guest", "GuestPass321!", false);
    
    // Create sessions for test users
    std::string adminSession = createSession("admin");
    std::string modSession = createSession("moderator");
    std::string user1Session = createSession("user1");
    std::string user2Session = createSession("user2");
    std::string guestSession = createSession("guest");
    
    std::cout << "=== Test Cases for Moderator Check ===" << std::endl << std::endl;
    
    // Test Case 1: Valid moderator (admin)
    std::string query1 = "username=admin&password=AdminPass123!&session=" + adminSession;
    bool result1 = check_mod(query1);
    std::cout << "Test 1 - Valid moderator (admin): " << (result1 ? "PASS" : "FAIL") << std::endl;
    std::cout << "Expected: true, Got: " << std::boolalpha << result1 << std::endl << std::endl;
    
    // Test Case 2: Valid moderator (moderator user)
    std::string query2 = "username=moderator&password=ModPass456!&session=" + modSession;
    bool result2 = check_mod(query2);
    std::cout << "Test 2 - Valid moderator (moderator): " << (result2 ? "PASS" : "FAIL") << std::endl;
    std::cout << "Expected: true, Got: " << std::boolalpha << result2 << std::endl << std::endl;
    
    // Test Case 3: Valid user but not moderator
    std::string query3 = "username=user1&password=UserPass789!&session=" + user1Session;
    bool result3 = check_mod(query3);
    std::cout << "Test 3 - Valid user but not moderator: " << (!result3 ? "PASS" : "FAIL") << std::endl;
    std::cout << "Expected: false, Got: " << std::boolalpha << result3 << std::endl << std::endl;
    
    // Test Case 4: Invalid credentials
    std::string query4 = "username=admin&password=WrongPassword&session=" + adminSession;
    bool result4 = check_mod(query4);
    std::cout << "Test 4 - Invalid credentials: " << (!result4 ? "PASS" : "FAIL") << std::endl;
    std::cout << "Expected: false, Got: " << std::boolalpha << result4 << std::endl << std::endl;
    
    // Test Case 5: Invalid session token
    std::string query5 = "username=admin&password=AdminPass123!&session=InvalidSession123";
    bool result5 = check_mod(query5);
    std::cout << "Test 5 - Invalid session token: " << (!result5 ? "PASS" : "FAIL") << std::endl;
    std::cout << "Expected: false, Got: " << std::boolalpha << result5 << std::endl << std::endl;
    
    // Cleanup OpenSSL
    EVP_cleanup();
    
    return 0;
}
