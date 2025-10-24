#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <map>
#include <sstream>
#include <iomanip>

class Task135 {
private:
    // Simulates a server-side session store
    static std::map<std::string, std::string> sessionStore;

public:
    /**
     * Generates a cryptographically secure CSRF token.
     * Uses std::random_device as a source of non-deterministic random numbers.
     * Note: The quality of std::random_device is implementation-defined.
     * On high-security systems, OS-specific APIs (e.g., /dev/urandom) might be preferred.
     * @return A hex-encoded CSRF token string.
     */
    static std::string generateCsrfToken() {
        std::vector<unsigned char> buffer(32);
        std::random_device rd;
        
        // Fill buffer with random values
        for (size_t i = 0; i < buffer.size(); ++i) {
            buffer[i] = static_cast<unsigned char>(rd() % 256);
        }

        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (const auto& byte : buffer) {
            ss << std::setw(2) << static_cast<int>(byte);
        }
        return ss.str();
    }

    /**
     * Stores the CSRF token in the simulated session store.
     * @param sessionId The user's session identifier.
     * @param token The CSRF token to store.
     */
    static void storeToken(const std::string& sessionId, const std::string& token) {
        sessionStore[sessionId] = token;
    }

    /**
     * Performs a constant-time comparison of two strings to prevent timing attacks.
     * @param a The first string.
     * @param b The second string.
     * @return True if strings are equal, false otherwise.
     */
    static bool constantTimeEquals(const std::string& a, const std::string& b) {
        if (a.length() != b.length()) {
            return false;
        }

        // Use bitwise operations to prevent short-circuiting
        int result = 0;
        for (size_t i = 0; i < a.length(); ++i) {
            result |= (a[i] ^ b[i]);
        }

        return result == 0;
    }

    /**
     * Validates a received CSRF token against the one stored in the session.
     * @param sessionId The user's session identifier.
     * @param receivedToken The token received from the client request.
     * @return True if the token is valid, false otherwise.
     */
    static bool validateCsrfToken(const std::string& sessionId, const std::string& receivedToken) {
        auto it = sessionStore.find(sessionId);
        if (it == sessionStore.end()) {
            return false; // No token found for this session
        }
        
        return constantTimeEquals(it->second, receivedToken);
    }
};

// Initialize the static session store
std::map<std::string, std::string> Task135::sessionStore;

int main() {
    // --- Test Cases ---
    std::string sessionId1 = "user_session_abc123";
    std::string sessionId2 = "user_session_def456";

    // Test Case 1: Successful Validation
    std::cout << "--- Test Case 1: Successful Validation ---" << std::endl;
    std::string token1 = Task135::generateCsrfToken();
    Task135::storeToken(sessionId1, token1);
    std::cout << "Generated Token: " << token1 << std::endl;
    std::cout << "Is token valid? " << (Task135::validateCsrfToken(sessionId1, token1) ? "true" : "false") << std::endl << std::endl;

    // Test Case 2: Failed Validation (Incorrect Token)
    std::cout << "--- Test Case 2: Failed Validation (Incorrect Token) ---" << std::endl;
    std::string correctToken = Task135::generateCsrfToken();
    Task135::storeToken(sessionId2, correctToken);
    std::string incorrectToken = "this_is_a_wrong_token";
    std::cout << "Stored Token (first 10 chars): " << correctToken.substr(0, 10) << "..." << std::endl;
    std::cout << "Submitted Token: " << incorrectToken << std::endl;
    std::cout << "Is token valid? " << (Task135::validateCsrfToken(sessionId2, incorrectToken) ? "true" : "false") << std::endl << std::endl;

    // Test Case 3: Failed Validation (No Token in Session)
    std::cout << "--- Test Case 3: Failed Validation (No Token in Session) ---" << std::endl;
    std::string nonExistentSessionId = "non_existent_session_xyz789";
    std::string submittedToken = Task135::generateCsrfToken();
    std::cout << "Submitted Token (first 10 chars): " << submittedToken.substr(0, 10) << "..." << std::endl;
    std::cout << "Is token for non-existent session valid? " << (Task135::validateCsrfToken(nonExistentSessionId, submittedToken) ? "true" : "false") << std::endl << std::endl;

    // Test Case 4: Failed Validation (Empty Submitted Token)
    std::cout << "--- Test Case 4: Failed Validation (Empty Submitted Token) ---" << std::endl;
    std::cout << "Is empty token valid? " << (Task135::validateCsrfToken(sessionId1, "") ? "true" : "false") << std::endl << std::endl;

    // Test Case 5: Token Re-generation and Validation
    std::cout << "--- Test Case 5: Token Re-generation and Validation ---" << std::endl;
    auto it = Task135::sessionStore.find(sessionId1);
    std::string oldToken = (it != Task135::sessionStore.end()) ? it->second : "N/A";
    std::string newToken = Task135::generateCsrfToken();
    Task135::storeToken(sessionId1, newToken);
    std::cout << "Old token (first 10 chars): " << oldToken.substr(0, 10) << "..." << std::endl;
    std::cout << "New token (first 10 chars): " << newToken.substr(0, 10) << "..." << std::endl;
    std::cout << "Is old token still valid? " << (Task135::validateCsrfToken(sessionId1, oldToken) ? "true" : "false") << std::endl;
    std::cout << "Is new token valid? " << (Task135::validateCsrfToken(sessionId1, newToken) ? "true" : "false") << std::endl;

    return 0;
}