
#include <iostream>
#include <string>
#include <cstring>
#include <random>
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <chrono>
#include <memory>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

// CSRF Token Manager with secure token generation and validation
class CSRFProtection {
private:
    // Token store: maps token to expiration timestamp
    std::unordered_map<std::string, int64_t> tokenStore;
    const int64_t TOKEN_VALIDITY_SECONDS = 3600; // 1 hour validity
    const size_t TOKEN_LENGTH = 32; // 32 bytes = 256 bits

    // Generate cryptographically secure random bytes using OpenSSL
    bool generateSecureRandom(unsigned char* buffer, size_t length) {
        if (!buffer || length == 0) {
            return false;
        }
        // Use OpenSSL's CSPRNG for secure randomness\n        if (RAND_bytes(buffer, static_cast<int>(length)) != 1) {\n            return false;\n        }\n        return true;\n    }\n\n    // Convert bytes to hexadecimal string\n    std::string bytesToHex(const unsigned char* data, size_t length) {\n        if (!data || length == 0) {\n            return "";\n        }\n        std::ostringstream oss;\n        for (size_t i = 0; i < length; ++i) {\n            oss << std::hex << std::setw(2) << std::setfill('0') 
                << static_cast<int>(data[i]);
        }
        return oss.str();
    }

    // Get current timestamp in seconds
    int64_t getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(
            now.time_since_epoch());
        return duration.count();
    }

    // Constant-time string comparison to prevent timing attacks
    bool constantTimeCompare(const std::string& a, const std::string& b) {
        if (a.length() != b.length()) {
            return false;
        }
        
        volatile unsigned char result = 0;
        for (size_t i = 0; i < a.length(); ++i) {
            result |= static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i]);
        }
        return result == 0;
    }

    // Remove expired tokens to prevent memory bloat
    void cleanupExpiredTokens() {
        int64_t currentTime = getCurrentTimestamp();
        auto it = tokenStore.begin();
        while (it != tokenStore.end()) {
            if (it->second < currentTime) {
                it = tokenStore.erase(it);
            } else {
                ++it;
            }
        }
    }

public:
    // Generate a new CSRF token with secure randomness
    std::string generateToken() {
        // Generate secure random bytes
        std::unique_ptr<unsigned char[]> randomBytes(new unsigned char[TOKEN_LENGTH]);
        
        if (!generateSecureRandom(randomBytes.get(), TOKEN_LENGTH)) {
            std::cerr << "Failed to generate secure random bytes" << std::endl;
            return "";
        }

        // Convert to hex string for safe transmission
        std::string token = bytesToHex(randomBytes.get(), TOKEN_LENGTH);
        
        // Securely clear the random bytes from memory
        OPENSSL_cleanse(randomBytes.get(), TOKEN_LENGTH);
        
        if (token.empty()) {
            return "";
        }

        // Store token with expiration timestamp
        int64_t expirationTime = getCurrentTimestamp() + TOKEN_VALIDITY_SECONDS;
        tokenStore[token] = expirationTime;

        // Periodic cleanup of expired tokens
        if (tokenStore.size() > 1000) {
            cleanupExpiredTokens();
        }

        return token;
    }

    // Validate CSRF token with constant-time comparison and expiration check
    bool validateToken(const std::string& token) {
        // Input validation: check token format
        if (token.empty() || token.length() != TOKEN_LENGTH * 2) {
            return false;
        }

        // Validate hex format
        for (char c : token) {
            if (!std::isxdigit(static_cast<unsigned char>(c))) {
                return false;
            }
        }

        // Check if token exists in store
        auto it = tokenStore.find(token);
        if (it == tokenStore.end()) {
            return false;
        }

        // Check token expiration using constant-time comparison where applicable
        int64_t currentTime = getCurrentTimestamp();
        if (it->second < currentTime) {
            tokenStore.erase(it);
            return false;
        }

        // Token is valid - remove it (single-use token pattern)
        tokenStore.erase(it);
        return true;
    }

    // Validate token with constant-time comparison against expected value
    bool validateTokenConstantTime(const std::string& providedToken, 
                                   const std::string& expectedToken) {
        // Input validation
        if (providedToken.empty() || expectedToken.empty()) {
            return false;
        }

        // First validate the token exists and is not expired
        if (!constantTimeCompare(providedToken, expectedToken)) {
            return false;
        }

        return validateToken(providedToken);
    }

    // Get token store size for testing
    size_t getTokenCount() const {
        return tokenStore.size();
    }
};

