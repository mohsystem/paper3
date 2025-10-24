#include <iostream>
#include <string>
#include <unordered_map>
#include <random>
#include <algorithm>

// Simulates server-side session storage (SessionID -> CSRF_Token)
std::unordered_map<std::string, std::string> session_storage;

/**
 * @brief Generates a random CSRF token.
 *
 * NOTE: For production, use a cryptographically secure random number generator,
 * e.g., from a library like OpenSSL. This is a simplified example.
 *
 * @param length The desired length of the token.
 * @return A random string token.
 */
std::string generateCsrfToken(size_t length = 32) {
    static const std::string chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string token;
    token.reserve(length);
    
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<int> distribution(0, chars.length() - 1);

    for (size_t i = 0; i < length; ++i) {
        token += chars[distribution(generator)];
    }
    return token;
}

/**
 * @brief Stores the CSRF token in the simulated session.
 * @param sessionId The user's session identifier.
 * @param token The CSRF token to store.
 */
void storeTokenInSession(const std::string& sessionId, const std::string& token) {
    session_storage[sessionId] = token;
}

/**
 * @brief Validates the received token against the one stored in the session.
 * 
 * NOTE: This uses a simple string comparison. For enhanced security against
 * timing attacks, a constant-time comparison function should be used.
 * 
 * @param sessionId The user's session identifier.
 * @param receivedToken The token received from the client.
 * @return true if the tokens match, false otherwise.
 */
bool validateCsrfToken(const std::string& sessionId, const std::string& receivedToken) {
    if (receivedToken.empty() || sessionId.empty()) {
        return false;
    }
    
    auto it = session_storage.find(sessionId);
    if (it == session_storage.end()) {
        return false; // No token stored for this session
    }
    
    const std::string& storedToken = it->second;
    return storedToken == receivedToken;
}

/**
 * @brief Simulates processing a protected action.
 * @param sessionId The user's session identifier.
 * @param receivedToken The token received from the client.
 */
void processRequest(const std::string& sessionId, const std::string& receivedToken) {
    std::cout << "Processing request for session: " << sessionId << " with token: " << receivedToken << std::endl;
    if (validateCsrfToken(sessionId, receivedToken)) {
        std::cout << "SUCCESS: CSRF token is valid. Action allowed." << std::endl;
    } else {
        std::cout << "FAILURE: CSRF token is invalid or missing. Action rejected." << std::endl;
    }
    std::cout << "-------------------------------------------------" << std::endl;
}

int main() {
    // --- Test Case 1: Valid Request ---
    std::cout << "Test Case 1: Valid Request" << std::endl;
    std::string sessionId1 = "user_session_abc123";
    std::string token1 = generateCsrfToken();
    storeTokenInSession(sessionId1, token1);
    // User submits the form with the correct token
    processRequest(sessionId1, token1);

    // --- Test Case 2: Invalid/Incorrect Token ---
    std::cout << "Test Case 2: Invalid/Incorrect Token" << std::endl;
    std::string sessionId2 = "user_session_def456";
    std::string token2 = generateCsrfToken();
    storeTokenInSession(sessionId2, token2);
    // Attacker tries to submit the form with a guessed/wrong token
    processRequest(sessionId2, "wrong_token_-_");
    
    // --- Test Case 3: Missing Token ---
    std::cout << "Test Case 3: Missing Token" << std::endl;
    std::string sessionId3 = "user_session_ghi789";
    std::string token3 = generateCsrfToken();
    storeTokenInSession(sessionId3, token3);
    // Attacker's request is missing the token
    processRequest(sessionId3, "");

    // --- Test Case 4: No Token in Session ---
    std::cout << "Test Case 4: No Token in Session" << std::endl;
    std::string sessionId4 = "user_session_jkl012";
    // No token is generated or stored for this session
    // A request comes in with some token
    processRequest(sessionId4, "some_random_token");

    // --- Test Case 5: Token Mismatch between Users ---
    std::cout << "Test Case 5: Token Mismatch between Users" << std::endl;
    std::string userA_sessionId = "user_A_session";
    std::string userA_token = generateCsrfToken();
    storeTokenInSession(userA_sessionId, userA_token);
    
    std::string userB_sessionId = "user_B_session";
    std::string userB_token = generateCsrfToken();
    storeTokenInSession(userB_sessionId, userB_token);

    std::cout << "Attacker tries to use User B's token for User A's session:" << std::endl;
    // Attacker on a different session tries to use User B's token to attack User A
    processRequest(userA_sessionId, userB_token);

    return 0;
}