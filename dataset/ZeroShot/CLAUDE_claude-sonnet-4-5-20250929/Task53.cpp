
#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <random>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <vector>

class Task53 {
private:
    static const int SESSION_TIMEOUT_SECONDS = 1800;
    static const int TOKEN_LENGTH = 32;
    
    class Session {
    public:
        std::string sessionId;
        std::string userId;
        time_t createdAt;
        time_t lastAccessedAt;
        std::map<std::string, std::string> attributes;
        
        Session(const std::string& sid, const std::string& uid)
            : sessionId(sid), userId(uid) {
            createdAt = std::time(nullptr);
            lastAccessedAt = createdAt;
        }
        
        bool isExpired(int timeoutSeconds) {
            time_t currentTime = std::time(nullptr);
            return (currentTime - lastAccessedAt) > timeoutSeconds;
        }
        
        void updateLastAccessed() {
            lastAccessedAt = std::time(nullptr);
        }
    };
    
    std::map<std::string, std::shared_ptr<Session>> sessions;
    std::mutex sessionMutex;
    std::random_device rd;
    std::mt19937 gen;
    
    std::string generateSecureToken() {
        std::uniform_int_distribution<> dis(0, 255);
        std::stringstream ss;
        
        for (int i = 0; i < TOKEN_LENGTH; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << dis(gen);
        }
        
        return ss.str();
    }
    
public:
    Task53() : gen(rd()) {}
    
    std::string createSession(const std::string& userId) {
        if (userId.empty()) {
            return "";
        }
        
        std::string sessionId = generateSecureToken();
        auto session = std::make_shared<Session>(sessionId, userId);
        
        std::lock_guard<std::mutex> lock(sessionMutex);
        sessions[sessionId] = session;
        
        return sessionId;
    }
    
    bool validateSession(const std::string& sessionId) {
        if (sessionId.empty()) {
            return false;
        }
        
        std::lock_guard<std::mutex> lock(sessionMutex);
        auto it = sessions.find(sessionId);
        
        if (it == sessions.end()) {
            return false;
        }
        
        if (it->second->isExpired(SESSION_TIMEOUT_SECONDS)) {
            sessions.erase(it);
            return false;
        }
        
        it->second->updateLastAccessed();
        return true;
    }
    
    std::string getUserId(const std::string& sessionId) {
        if (!validateSession(sessionId)) {
            return "";
        }
        
        std::lock_guard<std::mutex> lock(sessionMutex);
        auto it = sessions.find(sessionId);
        
        return (it != sessions.end()) ? it->second->userId : "";
    }
    
    bool invalidateSession(const std::string& sessionId) {
        if (sessionId.empty()) {
            return false;
        }
        
        std::lock_guard<std::mutex> lock(sessionMutex);
        return sessions.erase(sessionId) > 0;
    }
    
    void setAttribute(const std::string& sessionId, const std::string& key, 
                     const std::string& value) {
        if (validateSession(sessionId) && !key.empty()) {
            std::lock_guard<std::mutex> lock(sessionMutex);
            auto it = sessions.find(sessionId);
            if (it != sessions.end()) {
                it->second->attributes[key] = value;
            }
        }
    }
    
    std::string getAttribute(const std::string& sessionId, const std::string& key) {
        if (validateSession(sessionId) && !key.empty()) {
            std::lock_guard<std::mutex> lock(sessionMutex);
            auto it = sessions.find(sessionId);
            if (it != sessions.end()) {
                auto attrIt = it->second->attributes.find(key);
                if (attrIt != it->second->attributes.end()) {
                    return attrIt->second;
                }
            }
        }
        return "";
    }
    
    void cleanupExpiredSessions() {
        std::lock_guard<std::mutex> lock(sessionMutex);
        std::vector<std::string> expiredSessions;
        
        for (const auto& entry : sessions) {
            if (entry.second->isExpired(SESSION_TIMEOUT_SECONDS)) {
                expiredSessions.push_back(entry.first);
            }
        }
        
        for (const auto& sessionId : expiredSessions) {
            sessions.erase(sessionId);
        }
    }
};

int main() {
    Task53 sessionManager;
    
    // Test Case 1: Create a session
    std::cout << "Test Case 1: Create Session" << std::endl;
    std::string session1 = sessionManager.createSession("user123");
    std::cout << "Session created: " << (!session1.empty() ? "true" : "false") << std::endl;
    std::cout << "Session ID length: " << session1.length() << std::endl;
    
    // Test Case 2: Validate session
    std::cout << "\\nTest Case 2: Validate Session" << std::endl;
    bool isValid = sessionManager.validateSession(session1);
    std::cout << "Session valid: " << (isValid ? "true" : "false") << std::endl;
    std::string userId = sessionManager.getUserId(session1);
    std::cout << "User ID: " << userId << std::endl;
    
    // Test Case 3: Set and get attributes
    std::cout << "\\nTest Case 3: Session Attributes" << std::endl;
    sessionManager.setAttribute(session1, "username", "JohnDoe");
    sessionManager.setAttribute(session1, "role", "admin");
    std::string username = sessionManager.getAttribute(session1, "username");
    std::string role = sessionManager.getAttribute(session1, "role");
    std::cout << "Username: " << username << std::endl;
    std::cout << "Role: " << role << std::endl;
    
    // Test Case 4: Invalidate session
    std::cout << "\\nTest Case 4: Invalidate Session" << std::endl;
    bool invalidated = sessionManager.invalidateSession(session1);
    std::cout << "Session invalidated: " << (invalidated ? "true" : "false") << std::endl;
    bool stillValid = sessionManager.validateSession(session1);
    std::cout << "Session still valid: " << (stillValid ? "true" : "false") << std::endl;
    
    // Test Case 5: Multiple sessions and cleanup
    std::cout << "\\nTest Case 5: Multiple Sessions" << std::endl;
    std::string session2 = sessionManager.createSession("user456");
    std::string session3 = sessionManager.createSession("user789");
    std::cout << "Session 2 created: " << (!session2.empty() ? "true" : "false") << std::endl;
    std::cout << "Session 3 created: " << (!session3.empty() ? "true" : "false") << std::endl;
    sessionManager.cleanupExpiredSessions();
    std::cout << "Cleanup completed" << std::endl;
    std::cout << "Session 2 valid: " << (sessionManager.validateSession(session2) ? "true" : "false") << std::endl;
    
    return 0;
}
