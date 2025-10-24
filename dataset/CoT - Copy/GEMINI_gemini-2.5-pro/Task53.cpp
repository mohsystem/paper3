#include <iostream>
#include <string>
#include <unordered_map>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include <thread>

// Session data structure
struct Session {
    std::string username;
    std::chrono::steady_clock::time_point creationTime;
};

class Task53 {
private:
    std::unordered_map<std::string, Session> activeSessions;
    static const int SESSION_TIMEOUT_SECONDS = 3;

    // Generates a random, non-cryptographically secure session ID.
    // WARNING: For production, use a dedicated library like OpenSSL's RAND_bytes
    // for cryptographically secure random number generation.
    std::string generateSessionId() {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<unsigned long long> dis;

        std::stringstream ss;
        ss << std::hex << std::setw(16) << std::setfill('0') << dis(gen);
        ss << std::hex << std::setw(16) << std::setfill('0') << dis(gen);
        ss << std::hex << std::setw(16) << std::setfill('0') << dis(gen);
        ss << std::hex << std::setw(16) << std::setfill('0') << dis(gen);
        
        return ss.str();
    }

public:
    /**
     * Creates a new session for a user.
     * @param username The username for whom to create the session.
     * @return The generated session ID.
     */
    std::string createSession(const std::string& username) {
        if (username.empty()) {
            return "";
        }
        std::string sessionId = generateSessionId();
        Session session = {username, std::chrono::steady_clock::now()};
        activeSessions[sessionId] = session;
        std::cout << "Session created for " << username << " with ID: " << sessionId << std::endl;
        return sessionId;
    }

    /**
     * Checks if a session is valid and not expired.
     * @param sessionId The session ID to validate.
     * @return true if the session is valid, false otherwise.
     */
    bool isSessionValid(const std::string& sessionId) {
        auto it = activeSessions.find(sessionId);
        if (it == activeSessions.end()) {
            return false; // Session does not exist.
        }

        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.creationTime);

        if (duration.count() > SESSION_TIMEOUT_SECONDS) {
            // Session has expired, remove it.
            activeSessions.erase(it);
            std::cout << "Session " << sessionId << " expired." << std::endl;
            return false;
        }

        return true;
    }

    /**
     * Invalidates/removes a user session.
     * @param sessionId The session ID to invalidate.
     */
    void invalidateSession(const std::string& sessionId) {
        if (activeSessions.erase(sessionId) > 0) {
            std::cout << "Session " << sessionId << " invalidated." << std::endl;
        } else {
            std::cout << "Session " << sessionId << " not found for invalidation." << std::endl;
        }
    }
};

int main() {
    Task53 sessionManager;
    std::cout << "--- Session Management Test ---" << std::endl;

    // Test Case 1: Create a session and validate it
    std::cout << "\n--- Test Case 1: Create and Validate ---" << std::endl;
    std::string user1SessionId = sessionManager.createSession("user1");
    std::cout << "Is user1's session valid? " << (sessionManager.isSessionValid(user1SessionId) ? "true" : "false") << std::endl;

    // Test Case 2: Validate a non-existent session
    std::cout << "\n--- Test Case 2: Validate Non-Existent Session ---" << std::endl;
    std::cout << "Is 'invalid-session-id' valid? " << (sessionManager.isSessionValid("invalid-session-id") ? "true" : "false") << std::endl;

    // Test Case 3: Invalidate a session and check it again
    std::cout << "\n--- Test Case 3: Invalidate and Re-Validate ---" << std::endl;
    sessionManager.invalidateSession(user1SessionId);
    std::cout << "Is user1's session valid after invalidation? " << (sessionManager.isSessionValid(user1SessionId) ? "true" : "false") << std::endl;

    // Test Case 4: Test session timeout
    std::cout << "\n--- Test Case 4: Session Timeout ---" << std::endl;
    std::string user2SessionId = sessionManager.createSession("user2");
    std::cout << "Is user2's session valid immediately? " << (sessionManager.isSessionValid(user2SessionId) ? "true" : "false") << std::endl;
    const int timeout = 3;
    std::cout << "Waiting for " << (timeout + 1) << " seconds to test timeout..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(timeout + 1));
    std::cout << "Is user2's session valid after timeout? " << (sessionManager.isSessionValid(user2SessionId) ? "true" : "false") << std::endl;

    // Test Case 5: Create multiple sessions
    std::cout << "\n--- Test Case 5: Multiple Sessions ---" << std::endl;
    std::string user3SessionId = sessionManager.createSession("user3");
    std::string user4SessionId = sessionManager.createSession("user4");
    std::cout << "Is user3's session valid? " << (sessionManager.isSessionValid(user3SessionId) ? "true" : "false") << std::endl;
    std::cout << "Is user4's session valid? " << (sessionManager.isSessionValid(user4SessionId) ? "true" : "false") << std::endl;
    sessionManager.invalidateSession(user3SessionId);
    std::cout << "Is user3's session valid after invalidation? " << (sessionManager.isSessionValid(user3SessionId) ? "true" : "false") << std::endl;
    std::cout << "Is user4's session still valid? " << (sessionManager.isSessionValid(user4SessionId) ? "true" : "false") << std::endl;

    return 0;
}