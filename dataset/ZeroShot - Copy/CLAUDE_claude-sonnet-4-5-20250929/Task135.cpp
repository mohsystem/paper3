
#include <iostream>
#include <string>
#include <map>
#include <random>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>

class Task135 {
private:
    static const int TOKEN_LENGTH = 32;
    static const long TOKEN_EXPIRY_TIME = 30 * 60; // 30 minutes in seconds
    
    struct TokenData {
        std::string token;
        time_t timestamp;
    };
    
    std::map<std::string, TokenData> tokenStore;
    std::random_device rd;
    std::mt19937 gen;
    
    std::string base64_encode(const unsigned char* data, size_t len) {
        static const char* base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
        
        std::string result;
        int val = 0;
        int valb = -6;
        
        for (size_t i = 0; i < len; i++) {
            val = (val << 8) + data[i];
            valb += 8;
            while (valb >= 0) {
                result.push_back(base64_chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        
        if (valb > -6) {
            result.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
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
    
    std::string generateToken(const std::string& sessionId) {
        unsigned char tokenBytes[TOKEN_LENGTH];
        std::uniform_int_distribution<> dis(0, 255);
        
        for (int i = 0; i < TOKEN_LENGTH; i++) {
            tokenBytes[i] = static_cast<unsigned char>(dis(gen));
        }
        
        std::string token = base64_encode(tokenBytes, TOKEN_LENGTH);
        
        TokenData tokenData;
        tokenData.token = token;
        tokenData.timestamp = time(nullptr);
        
        tokenStore[sessionId] = tokenData;
        
        return token;
    }
    
    bool validateToken(const std::string& sessionId, const std::string& token) {
        if (sessionId.empty() || token.empty()) {
            return false;
        }
        
        auto it = tokenStore.find(sessionId);
        if (it == tokenStore.end()) {
            return false;
        }
        
        TokenData& tokenData = it->second;
        
        // Check if token has expired
        time_t currentTime = time(nullptr);
        if (currentTime - tokenData.timestamp > TOKEN_EXPIRY_TIME) {
            tokenStore.erase(sessionId);
            return false;
        }
        
        return constantTimeEquals(tokenData.token, token);
    }
    
    void invalidateToken(const std::string& sessionId) {
        tokenStore.erase(sessionId);
    }
    
    void cleanupExpiredTokens() {
        time_t currentTime = time(nullptr);
        auto it = tokenStore.begin();
        
        while (it != tokenStore.end()) {
            if (currentTime - it->second.timestamp > TOKEN_EXPIRY_TIME) {
                it = tokenStore.erase(it);
            } else {
                ++it;
            }
        }
    }
};

int main() {
    Task135 csrfProtection;
    
    std::cout << "=== CSRF Protection Test Cases ===" << std::endl << std::endl;
    
    // Test Case 1: Generate and validate valid token
    std::cout << "Test Case 1: Generate and validate valid token" << std::endl;
    std::string session1 = "session123";
    std::string token1 = csrfProtection.generateToken(session1);
    std::cout << "Generated Token: " << token1 << std::endl;
    std::cout << "Validation Result: " << (csrfProtection.validateToken(session1, token1) ? "true" : "false") << std::endl;
    std::cout << std::endl;
    
    // Test Case 2: Validate with wrong token
    std::cout << "Test Case 2: Validate with wrong token" << std::endl;
    std::string session2 = "session456";
    std::string token2 = csrfProtection.generateToken(session2);
    std::string wrongToken = "wrongToken123";
    std::cout << "Generated Token: " << token2 << std::endl;
    std::cout << "Wrong Token: " << wrongToken << std::endl;
    std::cout << "Validation Result: " << (csrfProtection.validateToken(session2, wrongToken) ? "true" : "false") << std::endl;
    std::cout << std::endl;
    
    // Test Case 3: Validate with non-existent session
    std::cout << "Test Case 3: Validate with non-existent session" << std::endl;
    std::string nonExistentSession = "nonExistent";
    std::string token3 = "someToken";
    std::cout << "Validation Result: " << (csrfProtection.validateToken(nonExistentSession, token3) ? "true" : "false") << std::endl;
    std::cout << std::endl;
    
    // Test Case 4: Invalidate token and try to use it
    std::cout << "Test Case 4: Invalidate token and try to use it" << std::endl;
    std::string session4 = "session789";
    std::string token4 = csrfProtection.generateToken(session4);
    std::cout << "Generated Token: " << token4 << std::endl;
    std::cout << "Validation Before Invalidation: " << (csrfProtection.validateToken(session4, token4) ? "true" : "false") << std::endl;
    csrfProtection.invalidateToken(session4);
    std::cout << "Validation After Invalidation: " << (csrfProtection.validateToken(session4, token4) ? "true" : "false") << std::endl;
    std::cout << std::endl;
    
    // Test Case 5: Multiple sessions with different tokens
    std::cout << "Test Case 5: Multiple sessions with different tokens" << std::endl;
    std::string sessionA = "sessionA";
    std::string sessionB = "sessionB";
    std::string tokenA = csrfProtection.generateToken(sessionA);
    std::string tokenB = csrfProtection.generateToken(sessionB);
    std::cout << "Session A Token: " << tokenA << std::endl;
    std::cout << "Session B Token: " << tokenB << std::endl;
    std::cout << "Validate A with A's token: " << (csrfProtection.validateToken(sessionA, tokenA) ? "true" : "false") << std::endl;
    std::cout << "Validate B with B's token: " << (csrfProtection.validateToken(sessionB, tokenB) ? "true" : "false") << std::endl;
    std::cout << "Validate A with B's token: " << (csrfProtection.validateToken(sessionA, tokenB) ? "true" : "false") << std::endl;
    std::cout << "Validate B with A's token: " << (csrfProtection.validateToken(sessionB, tokenA) ? "true" : "false") << std::endl;
    
    return 0;
}
