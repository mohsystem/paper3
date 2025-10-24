
#include <iostream>
#include <string>
#include <unordered_map>
#include <ctime>
#include <cstring>
#include <random>
#include <iomanip>
#include <sstream>
#include <memory>
#include <algorithm>

// Session timeout in seconds (30 minutes)
const int SESSION_TIMEOUT = 1800;
const size_t SESSION_ID_LENGTH = 32;
const size_t MAX_SESSIONS = 10000;

struct Session {
    std::string userId;
    time_t createdAt;
    time_t lastAccessedAt;
    
    Session() : userId(""), createdAt(0), lastAccessedAt(0) {}
    Session(const std::string& uid, time_t created) 
        : userId(uid), createdAt(created), lastAccessedAt(created) {}
};

class SessionManager {
private:
    std::unordered_map<std::string, Session> sessions;
    std::random_device rd;
    std::mt19937 gen;
    
    // Generate cryptographically secure random session ID
    std::string generateSessionId() {
        // Use random_device for cryptographically secure randomness
        std::uniform_int_distribution<> dis(0, 15);
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        
        for (size_t i = 0; i < SESSION_ID_LENGTH; ++i) {
            ss << std::setw(1) << dis(rd);
        }
        return ss.str();
    }
    
    // Validate user ID format and length
    bool isValidUserId(const std::string& userId) const {
        // Check length - prevent excessively long input
        if (userId.empty() || userId.length() > 256) {
            return false;
        }
        
        // Validate characters: alphanumeric, underscore, hyphen only
        for (char c : userId) {
            if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-') {
                return false;
            }
        }
        return true;
    }
    
    // Validate session ID format
    bool isValidSessionId(const std::string& sessionId) const {
        // Check exact length
        if (sessionId.length() != SESSION_ID_LENGTH) {
            return false;
        }
        
        // Validate hex characters only
        for (char c : sessionId) {
            if (!std::isxdigit(static_cast<unsigned char>(c))) {
                return false;
            }
        }
        return true;
    }

public:
    SessionManager() : gen(rd()) {}
    
    // Create a new session for a user
    std::string createSession(const std::string& userId) {
        // Validate user ID input
        if (!isValidUserId(userId)) {
            return "";
        }
        
        // Enforce maximum number of sessions to prevent resource exhaustion
        if (sessions.size() >= MAX_SESSIONS) {
            return "";
        }
        
        // Generate unique session ID with collision check
        std::string sessionId;
        int attempts = 0;
        const int MAX_ATTEMPTS = 100;
        
        do {
            sessionId = generateSessionId();
            attempts++;
            if (attempts > MAX_ATTEMPTS) {
                return ""; // Fail safely if cannot generate unique ID
            }
        } while (sessions.find(sessionId) != sessions.end());
        
        time_t now = std::time(nullptr);
        if (now == static_cast<time_t>(-1)) {
            return ""; // Time function failed
        }
        
        sessions[sessionId] = Session(userId, now);
        return sessionId;
    }
    
    // Validate and retrieve session
    bool validateSession(const std::string& sessionId, std::string& userId) {
        // Validate session ID format first
        if (!isValidSessionId(sessionId)) {
            return false;
        }
        
        auto it = sessions.find(sessionId);
        if (it == sessions.end()) {
            return false;
        }
        
        time_t now = std::time(nullptr);
        if (now == static_cast<time_t>(-1)) {
            return false;
        }
        
        // Check if session has expired
        if (now - it->second.lastAccessedAt > SESSION_TIMEOUT) {
            sessions.erase(it);
            return false;
        }
        
        // Update last accessed time
        it->second.lastAccessedAt = now;
        userId = it->second.userId;
        return true;
    }
    
    // Destroy a session
    bool destroySession(const std::string& sessionId) {
        // Validate session ID format
        if (!isValidSessionId(sessionId)) {
            return false;
        }
        
        auto it = sessions.find(sessionId);
        if (it == sessions.end()) {
            return false;
        }
        
        sessions.erase(it);
        return true;
    }
    
    // Clean up expired sessions
    size_t cleanupExpiredSessions() {
        time_t now = std::time(nullptr);
        if (now == static_cast<time_t>(-1)) {
            return 0;
        }
        
        size_t removed = 0;
        auto it = sessions.begin();
        while (it != sessions.end()) {
            if (now - it->second.lastAccessedAt > SESSION_TIMEOUT) {
                it = sessions.erase(it);
                removed++;
            } else {
                ++it;
            }
        }
        return removed;
    }
    
    size_t getSessionCount() const {
        return sessions.size();
    }
};

int main() {
    SessionManager manager;
    
    // Test case 1: Create valid sessions
    std::cout << "Test 1: Create valid sessions" << std::endl;
    std::string session1 = manager.createSession("user123");
    std::string session2 = manager.createSession("admin_user");
    std::cout << "Created session 1: " << (session1.empty() ? "FAILED" : "SUCCESS") << std::endl;
    std::cout << "Created session 2: " << (session2.empty() ? "FAILED" : "SUCCESS") << std::endl;
    std::cout << "Active sessions: " << manager.getSessionCount() << std::endl << std::endl;
    
    // Test case 2: Validate sessions
    std::cout << "Test 2: Validate sessions" << std::endl;
    std::string userId;
    bool valid1 = manager.validateSession(session1, userId);
    std::cout << "Session 1 valid: " << (valid1 ? "YES" : "NO") << ", User: " << userId << std::endl;
    bool valid2 = manager.validateSession(session2, userId);
    std::cout << "Session 2 valid: " << (valid2 ? "YES" : "NO") << ", User: " << userId << std::endl << std::endl;
    
    // Test case 3: Invalid session ID
    std::cout << "Test 3: Invalid session ID" << std::endl;
    bool valid3 = manager.validateSession("invalid_session_id", userId);
    std::cout << "Invalid session validated: " << (valid3 ? "YES (BUG!)" : "NO (correct)") << std::endl << std::endl;
    
    // Test case 4: Destroy session
    std::cout << "Test 4: Destroy session" << std::endl;
    bool destroyed = manager.destroySession(session1);
    std::cout << "Session destroyed: " << (destroyed ? "YES" : "NO") << std::endl;
    bool validAfterDestroy = manager.validateSession(session1, userId);
    std::cout << "Destroyed session still valid: " << (validAfterDestroy ? "YES (BUG!)" : "NO (correct)") << std::endl;
    std::cout << "Active sessions: " << manager.getSessionCount() << std::endl << std::endl;
    
    // Test case 5: Invalid user ID (injection attempt)
    std::cout << "Test 5: Invalid user ID with special characters" << std::endl;
    std::string session3 = manager.createSession("user'; DROP TABLE--");
    std::cout << "Session with invalid user ID created: " << (session3.empty() ? "NO (correct)" : "YES (BUG!)") << std::endl;
    std::string session4 = manager.createSession("valid_user-123");
    std::cout << "Session with valid user ID created: " << (session4.empty() ? "NO (BUG!)" : "YES (correct)") << std::endl;
    
    return 0;
}
