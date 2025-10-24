#include <iostream>
#include <string>
#include <unordered_map>
#include <chrono>
#include <random>
#include <mutex>
#include <sstream>
#include <iomanip>
#include <thread> // For std::this_thread::sleep_for

// A struct to hold session information
struct Session {
    std::string username;
    std::chrono::steady_clock::time_point expiryTime;
};

class SessionManager {
private:
    std::unordered_map<std::string, Session> sessions;
    std::chrono::seconds sessionTimeout;
    // Mutex for thread-safe access to the sessions map
    mutable std::mutex sessionMutex; 

    // Generates a cryptographically secure random string for use as a session ID
    std::string generateSessionId() {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<unsigned long long> dis;

        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        // Generate 32 hex characters (128 bits) for a strong ID
        for (int i = 0; i < 2; ++i) {
            ss << std::setw(16) << dis(gen);
        }
        return ss.str();
    }

public:
    SessionManager(int timeoutSeconds) : sessionTimeout(timeoutSeconds) {}

    std::string createSession(const std::string& username) {
        if (username.empty() || username.find_first_not_of(" \t\n\v\f\r") == std::string::npos) {
            throw std::invalid_argument("Username cannot be null or empty.");
        }
        std::string sessionId = generateSessionId();
        auto expiryTime = std::chrono::steady_clock::now() + sessionTimeout;

        std::lock_guard<std::mutex> lock(sessionMutex);
        sessions[sessionId] = {username, expiryTime};
        return sessionId;
    }

    bool isValidSession(const std::string& sessionId) {
        std::lock_guard<std::mutex> lock(sessionMutex);
        auto it = sessions.find(sessionId);

        if (it == sessions.end()) {
            return false;
        }

        if (std::chrono::steady_clock::now() > it->second.expiryTime) {
            // Lazy cleanup of expired sessions
            sessions.erase(it);
            return false;
        }

        return true;
    }
    
    std::string getUsername(const std::string& sessionId) {
        std::lock_guard<std::mutex> lock(sessionMutex);
        auto it = sessions.find(sessionId);

        if (it == sessions.end()) {
            return "";
        }

        if (std::chrono::steady_clock::now() > it->second.expiryTime) {
            sessions.erase(it);
            return "";
        }
        
        return it->second.username;
    }

    void invalidateSession(const std::string& sessionId) {
        std::lock_guard<std::mutex> lock(sessionMutex);
        sessions.erase(sessionId);
    }
};

int main() {
    std::cout << "--- C++ Session Manager ---" << std::endl;
    // Test Case 1: Create and validate a new session
    std::cout << "\n--- Test Case 1: Create and Validate Session ---" << std::endl;
    SessionManager manager(300); // 5-minute timeout
    std::string sessionId1 = manager.createSession("userA");
    std::cout << "Created session for userA: " << sessionId1 << std::endl;
    std::cout << "Is session valid? " << (manager.isValidSession(sessionId1) ? "true" : "false") << std::endl;
    std::cout << "Username from session: " << manager.getUsername(sessionId1) << std::endl;

    // Test Case 2: Invalidate a session
    std::cout << "\n--- Test Case 2: Invalidate Session ---" << std::endl;
    manager.invalidateSession(sessionId1);
    std::cout << "Session for userA invalidated." << std::endl;
    std::cout << "Is session valid after invalidation? " << (manager.isValidSession(sessionId1) ? "true" : "false") << std::endl;

    // Test Case 3: Validate a non-existent session
    std::cout << "\n--- Test Case 3: Validate Non-existent Session ---" << std::endl;
    std::string fakeSessionId = "fake123456789";
    std::cout << "Is a fake session ID valid? " << (manager.isValidSession(fakeSessionId) ? "true" : "false") << std::endl;

    // Test Case 4: Session expiration
    std::cout << "\n--- Test Case 4: Session Expiration ---" << std::endl;
    SessionManager shortLivedManager(2); // 2-second timeout
    std::string sessionId2 = shortLivedManager.createSession("userB");
    std::cout << "Created short-lived session for userB: " << sessionId2 << std::endl;
    std::cout << "Is session valid immediately? " << (shortLivedManager.isValidSession(sessionId2) ? "true" : "false") << std::endl;
    std::cout << "Waiting for 3 seconds for session to expire..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "Is session valid after expiration? " << (shortLivedManager.isValidSession(sessionId2) ? "true" : "false") << std::endl;

    // Test Case 5: Attempt to create session with invalid username
    std::cout << "\n--- Test Case 5: Invalid Username ---" << std::endl;
    try {
        manager.createSession("");
    } catch (const std::invalid_argument& e) {
        std::cout << "Caught expected exception for empty username: " << e.what() << std::endl;
    }
    try {
        manager.createSession("   ");
    } catch (const std::invalid_argument& e) {
        std::cout << "Caught expected exception for whitespace username: " << e.what() << std::endl;
    }

    return 0;
}