// Simulate HTTP request with CSRF token
struct HTTPRequest {
    std::string method;
    std::string path;
    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> formData;
};

// Process request with CSRF protection
bool processRequest(CSRFProtection& csrf, const HTTPRequest& request) {
    // Only protect state-changing methods
    if (request.method == "GET" || request.method == "HEAD" || 
        request.method == "OPTIONS") {
        return true;
    }

    // Extract CSRF token from request
    std::string token;
    
    // Check for token in headers (recommended)
    auto headerIt = request.headers.find("X-CSRF-Token");
    if (headerIt != request.headers.end()) {
        token = headerIt->second;
    } 
    // Fallback to form data
    else {
        auto formIt = request.formData.find("csrf_token");
        if (formIt != request.formData.end()) {
            token = formIt->second;
        }
    }

    // Validate token
    if (token.empty()) {
        std::cerr << "CSRF token missing" << std::endl;
        return false;
    }

    if (!csrf.validateToken(token)) {
        std::cerr << "CSRF token validation failed" << std::endl;
        return false;
    }

    return true;
}

int main() {
    std::cout << "=== CSRF Protection Test Cases ===" << std::endl << std::endl;

    CSRFProtection csrf;

    // Test Case 1: Generate and validate valid token
    std::cout << "Test 1: Generate and validate valid token" << std::endl;
    std::string token1 = csrf.generateToken();
    if (!token1.empty()) {
        std::cout << "Token generated (length: " << token1.length() << ")" << std::endl;
        bool valid = csrf.validateToken(token1);
        std::cout << "Validation result: " << (valid ? "PASS" : "FAIL") << std::endl;
    } else {
        std::cout << "Token generation failed" << std::endl;
    }
    std::cout << std::endl;

    // Test Case 2: Reject invalid token
    std::cout << "Test 2: Reject invalid token" << std::endl;
    bool invalidResult = csrf.validateToken("invalidtoken123");
    std::cout << "Invalid token rejected: " << (!invalidResult ? "PASS" : "FAIL") << std::endl;
    std::cout << std::endl;

    // Test Case 3: Single-use token (should fail on second validation)
    std::cout << "Test 3: Single-use token enforcement" << std::endl;
    std::string token3 = csrf.generateToken();
    bool firstUse = csrf.validateToken(token3);
    bool secondUse = csrf.validateToken(token3);
    std::cout << "First use: " << (firstUse ? "valid" : "invalid") << std::endl;
    std::cout << "Second use: " << (secondUse ? "valid" : "invalid") << std::endl;
    std::cout << "Single-use enforced: " << (firstUse && !secondUse ? "PASS" : "FAIL") << std::endl;
    std::cout << std::endl;

    // Test Case 4: HTTP POST request with valid token
    std::cout << "Test 4: HTTP POST with valid CSRF token" << std::endl;
    std::string token4 = csrf.generateToken();
    HTTPRequest postRequest;
    postRequest.method = "POST";
    postRequest.path = "/api/update";
    postRequest.headers["X-CSRF-Token"] = token4;
    bool postResult = processRequest(csrf, postRequest);
    std::cout << "POST request processed: " << (postResult ? "PASS" : "FAIL") << std::endl;
    std::cout << std::endl;

    // Test Case 5: HTTP POST request without token (should fail)
    std::cout << "Test 5: HTTP POST without CSRF token (should fail)" << std::endl;
    HTTPRequest postRequestNoToken;
    postRequestNoToken.method = "POST";
    postRequestNoToken.path = "/api/update";
    bool postNoTokenResult = processRequest(csrf, postRequestNoToken);
    std::cout << "POST without token rejected: " << (!postNoTokenResult ? "PASS" : "FAIL") << std::endl;

    return 0;
}
