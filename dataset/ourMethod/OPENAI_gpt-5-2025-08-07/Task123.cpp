#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <random>
#include <cctype>

struct Session {
    std::string user;
    long long expires_at;
    long long last_access;
};

class SessionManager {
public:
    bool addSession(const std::string& token, const std::string& user, long long ttlSeconds) {
        if (!isValidToken(token) || !isValidUser(user)) return false;
        long long ttl = clampTtl(ttlSeconds);
        if (ttl <= 0) return false;
        if (sessions.find(token) != sessions.end()) return false;
        long long now = nowSeconds();
        sessions.emplace(token, Session{user, now + ttl, now});
        return true;
    }

    std::string validateSession(const std::string& token) {
        if (!isValidToken(token)) return "";
        auto it = sessions.find(token);
        if (it == sessions.end()) return "";
        long long now = nowSeconds();
        if (now > it->second.expires_at) {
            sessions.erase(it);
            return "";
        }
        it->second.last_access = now;
        return it->second.user;
    }

    bool refreshSession(const std::string& oldToken, const std::string& newToken, long long ttlSeconds) {
        if (!isValidToken(oldToken) || !isValidToken(newToken)) return false;
        if (constantTimeEquals(oldToken, newToken)) return false;
        auto it = sessions.find(oldToken);
        if (it == sessions.end()) return false;
        long long now = nowSeconds();
        if (now > it->second.expires_at) {
            sessions.erase(it);
            return false;
        }
        if (sessions.find(newToken) != sessions.end()) return false;
        long long ttl = clampTtl(ttlSeconds);
        Session ns{it->second.user, now + ttl, now};
        sessions.erase(it);
        sessions.emplace(newToken, ns);
        return true;
    }

    bool revokeSession(const std::string& token) {
        if (!isValidToken(token)) return false;
        return sessions.erase(token) > 0;
    }

    int pruneExpired() {
        int removed = 0;
        long long now = nowSeconds();
        for (auto it = sessions.begin(); it != sessions.end(); ) {
            if (now > it->second.expires_at) {
                it = sessions.erase(it);
                ++removed;
            } else {
                ++it;
            }
        }
        return removed;
    }

    int countActive() {
        pruneExpired();
        return static_cast<int>(sessions.size());
    }

private:
    std::unordered_map<std::string, Session> sessions;

    static bool isValidToken(const std::string& token) {
        if (token.size() < 16 || token.size() > 256) return false;
        for (unsigned char c : token) {
            if (!(std::isalnum(c) || c == '-' || c == '_')) return false;
        }
        return true;
    }

    static bool isValidUser(const std::string& user) {
        if (user.empty() || user.size() > 64) return false;
        for (unsigned char c : user) {
            if (!(std::isalnum(c) || c == '-' || c == '_' || c == '.')) return false;
        }
        return true;
    }

    static long long clampTtl(long long ttl) {
        if (ttl < 1) return 0;
        const long long max = 86400;
        if (ttl > max) return max;
        return ttl;
    }

    static bool constantTimeEquals(const std::string& a, const std::string& b) {
        size_t la = a.size(), lb = b.size();
        size_t lmax = la > lb ? la : lb;
        unsigned char diff = static_cast<unsigned char>(la ^ lb);
        for (size_t i = 0; i < lmax; ++i) {
            unsigned char xa = i < la ? static_cast<unsigned char>(a[i]) : 0;
            unsigned char xb = i < lb ? static_cast<unsigned char>(b[i]) : 0;
            diff |= static_cast<unsigned char>(xa ^ xb);
        }
        return diff == 0;
    }

    static long long nowSeconds() {
        using namespace std::chrono;
        return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
    }
};

// Helper: generate token for tests using std::random_device, hex-encoded (do not print tokens)
static std::string genToken(size_t nbytes) {
    if (nbytes < 16) nbytes = 16;
    if (nbytes > 64) nbytes = 64;
    std::random_device rd;
    std::vector<unsigned char> bytes(nbytes);
    for (size_t i = 0; i < nbytes; ++i) bytes[i] = static_cast<unsigned char>(rd());
    static const char* hex = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_";
    // Map each byte into URL-safe alphabet (64 chars)
    std::string out;
    out.reserve(nbytes);
    for (unsigned char b : bytes) {
        out.push_back(hex[b % 64]);
    }
    return out;
}

int main() {
    SessionManager mgr;

    // Test 1: Create and validate
    std::string tok1 = genToken(32);
    bool add1 = mgr.addSession(tok1, "alice", 5);
    std::string v1 = mgr.validateSession(tok1);
    std::cout << "T1 add=" << (add1 ? "true" : "false")
              << " validUser=" << v1 << " active=" << mgr.countActive() << "\n";

    // Test 2: Refresh token
    std::string tok2 = genToken(32);
    bool ref = mgr.refreshSession(tok1, tok2, 5);
    std::string vOld = mgr.validateSession(tok1);
    std::string vNew = mgr.validateSession(tok2);
    std::cout << "T2 refresh=" << (ref ? "true" : "false")
              << " oldValid='" << vOld << "' newValid='" << vNew
              << "' active=" << mgr.countActive() << "\n";

    // Test 3: Revoke
    bool rev = mgr.revokeSession(tok2);
    std::string v3 = mgr.validateSession(tok2);
    std::cout << "T3 revoke=" << (rev ? "true" : "false")
              << " postRevokeValid='" << v3 << "' active=" << mgr.countActive() << "\n";

    // Test 4: Expiration
    std::string tok3 = genToken(24);
    bool add2 = mgr.addSession(tok3, "bob", 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    mgr.pruneExpired();
    std::string v4 = mgr.validateSession(tok3);
    std::cout << "T4 add=" << (add2 ? "true" : "false")
              << " afterExpireValid='" << v4 << "' active=" << mgr.countActive() << "\n";

    // Test 5: Invalid token
    bool addBad = mgr.addSession("short", "charlie", 10);
    std::cout << "T5 invalidTokenAdd=" << (addBad ? "true" : "false")
              << " active=" << mgr.countActive() << "\n";

    return 0;
}