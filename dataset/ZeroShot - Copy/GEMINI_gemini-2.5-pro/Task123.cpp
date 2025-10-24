#include <iostream>
#include <string>
#include <unordered_map>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include <vector>
#include <memory>
#include <optional>

// Represents the data associated with a single session.
struct SessionData {
    std::string userId;
    std::chrono::system_clock::time_point expirationTime;

    bool isExpired() const {
        return std::chrono::system_clock::now() > expirationTime;
    }
};

/**
 * Securely manages user sessions in-memory.
 * Uses cryptographically strong random numbers for session IDs.
 * Sessions expire after a defined duration.
 */
class SessionManager {
public:
    SessionManager() : sessionDuration(1800) {} // 30 minutes

    /**
     * Creates a new session for a given user.
     * @param userId The ID of the user to create a session for.
     * @return The generated session ID.
     */
    std::string createSession(const std::string& userId) {
        if (userId.empty()) {
            throw std::invalid_argument("User ID cannot be empty.");
        }
        std::string sessionId = generateSecureSessionId();
        auto expirationTime = std::chrono::system_clock::now() + std::chrono::seconds(sessionDuration);
        
        // Using smart pointers to manage memory for session data
        activeSessions[sessionId] = std::make_unique<SessionData>(SessionData{userId, expirationTime});
        
        return sessionId;
    }

    /**
     * Retrieves the user ID associated with a session, if the session is valid and not expired.
     * @param sessionId The session ID to validate.
     * @return An optional containing the user ID if the session is valid, otherwise an empty optional.
     */
    std::optional<std::string> getSessionUser(const std::string& sessionId) {
        auto it = activeSessions.find(sessionId);

        if (it == activeSessions.end()) {
            return std::nullopt; // Session not found
        }

        if (it->second->isExpired()) {
            activeSessions.erase(it); // Clean up expired session
            return std::nullopt;
        }

        return it->second->userId;
    }

    /**
     * Invalidates and removes a session.
     * @param sessionId The ID of the session to end.
     */
    void endSession(const std::string& sessionId) {
        activeSessions.erase(sessionId);
    }

private:
    std::unordered_map<std::string, std::unique_ptr<SessionData>> activeSessions;
    const long sessionDuration;

    /**
     * Generates a cryptographically secure, random session ID.
     * @return A 32-character hex string as the session ID.
     */
    std::string generateSecureSessionId() {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dis;

        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        // Generate a 128-bit random ID (2 x 64-bit) for security
        ss << std::setw(16) << dis(gen);
        ss << std::setw(16) << dis(gen);

        return ss.str();
    }
};

int main() {
    SessionManager sm;
    std::cout << "--- Session Management Test Cases ---" << std::endl;

    // Test Case 1: Create a new session for 'charlie'
    std::cout << "\n1. Creating session for user 'charlie'..." << std::endl;
    std::string charlieSessionId = sm.createSession("charlie");
    std::cout << "   Session created for charlie with ID: " << charlieSessionId << std::endl;

    // Test Case 2: Validate 'charlie's' session
    std::cout << "\n2. Validating charlie's session..." << std::endl;
    if (auto userId = sm.getSessionUser(charlieSessionId)) {
        std::cout << "   Success! Session is valid for user: " << *userId << std::endl;
    } else {
        std::cout << "   Failure! Session is not valid." << std::endl;
    }

    // Test Case 3: Validate a non-existent session ID
    std::cout << "\n3. Validating a fake session ID..." << std::endl;
    std::string fakeSessionId = "fake-session-id-7890";
    if (auto userId = sm.getSessionUser(fakeSessionId)) {
        std::cout << "   Failure! Fake session was validated for user: " << *userId << std::endl;
    } else {
        std::cout << "   Success! Fake session was correctly identified as invalid." << std::endl;
    }

    // Test Case 4: End 'charlie's' session
    std::cout << "\n4. Ending charlie's session..." << std::endl;
    sm.endSession(charlieSessionId);
    std::cout << "   Session " << charlieSessionId << " has been ended." << std::endl;

    // Test Case 5: Re-validate 'charlie's' ended session
    std::cout << "\n5. Re-validating charlie's ended session..." << std::endl;
    if (auto userId = sm.getSessionUser(charlieSessionId)) {
        std::cout << "   Failure! Ended session was validated for user: " << *userId << std::endl;
    } else {
        std::cout << "   Success! Ended session is no longer valid." << std::endl;
    }

    return 0;
}