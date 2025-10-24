
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <random>

class Task53 {
private:
    struct Session {
        std::string sessionId;
        std::string userId;
        time_t createdAt;
        time_t lastAccessedAt;
        std::map<std::string, std::string> data;
        
        Session(const std::string& sid, const std::string& uid) 
            : sessionId(sid), userId(uid) {
            createdAt = time(nullptr);
            lastAccessedAt = time(nullptr);
        }
        
        void updateLastAccessed() {
            lastAccessedAt = time(nullptr);
        }
        
        bool isExpired(int timeoutMinutes) const {
            time_t now = time(nullptr);
            double seconds = difftime(now, lastAccessedAt);
            return (seconds / 60) >= timeoutMinutes;
        }
    };
    
    std::map<std::string, Session> sessions;
    int sessionTimeout;
    
    std::string generateUUID() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 15);
        
        const char* hex = "0123456789abcdef";
        std::stringstream ss;
        
        for (int i = 0; i < 32; i++) {
            if (i == 8 || i == 12 || i == 16 || i == 20) {
                ss << "-";
            }
            ss << hex[dis(gen)];
        }
        return ss.str();
    }
    
public:
    Task53(int timeout) : sessionTimeout(timeout) {}
    
    std::string createSession(const std::string& userId) {
        std::string sessionId = generateUUID();
        sessions.emplace(sessionId, Session(sessionId, userId));
        return sessionId;
    }
    
    bool validateSession(const std::string& sessionId) {
        auto it = sessions.find(sessionId);
        if (it == sessions.end()) {
            return false;
        }
        if (it->second.isExpired(sessionTimeout)) {
            sessions.erase(it);
            return false;
        }
        it->second.updateLastAccessed();
        return true;
    }
    
    void destroySession(const std::string& sessionId) {
        sessions.erase(sessionId);
    }
    
    void setSessionData(const std::string& sessionId, const std::string& key, const std::string& value) {
        auto it = sessions.find(sessionId);
        if (it != sessions.end() && !it->second.isExpired(sessionTimeout)) {
            it->second.data[key] = value;
            it->second.updateLastAccessed();
        }
    }
    
    std::string getSessionData(const std::string& sessionId, const std::string& key) {
        auto it = sessions.find(sessionId);
        if (it != sessions.end() && !it->second.isExpired(sessionTimeout)) {
            it->second.updateLastAccessed();
            auto dataIt = it->second.data.find(key);
            if (dataIt != it->second.data.end()) {
                return dataIt->second;
            }
        }
        return "";
    }
    
    std::string getUserId(const std::string& sessionId) {
        auto it = sessions.find(sessionId);
        if (it != sessions.end() && !it->second.isExpired(sessionTimeout)) {
            return it->second.userId;
        }
        return "";
    }
    
    void cleanupExpiredSessions() {
        std::vector<std::string> expiredSessions;
        for (auto& pair : sessions) {
            if (pair.second.isExpired(sessionTimeout)) {
                expiredSessions.push_back(pair.first);
            }
        }
        for (const auto& sessionId : expiredSessions) {
            sessions.erase(sessionId);
        }
    }
    
    int getActiveSessionCount() {
        cleanupExpiredSessions();
        return sessions.size();
    }
};

int main() {
    std::cout << "Test Case 1: Create and validate session\\n";
    Task53 manager1(30);
    std::string sessionId1 = manager1.createSession("user123");
    std::cout << "Session created: " << sessionId1 << "\\n";
    std::cout << "Session valid: " << (manager1.validateSession(sessionId1) ? "true" : "false") << "\\n";
    std::cout << "User ID: " << manager1.getUserId(sessionId1) << "\\n\\n";
    
    std::cout << "Test Case 2: Store and retrieve session data\\n";
    Task53 manager2(30);
    std::string sessionId2 = manager2.createSession("user456");
    manager2.setSessionData(sessionId2, "username", "JohnDoe");
    manager2.setSessionData(sessionId2, "email", "john@example.com");
    std::cout << "Username: " << manager2.getSessionData(sessionId2, "username") << "\\n";
    std::cout << "Email: " << manager2.getSessionData(sessionId2, "email") << "\\n\\n";
    
    std::cout << "Test Case 3: Destroy session\\n";
    Task53 manager3(30);
    std::string sessionId3 = manager3.createSession("user789");
    std::cout << "Before destroy - Valid: " << (manager3.validateSession(sessionId3) ? "true" : "false") << "\\n";
    manager3.destroySession(sessionId3);
    std::cout << "After destroy - Valid: " << (manager3.validateSession(sessionId3) ? "true" : "false") << "\\n\\n";
    
    std::cout << "Test Case 4: Multiple sessions\\n";
    Task53 manager4(30);
    std::string session1 = manager4.createSession("user1");
    std::string session2 = manager4.createSession("user2");
    std::string session3 = manager4.createSession("user3");
    std::cout << "Active sessions: " << manager4.getActiveSessionCount() << "\\n";
    manager4.destroySession(session2);
    std::cout << "After destroying one: " << manager4.getActiveSessionCount() << "\\n\\n";
    
    std::cout << "Test Case 5: Invalid session operations\\n";
    Task53 manager5(30);
    std::cout << "Validate non-existent session: " << (manager5.validateSession("invalid-id") ? "true" : "false") << "\\n";
    std::cout << "Get data from non-existent session: " << manager5.getSessionData("invalid-id", "key") << "\\n";
    std::string validSession = manager5.createSession("user999");
    manager5.destroySession(validSession);
    std::cout << "Get user from destroyed session: " << manager5.getUserId(validSession) << "\\n";
    
    return 0;
}
