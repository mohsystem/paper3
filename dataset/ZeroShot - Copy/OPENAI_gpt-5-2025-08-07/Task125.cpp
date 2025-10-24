#include <bits/stdc++.h>
using namespace std;

class Task125 {
private:
    static bool canonicalize(const string& input, string& out) {
        if (input.empty()) return false;
        // Trim spaces
        size_t start = 0, end = input.size();
        while (start < end && isspace(static_cast<unsigned char>(input[start]))) ++start;
        while (end > start && isspace(static_cast<unsigned char>(input[end - 1]))) --end;
        if (start >= end) return false;
        size_t len = end - start;
        if (len > 32) return false;

        out.clear();
        out.reserve(len);
        for (size_t i = start; i < end; ++i) {
            unsigned char ch = static_cast<unsigned char>(input[i]);
            if (ch >= 'A' && ch <= 'Z') ch = static_cast<unsigned char>(ch + 32);
            if (!((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == '_' || ch == '-')) {
                return false;
            }
            out.push_back(static_cast<char>(ch));
        }
        return true;
    }

    static const unordered_map<string, string>& userRoles() {
        static const unordered_map<string, string> m = {
            {"alice", "admin"},
            {"bob", "editor"},
            {"carol", "viewer"},
            {"dave", "viewer"},
            {"eve", "editor"}
        };
        return m;
    }

    static const unordered_map<string, unordered_set<string>>& roleResources() {
        static const unordered_map<string, unordered_set<string>> m = {
            {"admin", {"config", "reports", "dashboard", "users", "logs"}},
            {"editor", {"reports", "dashboard"}},
            {"viewer", {"dashboard"}}
        };
        return m;
    }

public:
    static vector<string> getAccessibleResources(const string& username) {
        string u;
        if (!canonicalize(username, u)) return {};
        auto itRole = userRoles().find(u);
        if (itRole == userRoles().end()) return {};
        auto itRes = roleResources().find(itRole->second);
        if (itRes == roleResources().end()) return {};
        vector<string> res(itRes->second.begin(), itRes->second.end());
        sort(res.begin(), res.end());
        return res;
    }

    static bool canAccess(const string& username, const string& resource) {
        string u, r;
        if (!canonicalize(username, u) || !canonicalize(resource, r)) return false;
        auto itRole = userRoles().find(u);
        if (itRole == userRoles().end()) return false;
        auto itRes = roleResources().find(itRole->second);
        if (itRes == roleResources().end()) return false;
        return itRes->second.find(r) != itRes->second.end();
    }

    static string requestAccess(const string& username, const string& resource) {
        if (canAccess(username, resource)) {
            return "GRANTED: " + username + " -> " + resource;
        }
        return "DENIED: " + username + " -> " + resource;
    }
};

int main() {
    // 5 test cases
    vector<pair<string, string>> tests = {
        {"alice", "logs"},
        {"bob", "config"},
        {"carol", "dashboard"},
        {"unknown", "reports"},
        {"eve", "reports"}
    };

    for (const auto& t : tests) {
        cout << Task125::requestAccess(t.first, t.second) << "\n";
    }

    vector<string> users = {"alice", "bob", "carol", "unknown", "eve"};
    for (const auto& u : users) {
        auto res = Task125::getAccessibleResources(u);
        cout << "Resources for " << u << ": [";
        for (size_t i = 0; i < res.size(); ++i) {
            cout << res[i];
            if (i + 1 < res.size()) cout << ", ";
        }
        cout << "]\n";
    }
    return 0;
}