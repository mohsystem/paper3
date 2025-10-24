#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <thread>

#include <openssl/rand.h>
#include <openssl/crypto.h>

// Helper to convert bytes to a hex string
std::string bytesToHex(const std::vector<unsigned char>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned char b : bytes) {
        ss << std::setw(2) << static_cast<int>(b);
    }
    return ss.str();
}

struct SessionData {
    std::string userId;
    std::chrono::steady_clock::time_point expiration;
};

class SessionManager {
public:
    SessionManager() = default;

    // Disable copy and move
    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;
    SessionManager(SessionManager&&) = delete;
    SessionManager& operator=(SessionManager&&) = delete;

    std::string createSession(const std::string& userId) {
        if (userId.empty()) {
            throw std::invalid_argument("User ID cannot be empty.");
        }

        std::vector<unsigned char> buffer(SESSION_ID_BYTES);
        if (RAND_bytes(buffer.data(), buffer.size()) != 1) {
            throw std::runtime_error("Failed to generate random bytes for session ID.");
        }
        
        std::string sessionId = bytesToHex(buffer);
        auto expiration = std::chrono::steady_clock::now() + SESSION_TIMEOUT;

        std::lock_guard<std::mutex> lock(sessionMutex);
        sessions[sessionId] = {userId, expiration};
        
        return sessionId;
    }

    std::string getSessionUser(const std::string& sessionId) {
        if (sessionId.empty()) {
            return "";
        }

        std::lock_guard<std::mutex> lock(sessionMutex);
        auto it = sessions.find(sessionId);

        if (it == sessions.end()) {
            return "";
        }

        if (it->second.expiration < std::chrono::steady_clock::now()) {
            sessions.erase(it); // Lazily remove expired session
            return "";
        }

        return it->second.userId;
    }

    void invalidateSession(const std::string& sessionId) {
        if (sessionId.empty()) {
            return;
        }

        std::lock_guard<std::mutex> lock(sessionMutex);
        sessions.erase(sessionId);
    }

    void setSessionTimeout(std::chrono::milliseconds timeout) {
        SESSION_TIMEOUT = timeout;
    }

private:
    static const int SESSION_ID_BYTES = 32;
    std::chrono::milliseconds SESSION_TIMEOUT{std::chrono::minutes(30)};

    std::unordered_map<std::string, SessionData> sessions;
    std::mutex sessionMutex;
};

void run_tests() {
    SessionManager sessionManager;

    std::cout << "--- Test Case 1: Create and validate a session ---" << std::endl;
    std::string userId1 = "user-123";
    std::string sessionId1 = sessionManager.createSession(userId1);
    std::cout << "Created session for " << userId1 << std::endl;
    std::string retrievedUser1 = sessionManager.getSessionUser(sessionId1);
    std::cout << "Validated session, user is: " << retrievedUser1 << std::endl;
    std::cout << "Test Case 1 Passed: " << (userId1 == retrievedUser1 ? "True" : "False") << std::endl << std::endl;

    std::cout << "--- Test Case 2: Invalidate a session ---" << std::endl;
    sessionManager.invalidateSession(sessionId1);
    std::string retrievedUser2 = sessionManager.getSessionUser(sessionId1);
    std::cout << "After invalidation, user is: " << (retrievedUser2.empty() ? "Not Found" : retrievedUser2) << std::endl;
    std::cout << "Test Case 2 Passed: " << (retrievedUser2.empty() ? "True" : "False") << std::endl << std::endl;

    std::cout << "--- Test Case 3: Validate a non-existent session ---" << std::endl;
    std::string retrievedUser3 = sessionManager.getSessionUser("non-existent-session-id");
    std::cout << "Validating non-existent session, user is: " << (retrievedUser3.empty() ? "Not Found" : retrievedUser3) << std::endl;
    std::cout << "Test Case 3 Passed: " << (retrievedUser3.empty() ? "True" : "False") << std::endl << std::endl;

    std::cout << "--- Test Case 4: Session expiration ---" << std::endl;
    SessionManager shortLivedManager;
    shortLivedManager.setSessionTimeout(std::chrono::milliseconds(100));
    std::string userId4 = "user-456";
    std::string sessionId4 = shortLivedManager.createSession(userId4);
    std::cout << "Created short-lived session for " << userId4 << std::endl;
    std::cout << "Waiting for session to expire..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    std::string retrievedUser4 = shortLivedManager.getSessionUser(sessionId4);
    std::cout << "After expiration, user is: " << (retrievedUser4.empty() ? "Not Found" : retrievedUser4) << std::endl;
    std::cout << "Test Case 4 Passed: " << (retrievedUser4.empty() ? "True" : "False") << std::endl << std::endl;

    std::cout << "--- Test Case 5: Handle null/empty inputs ---" << std::endl;
    bool test5Passed = true;
    try {
        sessionManager.createSession("");
        test5Passed = false; // Should have thrown
    } catch (const std::invalid_argument& e) {
        std::cout << "Caught expected exception for createSession(\"\"): " << e.what() << std::endl;
    }
    std::string retrievedUser5 = sessionManager.getSessionUser("");
    std::cout << "getSessionUser(\"\") returned empty: " << (retrievedUser5.empty() ? "True" : "False") << std::endl;
    if (!retrievedUser5.empty()) test5Passed = false;
    std::cout << "Test Case 5 Passed: " << (test5Passed ? "True" : "False") << std::endl << std::endl;
}

int main() {
    run_tests();
    return 0;
}