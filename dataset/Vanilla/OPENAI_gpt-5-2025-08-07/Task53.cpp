#include <bits/stdc++.h>
using namespace std;

struct Session {
    string sessionId;
    string userId;
    long long expiryMs;
    unordered_map<string, string> attrs;
};

class SessionManager {
    unordered_map<string, Session> sessions;
    long long nowMs() const {
        using namespace std::chrono;
        return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
    }
    string genId() {
        static atomic<unsigned long long> ctr{0};
        unsigned long long v = ++ctr;
        ostringstream os;
        os << "S" << v << "_" << nowMs();
        return os.str();
    }
public:
    string create_session(const string& user_id, long long ttl_ms) {
        string sid = genId();
        Session s{sid, user_id, nowMs() + ttl_ms, {}};
        sessions[sid] = std::move(s);
        return sid;
    }
    bool validate_session(const string& session_id) {
        auto it = sessions.find(session_id);
        if (it == sessions.end()) return false;
        if (it->second.expiryMs < nowMs()) { sessions.erase(it); return false; }
        return true;
    }
    optional<string> get_user(const string& session_id) {
        if (!validate_session(session_id)) return nullopt;
        return sessions[session_id].userId;
    }
    bool set_attribute(const string& session_id, const string& key, const string& value) {
        if (!validate_session(session_id)) return false;
        sessions[session_id].attrs[key] = value;
        return true;
    }
    optional<string> get_attribute(const string& session_id, const string& key) {
        if (!validate_session(session_id)) return nullopt;
        auto& m = sessions[session_id].attrs;
        auto it = m.find(key);
        if (it == m.end()) return nullopt;
        return it->second;
    }
    bool touch(const string& session_id, long long ttl_extend_ms) {
        if (!validate_session(session_id)) return false;
        sessions[session_id].expiryMs = nowMs() + ttl_extend_ms;
        return true;
    }
    bool invalidate_session(const string& session_id) {
        return sessions.erase(session_id) > 0;
    }
    int cleanup_expired() {
        int removed = 0;
        long long t = nowMs();
        vector<string> toErase;
        toErase.reserve(sessions.size());
        for (auto& kv : sessions) {
            if (kv.second.expiryMs < t) toErase.push_back(kv.first);
        }
        for (auto& sid : toErase) { sessions.erase(sid); removed++; }
        return removed;
    }
    int count_sessions() const { return (int)sessions.size(); }
};

int main() {
    SessionManager sm;

    string sid1 = sm.create_session("userA", 10000);
    cout << "T1_valid=" << (sm.validate_session(sid1) ? "1" : "0")
         << ", user=" << (sm.get_user(sid1).value_or("null"))
         << ", count=" << sm.count_sessions() << "\n";

    sm.set_attribute(sid1, "role", "admin");
    cout << "T2_role=" << sm.get_attribute(sid1, "role").value_or("null") << "\n";

    bool touched = sm.touch(sid1, 10000);
    cout << "T3_touched=" << (touched ? "1" : "0")
         << ", stillValid=" << (sm.validate_session(sid1) ? "1" : "0") << "\n";

    sm.invalidate_session(sid1);
    cout << "T4_validAfterInvalidate=" << (sm.validate_session(sid1) ? "1" : "0")
         << ", count=" << sm.count_sessions() << "\n";

    string sid2 = sm.create_session("userB", 0);
    string sid3 = sm.create_session("userC", -1000);
    int removed = sm.cleanup_expired();
    cout << "T5_removed=" << removed << ", finalCount=" << sm.count_sessions() << "\n";

    return 0;
}