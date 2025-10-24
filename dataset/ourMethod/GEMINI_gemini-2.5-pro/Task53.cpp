#include <iostream>
#include <string>
#include <unordered_map>
#include <chrono>
#include <vector>
#include <mutex>
#include <iomanip>
#include <sstream>
#include <memory>
#include <openssl/rand.h>
#include <openssl/crypto.h>

// Constants
constexpr size_t SESSION_ID_BYTES = 32;
constexpr std::chrono::minutes SESSION_DURATION(30);

// Helper function to convert bytes to a hex string
std::string bytes_to_hex(const unsigned char* data, size_t len) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) {
        ss << std::setw(2) << static_cast<unsigned int>(data[i]);
    }
    return ss.str();
}

// Session data structure
struct Session {
    std::string username;
    std::chrono::steady_clock::time_point expiration;

    bool is_expired() const {
        return std::chrono::steady_clock::now() > expiration;
    }
};

class SessionManager {
public:
    SessionManager() = default;
    ~SessionManager() = default;

    // Prevent copying and assignment
    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;

    // Creates a new session for a user and returns the session ID.
    // Returns an empty string on failure.
    std::string create_session(const std::string& username) {
        if (username.empty()) {
            return "";
        }

        unsigned char session_id_bytes[SESSION_ID_BYTES];
        std::string session_id_hex;
        
        std::lock_guard<std::mutex> lock(mtx_);

        // Generate a unique session ID
        while (true) {
            if (RAND_bytes(session_id_bytes, sizeof(session_id_bytes)) != 1) {
                // OpenSSL's random number generator failed
                return ""; 
            }
            session_id_hex = bytes_to_hex(session_id_bytes, sizeof(session_id_bytes));
            if (sessions_.find(session_id_hex) == sessions_.end()) {
                break; // Unique ID found
            }
        }

        Session new_session;
        new_session.username = username;
        new_session.expiration = std::chrono::steady_clock::now() + SESSION_DURATION;
        
        sessions_[session_id_hex] = new_session;

        return session_id_hex;
    }

    // Retrieves the username for a given session ID.
    // Returns an empty string if the session is not found or is expired.
    std::string get_session_user(const std::string& session_id) {
        if (session_id.length() != SESSION_ID_BYTES * 2) {
             return ""; // Invalid format
        }

        std::lock_guard<std::mutex> lock(mtx_);
        
        auto it = sessions_.find(session_id);
        if (it == sessions_.end()) {
            return ""; // Session not found
        }

        if (it->second.is_expired()) {
            sessions_.erase(it); // Clean up expired session
            return "";
        }

        return it->second.username;
    }

    // Ends a session.
    // Returns true if the session was found and removed, false otherwise.
    bool end_session(const std::string& session_id) {
         if (session_id.length() != SESSION_ID_BYTES * 2) {
             return false; // Invalid format
        }
        
        std::lock_guard<std::mutex> lock(mtx_);
        
        // unordered_map::erase(key) returns number of elements erased.
        return sessions_.erase(session_id) > 0;
    }

private:
    std::unordered_map<std::string, Session> sessions_;
    std::mutex mtx_;
};

int main() {
    SessionManager manager;

    std::cout << "--- Test Case 1: Create and validate a session ---" << std::endl;
    std::string user1 = "alice";
    std::string session1_id = manager.create_session(user1);
    if (!session1_id.empty()) {
        std::cout << "Created session for " << user1 << "." << std::endl;
        std::string found_user = manager.get_session_user(session1_id);
        std::cout << "Session ID " << (session1_id.substr(0, 16) + "...") << " belongs to: " << found_user << std::endl;
        if (found_user != user1) {
            std::cout << "TEST FAILED: User mismatch." << std::endl;
        } else {
            std::cout << "TEST PASSED" << std::endl;
        }
    } else {
        std::cerr << "TEST FAILED: Session creation failed." << std::endl;
    }
    std::cout << std::endl;

    std::cout << "--- Test Case 2: Validate a non-existent session ---" << std::endl;
    std::string non_existent_id = "0000000000000000000000000000000000000000000000000000000000000000";
    std::string found_user2 = manager.get_session_user(non_existent_id);
    std::cout << "Searching for non-existent session ID..." << std::endl;
    if (found_user2.empty()) {
        std::cout << "Session not found, as expected." << std::endl;
        std::cout << "TEST PASSED" << std::endl;
    } else {
        std::cerr << "TEST FAILED: Found a user for a non-existent session." << std::endl;
    }
    std::cout << std::endl;

    std::cout << "--- Test Case 3: Create and then end a session ---" << std::endl;
    std::string user2 = "bob";
    std::string session2_id = manager.create_session(user2);
    if (!session2_id.empty()) {
        std::cout << "Created session for " << user2 << "." << std::endl;
        bool ended = manager.end_session(session2_id);
        if (ended) {
            std::cout << "Session ended successfully." << std::endl;
            std::string found_user3 = manager.get_session_user(session2_id);
            if (found_user3.empty()) {
                std::cout << "Verified session is gone." << std::endl;
                std::cout << "TEST PASSED" << std::endl;
            } else {
                std::cerr << "TEST FAILED: Session still exists after being ended." << std::endl;
            }
        } else {
            std::cerr << "TEST FAILED: Failed to end session." << std::endl;
        }
    } else {
        std::cerr << "TEST FAILED: Session creation failed." << std::endl;
    }
    std::cout << std::endl;

    std::cout << "--- Test Case 4: Try to end a session that doesn't exist ---" << std::endl;
    bool ended_non_existent = manager.end_session(non_existent_id);
    if (!ended_non_existent) {
        std::cout << "Attempt to end non-existent session failed, as expected." << std::endl;
        std::cout << "TEST PASSED" << std::endl;
    } else {
        std::cerr << "TEST FAILED: end_session returned true for a non-existent session." << std::endl;
    }
    std::cout << std::endl;
    
    std::cout << "--- Test Case 5: Create session for another user ---" << std::endl;
    std::string user3 = "charlie";
    std::string session3_id = manager.create_session(user3);
    if (!session3_id.empty()) {
        std::cout << "Created session for " << user3 << "." << std::endl;
        std::string found_user4 = manager.get_session_user(session3_id);
        std::cout << "Session ID " << (session3_id.substr(0, 16) + "...") << " belongs to: " << found_user4 << std::endl;
        if (found_user4 == user3) {
            std::string still_found_user1 = manager.get_session_user(session1_id);
             if (still_found_user1 == user1) {
                std::cout << "Original session for 'alice' is still valid." << std::endl;
                std::cout << "TEST PASSED" << std::endl;
            } else {
                 std::cerr << "TEST FAILED: Original session for 'alice' is gone." << std::endl;
            }
        } else {
            std::cerr << "TEST FAILED: User mismatch for 'charlie'." << std::endl;
        }
    } else {
        std::cerr << "TEST FAILED: Session creation failed for 'charlie'." << std::endl;
    }

    return 0;
}