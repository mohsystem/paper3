
#include <iostream>
#include <string>
#include <map>
#include <random>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <algorithm>

class Task135 {
private:
    static const int TOKEN_LENGTH = 32;
    static const long long TOKEN_EXPIRY_MS = 30 * 60 * 1000; // 30 minutes

    struct TokenData {
        std::string token;
        long long createdAt;
        std::string sessionId;

        TokenData() : createdAt(0) {}
        
        TokenData(const std::string& tok, long long created, const std::string& sid)
            : token(tok), createdAt(created), sessionId(sid) {}

        bool isExpired() const {
            auto now = std::chrono::system_clock::now();
            auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
            return now_ms - createdAt > TOKEN_EXPIRY_MS;
        }
    };

    std::map<std::string, TokenData> tokenStore;
    std::random_device rd;
    std::mt19937 gen;

    std::string base64_encode(const unsigned char* data, size_t len) {
        static const char base64_chars[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
        std::string encoded;
        int i = 0;
        unsigned char array_3[3];
        unsigned char array_4[4];

        while (len--) {
            array_3[i++] = *(data++);
            if (i == 3) {
                array_4[0] = (array_3[0] & 0xfc) >> 2;
                array_4[1] = ((array_3[0] & 0x03) << 4) + ((array_3[1] & 0xf0) >> 4);
                array_4[2] = ((array_3[1] & 0x0f) << 2) + ((array_3[2] & 0xc0) >> 6);
                array_4[3] = array_3[2] & 0x3f;

                for (i = 0; i < 4; i++)
                    encoded += base64_chars[array_4[i]];
                i = 0;
            }
        }

        if (i) {
            for (int j = i; j < 3; j++)
                array_3[j] = '\\0';

            array_4[0] = (array_3[0] & 0xfc) >> 2;
            array_4[1] = ((array_3[0] & 0x03) << 4) + ((array_3[1] & 0xf0) >> 4);
            array_4[2] = ((array_3[1] & 0x0f) << 2) + ((array_3[2] & 0xc0) >> 6);

            for (int j = 0; j < i + 1; j++)
                encoded += base64_chars[array_4[j]];
        }

        return encoded;
    }

public:
    Task135() : gen(rd()) {}

    std::string generateToken(const std::string& sessionId) {
        unsigned char randomBytes[TOKEN_LENGTH];
        std::uniform_int_distribution<> dis(0, 255);
        
        for (int i = 0; i < TOKEN_LENGTH; i++) {
            randomBytes[i] = static_cast<unsigned char>(dis(gen));
        }
        
        std::string token = base64_encode(randomBytes, TOKEN_LENGTH);
        
        auto now = std::chrono::system_clock::now();
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        
        TokenData tokenData(token, now_ms, sessionId);
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

        if (it->second.isExpired()) {
            tokenStore.erase(it);
            return false;
        }

        return it->second.token == token;
    }

    void invalidateToken(const std::string& sessionId) {
        tokenStore.erase(sessionId);
    }

    void cleanExpiredTokens() {
        auto it = tokenStore.begin();
        while (it != tokenStore.end()) {
            if (it->second.isExpired()) {
                it = tokenStore.erase(it);
            } else {
                ++it;
            }
        }
    }

    int getActiveTokenCount() {
        cleanExpiredTokens();
        return tokenStore.size();
    }
};

int main() {
    Task135 csrfProtection;

    // Test Case 1: Generate and validate token
    std::cout << "Test Case 1: Generate and validate token" << std::endl;
    std::string session1 = "session_123";
    std::string token1 = csrfProtection.generateToken(session1);
    std::cout << "Generated token: " << token1 << std::endl;
    std::cout << "Token valid: " << (csrfProtection.validateToken(session1, token1) ? "true" : "false") << std::endl;
    std::cout << std::endl;

    // Test Case 2: Invalid token validation
    std::cout << "Test Case 2: Invalid token validation" << std::endl;
    std::string session2 = "session_456";
    std::string token2 = csrfProtection.generateToken(session2);
    std::cout << "Valid token check: " << (csrfProtection.validateToken(session2, token2) ? "true" : "false") << std::endl;
    std::cout << "Invalid token check: " << (csrfProtection.validateToken(session2, "invalid_token") ? "true" : "false") << std::endl;
    std::cout << std::endl;

    // Test Case 3: Token invalidation
    std::cout << "Test Case 3: Token invalidation" << std::endl;
    std::string session3 = "session_789";
    std::string token3 = csrfProtection.generateToken(session3);
    std::cout << "Before invalidation: " << (csrfProtection.validateToken(session3, token3) ? "true" : "false") << std::endl;
    csrfProtection.invalidateToken(session3);
    std::cout << "After invalidation: " << (csrfProtection.validateToken(session3, token3) ? "true" : "false") << std::endl;
    std::cout << std::endl;

    // Test Case 4: Multiple sessions
    std::cout << "Test Case 4: Multiple sessions" << std::endl;
    std::string sessionA = "session_A";
    std::string sessionB = "session_B";
    std::string tokenA = csrfProtection.generateToken(sessionA);
    std::string tokenB = csrfProtection.generateToken(sessionB);
    std::cout << "Session A valid: " << (csrfProtection.validateToken(sessionA, tokenA) ? "true" : "false") << std::endl;
    std::cout << "Session B valid: " << (csrfProtection.validateToken(sessionB, tokenB) ? "true" : "false") << std::endl;
    std::cout << "Cross-session invalid: " << (csrfProtection.validateToken(sessionA, tokenB) ? "true" : "false") << std::endl;
    std::cout << "Active tokens: " << csrfProtection.getActiveTokenCount() << std::endl;
    std::cout << std::endl;

    // Test Case 5: Empty string handling
    std::cout << "Test Case 5: Empty string handling" << std::endl;
    std::cout << "Empty session: " << (csrfProtection.validateToken("", "token") ? "true" : "false") << std::endl;
    std::cout << "Empty token: " << (csrfProtection.validateToken("session", "") ? "true" : "false") << std::endl;
    std::cout << "Both empty: " << (csrfProtection.validateToken("", "") ? "true" : "false") << std::endl;

    return 0;
}
