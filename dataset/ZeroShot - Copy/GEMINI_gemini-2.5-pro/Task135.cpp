#include <iostream>
#include <string>
#include <unordered_map>
#include <random>
#include <sstream>
#include <iomanip>
#include <cassert>

class Task135 {
private:
    // Simulates a server-side session store (e.g., in-memory, Redis, etc.)
    // Maps Session ID -> CSRF Token
    static std::unordered_map<std::string, std::string> sessionStore;

    /**
     * Generates a cryptographically strong random token.
     * In a real application, use a more robust entropy source if available.
     * @return A new CSRF token as a hex string.
     */
    static std::string generateToken() {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<unsigned long long> dis;

        std::stringstream ss;
        // Generate 32 bytes of random data (4 * 8 bytes)
        for(int i = 0; i < 4; ++i) {
            ss << std::hex << std::setw(16) << std::setfill('0') << dis(gen);
        }
        return ss.str();
    }

public:
    /**
     * Simulates a user requesting a page with a form.
     * Generates and stores a CSRF token in the user's session.
     *
     * @param sessionId The user's session identifier.
     * @return The generated CSRF token.
     */
    static std::string generateCsrfToken(const std::string& sessionId) {
        if (sessionId.empty()) {
            throw std::invalid_argument("Session ID cannot be empty.");
        }
        std::string token = generateToken();
        sessionStore[sessionId] = token;
        return token;
    }

    /**
     * Simulates a form submission and validates the CSRF token.
     *
     * @param sessionId The user's session identifier.
     * @param submittedToken The CSRF token received from the submitted form.
     * @return true if the token is valid, false otherwise.
     */
    static bool validateCsrfToken(const std::string& sessionId, const std::string& submittedToken) {
        if (sessionId.empty() || submittedToken.empty()) {
            return false;
        }

        auto it = sessionStore.find(sessionId);
        if (it == sessionStore.end()) {
            return false; // No session or token found for this user
        }

        // IMPORTANT: In a real application, use a constant-time comparison function
        // to prevent timing attacks. Standard string comparison is not secure for this.
        return it->second == submittedToken;
    }
};

// Initialize static member
std::unordered_map<std::string, std::string> Task135::sessionStore;

int main() {
    std::cout << "--- CSRF Protection Simulation ---" << std::endl;

    std::string user1SessionId = "session_abc_123";
    std::string user2SessionId = "session_xyz_789";

    // Test Case 1: Valid request
    std::cout << "\n--- Test Case 1: Valid Request ---" << std::endl;
    std::string user1Token = Task135::generateCsrfToken(user1SessionId);
    std::cout << "User 1 generated token: " << user1Token << std::endl;
    bool isValid1 = Task135::validateCsrfToken(user1SessionId, user1Token);
    std::cout << "Form submission with correct token is valid: " << std::boolalpha << isValid1 << std::endl;
    assert(isValid1);

    // Test Case 2: Invalid request (wrong token)
    std::cout << "\n--- Test Case 2: Invalid Request (Wrong Token) ---" << std::endl;
    std::string attackerToken = "fake_malicious_token";
    std::cout << "User 1 submitting with a wrong token: " << attackerToken << std::endl;
    bool isValid2 = Task135::validateCsrfToken(user1SessionId, attackerToken);
    std::cout << "Form submission with wrong token is valid: " << std::boolalpha << isValid2 << std::endl;
    assert(!isValid2);

    // Test Case 3: Invalid request (missing token)
    std::cout << "\n--- Test Case 3: Invalid Request (Missing Token) ---" << std::endl;
    bool isValid3 = Task135::validateCsrfToken(user1SessionId, "");
    std::cout << "Form submission with missing token is valid: " << std::boolalpha << isValid3 << std::endl;
    assert(!isValid3);

    // Test Case 4: Invalid request (token from a different user's session)
    std::cout << "\n--- Test Case 4: Invalid Request (Token from another session) ---" << std::endl;
    std::string user2Token = Task135::generateCsrfToken(user2SessionId);
    std::cout << "User 2 generated token: " << user2Token << std::endl;
    std::cout << "User 1 attempts to submit form with User 2's token." << std::endl;
    bool isValid4 = Task135::validateCsrfToken(user1SessionId, user2Token);
    std::cout << "Submission is valid: " << std::boolalpha << isValid4 << std::endl;
    assert(!isValid4);
    
    // Test Case 5: Valid request after token regeneration
    std::cout << "\n--- Test Case 5: Valid Request after Token Regeneration ---" << std::endl;
    std::cout << "User 1 old token: " << user1Token << std::endl;
    std::string user1NewToken = Task135::generateCsrfToken(user1SessionId); // Token is regenerated
    std::cout << "User 1 new token: " << user1NewToken << std::endl;
    bool isValid5_old = Task135::validateCsrfToken(user1SessionId, user1Token); // Old token is now invalid
    bool isValid5_new = Task135::validateCsrfToken(user1SessionId, user1NewToken); // New token is valid
    std::cout << "Submission with old token is valid: " << std::boolalpha << isValid5_old << std::endl;
    std::cout << "Submission with new token is valid: " << std::boolalpha << isValid5_new << std::endl;
    assert(!isValid5_old);
    assert(isValid5_new);

    return 0;
}