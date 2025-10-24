#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <chrono>
#include <cstring>

#if defined(_WIN32)
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#elif defined(__linux__)
#include <sys/random.h>
#include <unistd.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

struct Session {
    std::string token;
    std::string userId;
    std::string clientIP;
    std::string userAgent;
    uint64_t expiresAtMs;
};

static bool ct_equals(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    unsigned char r = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        r |= static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i]);
    }
    return r == 0;
}

static bool secure_random_bytes(unsigned char* buf, size_t len) {
#if defined(_WIN32)
    return BCryptGenRandom(NULL, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG) == 0;
#elif defined(__linux__)
    ssize_t n = getrandom(buf, len, 0);
    return n == (ssize_t)len;
#else
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return false;
    size_t off = 0;
    while (off < len) {
        ssize_t n = read(fd, buf + off, len - off);
        if (n <= 0) { close(fd); return false; }
        off += (size_t)n;
    }
    close(fd);
    return true;
#endif
}

static std::string hex_of(const unsigned char* data, size_t len) {
    static const char* hex = "0123456789abcdef";
    std::string out;
    out.resize(len * 2);
    for (size_t i = 0; i < len; ++i) {
        out[2*i] = hex[(data[i] >> 4) & 0xF];
        out[2*i + 1] = hex[data[i] & 0xF];
    }
    return out;
}

class SessionManager {
public:
    explicit SessionManager(uint64_t defaultTtlMs = 1800000ULL) : defaultTtlMs_(defaultTtlMs) {}

    std::string createSession(const std::string& userId, const std::string& clientIP, const std::string& userAgent, uint64_t ttlMs = 0) {
        cleanupExpired();
        if (bad(userId, 128) || bad(clientIP, 64) || bad(userAgent, 256)) return "";
        uint64_t ttl = (ttlMs == 0 ? defaultTtlMs_ : ttlMs);
        if (ttl < 1000ULL) ttl = 1000ULL;
        if (ttl > 86400000ULL) ttl = 86400000ULL;

        unsigned char buf[32];
        if (!secure_random_bytes(buf, sizeof(buf))) return "";
        std::string token = hex_of(buf, sizeof(buf));
        uint64_t expiresAt = nowMs() + ttl;

        Session s{token, userId, clientIP, userAgent, expiresAt};
        sessions_[token] = s;
        userIndex_[userId].insert(token);
        return token;
    }

    bool validateSession(const std::string& token, const std::string& clientIP, const std::string& userAgent) {
        cleanupExpired();
        if (bad(token, 128) || bad(clientIP, 64) || bad(userAgent, 256)) return false;
        auto it = sessions_.find(token);
        if (it == sessions_.end()) return false;
        Session& s = it->second;
        if (s.expiresAtMs <= nowMs()) {
            terminateSession(token);
            return false;
        }
        return ct_equals(s.clientIP, clientIP) && ct_equals(s.userAgent, userAgent);
    }

    std::string getUserIdIfValid(const std::string& token, const std::string& clientIP, const std::string& userAgent) {
        return validateSession(token, clientIP, userAgent) ? sessions_[token].userId : "";
    }

    std::string refreshSession(const std::string& token, const std::string& clientIP, const std::string& userAgent) {
        cleanupExpired();
        if (!validateSession(token, clientIP, userAgent)) return "";
        auto it = sessions_.find(token);
        if (it == sessions_.end()) return "";
        Session old = it->second;

        unsigned char buf[32];
        if (!secure_random_bytes(buf, sizeof(buf))) return "";
        std::string newTok = hex_of(buf, sizeof(buf));
        Session newer{newTok, old.userId, old.clientIP, old.userAgent, nowMs() + defaultTtlMs_};

        sessions_[newTok] = newer;
        userIndex_[old.userId].insert(newTok);
        terminateSession(token);
        return newTok;
    }

    bool terminateSession(const std::string& token) {
        auto it = sessions_.find(token);
        if (it == sessions_.end()) return false;
        std::string uid = it->second.userId;
        sessions_.erase(it);
        auto uit = userIndex_.find(uid);
        if (uit != userIndex_.end()) {
            uit->second.erase(token);
            if (uit->second.empty()) userIndex_.erase(uit);
        }
        return true;
    }

    int terminateAllForUser(const std::string& userId) {
        auto uit = userIndex_.find(userId);
        if (uit == userIndex_.end()) return 0;
        int count = 0;
        for (const auto& tok : uit->second) {
            if (sessions_.erase(tok) > 0) count++;
        }
        userIndex_.erase(uit);
        return count;
    }

private:
    static bool bad(const std::string& s, size_t maxlen) {
        return s.empty() || s.size() > maxlen;
    }

    static uint64_t nowMs() {
        using namespace std::chrono;
        return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    }

    void cleanupExpired() {
        uint64_t now = nowMs();
        std::vector<std::string> toErase;
        for (const auto& kv : sessions_) {
            if (kv.second.expiresAtMs <= now) {
                toErase.push_back(kv.first);
            }
        }
        for (const auto& t : toErase) {
            auto it = sessions_.find(t);
            if (it != sessions_.end()) {
                std::string uid = it->second.userId;
                sessions_.erase(it);
                auto uit = userIndex_.find(uid);
                if (uit != userIndex_.end()) {
                    uit->second.erase(t);
                    if (uit->second.empty()) userIndex_.erase(uit);
                }
            }
        }
    }

    uint64_t defaultTtlMs_;
    std::unordered_map<std::string, Session> sessions_;
    std::unordered_map<std::string, std::unordered_set<std::string>> userIndex_;
};

int main() {
    SessionManager sm(1800000ULL);
    std::string ip = "203.0.113.10";
    std::string ua = "ExampleBrowser/1.0";
    std::string badIp = "198.51.100.5";

    std::string tok1 = sm.createSession("alice", ip, ua);
    std::cout << "T1 valid: " << (sm.validateSession(tok1, ip, ua) ? "true" : "false") << "\n";

    std::cout << "T2 wrong IP valid: " << (sm.validateSession(tok1, badIp, ua) ? "true" : "false") << "\n";

    std::string tok2 = sm.refreshSession(tok1, ip, ua);
    std::cout << "T3 old valid: " << (sm.validateSession(tok1, ip, ua) ? "true" : "false") << "\n";
    std::cout << "T3 new valid: " << (sm.validateSession(tok2, ip, ua) ? "true" : "false") << "\n";

    std::string tok3 = sm.createSession("bob", ip, ua, 1000ULL);
#if defined(_WIN32)
    Sleep(1200);
#else
    usleep(1200 * 1000);
#endif
    std::cout << "T4 expired valid: " << (sm.validateSession(tok3, ip, ua) ? "true" : "false") << "\n";

    std::string tok4 = sm.createSession("carol", ip, ua);
    std::cout << "T5 before terminate: " << (sm.validateSession(tok4, ip, ua) ? "true" : "false") << "\n";
    sm.terminateSession(tok4);
    std::cout << "T5 after terminate: " << (sm.validateSession(tok4, ip, ua) ? "true" : "false") << "\n";

    return 0;
}