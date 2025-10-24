#include <bits/stdc++.h>
using namespace std;

class Task123 {
public:
    struct Session {
        string id;
        string userId;
        long long createdAt;
        long long lastAccess;
        long long ttlMillis;
        bool invalidated;
    };

    class SessionManager {
        unordered_map<string, Session> sessions;
        long long counter = 0;

        bool expired(const Session& s, long long now) const {
            return (now - s.lastAccess) > s.ttlMillis;
        }
    public:
        string createSession(const string& userId, long long nowMillis, long long ttlMillis) {
            counter += 1;
            string id = "S" + to_string(counter);
            sessions[id] = Session{id, userId, nowMillis, nowMillis, ttlMillis, false};
            return id;
        }

        bool isActive(const string& sessionId, long long nowMillis) {
            auto it = sessions.find(sessionId);
            if (it == sessions.end()) return false;
            Session& s = it->second;
            if (s.invalidated) return false;
            if (expired(s, nowMillis)) return false;
            return true;
        }

        bool touch(const string& sessionId, long long nowMillis) {
            auto it = sessions.find(sessionId);
            if (it == sessions.end()) return false;
            Session& s = it->second;
            if (s.invalidated || expired(s, nowMillis)) return false;
            s.lastAccess = nowMillis;
            return true;
        }

        bool invalidate(const string& sessionId) {
            auto it = sessions.find(sessionId);
            if (it == sessions.end()) return false;
            Session& s = it->second;
            if (s.invalidated) return false;
            s.invalidated = true;
            return true;
        }

        int purgeExpired(long long nowMillis) {
            vector<string> toRemove;
            for (auto& kv : sessions) {
                const Session& s = kv.second;
                if (s.invalidated || expired(s, nowMillis)) {
                    toRemove.push_back(kv.first);
                }
            }
            for (auto& id : toRemove) sessions.erase(id);
            return (int)toRemove.size();
        }

        vector<string> listActiveSessions(const string& userId, long long nowMillis) {
            vector<string> res;
            for (auto& kv : sessions) {
                const Session& s = kv.second;
                if (s.userId == userId && !s.invalidated && !expired(s, nowMillis)) {
                    res.push_back(s.id);
                }
            }
            sort(res.begin(), res.end());
            return res;
        }

        string getSessionInfo(const string& sessionId) {
            auto it = sessions.find(sessionId);
            if (it == sessions.end()) return string();
            const Session& s = it->second;
            ostringstream oss;
            oss << "Session{id='" << s.id << "', userId='" << s.userId << "', createdAt=" << s.createdAt
                << ", lastAccess=" << s.lastAccess << ", ttlMillis=" << s.ttlMillis
                << ", invalidated=" << (s.invalidated ? "true" : "false") << "}";
            return oss.str();
        }
    };
};

int main() {
    Task123::SessionManager mgr;

    // Test 1
    string s1 = mgr.createSession("alice", 1000, 30000);
    cout << "Test1:isActive(s1@1000)=" << (mgr.isActive(s1, 1000) ? "true" : "false") << "\n";
    auto l1 = mgr.listActiveSessions("alice", 1000);
    cout << "Test1:listActive(alice@1000)=[";
    for (size_t i=0;i<l1.size();++i){ if(i) cout<<", "; cout<<l1[i]; } cout << "]\n";

    // Test 2
    cout << "Test2:touch(s1@20000)=" << (mgr.touch(s1, 20000) ? "true" : "false") << "\n";
    cout << "Test2:isActive(s1@20000)=" << (mgr.isActive(s1, 20000) ? "true" : "false") << "\n";

    // Test 3
    string s2 = mgr.createSession("alice", 21000, 30000);
    auto l3 = mgr.listActiveSessions("alice", 21000);
    cout << "Test3:listActive(alice@21000)=[";
    for (size_t i=0;i<l3.size();++i){ if(i) cout<<", "; cout<<l3[i]; } cout << "]\n";

    // Test 4
    string sb = mgr.createSession("bob", 1000, 10000);
    cout << "Test4:isActive(sb@12000)=" << (mgr.isActive(sb, 12000) ? "true" : "false") << "\n";
    cout << "Test4:purged@12000=" << mgr.purgeExpired(12000) << "\n";
    auto l4 = mgr.listActiveSessions("bob", 12000);
    cout << "Test4:listActive(bob@12000)=[";
    for (size_t i=0;i<l4.size();++i){ if(i) cout<<", "; cout<<l4[i]; } cout << "]\n";

    // Test 5
    cout << "Test5:invalidate(s1)=" << (mgr.invalidate(s1) ? "true" : "false") << "\n";
    auto l5 = mgr.listActiveSessions("alice", 22000);
    cout << "Test5:listActive(alice@22000)=[";
    for (size_t i=0;i<l5.size();++i){ if(i) cout<<", "; cout<<l5[i]; } cout << "]\n";

    return 0;
}