
#include <iostream>
#include <string>
#include <map>
#include <random>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <memory>
#include <ctime>

class Task43 {
private:
    static const int SESSION_TIMEOUT_SECONDS = 1800; // 30 minutes
    static const int SESSION_ID_LENGTH = 32;
    
    struct Session {
        std::string sessionId;
        std::string userId;
        std::chrono::system_clock::time_point createdAt;
        std::chrono::system_clock::time_point lastAccessedAt;
        std::map<std::string, std::string> attributes;
        
        Session(const std::string& sid, const std::string& uid) 
            : sessionId(sid), userId(uid) {
            createdAt = std::chrono::system_clock::now();
            lastAccessedAt = createdAt;
        }
        
        bool isExpired() const {
            auto now = std::chrono::system_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                now - lastAccessedAt).count();
            return elapsed > SESSION_TIMEOUT_SECONDS;
        }
        
        void updateLastAccessed() {
            lastAccessedAt = std::chrono::system_clock::now();
        }
    };
    
    static std::map<std::string, Session> sessions;
    
    static std::string generateSecureToken(size_t length) {
        const char charset[] = 
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "-_";
        
        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<> distribution(0, sizeof(charset) - 2);
        
        std::string token;
        token.reserve(length);
        for (size_t i = 0; i < length; ++i) {
            token += charset[distribution(generator)];
        }
        return token;
    }
    
public:
    static std::string createSession(const std::string& userId) {
        if (userId.empty()) {
            return "";
        }
        
        cleanExpiredSessions();
        
        std::string sessionId = generateSecureToken(SESSION_ID_LENGTH);
        
        // Ensure uniqueness
        while (sessions.find(sessionId) != sessions.end()) {
            sessionId = generateSecureToken(SESSION_ID_LENGTH);
        }
        
        sessions.emplace(sessionId, Session(sessionId, userId));
        return sessionId;
    }
    
    static bool validateSession(const std::string& sessionId) {
        if (sessionId.empty()) {
            return false;
        }
        
        auto it = sessions.find(sessionId);
        if (it == sessions.end()) {
            return false;
        }
        
        if (it->second.isExpired()) {
            terminateSession(sessionId);
            return false;
        }
        
        it->second.updateLastAccessed();
        return true;
    }
    
    static std::string getSessionUserId(const std::string& sessionId) {
        if (!validateSession(sessionId)) {
            return "";
        }
        return sessions[sessionId].userId;
    }
    
    static bool terminateSession(const std::string& sessionId) {
        if (sessionId.empty()) {
            return false;
        }
        return sessions.erase(sessionId) > 0;
    }
    
    static void cleanExpiredSessions() {
        auto it = sessions.begin();
        while (it != sessions.end()) {
            if (it->second.isExpired()) {
                it = sessions.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    static bool setSessionAttribute(const std::string& sessionId, 
                                    const std::string& key, 
                                    const std::string& value) {
        if (!validateSession(sessionId) || key.empty()) {
            return false;
        }
        sessions[sessionId].attributes[key] = value;
        return true;
    }
    
    static std::string getSessionAttribute(const std::string& sessionId, 
                                           const std::string& key) {
        if (!validateSession(sessionId) || key.empty()) {
            return "";
        }
        auto it = sessions[sessionId].attributes.find(key);
        if (it != sessions[sessionId].attributes.end()) {
            return it->second;
        }
        return "";
    }
    
    static int getActiveSessionCount() {
        cleanExpiredSessions();
        return sessions.size();
    }
};

std::map<std::string, Task43::Session> Task43::sessions;

int main() {
    std::cout << "=== Secure Session Management Test Cases ===\\n\\n";
    
    // Test Case 1: Create and validate session
    std::cout << "Test Case 1: Create and Validate Session\\n";
    std::string session1 = Task43::createSession("user123");
    std::cout << "Session created: " << (!session1.empty()) << "\\n";
    std::cout << "Session valid: " << Task43::validateSession(session1) << "\\n";
    std::cout << "User ID: " << Task43::getSessionUserId(session1) << "\\n\\n";
    
    // Test Case 2: Session attributes
    std::cout << "Test Case 2: Session Attributes\\n";
    std::string session2 = Task43::createSession("user456");
    Task43::setSessionAttribute(session2, "role", "admin");
    Task43::setSessionAttribute(session2, "loginTime", "2024-01-01");
    std::cout << "Role attribute: " << Task43::getSessionAttribute(session2, "role") << "\\n";
    std::cout << "Login time set: " << (!Task43::getSessionAttribute(session2, "loginTime").empty()) << "\\n\\n";
    
    // Test Case 3: Invalid session handling
    std::cout << "Test Case 3: Invalid Session Handling\\n";
    std::cout << "Empty session valid: " << Task43::validateSession("") << "\\n";
    std::cout << "Fake session valid: " << Task43::validateSession("fake-session-id") << "\\n\\n";
    
    // Test Case 4: Session termination
    std::cout << "Test Case 4: Session Termination\\n";
    std::string session3 = Task43::createSession("user789");
    std::cout << "Session created: " << Task43::validateSession(session3) << "\\n";
    bool terminated = Task43::terminateSession(session3);
    std::cout << "Session terminated: " << terminated << "\\n";
    std::cout << "Session still valid: " << Task43::validateSession(session3) << "\\n\\n";
    
    // Test Case 5: Active session count
    std::cout << "Test Case 5: Active Session Count\\n";
    Task43::createSession("user001");
    Task43::createSession("user002");
    Task43::createSession("user003");
    int count = Task43::getActiveSessionCount();
    std::cout << "Active sessions: " << count << "\\n";
    Task43::cleanExpiredSessions();
    std::cout << "After cleanup: " << Task43::getActiveSessionCount() << "\\n\\n";
    
    return 0;
}
