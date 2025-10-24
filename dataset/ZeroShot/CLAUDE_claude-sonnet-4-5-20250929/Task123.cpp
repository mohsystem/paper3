
#include <iostream>
#include <string>
#include <map>
#include <ctime>
#include <random>
#include <sstream>
#include <iomanip>
#include <cstring>

class Task123 {
private:
    static const int SESSION_TIMEOUT_MINUTES = 30;
    static const int TOKEN_LENGTH = 32;
    
    struct Session {
        std::string userId;
        std::string token;
        time_t createdAt;
        time_t lastAccessedAt;
        
        Session() : createdAt(0), lastAccessedAt(0) {}
        
        Session(const std::string& uid, const std::string& tok) 
            : userId(uid), token(tok) {
            createdAt = time(nullptr);
            lastAccessedAt = time(nullptr);
        }
        
        bool isExpired() const {
            time_t now = time(nullptr);
            double seconds = difftime(now, lastAccessedAt);
            return (seconds / 60) > SESSION_TIMEOUT_MINUTES;
        }
    };
    
    std::map<std::string, Session> sessions;
    std::random_device rd;
    std::mt19937 gen;
    
    std::string base64_encode(const unsigned char* data, size_t len) {
        static const char base64_chars[] = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
        
        std::string result;
        int i = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];
        
        while (len--) {
            char_array_3[i++] = *(data++);
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;
                
                for (i = 0; i < 4; i++)
                    result += base64_chars[char_array_4[i]];
                i = 0;
            }
        }
        
        if (i) {
            for (int j = i; j < 3; j++)
                char_array_3[j] = '\\0';
            
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            
            for (int j = 0; j < i + 1; j++)
                result += base64_chars[char_array_4[j]];
        }
        
        return result;
    }
    
public:
    Task123() : gen(rd()) {}
    
    std::string createSession(const std::string& userId) {
        if (userId.empty()) {
            return "";
        }
        
        unsigned char tokenBytes[TOKEN_LENGTH];
        std::uniform_int_distribution<> dis(0, 255);
        for (int i = 0; i < TOKEN_LENGTH; i++) {
            tokenBytes[i] = dis(gen);
        }
        
        std::string token = base64_encode(tokenBytes, TOKEN_LENGTH);
        Session session(userId, token);
        sessions[token] = session;
        
        return token;
    }
    
    bool validateSession(const std::string& token) {
        if (token.empty()) {
            return false;
        }
        
        auto it = sessions.find(token);
        if (it == sessions.end()) {
            return false;
        }
        
        if (it->second.isExpired()) {
            sessions.erase(it);
            return false;
        }
        
        it->second.lastAccessedAt = time(nullptr);
        return true;
    }
    
    std::string getSessionUser(const std::string& token) {
        if (!validateSession(token)) {
            return "";
        }
        
        auto it = sessions.find(token);
        return (it != sessions.end()) ? it->second.userId : "";
    }
    
    bool destroySession(const std::string& token) {
        if (token.empty()) {
            return false;
        }
        
        return sessions.erase(token) > 0;
    }
    
    void cleanupExpiredSessions() {
        auto it = sessions.begin();
        while (it != sessions.end()) {
            if (it->second.isExpired()) {
                it = sessions.erase(it);
            } else {
                ++it;
            }
        }
    }
};

int main() {
    Task123 sessionManager;
    
    // Test Case 1: Create a session for a user
    std::cout << "Test Case 1: Create Session" << std::endl;
    std::string token1 = sessionManager.createSession("user123");
    std::cout << "Token created: " << (!token1.empty() ? "Yes" : "No") << std::endl;
    std::cout << std::endl;
    
    // Test Case 2: Validate an existing session
    std::cout << "Test Case 2: Validate Session" << std::endl;
    bool isValid = sessionManager.validateSession(token1);
    std::cout << "Session valid: " << (isValid ? "true" : "false") << std::endl;
    std::cout << std::endl;
    
    // Test Case 3: Get user from session
    std::cout << "Test Case 3: Get Session User" << std::endl;
    std::string userId = sessionManager.getSessionUser(token1);
    std::cout << "User ID: " << userId << std::endl;
    std::cout << std::endl;
    
    // Test Case 4: Destroy a session
    std::cout << "Test Case 4: Destroy Session" << std::endl;
    bool destroyed = sessionManager.destroySession(token1);
    std::cout << "Session destroyed: " << (destroyed ? "true" : "false") << std::endl;
    bool validAfterDestroy = sessionManager.validateSession(token1);
    std::cout << "Session valid after destroy: " << (validAfterDestroy ? "true" : "false") << std::endl;
    std::cout << std::endl;
    
    // Test Case 5: Invalid session token
    std::cout << "Test Case 5: Invalid Session Token" << std::endl;
    bool invalidToken = sessionManager.validateSession("invalid_token_xyz");
    std::cout << "Invalid token validation: " << (invalidToken ? "true" : "false") << std::endl;
    std::string userFromInvalid = sessionManager.getSessionUser("invalid_token_xyz");
    std::cout << "User from invalid token: " << (userFromInvalid.empty() ? "NULL" : userFromInvalid) << std::endl;
    
    return 0;
}
