#include <iostream>
#include <string>
#include <unordered_map>
#include <chrono>
#include <vector>
#include <mutex>
#include <fstream>
#include <stdexcept>
#include <thread>
#include <memory>

// A utility function to Base64 encode a byte vector.
// This is a simplified implementation for demonstration purposes.
std::string base64_encode(const std::vector<unsigned char>& data) {
    const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    for (unsigned char const& c : data) {
        char_array_3[i++] = c;
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while ((i++ < 3))
            ret += '=';
    }

    return ret;
}

// Function to generate a secure random byte vector.
// Note: This implementation uses /dev/urandom and is suitable for POSIX systems (Linux, macOS).
// For Windows, you would use the Cryptography API (e.g., BCryptGenRandom).
std::vector<unsigned char> generate_random_bytes(size_t len) {
    std::ifstream urandom("/dev/urandom", std::ios::in | std::ios::binary);
    if (!urandom) {
        throw std::runtime_error("Failed to open /dev/urandom");
    }
    std::vector<unsigned char> buffer(len);
    urandom.read(reinterpret_cast<char*>(buffer.data()), len);
    if (!urandom) {
        throw std::runtime_error("Failed to read from /dev/urandom");
    }
    return buffer;
}


class Task43 {
public:
    struct Session {
        std::string userId;
        std::chrono::steady_clock::time_point creationTime;
        std::chrono::steady_clock::time_point lastAccessedTime;

        Session(std::string uId) : userId(std::move(uId)) {
            creationTime = std::chrono::steady_clock::now();
            lastAccessedTime = creationTime;
        }
    };

    class SessionManager {
    private:
        std::unordered_map<std::string, Session> sessionStore;
        std::mutex storeMutex; // Mutex to protect access to the session store

    public:
        static constexpr long ABSOLUTE_TIMEOUT_SECONDS = 5;
        static constexpr long IDLE_TIMEOUT_SECONDS = 2;

        std::string createSession(const std::string& userId) {
            auto random_bytes = generate_random_bytes(32);
            std::string sessionId = base64_encode(random_bytes);
            
            std::lock_guard<std::mutex> lock(storeMutex);
            sessionStore.emplace(sessionId, Session(userId));
            return sessionId;
        }

        std::shared_ptr<Session> getSession(const std::string& sessionId) {
            if (sessionId.empty()) {
                return nullptr;
            }

            std::lock_guard<std::mutex> lock(storeMutex);
            auto it = sessionStore.find(sessionId);

            if (it == sessionStore.end()) {
                return nullptr; // Session not found
            }

            auto& session = it->second;
            auto now = std::chrono::steady_clock::now();

            if (std::chrono::duration_cast<std::chrono::seconds>(now - session.creationTime).count() > ABSOLUTE_TIMEOUT_SECONDS) {
                sessionStore.erase(it);
                return nullptr; // Absolute timeout
            }

            if (std::chrono::duration_cast<std::chrono::seconds>(now - session.lastAccessedTime).count() > IDLE_TIMEOUT_SECONDS) {
                sessionStore.erase(it);
                return nullptr; // Idle timeout
            }

            session.lastAccessedTime = now;
            return std::make_shared<Session>(session);
        }

        void invalidateSession(const std::string& sessionId) {
            if (!sessionId.empty()) {
                std::lock_guard<std::mutex> lock(storeMutex);
                sessionStore.erase(sessionId);
            }
        }
    };
};

int main() {
    Task43::SessionManager manager;
    std::cout << "Running CPP Session Manager Test Cases..." << std::endl;
    std::cout << "Absolute Timeout: " << Task43::SessionManager::ABSOLUTE_TIMEOUT_SECONDS 
              << "s, Idle Timeout: " << Task43::SessionManager::IDLE_TIMEOUT_SECONDS << "s\n" << std::endl;

    // Test Case 1: Create and validate a new session
    std::cout << "--- Test Case 1: Create and Validate Session ---" << std::endl;
    std::string sessionId1 = manager.createSession("user123");
    std::cout << "Created session for user123: " << sessionId1 << std::endl;
    auto s1 = manager.getSession(sessionId1);
    std::cout << "Session valid? " << (s1 ? "Yes, for user " + s1->userId : "No") << std::endl;
    std::cout << std::endl;

    // Test Case 2: Invalidate a session
    std::cout << "--- Test Case 2: Invalidate Session ---" << std::endl;
    manager.invalidateSession(sessionId1);
    std::cout << "Session invalidated." << std::endl;
    s1 = manager.getSession(sessionId1);
    std::cout << "Session valid after invalidation? " << (s1 != nullptr) << std::endl;
    std::cout << std::endl;

    // Test Case 3: Idle timeout
    std::cout << "--- Test Case 3: Idle Timeout ---" << std::endl;
    std::string sessionId2 = manager.createSession("user456");
    std::cout << "Created session for user456: " << sessionId2 << std::endl;
    std::cout << "Waiting for " << (Task43::SessionManager::IDLE_TIMEOUT_SECONDS + 1) << " seconds to trigger idle timeout..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(Task43::SessionManager::IDLE_TIMEOUT_SECONDS + 1));
    auto s2 = manager.getSession(sessionId2);
    std::cout << "Session valid after idle period? " << (s2 != nullptr) << std::endl;
    std::cout << std::endl;

    // Test Case 4: Absolute timeout
    std::cout << "--- Test Case 4: Absolute Timeout ---" << std::endl;
    std::string sessionId3 = manager.createSession("user789");
    std::cout << "Created session for user789: " << sessionId3 << std::endl;
    std::cout << "Accessing session every second to prevent idle timeout..." << std::endl;
    for (int i = 0; i < Task43::SessionManager::ABSOLUTE_TIMEOUT_SECONDS - 1; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        manager.getSession(sessionId3); // Keep it active
        std::cout << "Accessed at second " << (i + 1) << std::endl;
    }
    std::cout << "Waiting for absolute timeout..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    auto s3 = manager.getSession(sessionId3);
    std::cout << "Session valid after absolute timeout period? " << (s3 != nullptr) << std::endl;
    std::cout << std::endl;

    // Test Case 5: Prevent idle timeout by activity
    std::cout << "--- Test Case 5: Prevent Idle Timeout ---" << std::endl;
    std::string sessionId4 = manager.createSession("userABC");
    std::cout << "Created session for userABC: " << sessionId4 << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(Task43::SessionManager::IDLE_TIMEOUT_SECONDS - 1));
    std::cout << "Accessing session just before idle timeout..." << std::endl;
    auto s4_active = manager.getSession(sessionId4);
    std::cout << "Session is still active? " << (s4_active != nullptr) << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(Task43::SessionManager::IDLE_TIMEOUT_SECONDS - 1));
    s4_active = manager.getSession(sessionId4);
    std::cout << "Accessing again. Session still active? " << (s4_active != nullptr) << std::endl;
    std::cout << std::endl;

    return 0;
}