#include <bits/stdc++.h>
using namespace std;

enum class Permission { READ, WRITE, DELETE_, EXECUTE, ADMIN };
enum class Role { GUEST, USER, MODERATOR, ADMINISTRATOR };

static const map<Role, set<Permission>> ROLE_PERMISSIONS = {
    { Role::GUEST, { Permission::READ } },
    { Role::USER, { Permission::READ, Permission::WRITE } },
    { Role::MODERATOR, { Permission::READ, Permission::WRITE, Permission::DELETE_ } },
    { Role::ADMINISTRATOR, { Permission::READ, Permission::WRITE, Permission::DELETE_, Permission::EXECUTE, Permission::ADMIN } }
};

static string sanitizeName(const string& input) {
    string out;
    out.reserve(64);
    size_t i = 0;
    // trim
    size_t start = 0, end = input.size();
    while (start < end && isspace(static_cast<unsigned char>(input[start]))) start++;
    while (end > start && isspace(static_cast<unsigned char>(input[end-1]))) end--;
    for (size_t j = start; j < end; ++j) {
        char c = input[j];
        if (isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '.' || c == '-') {
            out.push_back(c);
            if (out.size() >= 64) break;
        }
    }
    return out;
}

struct User {
    string username;
    set<Role> roles;
    set<Permission> directPermissions;
    explicit User(string u) : username(std::move(u)) {}
};

class PermissionManager {
private:
    map<string, unique_ptr<User>> users;
    mutable std::mutex mtx;

public:
    bool createUser(const string& rawUsername) {
        string username = sanitizeName(rawUsername);
        if (username.empty()) return false;
        lock_guard<mutex> lock(mtx);
        if (users.find(username) != users.end()) return false;
        users[username] = make_unique<User>(username);
        return true;
    }

    bool deleteUser(const string& rawUsername) {
        string username = sanitizeName(rawUsername);
        lock_guard<mutex> lock(mtx);
        return users.erase(username) > 0;
    }

    bool assignRole(const string& rawUsername, Role role) {
        string username = sanitizeName(rawUsername);
        lock_guard<mutex> lock(mtx);
        auto it = users.find(username);
        if (it == users.end()) return false;
        auto& roles = it->second->roles;
        if (roles.count(role)) return false;
        roles.insert(role);
        return true;
    }

    bool revokeRole(const string& rawUsername, Role role) {
        string username = sanitizeName(rawUsername);
        lock_guard<mutex> lock(mtx);
        auto it = users.find(username);
        if (it == users.end()) return false;
        auto& roles = it->second->roles;
        auto rit = roles.find(role);
        if (rit == roles.end()) return false;
        roles.erase(rit);
        return true;
    }

    bool grantPermission(const string& rawUsername, Permission perm) {
        string username = sanitizeName(rawUsername);
        lock_guard<mutex> lock(mtx);
        auto it = users.find(username);
        if (it == users.end()) return false;
        auto& perms = it->second->directPermissions;
        if (perms.count(perm)) return false;
        perms.insert(perm);
        return true;
    }

    bool revokePermission(const string& rawUsername, Permission perm) {
        string username = sanitizeName(rawUsername);
        lock_guard<mutex> lock(mtx);
        auto it = users.find(username);
        if (it == users.end()) return false;
        auto& perms = it->second->directPermissions;
        auto pit = perms.find(perm);
        if (pit == perms.end()) return false;
        perms.erase(pit);
        return true;
    }

    bool hasPermission(const string& rawUsername, Permission perm) const {
        string username = sanitizeName(rawUsername);
        lock_guard<mutex> lock(mtx);
        auto it = users.find(username);
        if (it == users.end()) return false;
        auto& user = it->second;
        if (user->directPermissions.count(perm)) return true;
        for (const auto& r : user->roles) {
            auto rp = ROLE_PERMISSIONS.find(r);
            if (rp != ROLE_PERMISSIONS.end() && rp->second.count(perm)) return true;
        }
        return false;
    }

    set<Permission> listEffectivePermissions(const string& rawUsername) const {
        string username = sanitizeName(rawUsername);
        lock_guard<mutex> lock(mtx);
        auto it = users.find(username);
        if (it == users.end()) return {};
        set<Permission> eff = it->second->directPermissions;
        for (const auto& r : it->second->roles) {
            auto rp = ROLE_PERMISSIONS.find(r);
            if (rp != ROLE_PERMISSIONS.end()) {
                eff.insert(rp->second.begin(), rp->second.end());
            }
        }
        return eff;
    }

    set<string> listUsers() const {
        lock_guard<mutex> lock(mtx);
        set<string> names;
        for (auto& kv : users) names.insert(kv.first);
        return names;
    }
};

// Helpers for printing
static string permToStr(Permission p) {
    switch (p) {
        case Permission::READ: return "READ";
        case Permission::WRITE: return "WRITE";
        case Permission::DELETE_: return "DELETE";
        case Permission::EXECUTE: return "EXECUTE";
        case Permission::ADMIN: return "ADMIN";
    }
    return "?";
}

int main() {
    PermissionManager pm;

    // Test 1
    cout << "Test 1:\n";
    cout << boolalpha << pm.createUser("alice") << "\n";
    cout << pm.assignRole("alice", Role::USER) << "\n";
    cout << pm.hasPermission("alice", Permission::READ) << "\n";
    cout << pm.hasPermission("alice", Permission::DELETE_) << "\n";

    // Test 2
    cout << "Test 2:\n";
    cout << pm.grantPermission("alice", Permission::WRITE) << "\n";
    cout << pm.hasPermission("alice", Permission::WRITE) << "\n";
    auto eff1 = pm.listEffectivePermissions("alice");
    for (auto p : eff1) cout << permToStr(p) << " ";
    cout << "\n";

    // Test 3
    cout << "Test 3:\n";
    cout << pm.createUser("bob") << "\n";
    cout << pm.assignRole("bob", Role::ADMINISTRATOR) << "\n";
    cout << pm.hasPermission("bob", Permission::ADMIN) << "\n";
    cout << pm.hasPermission("bob", Permission::DELETE_) << "\n";
    auto eff2 = pm.listEffectivePermissions("bob");
    for (auto p : eff2) cout << permToStr(p) << " ";
    cout << "\n";

    // Test 4
    cout << "Test 4:\n";
    cout << pm.revokePermission("alice", Permission::WRITE) << "\n";
    cout << pm.hasPermission("alice", Permission::WRITE) << "\n";
    auto eff3 = pm.listEffectivePermissions("alice");
    for (auto p : eff3) cout << permToStr(p) << " ";
    cout << "\n";

    // Test 5
    cout << "Test 5:\n";
    cout << pm.createUser("  Eve!!  ") << "\n";
    cout << pm.createUser("Eve") << "\n";
    auto users = pm.listUsers();
    for (auto& u : users) cout << u << " ";
    cout << "\n";

    return 0;
}