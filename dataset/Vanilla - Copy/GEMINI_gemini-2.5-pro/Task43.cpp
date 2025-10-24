#include <iostream>
#include <string>
#include <unordered_map>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include <optional>

// Note: C++ does not have a standard library for creating a web application.
// This code simulates the core logic of a secure session manager that would
// be part of a web application backend. It does not run a live server.

class SessionManager {
private:
    struct Session {
        std::string username;
        std::chrono::steady_clock::time_point expires;
    };

    std::unordered_map<std::string, Session> sessions;
    const std::chrono::seconds session_duration = std::chrono::minutes(5);

    // Securely generates a random session ID
    std::string generate_session_id() {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<unsigned long long> dis;
        
        std::stringstream ss;
        ss << std::hex << std::setw(16) << std::setfill('0') << dis(gen);
        ss << std::hex << std::setw(16) << std::setfill('0') << dis(gen);
        return ss.str();
    }

public:
    // Creates a new session for a user
    std::string createSession(const std::string& username) {
        std::string session_id = generate_session_id();
        auto expires = std::chrono::steady_clock::now() + session_duration;
        sessions[session_id] = {username, expires};
        return session_id;
    }

    // Retrieves the username associated with a session ID, if valid and not expired
    std::optional<std::string> getSessionUser(const std::string& session_id) {
        auto it = sessions.find(session_id);
        if (it == sessions.end()) {
            return std::nullopt; // Session not found
        }

        if (std::chrono::steady_clock::now() > it->second.expires) {
            sessions.erase(it); // Clean up expired session
            return std::nullopt; // Session expired
        }

        return it->second.username;
    }

    // Terminates a given session
    void terminateSession(const std::string& session_id) {
        sessions.erase(session_id);
    }
};

int main() {
    SessionManager manager;

    std::cout << "--- Session Management Test Cases ---" << std::endl;

    // Test Case 1: Create a new session for 'testuser'
    std::cout << "\n1. Creating session for 'testuser'..." << std::endl;
    std::string sid = manager.createSession("testuser");
    std::cout << "   Session created with ID: " << sid << std::endl;

    // Test Case 2: Validate the session and get the user
    std::cout << "\n2. Validating session..." << std::endl;
    auto user = manager.getSessionUser(sid);
    if (user) {
        std::cout << "   Success: Session is valid for user '" << *user << "'." << std::endl;
    } else {
        std::cout << "   Failure: Session is not valid." << std::endl;
    }

    // Test Case 3: Try to validate a non-existent session
    std::cout << "\n3. Validating a fake session ID..." << std::endl;
    user = manager.getSessionUser("fake_session_id");
    if (user) {
        std::cout << "   Failure: Fake session was somehow valid for user '" << *user << "'." << std::endl;
    } else {
        std::cout << "   Success: Fake session is correctly identified as invalid." << std::endl;
    }
    
    // Test Case 4: Terminate the session (logout)
    std::cout << "\n4. Terminating session..." << std::endl;
    manager.terminateSession(sid);
    std::cout << "   Session with ID " << sid << " terminated." << std::endl;

    // Test Case 5: Try to validate the terminated session
    std::cout << "\n5. Validating terminated session..." << std::endl;
    user = manager.getSessionUser(sid);
    if (user) {
        std::cout << "   Failure: Terminated session was somehow still valid for user '" << *user << "'." << std::endl;
    } else {
        std::cout << "   Success: Terminated session is correctly identified as invalid." << std::endl;
    }

    return 0;
}