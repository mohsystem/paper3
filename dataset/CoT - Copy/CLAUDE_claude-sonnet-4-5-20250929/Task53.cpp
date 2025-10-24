
#include <iostream>
#include <string>
#include <map>
#include <set>
#include <ctime>
#include <random>
#include <sstream>
#include <iomanip>
#include <memory>

class Task53 {
private:
    static const int SESSION_TIMEOUT_SECONDS = 1800;
    static const int ABSOLUTE_TIMEOUT_SECONDS = 7200;
    static const int SESSION_ID_LENGTH = 32;
    
    struct Session {
        std::string sessionId;
        std::string userId;
        time_t createdAt;
        time_t lastAccessedAt;
        std::map<std::string, std::string> attributes;
        
        Session(const std::string& sid, const std::string& uid) 
            : sessionId(sid), userId(uid) {
            createdAt = time(nullptr);
            lastAccessedAt = createdAt;
        }
        
        bool isExpired() const {
            time_t now = time(nullptr);
            bool idleExpired = (now - lastAccessedAt) > SESSION_TIMEOUT_SECONDS;
            bool absoluteExpired = (now - createdAt) > ABSOLUTE_TIMEOUT_SECONDS;
            return idleExpired || absoluteExpired;
        }
        
        void updateAccess() {
            lastAccessedAt = time(nullptr);
        }
    };
    
    std::map<std::string, std::shared_ptr<Session>> sessions;
    std::map<std::string, std::set<std::string>> userSessions;
    std::random_device rd;
    std::mt19937 gen;
    
    std::string generateSessionId() {
        std::uniform_int_distribution<> dis(0, 255);
        std::ostringstream oss;
        
        for (int i = 0; i < SESSION_ID_LENGTH; i++) {
            oss << std::hex << std::setw(2) << std::setfill('0') << dis(gen);
        }
        
        return oss.str();
    }
    
public:
    Task53() : gen(rd()) {}
    
    std::string createSession(const std::string& userId) {
        if (userId.empty()) {
            return "";
        }
        
        std::string sessionId = generateSessionId();
        auto session = std::make_shared<Session>(sessionId, userId);
        sessions[sessionId] = session;
        userSessions[userId].insert(sessionId);
        
        return sessionId;
    }
    
    bool validateSession(const std::string& sessionId) {
        if (sessionId.empty()) {
            return false;
        }
        
        auto it = sessions.find(sessionId);
        if (it == sessions.end()) {
            return false;
        }
        
        if (it->second->isExpired()) {
            invalidateSession(sessionId);
            return false;
        }
        
        it->second->updateAccess();
        return true;
    }
    
    void invalidateSession(const std::string& sessionId) {
        auto it = sessions.find(sessionId);
        if (it != sessions.end()) {
            std::string userId = it->second->userId;
            sessions.erase(it);
            
            auto userIt = userSessions.find(userId);
            if (userIt != userSessions.end()) {
                userIt->second.erase(sessionId);
                if (userIt->second.empty()) {
                    userSessions.erase(userIt);
                }
            }
        }
    }
    
    void invalidateAllUserSessions(const std::string& userId) {
        auto userIt = userSessions.find(userId);
        if (userIt != userSessions.end()) {
            for (const auto& sessionId : userIt->second) {
                sessions.erase(sessionId);
            }
            userSessions.erase(userIt);
        }
    }
    
    int getActiveSessionCount(const std::string& userId) {
        auto userIt = userSessions.find(userId);
        if (userIt == userSessions.end()) {
            return 0;
        }
        
        int count = 0;
        std::set<std::string> sessionsToCheck = userIt->second;
        for (const auto& sessionId : sessionsToCheck) {
            if (validateSession(sessionId)) {
                count++;
            }
        }
        return count;
    }
    
    void cleanupExpiredSessions() {
        std::vector<std::string> toRemove;
        for (const auto& pair : sessions) {
            if (pair.second->isExpired()) {
                toRemove.push_back(pair.first);
            }
        }
        for (const auto& sessionId : toRemove) {
            invalidateSession(sessionId);
        }
    }
};

int main() {
    Task53 sessionManager;
    
    // Test Case 1: Create and validate session
    std::cout << "Test Case 1: Create and validate session" << std::endl;
    std::string session1 = sessionManager.createSession("user1");
    std::cout << "Session created: " << (!session1.empty()) << std::endl;
    std::cout << "Session valid: " << sessionManager.validateSession(session1) << std::endl;
    std::cout << std::endl;
    
    // Test Case 2: Multiple sessions for same user
    std::cout << "Test Case 2: Multiple sessions for same user" << std::endl;
    std::string session2 = sessionManager.createSession("user1");
    std::string session3 = sessionManager.createSession("user1");
    std::cout << "Active sessions for user1: " << sessionManager.getActiveSessionCount("user1") << std::endl;
    std::cout << std::endl;
    
    // Test Case 3: Invalidate specific session
    std::cout << "Test Case 3: Invalidate specific session" << std::endl;
    sessionManager.invalidateSession(session2);
    std::cout << "Session2 valid after invalidation: " << sessionManager.validateSession(session2) << std::endl;
    std::cout << "Active sessions for user1: " << sessionManager.getActiveSessionCount("user1") << std::endl;
    std::cout << std::endl;
    
    // Test Case 4: Invalidate all user sessions
    std::cout << "Test Case 4: Invalidate all user sessions" << std::endl;
    sessionManager.invalidateAllUserSessions("user1");
    std::cout << "Session1 valid after user invalidation: " << sessionManager.validateSession(session1) << std::endl;
    std::cout << "Active sessions for user1: " << sessionManager.getActiveSessionCount("user1") << std::endl;
    std::cout << std::endl;
    
    // Test Case 5: Invalid session handling
    std::cout << "Test Case 5: Invalid session handling" << std::endl;
    std::cout << "Empty session valid: " << sessionManager.validateSession("") << std::endl;
    std::cout << "Non-existent session valid: " << sessionManager.validateSession("invalid123") << std::endl;
    std::string nullUserSession = sessionManager.createSession("");
    std::cout << "Session created with empty user: " << (!nullUserSession.empty()) << std::endl;
    
    return 0;
}
