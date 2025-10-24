#include <iostream>
#include <string>
#include <unordered_map>
#include <random>
#include <sstream>
#include <iomanip>
#include <cassert>

// NOTE: This is a simulation of session management logic, not a functional
// web application. C++ does not have a standard library for web development.
// Building a web app requires external libraries (e.g., Boost.Beast, Crow).
// Also, for cryptographically secure random numbers, platform-specific APIs
// (like /dev/urandom on Linux) or a library like OpenSSL should be used
// instead of <random> for production code.

class SessionManager {
private:
    std::unordered_map<std::string, std::string> sessionStore;

    // Generates a random string to be used as a session ID.
    // WARNING: Not cryptographically secure. For demonstration purposes only.
    std::string generateSessionId(size_t length = 32) {
        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<> distribution(0, 255);
        
        std::stringstream ss;
        for (size_t i = 0; i < length; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << distribution(generator);
        }
        return ss.str();
    }

public:
    // Creates a new session for a user.
    // Returns the session ID.
    std::string createSession(const std::string& username) {
        if (username.empty()) {
            throw std::invalid_argument("Username cannot be empty.");
        }
        std::string sessionId = generateSessionId();
        sessionStore[sessionId] = username;
        return sessionId;
    }

    // Retrieves the username for a given session ID.
    // Returns an empty string if the session is not found.
    std::string getSessionUser(const std::string& sessionId) {
        auto it = sessionStore.find(sessionId);
        if (it != sessionStore.end()) {
            // In a real app, check for session expiration here.
            return it->second; // Return username
        }
        return ""; // Session not found
    }

    // Terminates a session.
    void invalidateSession(const std::string& sessionId) {
        sessionStore.erase(sessionId);
    }
};

int main() {
    SessionManager sessionManager;
    std::cout << "--- Secure Session Management Test ---" << std::endl;

    // Test Case 1: Create a session for user 'alice'
    std::cout << "\n1. Creating session for 'alice'..." << std::endl;
    std::string aliceSessionId = sessionManager.createSession("alice");
    std::cout << "   Alice's session ID created: " << aliceSessionId << std::endl;
    std::string user = sessionManager.getSessionUser(aliceSessionId);
    std::cout << "   Verified user for session ID: " << user << std::endl;
    assert(user == "alice");

    // Test Case 2: Create a session for user 'bob'
    std::cout << "\n2. Creating session for 'bob'..." << std::endl;
    std::string bobSessionId = sessionManager.createSession("bob");
    std::cout << "   Bob's session ID created: " << bobSessionId << std::endl;
    user = sessionManager.getSessionUser(bobSessionId);
    std::cout << "   Verified user for session ID: " << user << std::endl;
    assert(user == "bob");

    // Test Case 3: Validate an invalid/non-existent session ID
    std::cout << "\n3. Validating a non-existent session ID..." << std::endl;
    std::string fakeSessionId = "invalid-session-id-12345";
    user = sessionManager.getSessionUser(fakeSessionId);
    std::cout << "   User for fake session ID: " << (user.empty() ? "null" : user.c_str()) << std::endl;
    assert(user.empty());

    // Test Case 4: Invalidate Alice's session and try to use it again
    std::cout << "\n4. Invalidating Alice's session..." << std::endl;
    sessionManager.invalidateSession(aliceSessionId);
    std::cout << "   Alice's session has been invalidated." << std::endl;
    user = sessionManager.getSessionUser(aliceSessionId);
    std::cout << "   Attempting to verify Alice's old session ID: " << (user.empty() ? "null" : user.c_str()) << std::endl;
    assert(user.empty());

    // Test Case 5: Ensure Bob's session is still valid
    std::cout << "\n5. Verifying Bob's session is still active..." << std::endl;
    user = sessionManager.getSessionUser(bobSessionId);
    std::cout << "   User for Bob's session ID: " << user << std::endl;
    assert(user == "bob");

    std::cout << "\n--- All test cases passed ---" << std::endl;
    
    return 0;
}