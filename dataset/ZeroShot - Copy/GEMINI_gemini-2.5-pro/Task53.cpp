#include <iostream>
#include <string>
#include <unordered_map>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include <optional>
#include <stdexcept>
#include <thread>

/**
 * Represents a single user session.
 */
struct Session {
    std::string sessionId;
    std::string username;
    std::chrono::time_point<std::chrono::steady_clock> creationTime;
    std::chrono::time_point<std::chrono::steady_clock> lastAccessedTime;
};

/**
 * Manages the lifecycle of user sessions.
 */
class SessionManager {
private:
    // In a real multi-threaded app, access to this map must be protected by a mutex.
    std::unordered_map<std::string, Session> activeSessions;
    // Use a short timeout for demonstration purposes
    static constexpr std::chrono::seconds SESSION_TIMEOUT{10}; 

    // Securely generates a random hex string of a given byte length
    std::string generateSessionId(size_t length = 24) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(0, 255);
        
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (size_t i = 0; i < length; ++i) {
            ss << std::setw(2) << distrib(gen);
        }
        return ss.str();
    }

public:
    /**
     * Creates a new session for a user.
     * @param username The username to associate with the session.
     * @return A cryptographically secure session ID.
     */
    std::string createSession(const std::string& username) {
        if (username.empty()) {
            throw std::invalid_argument("Username cannot be empty.");
        }
        
        std::string sessionId = generateSessionId();
        auto now = std::chrono::steady_clock::now();
        
        Session session = {sessionId, username, now, now};
        activeSessions[sessionId] = session;
        
        return sessionId;
    }

    /**
     * Retrieves a session if it exists and is not expired.
     * @param sessionId The ID of the session to retrieve.
     * @return An optional containing the Session if valid, otherwise an empty optional.
     */
    std::optional<Session> getSession(const std::string& sessionId) {
        auto it = activeSessions.find(sessionId);
        if (it == activeSessions.end()) {
            return std::nullopt; // Session not found
        }
        
        auto now = std::chrono::steady_clock::now();
        auto duration_since_last_access = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.lastAccessedTime);
        
        if (duration_since_last_access > SESSION_TIMEOUT) {
            activeSessions.erase(it); // Session expired
            return std::nullopt;
        }
        
        it->second.lastAccessedTime = now; // Update access time
        return it->second;
    }

    /**
     * Deletes/invalidates a session.
     * @param sessionId The ID of the session to invalidate.
     */
    void invalidateSession(const std::string& sessionId) {
        activeSessions.erase(sessionId);
    }
};

class Task53 {
public:
    static void runTests() {
        std::cout << "--- Session Management Test ---" << std::endl;
        SessionManager manager;

        // Test Case 1: Create and validate a session
        std::cout << "\n--- Test Case 1: Create and Validate Session ---" << std::endl;
        std::string aliceSessionId = manager.createSession("alice");
        std::cout << "Created session for 'alice': " << aliceSessionId << std::endl;
        auto sessionOpt = manager.getSession(aliceSessionId);
        if (sessionOpt) {
            std::cout << "Session is valid. User: " << sessionOpt->username << std::endl;
        } else {
            std::cout << "Session is invalid." << std::endl;
        }

        // Test Case 2: Invalidate a session
        std::cout << "\n--- Test Case 2: Invalidate Session ---" << std::endl;
        std::string bobSessionId = manager.createSession("bob");
        std::cout << "Created session for 'bob': " << bobSessionId << std::endl;
        manager.invalidateSession(bobSessionId);
        std::cout << "Invalidated session for 'bob'." << std::endl;
        sessionOpt = manager.getSession(bobSessionId);
        if (sessionOpt) {
            std::cout << "Error: Session for 'bob' should be invalid." << std::endl;
        } else {
            std::cout << "Session for 'bob' is correctly invalidated." << std::endl;
        }

        // Test Case 3: Session timeout
        std::cout << "\n--- Test Case 3: Session Timeout ---" << std::endl;
        std::string charlieSessionId = manager.createSession("charlie");
        std::cout << "Created session for 'charlie': " << charlieSessionId << std::endl;
        std::cout << "Waiting for session to time out (more than 10 seconds)..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(11));
        sessionOpt = manager.getSession(charlieSessionId);
        if (sessionOpt) {
            std::cout << "Error: Session for 'charlie' should have timed out." << std::endl;
        } else {
            std::cout << "Session for 'charlie' correctly timed out." << std::endl;
        }
        
        // Test Case 4: Accessing invalid/non-existent session
        std::cout << "\n--- Test Case 4: Access Invalid Session ---" << std::endl;
        sessionOpt = manager.getSession("invalid-session-id");
        if (sessionOpt) {
            std::cout << "Error: A non-existent session was found." << std::endl;
        } else {
            std::cout << "Correctly handled non-existent session ID." << std::endl;
        }

        // Test Case 5: Session activity extends lifetime
        std::cout << "\n--- Test Case 5: Session Activity Extends Lifetime ---" << std::endl;
        std::string daveSessionId = manager.createSession("dave");
        std::cout << "Created session for 'dave': " << daveSessionId << std::endl;
        std::cout << "Waiting for 7 seconds..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(7));
        sessionOpt = manager.getSession(daveSessionId);
        if (sessionOpt) {
            std::cout << "Accessed session for 'dave' successfully. Lifetime should be extended." << std::endl;
        } else {
            std::cout << "Error: Session for 'dave' expired prematurely." << std::endl;
        }
        
        std::cout << "Waiting for another 7 seconds..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(7));
        sessionOpt = manager.getSession(daveSessionId);
        if (sessionOpt) {
            std::cout << "Session for 'dave' is still valid after 14 seconds due to activity." << std::endl;
        } else {
            std::cout << "Error: Session for 'dave' expired despite recent activity." << std::endl;
        }
    }
};

int main() {
    Task53::runTests();
    return 0;
}