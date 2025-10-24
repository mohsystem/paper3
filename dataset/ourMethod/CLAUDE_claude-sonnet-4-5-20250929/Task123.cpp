
#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <stdexcept>
#include <cstring>
#include <ctime>
#include <algorithm>
#include <random>
#include <sstream>
#include <iomanip>

// OpenSSL for cryptographic operations
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

/**
 * Secure user session management system in C++.
 * Uses OpenSSL for cryptographic operations.
 * Prevents session fixation, hijacking, and timing attacks.
 */

namespace SecureSession {

/**
 * Represents a user session with security attributes.
 */
class Session {
private:
    std::string session_id_;
    std::string user_id_;
    std::time_t created_at_;
    std::time_t last_activity_;
    std::string ip_address_;
    std::string user_agent_hash_;

public:
    Session(const std::string& session_id, const std::string& user_id,
            std::time_t created_at, std::time_t last_activity,
            const std::string& ip_address, const std::string& user_agent_hash)
        : session_id_(session_id), user_id_(user_id),
          created_at_(created_at), last_activity_(last_activity),
          ip_address_(ip_address), user_agent_hash_(user_agent_hash) {
        
        // Input validation in constructor
        if (session_id.empty()) {
            throw std::invalid_argument("session_id cannot be empty");
        }
        if (user_id.empty()) {
            throw std::invalid_argument("user_id cannot be empty");
        }
        if (ip_address.empty()) {
            throw std::invalid_argument("ip_address cannot be empty");
        }
        if (user_agent_hash.empty()) {
            throw std::invalid_argument("user_agent_hash cannot be empty");
        }
    }

    const std::string& getSessionId() const { return session_id_; }
    const std::string& getUserId() const { return user_id_; }
    std::time_t getCreatedAt() const { return created_at_; }
    std::time_t getLastActivity() const { return last_activity_; }
    void setLastActivity(std::time_t last_activity) { last_activity_ = last_activity; }
    const std::string& getIpAddress() const { return ip_address_; }
    const std::string& getUserAgentHash() const { return user_agent_hash_; }
};

/**
 * Secure session manager with protection against common attacks.
 */
class SessionManager {
private:
    std::map<std::string, std::unique_ptr<Session>> sessions_;
    int session_timeout_;   // in seconds
    int absolute_timeout_;  // in seconds
    unsigned char hmac_key_[32];  // 256-bit key

    /**
     * Generate cryptographically secure random bytes.
     * Uses OpenSSL's RAND_bytes which is cryptographically secure.\n     */\n    bool generateSecureRandom(unsigned char* buffer, size_t length) {\n        // RAND_bytes returns 1 on success, 0 on failure\n        if (RAND_bytes(buffer, static_cast<int>(length)) != 1) {\n            return false;\n        }\n        return true;\n    }\n\n    /**\n     * Hash a string using SHA-256.\n     */\n    std::string hashSHA256(const std::string& input) {\n        unsigned char hash[SHA256_DIGEST_LENGTH];\n        \n        // Use OpenSSL's SHA256 function
        SHA256(reinterpret_cast<const unsigned char*>(input.c_str()),
               input.length(), hash);

        // Convert to hex string
        std::stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') 
               << static_cast<int>(hash[i]);
        }
        return ss.str();
    }

    /**
     * Constant-time string comparison to prevent timing attacks.
     * Uses CRYPTO_memcmp from OpenSSL which is constant-time.
     */
    bool constantTimeEquals(const std::string& a, const std::string& b) {
        if (a.empty() || b.empty()) {
            return false;
        }
        
        // Strings must be same length for constant-time comparison
        if (a.length() != b.length()) {
            return false;
        }

        // CRYPTO_memcmp returns 0 if equal (constant-time)
        return CRYPTO_memcmp(a.data(), b.data(), a.length()) == 0;
    }

    /**
     * Encode bytes to Base64 URL-safe format.
     */
    std::string base64UrlEncode(const unsigned char* buffer, size_t length) {
        // Use OpenSSL's EVP_EncodeBlock for Base64 encoding\n        size_t encoded_length = ((length + 2) / 3) * 4;\n        std::vector<unsigned char> encoded(encoded_length + 1);\n        \n        int result = EVP_EncodeBlock(encoded.data(), buffer, \n                                     static_cast<int>(length));\n        if (result < 0) {\n            throw std::runtime_error("Base64 encoding failed");\n        }\n\n        std::string encoded_str(reinterpret_cast<char*>(encoded.data()), result);\n\n        // Convert to URL-safe format: replace + with -, / with _, remove padding\n        std::replace(encoded_str.begin(), encoded_str.end(), '+', '-');\n        std::replace(encoded_str.begin(), encoded_str.end(), '/', '_');\n        \n        // Remove padding '='\n        size_t padding_pos = encoded_str.find('=');\n        if (padding_pos != std::string::npos) {\n            encoded_str.erase(padding_pos);\n        }\n\n        return encoded_str;\n    }\n\npublic:\n    /**\n     * Initialize session manager with security parameters.\n     */\n    SessionManager(int session_timeout, int absolute_timeout)\n        : session_timeout_(session_timeout), absolute_timeout_(absolute_timeout) {\n        \n        // Input validation: ensure timeouts are positive\n        if (session_timeout <= 0) {\n            throw std::invalid_argument("session_timeout must be positive");\n        }\n        if (absolute_timeout <= 0) {\n            throw std::invalid_argument("absolute_timeout must be positive");\n        }\n\n        // Generate secure HMAC key using OpenSSL's cryptographically secure RNG
        if (!generateSecureRandom(hmac_key_, sizeof(hmac_key_))) {
            throw std::runtime_error("Failed to generate secure HMAC key");
        }
    }

