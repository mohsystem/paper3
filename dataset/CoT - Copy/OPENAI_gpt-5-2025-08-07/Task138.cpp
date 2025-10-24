// Minimal high-level comments: Implemented with validation, normalization, and safe containers.
#include <bits/stdc++.h>
using namespace std;

class PermissionManager {
private:
    unordered_map<string, unordered_set<string>> roles; // role -> perms
    unordered_map<string, unordered_set<string>> usersDirect; // user -> perms
    unordered_map<string, unordered_set<string>> usersRoles; // user -> roles

    static bool validChar(char c) {
        return (c >= 'a' && c <= 'z') ||
               (c >= '0' && c <= '9') ||
               c == '_' || c == '.' || c == ':' || c == '-';
    }

    static string trim(const string& s) {
        size_t b = 0, e = s.size();
        while (b < e && isspace((unsigned char)s[b])) b++;
        while (e > b && isspace((unsigned char)s[e - 1])) e--;
        return s.substr(b, e - b);
    }

    static string normalize(const string& in) {
        if (in.empty()) throw invalid_argument("empty not allowed");
        string t = trim(in);
        string out; out.reserve(t.size());
        for (char ch : t) out.push_back((char)tolower((unsigned char)ch));
        if (out.empty() || out.size() > 64) throw invalid_argument("invalid length");
        for (char c : out) if (!validChar(c)) throw invalid_argument("invalid chars: " + in);
        return out;
    }

public:
    bool addRole(const string& roleNameRaw) {
        string r = normalize(roleNameRaw);
        if (roles.count(r)) return false;
        roles.emplace(r, unordered_set<string>{});
        return true;
    }

    bool addUser(const string& usernameRaw) {
        string u = normalize(usernameRaw);
        if (usersDirect.count(u)) return false;
        usersDirect.emplace(u, unordered_set<string>{});
        usersRoles.emplace(u, unordered_set<string>{});
        return true;
    }

    bool addPermissionToRole(const string& roleNameRaw, const string& permissionRaw) {
        string r = normalize(roleNameRaw);
        string p = normalize(permissionRaw);
        auto it = roles.find(r);
        if (it == roles.end()) return false;
        auto& s = it->second;
        if (s.count(p)) return false;
        s.insert(p);
        return true;
    }

    bool removePermissionFromRole(const string& roleNameRaw, const string& permissionRaw) {
        string r = normalize(roleNameRaw);
        string p = normalize(permissionRaw);
        auto it = roles.find(r);
        if (it == roles.end()) return false;
        auto& s = it->second;
        if (!s.count(p)) return false;
        s.erase(p);
        return true;
    }

    bool grantPermissionToUser(const string& usernameRaw, const string& permissionRaw) {
        string u = normalize(usernameRaw);
        string p = normalize(permissionRaw);
        auto it = usersDirect.find(u);
        if (it == usersDirect.end()) return false;
        auto& s = it->second;
        if (s.count(p)) return false;
        s.insert(p);
        return true;
    }

    bool revokePermissionFromUser(const string& usernameRaw, const string& permissionRaw) {
        string u = normalize(usernameRaw);
        string p = normalize(permissionRaw);
        auto it = usersDirect.find(u);
        if (it == usersDirect.end()) return false;
        auto& s = it->second;
        if (!s.count(p)) return false;
        s.erase(p);
        return true;
    }

    bool assignRoleToUser(const string& usernameRaw, const string& roleNameRaw) {
        string u = normalize(usernameRaw);
        string r = normalize(roleNameRaw);
        if (!roles.count(r) || !usersRoles.count(u)) return false;
        auto& s = usersRoles[u];
        if (s.count(r)) return false;
        s.insert(r);
        return true;
    }

    bool unassignRoleFromUser(const string& usernameRaw, const string& roleNameRaw) {
        string u = normalize(usernameRaw);
        string r = normalize(roleNameRaw);
        auto it = usersRoles.find(u);
        if (it == usersRoles.end()) return false;
        auto& s = it->second;
        if (!s.count(r)) return false;
        s.erase(r);
        return true;
    }

    bool userHasPermission(const string& usernameRaw, const string& permissionRaw) {
        string u = normalize(usernameRaw);
        string p = normalize(permissionRaw);
        auto ud = usersDirect.find(u);
        if (ud == usersDirect.end()) return false;
        if (ud->second.count(p)) return true;
        auto ur = usersRoles.find(u);
        if (ur == usersRoles.end()) return false;
        for (const auto& r : ur->second) {
            auto it = roles.find(r);
            if (it != roles.end() && it->second.count(p)) return true;
        }
        return false;
    }

    vector<string> getEffectivePermissions(const string& usernameRaw) {
        string u = normalize(usernameRaw);
        vector<string> out;
        auto ud = usersDirect.find(u);
        if (ud == usersDirect.end()) return out;
        unordered_set<string> acc = ud->second;
        auto ur = usersRoles.find(u);
        if (ur != usersRoles.end()) {
            for (const auto& r : ur->second) {
                auto it = roles.find(r);
                if (it != roles.end()) for (const auto& p : it->second) acc.insert(p);
            }
        }
        out.reserve(acc.size());
        for (const auto& p : acc) out.push_back(p);
        return out;
    }
};

int main() {
    PermissionManager pm;

    // Test Case 1
    pm.addRole("admin");
    pm.addPermissionToRole("admin", "read");
    pm.addPermissionToRole("admin", "write");
    pm.addPermissionToRole("admin", "delete");
    pm.addUser("alice");
    pm.assignRoleToUser("alice", "admin");
    cout << "TC1 alice has delete: " << (pm.userHasPermission("alice", "delete") ? "true" : "false") << "\n";

    // Test Case 2
    pm.addRole("editor");
    pm.addPermissionToRole("editor", "write");
    pm.addUser("bob");
    pm.grantPermissionToUser("bob", "read");
    cout << "TC2 bob has write before role: " << (pm.userHasPermission("bob", "write") ? "true" : "false") << "\n";
    pm.assignRoleToUser("bob", "editor");
    cout << "TC2 bob has write after role: " << (pm.userHasPermission("bob", "write") ? "true" : "false") << "\n";

    // Test Case 3
    pm.removePermissionFromRole("admin", "delete");
    cout << "TC3 alice has delete after removal from role: " << (pm.userHasPermission("alice", "delete") ? "true" : "false") << "\n";

    // Test Case 4
    pm.grantPermissionToUser("alice", "delete");
    cout << "TC4 alice has delete after direct grant: " << (pm.userHasPermission("alice", "delete") ? "true" : "false") << "\n";

    // Test Case 5
    cout << "TC5 unknown user has read: " << (pm.userHasPermission("charlie", "read") ? "true" : "false") << "\n";
    cout << "TC5 adding duplicate role 'admin': " << (pm.addRole("admin") ? "true" : "false") << "\n";
    auto perms = pm.getEffectivePermissions("alice");
    sort(perms.begin(), perms.end());
    cout << "TC5 alice effective perms: [";
    for (size_t i = 0; i < perms.size(); ++i) {
        cout << perms[i];
        if (i + 1 < perms.size()) cout << ", ";
    }
    cout << "]\n";
    return 0;
}