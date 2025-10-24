
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <random>

class Task123 {
private:
    struct Session {
        std::string userId;
        std::string sessionId;
        time_t loginTime;
        time_t lastActivityTime;
        bool active;
        
        Session(const std::string& uid, const std::string& sid, time_t lt)
            : userId(uid), sessionId(sid), loginTime(lt), lastActivityTime(lt), active(true) {}
    };
    
    std::map<std::string, Session> sessions;
    
    std::string generateUUID() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 15);
        
        const char* hex = "0123456789abcdef";
        std::string uuid = "";
        for (int i = 0; i < 32; i++) {
            if (i == 8 || i == 12 || i == 16 || i == 20) uuid += "-";
            uuid += hex[dis(gen)];
        }
        return uuid;
    }
    
    std::string formatTime(time_t t) {
        char buffer[80];
        struct tm* timeinfo = localtime(&t);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        return std::string(buffer);
    }
    
public:
    Task123() {}
    
    std::string createSession(const std::string& userId) {
        std::string sessionId = generateUUID();
        time_t now = time(nullptr);
        sessions.insert({sessionId, Session(userId, sessionId, now)});
        return sessionId;
    }
    
    bool isSessionActive(const std::string& sessionId) {
        auto it = sessions.find(sessionId);
        return it != sessions.end() && it->second.active;
    }
    
    void updateActivity(const std::string& sessionId) {
        auto it = sessions.find(sessionId);
        if (it != sessions.end() && it->second.active) {
            it->second.lastActivityTime = time(nullptr);
        }
    }
    
    void endSession(const std::string& sessionId) {
        auto it = sessions.find(sessionId);
        if (it != sessions.end()) {
            it->second.active = false;
        }
    }
    
    std::string getSessionInfo(const std::string& sessionId) {
        auto it = sessions.find(sessionId);
        if (it == sessions.end()) {
            return "Session not found";
        }
        
        std::stringstream ss;
        ss << "User: " << it->second.userId 
           << ", Active: " << (it->second.active ? "true" : "false")
           << ", Login: " << formatTime(it->second.loginTime)
           << ", Last Activity: " << formatTime(it->second.lastActivityTime);
        return ss.str();
    }
    
    std::vector<std::string> getActiveSessions(const std::string& userId) {
        std::vector<std::string> result;
        for (const auto& pair : sessions) {
            if (pair.second.userId == userId && pair.second.active) {
                result.push_back(pair.second.sessionId);
            }
        }
        return result;
    }
    
    int getActiveSessionCount() {
        int count = 0;
        for (const auto& pair : sessions) {
            if (pair.second.active) {
                count++;
            }
        }
        return count;
    }
};

int main() {
    Task123 manager;
    
    std::cout << "Test Case 1: Create sessions for users" << std::endl;
    std::string session1 = manager.createSession("user1");
    std::string session2 = manager.createSession("user2");
    std::string session3 = manager.createSession("user1");
    std::cout << "Created sessions: " << session1.substr(0, 8) << "..., " 
              << session2.substr(0, 8) << "..., " << session3.substr(0, 8) << "..." << std::endl;
    std::cout << "Active session count: " << manager.getActiveSessionCount() << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 2: Check session status" << std::endl;
    std::cout << "Session 1 active: " << (manager.isSessionActive(session1) ? "true" : "false") << std::endl;
    std::cout << "Invalid session active: " << (manager.isSessionActive("invalid-id") ? "true" : "false") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 3: Update activity and get session info" << std::endl;
    manager.updateActivity(session1);
    std::cout << manager.getSessionInfo(session1) << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 4: Get active sessions for user" << std::endl;
    std::vector<std::string> user1Sessions = manager.getActiveSessions("user1");
    std::cout << "User1 has " << user1Sessions.size() << " active sessions" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 5: End session and verify" << std::endl;
    manager.endSession(session1);
    std::cout << "Session 1 active after ending: " << (manager.isSessionActive(session1) ? "true" : "false") << std::endl;
    std::cout << "Active session count: " << manager.getActiveSessionCount() << std::endl;
    user1Sessions = manager.getActiveSessions("user1");
    std::cout << "User1 now has " << user1Sessions.size() << " active sessions" << std::endl;
    
    return 0;
}