    /**
     * Create a new session with cryptographically secure ID.
     * Prevents session fixation by generating unpredictable IDs.
     */
    std::string createSession(const std::string& user_id,
                             const std::string& ip_address,
                             const std::string& user_agent) {
        // Input validation: ensure all parameters are non-empty
        if (user_id.empty()) {
            throw std::invalid_argument("user_id must be non-empty");
        }
        if (ip_address.empty()) {
            throw std::invalid_argument("ip_address must be non-empty");
        }
        if (user_agent.empty()) {
            throw std::invalid_argument("user_agent must be non-empty");
        }

        // Validate user_id length to prevent abuse
        if (user_id.length() > 256) {
            throw std::invalid_argument("user_id exceeds maximum length of 256");
        }

        // Generate cryptographically secure session ID (32 bytes = 256 bits)
        unsigned char random_bytes[32];
        if (!generateSecureRandom(random_bytes, sizeof(random_bytes))) {
            throw std::runtime_error("Failed to generate secure session ID");
        }

        std::string session_id = base64UrlEncode(random_bytes, sizeof(random_bytes));

        std::time_t current_time = std::time(nullptr);

        // Hash user agent to avoid storing sensitive browser fingerprint
        std::string user_agent_hash = hashSHA256(user_agent);

        // Create session object with security metadata using smart pointer
        auto session = std::make_unique<Session>(
            session_id,
            user_id,
            current_time,
            current_time,
            ip_address,
            user_agent_hash
        );

        // Store session in map (RAII ensures cleanup)
        sessions_[session_id] = std::move(session);

        return session_id;
    }

    /**
     * Validate session and check for hijacking attempts.
     * Uses constant-time comparison to prevent timing attacks.
     */
    std::string validateSession(const std::string& session_id,
                               const std::string& ip_address,
                               const std::string& user_agent) {
        // Input validation: ensure parameters are non-empty
        if (session_id.empty() || ip_address.empty() || user_agent.empty()) {
            return "";
        }

        // Validate session_id length to prevent abuse
        if (session_id.length() > 256) {
            return "";
        }

        // Retrieve session - fail closed if not found
        auto it = sessions_.find(session_id);
        if (it == sessions_.end()) {
            return "";
        }

        Session* session = it->second.get();
        std::time_t current_time = std::time(nullptr);

        // Check absolute timeout (maximum session lifetime)
        if (current_time - session->getCreatedAt() > absolute_timeout_) {
            destroySession(session_id);
            return "";
        }

        // Check idle timeout
        if (current_time - session->getLastActivity() > session_timeout_) {
            destroySession(session_id);
            return "";
        }

        // Verify IP address matches (prevents session hijacking)
        // Use constant-time comparison to prevent timing attacks
        if (!constantTimeEquals(session->getIpAddress(), ip_address)) {
            // IP mismatch - potential hijacking attempt
            destroySession(session_id);
            return "";
        }

        // Verify user agent hash matches
        std::string user_agent_hash = hashSHA256(user_agent);

        // Use constant-time comparison for user agent hash
        if (!constantTimeEquals(session->getUserAgentHash(), user_agent_hash)) {
            // User agent mismatch - potential hijacking attempt
            destroySession(session_id);
            return "";
        }

        // Update last activity timestamp (session still valid)
        session->setLastActivity(current_time);

        return session->getUserId();
    }

    /**
     * Securely destroy a session.
     */
    bool destroySession(const std::string& session_id) {
        // Input validation
        if (session_id.empty()) {
            return false;
        }

        // Remove session from storage (unique_ptr automatically cleans up)
        return sessions_.erase(session_id) > 0;
    }

