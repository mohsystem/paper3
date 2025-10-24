#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <mutex>
#include <chrono>
#include <cstdio>
#include <cstring>

#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#endif

// Chain-of-Through summary in comments:
// 1) Implement secure session manager with create/validate/get/refresh/invalidate/prune.
// 2) Security: OS CSPRNG, base64url tokens, expiration, rotate tokens on refresh, mutex for thread safety.
// 3) Parameter checks, avoid revealing secrets, careful time use.
// 4) Reviewed for potential issues.
// 5) Final secure version.

struct Session {
    std::string userId;
    int64_t createdAtMs;
    int64_t expiresAtMs;
};

struct SessionInfo {
    std::string userId;
    int64_t expiresAtMs;
};

static int64_t nowMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

// Secure random bytes using OS facilities
static bool secureRandomBytes(unsigned char* buf, size_t len) {
#if defined(_WIN32)
    NTSTATUS status = BCryptGenRandom(NULL, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status == 0;
#else
    FILE* f = std::fopen("/dev/urandom", "rb");
    if (!f) return false;
    size_t readTotal = 0;
    while (readTotal < len) {
        size_t r = std::fread(buf + readTotal, 1, len - readTotal, f);
        if (r == 0) break;
        readTotal += r;
    }
    std::fclose(f);
    return readTotal == len;
#endif
}

// Base64 URL-safe encode without padding
static std::string base64UrlEncode(const std::vector<unsigned char>& data) {
    static const char* b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    size_t i = 0;
    unsigned int val = 0;
    int valb = -6;
    for (unsigned char c : data) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(b64[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(b64[((val << 8) >> (valb + 8)) & 0x3F]);
    // Replace +/ with -_ and strip =
    for (char& ch : out) {
        if (ch == '+') ch = '-';
        else if (ch == '/') ch = '_';
    }
    while (out.size() % 4 != 0) out.push_back('='); // pad then remove
    // remove padding
    while (!out.empty() && out.back() == '=') out.pop_back();
    return out;
}

class SessionManager {
public:
    explicit SessionManager(int64_t ttlMs, size_t tokenBytes = 32)
        : ttlMs_(ttlMs), tokenBytes_(tokenBytes) {
        if (ttlMs_ <= 0) throw std::invalid_argument("ttlMs must be > 0");
        if (tokenBytes_ < 16) throw std::invalid_argument("tokenBytes must be >= 16");
    }

    std::string createSession(const std::string& userId) {
        if (userId.empty()) throw std::invalid_argument("userId required");
        std::string token = generateToken();
        int64_t now = nowMs();
        std::lock_guard<std::mutex> g(mu_);
        sessions_[token] = Session{userId, now, now + ttlMs_};
        return token;
    }

    bool validateSession(const std::string& token) {
        if (token.empty()) return false;
        int64_t now = nowMs();
        std::lock_guard<std::mutex> g(mu_);
        auto it = sessions_.find(token);
        if (it == sessions_.end()) return false;
        if (now > it->second.expiresAtMs) {
            sessions_.erase(it);
            return false;
        }
        return true;
    }

    SessionInfo* getSessionInfo(const std::string& token) {
        if (!validateSession(token)) return nullptr;
        std::lock_guard<std::mutex> g(mu_);
        auto it = sessions_.find(token);
        if (it == sessions_.end()) return nullptr;
        tmpInfo_ = SessionInfo{it->second.userId, it->second.expiresAtMs};
        return &tmpInfo_;
    }

    bool invalidateSession(const std::string& token) {
        if (token.empty()) return false;
        std::lock_guard<std::mutex> g(mu_);
        return sessions_.erase(token) > 0;
    }

    std::string refreshSession(const std::string& token) {
        if (!validateSession(token)) return std::string();
        int64_t now = nowMs();
        std::lock_guard<std::mutex> g(mu_);
        auto it = sessions_.find(token);
        if (it == sessions_.end()) return std::string();
        std::string newToken = generateToken();
        sessions_[newToken] = Session{it->second.userId, it->second.createdAtMs, now + ttlMs_};
        sessions_.erase(it);
        return newToken;
    }

    int pruneExpiredSessions() {
        int removed = 0;
        int64_t now = nowMs();
        std::lock_guard<std::mutex> g(mu_);
        for (auto it = sessions_.begin(); it != sessions_.end(); ) {
            if (now > it->second.expiresAtMs) {
                it = sessions_.erase(it);
                removed++;
            } else {
                ++it;
            }
        }
        return removed;
    }

private:
    std::string generateToken() {
        std::vector<unsigned char> buf(tokenBytes_);
        if (!secureRandomBytes(buf.data(), buf.size())) {
            throw std::runtime_error("CSPRNG failed");
        }
        return base64UrlEncode(buf);
    }

    std::unordered_map<std::string, Session> sessions_;
    std::mutex mu_;
    int64_t ttlMs_;
    size_t tokenBytes_;
    SessionInfo tmpInfo_; // reused buffer to return pointer safely within lock scope (callers use immediately)
};

int main() {
    SessionManager mgr(5000);

    // 1) Create and validate session
    std::string t1 = mgr.createSession("alice");
    std::cout << "Test1 valid: " << (mgr.validateSession(t1) ? "true" : "false") << "\n";

    // 2) Get session info
    SessionInfo* info = mgr.getSessionInfo(t1);
    std::cout << "Test2 info: " << (info ? ("SessionInfo(userId='" + info->userId + "', expiresAtMs=" + std::to_string(info->expiresAtMs) + ")") : "null") << "\n";

    // 3) Refresh session: old invalid, new valid
    std::string t1New = mgr.refreshSession(t1);
    std::cout << "Test3 new valid: " << (mgr.validateSession(t1New) ? "true" : "false")
              << ", old valid: " << (mgr.validateSession(t1) ? "true" : "false") << "\n";

    // 4) Invalidate session
    bool inv = mgr.invalidateSession(t1New);
    std::cout << "Test4 invalidated: " << (inv ? "true" : "false")
              << ", validate after: " << (mgr.validateSession(t1New) ? "true" : "false") << "\n";

    // 5) Expiration test
    SessionManager shortMgr(100);
    std::string t2 = shortMgr.createSession("bob");
    std::cout << "Test5 before expiry valid: " << (shortMgr.validateSession(t2) ? "true" : "false") << "\n";
#if defined(_WIN32)
    Sleep(200);
#else
    struct timespec ts; ts.tv_sec = 0; ts.tv_nsec = 200000000; nanosleep(&ts, nullptr);
#endif
    shortMgr.pruneExpiredSessions();
    std::cout << "Test5 after expiry valid: " << (shortMgr.validateSession(t2) ? "true" : "false") << "\n";

    return 0;
}