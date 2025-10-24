// Chain-of-Through Step 1: Problem understanding
// Secure in-memory session manager in C++.

// Chain-of-Through Step 2: Security requirements
// - Use strong randomness (std::random_device) for token
// - Clamp TTL, validate user id
// - Thread-safety with mutex

// Chain-of-Through Step 3/4: Implement and review with safe practices

// Chain-of-Through Step 5: Final code

#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <mutex>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include <algorithm>

class Task123 {
private:
    struct Session {
        std::string userId;
        int64_t createdAt;
        int64_t ttlSec;
        int64_t expiresAt;
    };

    static constexpr size_t TOKEN_BYTES = 32;
    static constexpr size_t MAX_USERID_LEN = 128;
    static constexpr int64_t MIN_TTL_SEC = 1;
    static constexpr int64_t MAX_TTL_SEC = 7 * 24 * 60 * 60;

    std::unordered_map<std::string, Session> sessions;
    std::mutex mtx;

    static int64_t nowSec() {
        using namespace std::chrono;
        return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
    }

    static int64_t clampTtl(int64_t ttl) {
        if (ttl < MIN_TTL_SEC) return MIN_TTL_SEC;
        if (ttl > MAX_TTL_SEC) return MAX_TTL_SEC;
        return ttl;
    }

    static bool validUserId(const std::string& userId) {
        return !userId.empty() && userId.size() <= MAX_USERID_LEN;
    }

    static std::string bytesToHex(const std::vector<unsigned char>& bytes) {
        std::ostringstream oss;
        for (auto b : bytes) {
            oss << std::hex << std::setw(2) << std::setfill('0') << (int)b;
        }
        return oss.str();
    }

    static std::string genToken() {
        std::random_device rd;
        std::vector<unsigned char> buf(TOKEN_BYTES);
        for (size_t i = 0; i < TOKEN_BYTES; ++i) {
            buf[i] = static_cast<unsigned char>(rd());
        }
        return bytesToHex(buf);
    }

public:
    std::string createSession(const std::string& userId, int64_t ttlSec) {
        if (!validUserId(userId)) {
            throw std::invalid_argument("Invalid userId");
        }
        int64_t ttl = clampTtl(ttlSec);
        std::string token = genToken();
        std::lock_guard<std::mutex> lock(mtx);
        while (sessions.find(token) != sessions.end()) {
            token = genToken();
        }
        int64_t now = nowSec();
        sessions[token] = Session{userId, now, ttl, now + ttl};
        return token;
    }

    bool isValid(const std::string& token) {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = sessions.find(token);
        if (it == sessions.end()) return false;
        int64_t now = nowSec();
        if (now >= it->second.expiresAt) {
            sessions.erase(it);
            return false;
        }
        return true;
    }

    bool refresh(const std::string& token) {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = sessions.find(token);
        if (it == sessions.end()) return false;
        int64_t now = nowSec();
        if (now >= it->second.expiresAt) {
            sessions.erase(it);
            return false;
        }
        it->second.expiresAt = now + it->second.ttlSec;
        return true;
    }

    bool destroy(const std::string& token) {
        std::lock_guard<std::mutex> lock(mtx);
        return sessions.erase(token) > 0;
    }

    int purgeExpired() {
        std::lock_guard<std::mutex> lock(mtx);
        int64_t now = nowSec();
        int removed = 0;
        for (auto it = sessions.begin(); it != sessions.end(); ) {
            if (now >= it->second.expiresAt) {
                it = sessions.erase(it);
                removed++;
            } else {
                ++it;
            }
        }
        return removed;
    }

    int revokeUserSessions(const std::string& userId) {
        if (!validUserId(userId)) return 0;
        std::lock_guard<std::mutex> lock(mtx);
        int removed = 0;
        for (auto it = sessions.begin(); it != sessions.end(); ) {
            if (it->second.userId == userId) {
                it = sessions.erase(it);
                removed++;
            } else {
                ++it;
            }
        }
        return removed;
    }

    int countActiveSessions() {
        purgeExpired();
        std::lock_guard<std::mutex> lock(mtx);
        return static_cast<int>(sessions.size());
    }

    // Test helper
    bool debugForceExpire(const std::string& token) {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = sessions.find(token);
        if (it == sessions.end()) return false;
        it->second.expiresAt = nowSec() - 1;
        return true;
    }
};

int main() {
    Task123 mgr;

    // Test 1
    std::string s1 = mgr.createSession("userA", 60);
    std::cout << "T1 valid: " << (mgr.isValid(s1) ? "true" : "false") << "\n";

    // Test 2
    std::string s2 = mgr.createSession("userB", 5);
    mgr.debugForceExpire(s2);
    std::cout << "T2 valid after force expire: " << (mgr.isValid(s2) ? "true" : "false") << "\n";
    std::cout << "T2 purge removed: " << mgr.purgeExpired() << "\n";

    // Test 3
    std::cout << "T3 refresh s1: " << (mgr.refresh(s1) ? "true" : "false") << "\n";
    std::cout << "T3 s1 still valid: " << (mgr.isValid(s1) ? "true" : "false") << "\n";

    // Test 4
    std::cout << "T4 destroy s1: " << (mgr.destroy(s1) ? "true" : "false") << "\n";
    std::cout << "T4 s1 valid after destroy: " << (mgr.isValid(s1) ? "true" : "false") << "\n";

    // Test 5
    std::string a = mgr.createSession("userC", 10);
    std::string b = mgr.createSession("userC", 10);
    std::string c = mgr.createSession("userC", 10);
    std::cout << "T5 revoke userC count: " << mgr.revokeUserSessions("userC") << "\n";
    std::cout << "T5 active sessions: " << mgr.countActiveSessions() << "\n";

    return 0;
}