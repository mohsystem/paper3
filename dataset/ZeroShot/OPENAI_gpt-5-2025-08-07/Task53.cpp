#include <bits/stdc++.h>
using namespace std;

struct Session {
    string id;
    string userId;
    long long createdAt;
    long long expiresAt;
    long long lastAccessed;
};

class SessionManager {
public:
    SessionManager(size_t idBytes = 32) : idBytes_(idBytes) {
        if (idBytes_ < 16) throw invalid_argument("idBytes too small");
    }

    string createSession(const string& userId, long long ttlSeconds) {
        if (userId.empty() || ttlSeconds <= 0) throw invalid_argument("invalid input");
        string sid = generateId();
        long long now = nowSec();
        Session s{sid, userId, now, now + ttlSeconds, now};
        sessions_[sid] = s;
        userSessions_[userId].insert(sid);
        return sid;
    }

    bool validateSession(const string& sessionId) {
        auto it = sessions_.find(sessionId);
        if (it == sessions_.end()) return false;
        long long now = nowSec();
        if (isExpired(it->second, now)) {
            invalidateSession(sessionId);
            return false;
        }
        it->second.lastAccessed = now;
        return true;
    }

    string getUserForSession(const string& sessionId) {
        auto it = sessions_.find(sessionId);
        if (it == sessions_.end()) return "";
        long long now = nowSec();
        if (isExpired(it->second, now)) {
            invalidateSession(sessionId);
            return "";
        }
        it->second.lastAccessed = now;
        return it->second.userId;
    }

    bool invalidateSession(const string& sessionId) {
        auto it = sessions_.find(sessionId);
        if (it == sessions_.end()) return false;
        auto uit = userSessions_.find(it->second.userId);
        if (uit != userSessions_.end()) {
            uit->second.erase(sessionId);
            if (uit->second.empty()) userSessions_.erase(uit);
        }
        sessions_.erase(it);
        return true;
    }

    int invalidateAllSessionsForUser(const string& userId) {
        int count = 0;
        auto uit = userSessions_.find(userId);
        if (uit != userSessions_.end()) {
            for (const auto& sid : uit->second) {
                if (sessions_.erase(sid) > 0) count++;
            }
            userSessions_.erase(uit);
        }
        return count;
    }

    bool touchSession(const string& sessionId, long long extendTtlSeconds) {
        if (extendTtlSeconds <= 0) return false;
        auto it = sessions_.find(sessionId);
        if (it == sessions_.end()) return false;
        long long now = nowSec();
        if (isExpired(it->second, now)) {
            invalidateSession(sessionId);
            return false;
        }
        it->second.expiresAt = max(it->second.expiresAt, now) + extendTtlSeconds;
        it->second.lastAccessed = now;
        return true;
    }

    string rotateSession(const string& sessionId) {
        auto it = sessions_.find(sessionId);
        if (it == sessions_.end()) return "";
        long long now = nowSec();
        if (isExpired(it->second, now)) {
            invalidateSession(sessionId);
            return "";
        }
        long long remaining = max(0LL, it->second.expiresAt - now);
        if (remaining == 0) {
            invalidateSession(sessionId);
            return "";
        }
        string newId = generateId();
        Session n{newId, it->second.userId, now, now + remaining, now};
        sessions_[newId] = n;
        userSessions_[n.userId].insert(newId);
        invalidateSession(sessionId);
        return newId;
    }

    int cleanupExpiredSessions() {
        long long now = nowSec();
        vector<pair<string,string>> toRemove;
        toRemove.reserve(sessions_.size());
        for (auto& kv : sessions_) {
            if (isExpired(kv.second, now)) {
                toRemove.emplace_back(kv.first, kv.second.userId);
            }
        }
        for (auto& pr : toRemove) {
            sessions_.erase(pr.first);
            auto uit = userSessions_.find(pr.second);
            if (uit != userSessions_.end()) {
                uit->second.erase(pr.first);
                if (uit->second.empty()) userSessions_.erase(uit);
            }
        }
        return static_cast<int>(toRemove.size());
    }

    bool secureCompareIds(const string& a, const string& b) const {
        return ctEquals(a, b);
    }

private:
    size_t idBytes_;
    unordered_map<string, Session> sessions_;
    unordered_map<string, unordered_set<string>> userSessions_;

    static long long nowSec() {
        return chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count();
    }

    static bool isExpired(const Session& s, long long now) {
        return s.expiresAt <= now;
    }

    static bool ctEquals(const string& a, const string& b) {
        const size_t la = a.size(), lb = b.size();
        size_t len = min(la, lb);
        unsigned char r = static_cast<unsigned char>(la ^ lb);
        for (size_t i = 0; i < len; ++i) {
            r |= static_cast<unsigned char>(a[i] ^ b[i]);
        }
        return r == 0;
    }

    string generateId() {
        // Generate idBytes_ random bytes and hex-encode
        vector<unsigned char> buf(idBytes_);
        fillRandom(buf.data(), buf.size());
        static const char* hex = "0123456789abcdef";
        string out;
        out.reserve(buf.size() * 2);
        for (unsigned char c : buf) {
            out.push_back(hex[c >> 4]);
            out.push_back(hex[c & 0x0F]);
        }
        if (sessions_.find(out) != sessions_.end()) {
            // unlikely collision; regenerate
            return generateId();
        }
        return out;
    }

    static void fillRandom(unsigned char* data, size_t len) {
        // Use std::random_device per byte
        static random_device rd;
        size_t i = 0;
        while (i < len) {
            unsigned int v = rd();
            size_t to = min(len - i, sizeof(v));
            memcpy(data + i, &v, to);
            i += to;
        }
    }
};

int main() {
    SessionManager sm;

    // Test 1: Create and validate
    string s1 = sm.createSession("alice", 5);
    cout << "Test1 valid=" << sm.validateSession(s1) << " user=" << sm.getUserForSession(s1) << "\n";

    // Test 2: Touch/extend
    cout << "Test2 touched=" << sm.touchSession(s1, 5) << " stillValid=" << sm.validateSession(s1) << "\n";

    // Test 3: Rotate session
    string rotated = sm.rotateSession(s1);
    cout << "Test3 oldValid=" << sm.validateSession(s1) << " newValid=" << sm.validateSession(rotated) << "\n";

    // Test 4: Expiration
    string s2 = sm.createSession("bob", 1);
    this_thread::sleep_for(chrono::milliseconds(1500));
    cout << "Test4 expiredValid=" << sm.validateSession(s2) << "\n";

    // Test 5: Invalidate all for user
    string u3s1 = sm.createSession("carol", 10);
    string u3s2 = sm.createSession("carol", 10);
    int invalidated = sm.invalidateAllSessionsForUser("carol");
    cout << "Test5 invalidated=" << invalidated << " s1Valid=" << sm.validateSession(u3s1) << " s2Valid=" << sm.validateSession(u3s2) << "\n";

    return 0;
}