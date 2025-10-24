#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <cassert>

// To compile: g++ -std=c++17 your_file_name.cpp -o your_program_name -lssl -lcrypto
// e.g. g++ -std=c++17 Task135.cpp -o task135 -lssl -lcrypto

/**
 * A simple CSRF protection manager using the synchronizer token pattern.
 * This class is thread-safe.
 */
class CsrfProtector {
public:
    CsrfProtector() = default;

    // Disallow copy and assignment
    CsrfProtector(const CsrfProtector&) = delete;
    CsrfProtector& operator=(const CsrfProtector&) = delete;

    /**
     * Generates a new, cryptographically secure token, stores it associated with
     * the session ID, and returns the hex-encoded token.
     *
     * @param sessionId A unique identifier for the user's session.
     * @return A hex-encoded CSRF token string.
     */
    std::string generateTokenForSession(const std::string& sessionId) {
        if (sessionId.empty()) {
            throw std::invalid_argument("Session ID cannot be empty.");
        }

        std::vector<unsigned char> tokenBytes(TOKEN_BYTE_LENGTH);
        if (RAND_bytes(tokenBytes.data(), tokenBytes.size()) != 1) {
            throw std::runtime_error("Failed to generate random bytes for CSRF token.");
        }

        std::string tokenHex = bytesToHex(tokenBytes);
        
        std::lock_guard<std::mutex> lock(mtx_);
        sessionStore_[sessionId] = tokenHex;
        
        return tokenHex;
    }

    /**
     * Validates a submitted token against the one stored for the session.
     * Uses a constant-time comparison to prevent timing attacks.
     *
     * @param sessionId The user's session identifier.
     * @param submittedToken The token received from the client.
     * @return true if the token is valid, false otherwise.
     */
    bool validateToken(const std::string& sessionId, const std::string& submittedToken) const {
        if (sessionId.empty() || submittedToken.empty()) {
            return false;
        }

        std::string storedToken;
        {
            std::lock_guard<std::mutex> lock(mtx_);
            auto it = sessionStore_.find(sessionId);
            if (it == sessionStore_.end()) {
                return false;
            }
            storedToken = it->second;
        }

        // Constant-time comparison
        return CRYPTO_memcmp(storedToken.c_str(), submittedToken.c_str(), storedToken.length()) == 0 &&
               storedToken.length() == submittedToken.length();
    }

    /**
     * Removes a token from the store, typically done on session invalidation.
     * @param sessionId The user's session identifier.
     */
    void removeToken(const std::string& sessionId) {
        if (!sessionId.empty()) {
            std::lock_guard<std::mutex> lock(mtx_);
            sessionStore_.erase(sessionId);
        }
    }

private:
    static const size_t TOKEN_BYTE_LENGTH = 32;
    // The session store maps session IDs to CSRF tokens.
    mutable std::mutex mtx_;
    std::map<std::string, std::string> sessionStore_;

    std::string bytesToHex(const std::vector<unsigned char>& bytes) const {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (const auto& byte : bytes) {
            ss << std::setw(2) << static_cast<int>(byte);
        }
        return ss.str();
    }
};

void runTests() {
    CsrfProtector csrfProtector;
    std::string sessionId1 = "user_session_abc123";
    std::string sessionId2 = "user_session_def456";

    std::cout << "--- Running CSRF Protection Test Cases ---" << std::endl;

    // Test Case 1: Valid token
    std::cout << "\n--- Test Case 1: Valid Token ---" << std::endl;
    std::string token1 = csrfProtector.generateTokenForSession(sessionId1);
    std::cout << "Generated Token for " << sessionId1 << ": " << token1 << std::endl;
    bool isValid1 = csrfProtector.validateToken(sessionId1, token1);
    std::cout << "Validation result: " << (isValid1 ? "PASS" : "FAIL") << std::endl;
    assert(isValid1);

    // Test Case 2: Invalid token
    std::cout << "\n--- Test Case 2: Invalid Token ---" << std::endl;
    std::string generatedToken2 = csrfProtector.generateTokenForSession(sessionId1);
    std::cout << "Generated Token for " << sessionId1 << ": " << generatedToken2 << std::endl;
    std::string wrongToken = "invalid_token_this_is_wrong_1234567890abcdef";
    bool isValid2 = csrfProtector.validateToken(sessionId1, wrongToken);
    std::cout << "Validation with wrong token result: " << (!isValid2 ? "PASS" : "FAIL") << std::endl;
    assert(!isValid2);

    // Test Case 3: Missing token in session
    std::cout << "\n--- Test Case 3: Missing Token in Session ---" << std::endl;
    bool isValid3 = csrfProtector.validateToken("non_existent_session", "some_token");
    std::cout << "Validation for non-existent session: " << (!isValid3 ? "PASS" : "FAIL") << std::endl;
    assert(!isValid3);

    // Test Case 4: Empty submitted token
    std::cout << "\n--- Test Case 4: Empty Submitted Token ---" << std::endl;
    std::string generatedToken4 = csrfProtector.generateTokenForSession(sessionId1);
    std::cout << "Generated Token for " << sessionId1 << ": " << generatedToken4 << std::endl;
    bool isValid4 = csrfProtector.validateToken(sessionId1, "");
    std::cout << "Validation with empty token: " << (!isValid4 ? "PASS" : "FAIL") << std::endl;
    assert(!isValid4);

    // Test Case 5: Token from a different session
    std::cout << "\n--- Test Case 5: Token from a Different Session ---" << std::endl;
    std::string tokenForSession1 = csrfProtector.generateTokenForSession(sessionId1);
    std::string tokenForSession2 = csrfProtector.generateTokenForSession(sessionId2);
    std::cout << "Token for " << sessionId1 << ": " << tokenForSession1 << std::endl;
    std::cout << "Token for " << sessionId2 << ": " << tokenForSession2 << std::endl;
    bool isValid5 = csrfProtector.validateToken(sessionId1, tokenForSession2);
    std::cout << "Validating session 1 with session 2's token: " << (!isValid5 ? "PASS" : "FAIL") << std::endl;
    assert(!isValid5);

    std::cout << "\n--- All test cases completed ---" << std::endl;
}

int main() {
    try {
        runTests();
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}