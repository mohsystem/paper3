
#include <iostream>
#include <string>
#include <map>
#include <random>
#include <ctime>
#include <stdexcept>
#include <cstring>
#include <sstream>
#include <iomanip>

class Task135 {
private:
    static const int TOKEN_LENGTH = 32;
    static const long TOKEN_EXPIRY_SECONDS = 3600;
    
    struct TokenData {
        std::string token;
        time_t expiryTime;
        std::string sessionId;
        
        TokenData() : expiryTime(0) {}
        TokenData(const std::string& t, time_t e, const std::string& s)
            : token(t), expiryTime(e), sessionId(s) {}
    };
    
    std::map<std::string, TokenData> tokenStore;
    std::random_device rd;
    std::mt19937 gen;
    
    std::string base64Encode(const unsigned char* data, size_t length) {
        const char* base64Chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
        std::string result;
        
        for (size_t i = 0; i < length; i += 3) {
            unsigned int val = data[i] << 16;
            if (i + 1 < length) val |= data[i + 1] << 8;
            if (i + 2 < length) val |= data[i + 2];
            
            result += base64Chars[(val >> 18) & 0x3F];
            result += base64Chars[(val >> 12) & 0x3F];
            result += (i + 1 < length) ? base64Chars[(val >> 6) & 0x3F] : '=';
            result += (i + 2 < length) ? base64Chars[val & 0x3F] : '=';
        }
        
        // Remove padding for URL-safe encoding
        while (!result.empty() && result.back() == '=') {
            result.pop_back();
        }
        
        return result;
    }
    
    bool constantTimeEquals(const std::string& a, const std::string& b) {
        if (a.length() != b.length()) {
            return false;
        }
        
        int result = 0;
        for (size_t i = 0; i < a.length(); i++) {
            result |= a[i] ^ b[i];
        }
        
        return result == 0;
    }
    
public:
    Task135() : gen(rd()) {}
    
    std::string generateCSRFToken(const std::string& sessionId) {
        if (sessionId.empty() || sessionId.find_first_not_of(' ') == std::string::npos) {
            throw std::invalid_argument("Session ID cannot be null or empty");
        }
        
        // Generate cryptographically secure random bytes
        unsigned char tokenBytes[TOKEN_LENGTH];
        std::uniform_int_distribution<> dis(0, 255);
        
        for (int i = 0; i < TOKEN_LENGTH; i++) {
            tokenBytes[i] = static_cast<unsigned char>(dis(gen));
        }
        
        std::string token = base64Encode(tokenBytes, TOKEN_LENGTH);
        
        time_t expiryTime = time(nullptr) + TOKEN_EXPIRY_SECONDS;
        tokenStore[sessionId] = TokenData(token, expiryTime, sessionId);
        
        return token;
    }
    
    bool validateCSRFToken(const std::string& sessionId, const std::string& token) {
        if (sessionId.empty() || sessionId.find_first_not_of(' ') == std::string::npos) {
            return false;
        }
        
        if (token.empty() || token.find_first_not_of(' ') == std::string::npos) {
            return false;
        }
        
        auto it = tokenStore.find(sessionId);
        if (it == tokenStore.end()) {
            return false;
        }
        
        TokenData& storedData = it->second;
        
        time_t currentTime = time(nullptr);
        if (currentTime > storedData.expiryTime) {
            tokenStore.erase(sessionId);
            return false;
        }
        
        bool isValid = constantTimeEquals(storedData.token, token);
        
        if (isValid) {
            tokenStore.erase(sessionId);
        }
        
        return isValid;
    }
    
    void invalidateToken(const std::string& sessionId) {
        if (!sessionId.empty()) {
            tokenStore.erase(sessionId);
        }
    }
};

int main() {
    std::cout << "CSRF Protection Test Cases:" << std::endl;
    std::cout << "===========================" << std::endl << std::endl;
    
    Task135 csrf;
    
    // Test Case 1: Valid token generation and validation
    std::cout << "Test Case 1: Valid token generation and validation" << std::endl;
    std::string session1 = "user_session_123";
    std::string token1 = csrf.generateCSRFToken(session1);
    std::cout << "Generated Token: " << token1 << std::endl;
    bool result1 = csrf.validateCSRFToken(session1, token1);
    std::cout << "Validation Result: " << std::boolalpha << result1 << std::endl;
    std::cout << "Expected: true" << std::endl << std::endl;
    
    // Test Case 2: Invalid token validation
    std::cout << "Test Case 2: Invalid token validation" << std::endl;
    std::string session2 = "user_session_456";
    std::string token2 = csrf.generateCSRFToken(session2);
    bool result2 = csrf.validateCSRFToken(session2, "invalid_token_xyz");
    std::cout << "Validation Result: " << std::boolalpha << result2 << std::endl;
    std::cout << "Expected: false" << std::endl << std::endl;
    
    // Test Case 3: Token reuse prevention
    std::cout << "Test Case 3: Token reuse prevention" << std::endl;
    std::string session3 = "user_session_789";
    std::string token3 = csrf.generateCSRFToken(session3);
    csrf.validateCSRFToken(session3, token3);
    bool result3 = csrf.validateCSRFToken(session3, token3);
    std::cout << "Second Validation Result: " << std::boolalpha << result3 << std::endl;
    std::cout << "Expected: false (token already used)" << std::endl << std::endl;
    
    // Test Case 4: Session mismatch
    std::cout << "Test Case 4: Session mismatch" << std::endl;
    std::string session4a = "user_session_aaa";
    std::string session4b = "user_session_bbb";
    std::string token4 = csrf.generateCSRFToken(session4a);
    bool result4 = csrf.validateCSRFToken(session4b, token4);
    std::cout << "Validation Result: " << std::boolalpha << result4 << std::endl;
    std::cout << "Expected: false" << std::endl << std::endl;
    
    // Test Case 5: Empty input validation
    std::cout << "Test Case 5: Empty input validation" << std::endl;
    bool result5a = csrf.validateCSRFToken("", "some_token");
    bool result5b = csrf.validateCSRFToken("session", "");
    std::cout << "Empty session validation: " << std::boolalpha << result5a << std::endl;
    std::cout << "Empty token validation: " << std::boolalpha << result5b << std::endl;
    std::cout << "Expected: false, false" << std::endl << std::endl;
    
    return 0;
}
