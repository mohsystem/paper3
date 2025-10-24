
#include <iostream>
#include <string>
#include <map>
#include <random>
#include <chrono>
#include <thread>
#include <mutex>
#include <algorithm>
#include <cstring>

class Task123 {
private:
    static const int SESSION_TIMEOUT_SECONDS = 1800;
    static const int TOKEN_LENGTH = 32;
    
    struct Session {
        std::string userId;
        std::chrono::time_point<std::chrono::system_clock> createdAt;
        std::chrono::time_point<std::chrono::system_clock> lastAccessedAt;
        std::map<std::string, std::string> attributes;
        
        Session(const std::string& uid) {
            if (uid.empty()) {
                throw std::invalid_argument("User ID cannot be empty");
            }
            userId = sanitizeInput(uid);
            createdAt = std::chrono::system_clock::now();
            lastAccessedAt = std::chrono::system_clock::now();
        }
        
        bool isExpired() const {
            auto now = std::chrono::system_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - lastAccessedAt);
            return duration.count() > SESSION_TIMEOUT_SECONDS;
        }
        
        void updateLastAccessed() {
            lastAccessedAt = std::chrono::system_clock::now();
        }
        
        void setAttribute(const std::string& key, const std::string& value) {
            if (!key.empty() && !value.empty()) {
                attributes[sanitizeInput(key)] = sanitizeInput(value);
            }
        }
        
        std::string getAttribute(const std::string& key) const {
            std::string sanitizedKey = sanitizeInput(key);
            auto it = attributes.find(sanitizedKey);
            return it != attributes.end() ? it->second : "";
        }
        
        static std::string sanitizeInput(const std::string& input) {
            if (input.empty()) return "";
            std::string result;
            for (char c : input) {
                if (isalnum(c) || c == '@' || c == '.' || c == '_' || c == '-') {
                    result += c;
                }
                if (result.length() >= 256) break;
            }
            return result;
        }
    };
    
    std::map<std::string, Session> sessions;
    std::mutex sessionMutex;
    std::random_device rd;
    std::mt19937 gen;
    
    std::string generateSecureToken() {
        const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-_";
        std::uniform_int_distribution<> dis(0, sizeof(charset) - 2);
        
        std::string token;
        token.reserve(TOKEN_LENGTH);
        for (int i = 0; i < TOKEN_LENGTH; ++i) {
            token += charset[dis(gen)];
        }
        return token;
    }
    
    void cleanupExpiredSessions() {
        std::lock_guard<std::mutex> lock(sessionMutex);
        for (auto it = sessions.begin(); it != sessions.end(); ) {
            if (it->second.isExpired()) {
                it = sessions.erase(it);
            } else {
                ++it;
            }
        }
    }
    
public:
    Task123() : gen(rd()) {}
    
    std::string createSession(const std::string& userId) {
        try {
            if (userId.empty()) {
                return "";
            }
            
            std::string sessionToken = generateSecureToken();
            
            std::lock_guard<std::mutex> lock(sessionMutex);
            sessions.emplace(sessionToken, Session(userId));
            
            return sessionToken;
        } catch (...) {
            return "";
        }
    }
    
    bool validateSession(const std::string& sessionToken) {
        try {
            if (sessionToken.empty()) {
                return false;
            }
            
            std::lock_guard<std::mutex> lock(sessionMutex);
            auto it = sessions.find(sessionToken);
            if (it == sessions.end() || it->second.isExpired()) {
                if (it != sessions.end()) {
                    sessions.erase(it);
                }
                return false;
            }
            
            it->second.updateLastAccessed();
            return true;
        } catch (...) {
            return false;
        }
    }
    
    std::string getSessionUserId(const std::string& sessionToken) {
        try {
            if (!validateSession(sessionToken)) {
                return "";
            }
            
            std::lock_guard<std::mutex> lock(sessionMutex);
            auto it = sessions.find(sessionToken);
            return it != sessions.end() ? it->second.userId : "";
        } catch (...) {
            return "";
        }
    }
    
    bool invalidateSession(const std::string& sessionToken) {
        try {
            if (sessionToken.empty()) {
                return false;
            }
            
            std::lock_guard<std::mutex> lock(sessionMutex);
            return sessions.erase(sessionToken) > 0;
        } catch (...) {
            return false;
        }
    }
    
    bool setSessionAttribute(const std::string& sessionToken, const std::string& key, const std::string& value) {
        try {
            if (!validateSession(sessionToken)) {
                return false;
            }
            
            std::lock_guard<std::mutex> lock(sessionMutex);
            auto it = sessions.find(sessionToken);
            if (it != sessions.end()) {
                it->second.setAttribute(key, value);
                return true;
            }
            return false;
        } catch (...) {
            return false;
        }
    }
    
    std::string getSessionAttribute(const std::string& sessionToken, const std::string& key) {
        try {
            if (!validateSession(sessionToken)) {
                return "";
            }
            
            std::lock_guard<std::mutex> lock(sessionMutex);
            auto it = sessions.find(sessionToken);
            return it != sessions.end() ? it->second.getAttribute(key) : "";
        } catch (...) {
            return "";
        }
    }
};

int main() {
    std::cout << "=== User Session Management Tests ===" << std::endl << std::endl;
    
    Task123 manager;
    
    // Test Case 1: Create valid session
    std::cout << "Test 1: Create Session" << std::endl;
    std::string token1 = manager.createSession("user123");
    std::cout << "Session created: " << (!token1.empty() ? "Success" : "Failed") << std::endl;
    std::cout << "Token length: " << token1.length() << std::endl;
    
    // Test Case 2: Validate existing session
    std::cout << "\\nTest 2: Validate Session" << std::endl;
    bool isValid = manager.validateSession(token1);
    std::cout << "Session validation: " << (isValid ? "Valid" : "Invalid") << std::endl;
    
    // Test Case 3: Set and get session attributes
    std::cout << "\\nTest 3: Session Attributes" << std::endl;
    manager.setSessionAttribute(token1, "email", "user@example.com");
    manager.setSessionAttribute(token1, "role", "admin");
    std::string email = manager.getSessionAttribute(token1, "email");
    std::string role = manager.getSessionAttribute(token1, "role");
    std::cout << "Email: " << email << std::endl;
    std::cout << "Role: " << role << std::endl;
    
    // Test Case 4: Get user ID from session
    std::cout << "\\nTest 4: Get User ID" << std::endl;
    std::string userId = manager.getSessionUserId(token1);
    std::cout << "User ID: " << userId << std::endl;
    
    // Test Case 5: Invalidate session
    std::cout << "\\nTest 5: Invalidate Session" << std::endl;
    bool invalidated = manager.invalidateSession(token1);
    std::cout << "Session invalidated: " << (invalidated ? "true" : "false") << std::endl;
    bool stillValid = manager.validateSession(token1);
    std::cout << "Session still valid: " << (stillValid ? "true" : "false") << std::endl;
    
    return 0;
}
