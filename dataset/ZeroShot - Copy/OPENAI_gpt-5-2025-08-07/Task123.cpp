#include <bits/stdc++.h>
using namespace std;

class SessionManager {
    struct Session {
        string userId;
        long long expiry; // epoch seconds
    };
    unordered_map<string, Session> sessions;
    static constexpr long long MAX_TTL_SECONDS = 60LL * 60LL * 24LL * 30LL;

    static bool ct_equal(const string& a, const string& b) {
        if (a.size() != b.size()) {
            // still perform loop to avoid timing leakage about early return
            size_t maxlen = max(a.size(), b.size());
            volatile unsigned char acc = 0;
            for (size_t i = 0; i < maxlen; ++i) {
                unsigned char av = i < a.size() ? (unsigned char)a[i] : 0;
                unsigned char bv = i < b.size() ? (unsigned char)b[i] : 0;
                acc |= (unsigned char)(av ^ bv);
            }
            return acc == 0;
        }
        volatile unsigned char acc = 0;
        for (size_t i = 0; i < a.size(); ++i) {
            acc |= (unsigned char)(a[i] ^ b[i]);
        }
        return acc == 0;
    }

    static long long nowSeconds() {
        return chrono::duration_cast<chrono::seconds>(
                   chrono::system_clock::now().time_since_epoch())
            .count();
    }

    static string randomTokenHex(size_t bytesLen = 32) {
        vector<unsigned char> buf(bytesLen);
        std::random_device rd;
        for (size_t i = 0; i < bytesLen; ++i) {
            buf[i] = static_cast<unsigned char>(rd());
        }
        static const char* hex = "0123456789abcdef";
        string out;
        out.reserve(bytesLen * 2);
        for (auto b : buf) {
            out.push_back(hex[(b >> 4) & 0xF]);
            out.push_back(hex[b & 0xF]);
        }
        return out;
    }

public:
    string create_session(const string& userId, long long ttlSeconds) {
        if (userId.empty() || ttlSeconds <= 0) return string();
        if (ttlSeconds > MAX_TTL_SECONDS) ttlSeconds = MAX_TTL_SECONDS;
        string token = randomTokenHex(32);
        while (sessions.find(token) != sessions.end()) {
            token = randomTokenHex(32);
        }
        sessions[token] = Session{userId, nowSeconds() + ttlSeconds};
        return token;
    }

    string validate_session(const string& token) {
        if (token.empty()) return string();
        auto it = sessions.find(token);
        if (it == sessions.end()) return string();

        // Defensive constant-time identity check
        bool matched = false;
        for (auto& kv : sessions) {
            if (&kv.second == &it->second) {
                matched = ct_equal(kv.first, token);
                break;
            }
        }
        if (!matched) return string();

        long long now = nowSeconds();
        if (now > it->second.expiry) {
            sessions.erase(it);
            return string();
        }
        return it->second.userId;
    }

    bool refresh_session(const string& token, long long additionalTtlSeconds) {
        if (token.empty() || additionalTtlSeconds <= 0) return false;
        if (additionalTtlSeconds > MAX_TTL_SECONDS) additionalTtlSeconds = MAX_TTL_SECONDS;
        auto it = sessions.find(token);
        if (it == sessions.end()) return false;
        long long now = nowSeconds();
        if (now > it->second.expiry) {
            sessions.erase(it);
            return false;
        }
        long long newExpiry = it->second.expiry + additionalTtlSeconds;
        long long maxExpiry = now + MAX_TTL_SECONDS;
        if (newExpiry > maxExpiry) newExpiry = maxExpiry;
        it->second.expiry = newExpiry;
        return true;
    }

    bool end_session(const string& token) {
        if (token.empty()) return false;
        return sessions.erase(token) > 0;
    }

    int cleanup_expired() {
        long long now = nowSeconds();
        vector<string> to_remove;
        to_remove.reserve(sessions.size());
        for (auto& kv : sessions) {
            if (now > kv.second.expiry) {
                to_remove.push_back(kv.first);
            }
        }
        for (auto& k : to_remove) sessions.erase(k);
        return static_cast<int>(to_remove.size());
    }
};

int main() {
    SessionManager mgr;

    // Test 1
    string t1 = mgr.create_session("alice", 2);
    cout << "T1 token: " << t1 << "\n";
    cout << "T1 validate: " << mgr.validate_session(t1) << "\n";

    // Test 2
    cout << "T2 validate invalid: " << mgr.validate_session("invalid_token") << "\n";

    // Test 3
    cout << "T3 refreshed: " << (mgr.refresh_session(t1, 5) ? "true" : "false")
         << ", validate: " << mgr.validate_session(t1) << "\n";

    // Test 4
    cout << "T4 ended: " << (mgr.end_session(t1) ? "true" : "false")
         << ", validate after end: " << mgr.validate_session(t1) << "\n";

    // Test 5
    string t2 = mgr.create_session("bob", 1);
    cout << "T5 token: " << t2 << ", validate: " << mgr.validate_session(t2) << "\n";
    this_thread::sleep_for(chrono::seconds(2));
    cout << "T5 cleaned: " << mgr.cleanup_expired()
         << ", validate after expiry: " << mgr.validate_session(t2) << "\n";

    return 0;
}