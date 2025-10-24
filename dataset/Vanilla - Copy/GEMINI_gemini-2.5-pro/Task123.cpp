#include <iostream>
#include <string>
#include <unordered_map>
#include <ctime>
#include <sstream>
#include <cstdlib>

// Represents session data
struct Session {
    std::string username;
    time_t creationTime;
};

// In-memory store for sessions using an unordered_map for efficiency
static std::unordered_map<std::string, Session> sessionStore;

// A simple session ID generator (not cryptographically secure)
std::string generateSessionId() {
    static long long counter = 0;
    std::stringstream ss;
    ss << time(nullptr) << "-" << rand() << "-" << counter++;
    return ss.str();
}

/**
 * Creates a new session for a given user.
 * @param username The username to associate with the session.
 * @return The unique session ID.
 */
std::string createSession(const std::string& username) {
    std::string sessionId = generateSessionId();
    Session session = {username, time(nullptr)};
    sessionStore[sessionId] = session;
    std::cout << "Session created for " << username << " with ID: " << sessionId << std::endl;
    return sessionId;
}

/**
 * Retrieves the username associated with a session ID.
 * @param sessionId The session ID to look up.
 * @return The username, or an empty string if the session is not valid.
 */
std::string getSessionUser(const std::string& sessionId) {
    if (sessionStore.count(sessionId)) {
        return sessionStore.at(sessionId).username;
    }
    return ""; // Return empty string to indicate not found
}

/**
 * Invalidates/removes a user session.
 * @param sessionId The session ID to invalidate.
 */
void invalidateSession(const std::string& sessionId) {
    if (sessionStore.erase(sessionId) > 0) {
        std::cout << "Session invalidated: " << sessionId << std::endl;
    } else {
        std::cout << "Session to invalidate not found: " << sessionId << std::endl;
    }
}

/**
 * Checks if a session ID is currently valid.
 * @param sessionId The session ID to check.
 * @return true if the session exists, false otherwise.
 */
bool isSessionValid(const std::string& sessionId) {
    return sessionStore.count(sessionId) > 0;
}

int main() {
    // Seed the random number generator
    srand(time(nullptr));
    
    std::cout << "--- Starting Session Management Test ---" << std::endl;

    // Test Case 1: Create a session for user "alice"
    std::cout << "\n[Test Case 1: Create session for alice]" << std::endl;
    std::string aliceSessionId = createSession("alice");

    // Test Case 2: Check if alice's session is valid and get her username
    std::cout << "\n[Test Case 2: Validate alice's session]" << std::endl;
    std::cout << "Is alice's session valid? " << (isSessionValid(aliceSessionId) ? "true" : "false") << std::endl;
    std::string username = getSessionUser(aliceSessionId);
    std::cout << "Retrieved user for session " << aliceSessionId << ": " << username << std::endl;

    // Test Case 3: Create a second session for user "bob"
    std::cout << "\n[Test Case 3: Create session for bob]" << std::endl;
    std::string bobSessionId = createSession("bob");
    std::cout << "Is bob's session valid? " << (isSessionValid(bobSessionId) ? "true" : "false") << std::endl;

    // Test Case 4: Invalidate alice's session and verify
    std::cout << "\n[Test Case 4: Invalidate alice's session]" << std::endl;
    invalidateSession(aliceSessionId);
    std::cout << "Is alice's session now valid? " << (isSessionValid(aliceSessionId) ? "true" : "false") << std::endl;
    std::string invalidUser = getSessionUser(aliceSessionId);
    std::cout << "Attempting to get user for invalidated session: " 
              << (invalidUser.empty() ? "null" : invalidUser) << std::endl;

    // Test Case 5: Verify that bob's session is still valid
    std::cout << "\n[Test Case 5: Verify bob's session is unaffected]" << std::endl;
    std::cout << "Is bob's session still valid? " << (isSessionValid(bobSessionId) ? "true" : "false") << std::endl;
    std::cout << "Retrieved user for bob's session: " << getSessionUser(bobSessionId) << std::endl;

    std::cout << "\n--- Session Management Test Finished ---" << std::endl;

    return 0;
}