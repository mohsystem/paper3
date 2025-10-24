#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <algorithm>

class PermissionSystem {
private:
    std::unordered_map<std::string, std::unordered_set<std::string>> userRoles;
    std::unordered_map<std::string, std::unordered_set<std::string>> rolePerms;
    std::unordered_map<std::string, std::unordered_set<std::string>> userPerms;

    std::unordered_set<std::string>& getOrCreate(std::unordered_map<std::string, std::unordered_set<std::string>>& m, const std::string& k) {
        return m[k];
    }

public:
    void addUser(const std::string& user) {
        getOrCreate(userRoles, user);
        getOrCreate(userPerms, user);
    }

    void addRole(const std::string& role) {
        getOrCreate(rolePerms, role);
    }

    void grantRoleToUser(const std::string& user, const std::string& role) {
        addUser(user);
        addRole(role);
        userRoles[user].insert(role);
    }

    void revokeRoleFromUser(const std::string& user, const std::string& role) {
        getOrCreate(userRoles, user).erase(role);
    }

    void grantPermissionToRole(const std::string& role, const std::string& permission) {
        addRole(role);
        rolePerms[role].insert(permission);
    }

    void revokePermissionFromRole(const std::string& role, const std::string& permission) {
        getOrCreate(rolePerms, role).erase(permission);
    }

    void grantPermissionToUser(const std::string& user, const std::string& permission) {
        addUser(user);
        userPerms[user].insert(permission);
    }

    void revokePermissionFromUser(const std::string& user, const std::string& permission) {
        getOrCreate(userPerms, user).erase(permission);
    }

    bool hasPermission(const std::string& user, const std::string& permission) {
        if (getOrCreate(userPerms, user).count(permission)) return true;
        for (const auto& role : getOrCreate(userRoles, user)) {
            if (getOrCreate(rolePerms, role).count(permission)) return true;
        }
        return false;
    }

    std::vector<std::string> getUserPermissions(const std::string& user) {
        std::unordered_set<std::string> perms = getOrCreate(userPerms, user);
        for (const auto& role : getOrCreate(userRoles, user)) {
            auto& rp = getOrCreate(rolePerms, role);
            perms.insert(rp.begin(), rp.end());
        }
        std::vector<std::string> res(perms.begin(), perms.end());
        std::sort(res.begin(), res.end());
        return res;
    }
};

static void printVector(const std::vector<std::string>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        std::cout << v[i];
        if (i + 1 < v.size()) std::cout << ", ";
    }
    std::cout << "]\n";
}

int main() {
    PermissionSystem ps;

    // Test 1
    ps.addRole("admin");
    ps.grantPermissionToRole("admin", "read");
    ps.grantPermissionToRole("admin", "write");
    ps.grantPermissionToRole("admin", "delete");
    ps.addUser("alice");
    ps.grantRoleToUser("alice", "admin");
    std::cout << "Test1 alice read: " << (ps.hasPermission("alice", "read") ? "true" : "false") << "\n";
    std::cout << "Test1 alice write: " << (ps.hasPermission("alice", "write") ? "true" : "false") << "\n";
    std::cout << "Test1 alice delete: " << (ps.hasPermission("alice", "delete") ? "true" : "false") << "\n";

    // Test 2
    ps.addUser("bob");
    ps.grantPermissionToUser("bob", "read");
    std::cout << "Test2 bob read: " << (ps.hasPermission("bob", "read") ? "true" : "false") << "\n";
    std::cout << "Test2 bob write: " << (ps.hasPermission("bob", "write") ? "true" : "false") << "\n";

    // Test 3
    ps.addRole("editor");
    ps.grantPermissionToRole("editor", "read");
    ps.grantPermissionToRole("editor", "write");
    ps.addRole("viewer");
    ps.grantPermissionToRole("viewer", "read");
    ps.addUser("charlie");
    ps.grantRoleToUser("charlie", "editor");
    ps.grantRoleToUser("charlie", "viewer");
    std::cout << "Test3 charlie read: " << (ps.hasPermission("charlie", "read") ? "true" : "false") << "\n";
    std::cout << "Test3 charlie write: " << (ps.hasPermission("charlie", "write") ? "true" : "false") << "\n";

    // Test 4
    ps.grantRoleToUser("bob", "viewer");
    ps.revokePermissionFromUser("bob", "read");
    std::cout << "Test4 bob read via role: " << (ps.hasPermission("bob", "read") ? "true" : "false") << "\n";
    ps.revokeRoleFromUser("bob", "viewer");
    std::cout << "Test4 bob read after revoke role: " << (ps.hasPermission("bob", "read") ? "true" : "false") << "\n";

    // Test 5
    ps.addUser("dana");
    ps.grantRoleToUser("dana", "editor");
    ps.grantPermissionToUser("dana", "export");
    auto danaPerms = ps.getUserPermissions("dana");
    std::cout << "Test5 dana perms: ";
    printVector(danaPerms);

    return 0;
}