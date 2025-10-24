#include <iostream>
#include <string>
#include <unordered_map>
#include <random>
#include <sstream>

class Task53 {
private:
    std::unordered_map<std::string, std::string> activeSessions;

    // Helper to generate a somewhat unique ID without a UUID library
    std::string generateSessionId() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(0, 0xFFFFFFFF);
        std::stringstream ss;
        ss << std::hex << distrib(gen) << "-" << distrib(gen);
        return ss.str();
    }

public:
    std::string createSession(const std::string& username) {
        std::string sessionId = generateSessionId();
        activeSessions[sessionId] = username;
        std::cout << "Session created for " << username << " with ID: " << sessionId << std::endl;
        return sessionId;
    }

    std::string getUserFromSession(const std::string& sessionId) {
        if (activeSessions.count(sessionId)) {
            return activeSessions.at(sessionId);
        }
        return ""; // Return empty string if not found
    }

    void endSession(const std::string& sessionId) {
        if (activeSessions.count(sessionId)) {
            std::string username = activeSessions[sessionId];
            activeSessions.erase(sessionId);
            std::cout << "Session ended for " << username << " with ID: " << sessionId << std::endl;
        } else {
            std::cout << "Session with ID " << sessionId << " not found." << std::endl;
        }
    }

    bool isSessionActive(const std::string& sessionId) {
        return activeSessions.count(sessionId) > 0;
    }
};

int main() {
    Task53 sessionManager;
    std::cout << "--- Running Test Cases ---" << std::endl;

    // Test Case 1: Create a session for user 'alice'
    std::cout << "\n--- Test Case 1: Create Session ---" << std::endl;
    std::string aliceSessionId = sessionManager.createSession("alice");

    // Test Case 2: Check if alice's session is active
    std::cout << "\n--- Test Case 2: Check Active Session ---" << std::endl;
    std::cout << "Is alice's session active? " << (sessionManager.isSessionActive(aliceSessionId) ? "true" : "false") << std::endl;

    // Test Case 3: Get user from session ID
    std::cout << "\n--- Test Case 3: Get User from Session ---" << std::endl;
    std::string username = sessionManager.getUserFromSession(aliceSessionId);
    std::cout << "User for session " << aliceSessionId << " is: " << username << std::endl;

    // Test Case 4: Create another session for 'bob' and end 'alice's session
    std::cout << "\n--- Test Case 4: End Session ---" << std::endl;
    std::string bobSessionId = sessionManager.createSession("bob");
    sessionManager.endSession(aliceSessionId);
    
    // Test Case 5: Check if alice's session is still active and bob's is active
    std::cout << "\n--- Test Case 5: Verify Session Status ---" << std::endl;
    std::cout << "Is alice's session active after ending? " << (sessionManager.isSessionActive(aliceSessionId) ? "true" : "false") << std::endl;
    std::cout << "Is bob's session active? " << (sessionManager.isSessionActive(bobSessionId) ? "true" : "false") << std::endl;
    std::string nonExistentUser = sessionManager.getUserFromSession(aliceSessionId);
    std::cout << "Attempting to get user from ended session: " << (nonExistentUser.empty() ? "Not Found" : nonExistentUser) << std::endl;
    
    return 0;
}