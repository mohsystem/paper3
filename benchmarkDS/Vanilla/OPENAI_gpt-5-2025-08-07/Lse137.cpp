#include <bits/stdc++.h>
using namespace std;

class Lse137 {
public:
    static bool auth_mod(const string& username, const string& password, const string& session) {
        if (username.empty() || password.empty() || session.empty()) return false;
        unordered_map<string, string> user_pass{
            {"alice", "alice123"},
            {"bob", "bob123"},
            {"carol", "carol123"}
        };
        unordered_set<string> moderators{"alice", "bob"};
        auto it = user_pass.find(username);
        if (it == user_pass.end()) return false;
        if (it->second != password) return false;
        if (session != "sessOK") return false;
        return moderators.count(username) > 0;
    }

    static bool check_mod(const string& url) {
        auto getParam = [&](const string& key) -> string {
            size_t q = url.find('?');
            if (q == string::npos || q + 1 >= url.size()) return "";
            string qs = url.substr(q + 1);
            stringstream ss(qs);
            string pair;
            while (getline(ss, pair, '&')) {
                size_t eq = pair.find('=');
                if (eq == string::npos || eq == 0) continue;
                string k = pair.substr(0, eq);
                string v = pair.substr(eq + 1);
                if (k == key) return v;
            }
            return "";
        };
        string username = getParam("username");
        string password = getParam("password");
        string session = getParam("session");
        if (username.empty() && password.empty() && session.empty()) return false;
        return auth_mod(username, password, session);
    }
};

int main() {
    vector<string> tests = {
        "/check_mod?username=alice&password=alice123&session=sessOK",
        "/check_mod?username=bob&password=bob123&session=sessOK",
        "/check_mod?username=carol&password=carol123&session=sessOK",
        "/check_mod?username=alice&password=wrong&session=sessOK",
        "/check_mod?username=alice&password=alice123&session=bad"
    };
    for (auto& t : tests) {
        cout << (Lse137::check_mod(t) ? "true" : "false") << "\n";
    }
    return 0;
}