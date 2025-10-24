
#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <algorithm>

class Task43 {
public:
    static const int SESSION_TIMEOUT_MINUTES = 30;
    static const int TOKEN_LENGTH = 32;
    
    class Session {
    private:
        std::string sessionId;
        std::string userId;
        std::chrono::system_clock::time_point createdAt;
        std::chrono::system_clock::time_point lastAccessedAt;
        std::map<std::string, std::string> attributes;
        std::mutex mtx;
        
    public:
        Session(const std::string& sid, const std::string& uid) 
            : sessionId(sid), userId(uid) {
            createdAt = std::chrono::system_clock::now();
            lastAccessedAt = createdAt;
        }
        
        std::string getSessionId() const { return sessionId; }
        std::string getUserId() const { return userId; }
        
        void updateLastAccessed() {
            std::lock_guard<std::mutex> lock(mtx);
            lastAccessedAt = std::chrono::system_clock::now();
        }
        
        bool isExpired() const {
            auto now = std::chrono::system_clock::now();
            auto timeout = std::chrono::minutes(SESSION_TIMEOUT_MINUTES);
            return now > lastAccessedAt + timeout;
        }
        
        void setAttribute(const std::string& key, const std::string& value) {
            std::lock_guard<std::mutex> lock(mtx);
            attributes[key] = value;
        }
        
        std::string getAttribute(const std::string& key) {
            std::lock_guard<std::mutex> lock(mtx);
            auto it = attributes.find(key);
            return (it != attributes.end()) ? it->second : "";
        }
        
        std::string toString() const {
            std::ostringstream oss;
            oss << "Session{id='" << sessionId << "', userId='" << userId 
                << "', expired=" << (isExpired() ? "true" : "false") << "}";
            return oss.str();
        }
    };
    
    class SessionManager {
    private:
        std::map<std::string, std::shared_ptr<Session>> sessions;
        std::mutex mtx;
        std::random_device rd;
        std::mt19937 gen;
        
        std::string generateSessionId() {
            std::uniform_int_distribution<> dis(0, 255);
            std::ostringstream oss;
            
            for (int i = 0; i < TOKEN_LENGTH; ++i) {
                oss << std::hex << std::setw(2) << std::setfill('0') << dis(gen);
            }
            return oss.str();
        }
        
        void cleanupExpiredSessions() {
            auto it = sessions.begin();
            while (it != sessions.end()) {
                if (it->second->isExpired()) {
                    it = sessions.erase(it);
                } else {
                    ++it;
                }
            }
        }
        
    public:
        SessionManager() : gen(rd()) {}
        
        std::string createSession(const std::string& userId) {
            std::lock_guard<std::mutex> lock(mtx);
            std::string sessionId = generateSessionId();
            sessions[sessionId] = std::make_shared<Session>(sessionId, userId);
            cleanupExpiredSessions();
            return sessionId;
        }
        
        std::shared_ptr<Session> getSession(const std::string& sessionId) {
            std::lock_guard<std::mutex> lock(mtx);
            auto it = sessions.find(sessionId);
            if (it != sessions.end()) {
                if (it->second->isExpired()) {
                    sessions.erase(it);
                    return nullptr;
                }
                it->second->updateLastAccessed();
                return it->second;
            }
            return nullptr;
        }
        
        bool validateSession(const std::string& sessionId) {
            return getSession(sessionId) != nullptr;
        }
        
        void terminateSession(const std::string& sessionId) {
            std::lock_guard<std::mutex> lock(mtx);
            sessions.erase(sessionId);
        }
        
        void terminateUserSessions(const std::string& userId) {
            std::lock_guard<std::mutex> lock(mtx);
            auto it = sessions.begin();
            while (it != sessions.end()) {
                if (it->second->getUserId() == userId) {
                    it = sessions.erase(it);
                } else {
                    ++it;
                }
            }
        }
        
        int getActiveSessionCount() {
            std::lock_guard<std::mutex> lock(mtx);
            cleanupExpiredSessions();
            return sessions.size();
        }
    };
};

int main() {
    Task43::SessionManager manager;
    
    std::cout << "=== Test Case 1: Create Session ===" << std::endl;
    std::string sessionId1 = manager.createSession("user123");
    std::cout << "Created session: " << sessionId1 << std::endl;
    std::cout << "Session valid: " << (manager.validateSession(sessionId1) ? "true" : "false") << std::endl;
    
    std::cout << "\\n=== Test Case 2: Get Session and Update Access Time ===" << std::endl;
    auto session1 = manager.getSession(sessionId1);
    if (session1) {
        std::cout << session1->toString() << std::endl;
        session1->setAttribute("username", "JohnDoe");
        std::cout << "Username attribute: " << session1->getAttribute("username") << std::endl;
    }
    
    std::cout << "\\n=== Test Case 3: Multiple Sessions ===" << std::endl;
    std::string sessionId2 = manager.createSession("user456");
    std::string sessionId3 = manager.createSession("user789");
    std::cout << "Active sessions: " << manager.getActiveSessionCount() << std::endl;
    
    std::cout << "\\n=== Test Case 4: Terminate Specific Session ===" << std::endl;
    manager.terminateSession(sessionId2);
    std::cout << "Session 2 valid after termination: " 
              << (manager.validateSession(sessionId2) ? "true" : "false") << std::endl;
    std::cout << "Active sessions: " << manager.getActiveSessionCount() << std::endl;
    
    std::cout << "\\n=== Test Case 5: Terminate All User Sessions ===" << std::endl;
    std::string sessionId4 = manager.createSession("user123");
    std::cout << "Created another session for user123" << std::endl;
    std::cout << "Active sessions before cleanup: " << manager.getActiveSessionCount() << std::endl;
    manager.terminateUserSessions("user123");
    std::cout << "Active sessions after user123 cleanup: " << manager.getActiveSessionCount() << std::endl;
    std::cout << "Session 1 valid: " << (manager.validateSession(sessionId1) ? "true" : "false") << std::endl;
    std::cout << "Session 4 valid: " << (manager.validateSession(sessionId4) ? "true" : "false") << std::endl;
    
    return 0;
}