    /**
     * Remove all expired sessions from storage.
     */
    int cleanupExpiredSessions() {
        std::time_t current_time = std::time(nullptr);
        int removed_count = 0;

        // Iterate and remove expired sessions
        auto it = sessions_.begin();
        while (it != sessions_.end()) {
            Session* session = it->second.get();

            // Check both absolute and idle timeouts
            if (current_time - session->getCreatedAt() > absolute_timeout_ ||
                current_time - session->getLastActivity() > session_timeout_) {
                it = sessions_.erase(it);
                removed_count++;
            } else {
                ++it;
            }
        }

        return removed_count;
    }

    /**
     * Get session count (for testing/monitoring).
     */
    size_t getSessionCount() const {
        return sessions_.size();
    }

    // Destructor to clean up resources
    ~SessionManager() {
        // Clear HMAC key from memory using memset_s if available
        #ifdef __STDC_LIB_EXT1__
        memset_s(hmac_key_, sizeof(hmac_key_), 0, sizeof(hmac_key_));
        #else
        // Fallback: use volatile to prevent optimization
        volatile unsigned char* p = hmac_key_;
        for (size_t i = 0; i < sizeof(hmac_key_); i++) {
            p[i] = 0;
        }
        #endif
    }
};

} // namespace SecureSession

/**
 * Test cases demonstrating secure session management.
 */
int main() {
    std::cout << "=== Secure Session Manager Test Cases ===\\n\\n";

    try {
        // Initialize session manager with 5-minute idle timeout, 1-hour absolute timeout
        SecureSession::SessionManager manager(300, 3600);

        // Test Case 1: Create and validate a valid session
        std::cout << "Test 1: Create and validate valid session\\n";
        std::string session_id = manager.createSession(
            "user123",
            "192.168.1.100",
            "Mozilla/5.0 (Windows NT 10.0; Win64; x64)"
        );
        std::cout << "  Created session: " << session_id.substr(0, 16) << "...\\n";

        std::string user_id = manager.validateSession(
            session_id,
            "192.168.1.100",
            "Mozilla/5.0 (Windows NT 10.0; Win64; x64)"
        );
        std::cout << "  Validated user: " << user_id << "\\n";
        if (user_id != "user123") {
            throw std::runtime_error("Session validation failed");
        }
        std::cout << "  ✓ Pass\\n\\n";

        // Test Case 2: Reject session with mismatched IP (hijacking prevention)
        std::cout << "Test 2: Reject session with different IP address\\n";
        user_id = manager.validateSession(
            session_id,
            "192.168.1.200",  // Different IP
            "Mozilla/5.0 (Windows NT 10.0; Win64; x64)"
        );
        std::cout << "  Validation result: " << (user_id.empty() ? "null" : user_id) << "\\n";
        if (!user_id.empty()) {
            throw std::runtime_error("Should reject mismatched IP");
        }
        std::cout << "  ✓ Pass (correctly rejected)\\n\\n";

        // Test Case 3: Reject session with mismatched user agent
        std::cout << "Test 3: Reject session with different user agent\\n";
        std::string session_id2 = manager.createSession(
            "user456",
            "10.0.0.50",
            "Chrome/120.0.0.0"
        );
        user_id = manager.validateSession(
            session_id2,
            "10.0.0.50",
            "Firefox/121.0"  // Different user agent
        );
        std::cout << "  Validation result: " << (user_id.empty() ? "null" : user_id) << "\\n";
        if (!user_id.empty()) {
            throw std::runtime_error("Should reject mismatched user agent");
        }
        std::cout << "  ✓ Pass (correctly rejected)\\n\\n";

        // Test Case 4: Destroy session and verify it's invalid
        std::cout << "Test 4: Destroy session\\n";
        std::string session_id3 = manager.createSession(
            "user789",
            "172.16.0.10",
            "Safari/17.0"
        );
        bool destroyed = manager.destroySession(session_id3);
        std::cout << "  Session destroyed: " << (destroyed ? "true" : "false") << "\\n";

        user_id = manager.validateSession(
            session_id3,
            "172.16.0.10",
            "Safari/17.0"
        );
        std::cout << "  Validation after destroy: " << (user_id.empty() ? "null" : user_id) << "\\n";
        if (!user_id.empty()) {
            throw std::runtime_error("Destroyed session should be invalid");
        }
        std::cout << "  ✓ Pass\\n\\n";

        // Test Case 5: Input validation
        std::cout << "Test 5: Input validation for empty/invalid inputs\\n";
        try {
            manager.createSession("", "192.168.1.1", "Browser");
            std::cout << "  ✗ Failed to reject empty user_id\\n";
        } catch (const std::invalid_argument&) {
            std::cout << "  ✓ Correctly rejected empty user_id\\n";
        }

        std::string result = manager.validateSession("", "192.168.1.1", "Browser");
        if (!result.empty()) {
            throw std::runtime_error("Should reject empty session_id");
        }
        std::cout << "  ✓ Correctly rejected empty session_id\\n";
        std::cout << "  ✓ Pass\\n\\n";

        std::cout << "=== All test cases passed ===\\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
