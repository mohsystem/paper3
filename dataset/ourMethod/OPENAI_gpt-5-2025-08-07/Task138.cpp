#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>

class AccessManager {
public:
    bool createRole(const std::string& role) {
        if (!validName(role)) return false;
        if (roleToPerms.size() >= MAX_ROLES) return false;
        return roleToPerms.emplace(role, std::unordered_set<std::string>()).second;
    }

    bool removeRole(const std::string& role) {
        if (!validName(role)) return false;
        auto it = roleToPerms.find(role);
        if (it == roleToPerms.end()) return false;
        roleToPerms.erase(it);
        for (auto& kv : userToRoles) {
            kv.second.erase(role);
        }
        return true;
    }

    bool addPermissionToRole(const std::string& role, const std::string& perm) {
        if (!validName(role) || !validName(perm)) return false;
        auto it = roleToPerms.find(role);
        if (it == roleToPerms.end()) return false;
        if (it->second.size() >= MAX_PERMS_PER_ROLE) return false;
        return it->second.insert(perm).second;
    }

    bool removePermissionFromRole(const std::string& role, const std::string& perm) {
        if (!validName(role) || !validName(perm)) return false;
        auto it = roleToPerms.find(role);
        if (it == roleToPerms.end()) return false;
        return it->second.erase(perm) > 0;
    }

    bool createUser(const std::string& user) {
        if (!validName(user)) return false;
        if (userToRoles.size() >= MAX_USERS) return false;
        return userToRoles.emplace(user, std::unordered_set<std::string>()).second;
    }

    bool deleteUser(const std::string& user) {
        if (!validName(user)) return false;
        return userToRoles.erase(user) > 0;
    }

    bool assignRoleToUser(const std::string& user, const std::string& role) {
        if (!validName(user) || !validName(role)) return false;
        auto itR = roleToPerms.find(role);
        if (itR == roleToPerms.end()) return false;
        auto itU = userToRoles.find(user);
        if (itU == userToRoles.end()) return false;
        if (itU->second.size() >= MAX_ROLES_PER_USER) return false;
        return itU->second.insert(role).second;
    }

    bool revokeRoleFromUser(const std::string& user, const std::string& role) {
        if (!validName(user) || !validName(role)) return false;
        auto itU = userToRoles.find(user);
        if (itU == userToRoles.end()) return false;
        return itU->second.erase(role) > 0;
    }

    bool userHasPermission(const std::string& user, const std::string& perm) const {
        if (!validName(user) || !validName(perm)) return false;
        auto itU = userToRoles.find(user);
        if (itU == userToRoles.end()) return false;
        for (const auto& role : itU->second) {
            auto itR = roleToPerms.find(role);
            if (itR != roleToPerms.end() && itR->second.find(perm) != itR->second.end()) {
                return true;
            }
        }
        return false;
    }

    std::vector<std::string> listUserPermissions(const std::string& user) const {
        std::vector<std::string> out;
        if (!validName(user)) return out;
        auto itU = userToRoles.find(user);
        if (itU == userToRoles.end()) return out;
        std::unordered_set<std::string> perms;
        for (const auto& role : itU->second) {
            auto itR = roleToPerms.find(role);
            if (itR != roleToPerms.end()) {
                for (const auto& p : itR->second) perms.insert(p);
            }
        }
        out.assign(perms.begin(), perms.end());
        std::sort(out.begin(), out.end());
        return out;
    }

    std::vector<std::string> listUserRoles(const std::string& user) const {
        std::vector<std::string> out;
        if (!validName(user)) return out;
        auto itU = userToRoles.find(user);
        if (itU == userToRoles.end()) return out;
        out.assign(itU->second.begin(), itU->second.end());
        std::sort(out.begin(), out.end());
        return out;
    }

private:
    static constexpr size_t MAX_NAME_LEN = 64;
    static constexpr size_t MAX_USERS = 10000;
    static constexpr size_t MAX_ROLES = 10000;
    static constexpr size_t MAX_PERMS_PER_ROLE = 1000;
    static constexpr size_t MAX_ROLES_PER_USER = 1000;

    std::unordered_map<std::string, std::unordered_set<std::string>> roleToPerms;
    std::unordered_map<std::string, std::unordered_set<std::string>> userToRoles;

    static bool validName(const std::string& s) {
        if (s.empty() || s.size() > MAX_NAME_LEN) return false;
        for (char c : s) {
            if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-')) return false;
        }
        return true;
    }
};

int main() {
    AccessManager mgr;
    // Test 1
    std::cout << "Test1-CreateRoles: " << (mgr.createRole("admin") && mgr.createRole("editor")) << "\n";
    std::cout << "Test1-AddPermsAdmin: "
              << (mgr.addPermissionToRole("admin", "read_all")
                  && mgr.addPermissionToRole("admin", "write_all")
                  && mgr.addPermissionToRole("admin", "manage_users")) << "\n";
    std::cout << "Test1-AddPermsEditor: "
              << (mgr.addPermissionToRole("editor", "read_articles")
                  && mgr.addPermissionToRole("editor", "edit_articles")) << "\n";
    std::cout << "Test1-CreateUsers: " << (mgr.createUser("alice") && mgr.createUser("bob")) << "\n";
    std::cout << "Test1-AssignRoles: " << (mgr.assignRoleToUser("alice", "admin")
                                           && mgr.assignRoleToUser("bob", "editor")) << "\n";
    std::cout << "Test1-CheckPerms: " << (mgr.userHasPermission("alice", "manage_users")
                                           && !mgr.userHasPermission("bob", "manage_users")) << "\n";

    // Test 2
    std::cout << "Test2-InvalidRoleName: " << (!mgr.createRole("invalid role")) << "\n";
    std::cout << "Test2-InvalidUserName: " << (!mgr.createUser("")) << "\n";
    std::cout << "Test2-InvalidPermToRole: " << (!mgr.addPermissionToRole("admin", "bad perm")) << "\n";

    // Test 3
    std::cout << "Test3-DuplicateRole: " << (!mgr.createRole("admin")) << "\n";
    std::cout << "Test3-DuplicateUser: " << (!mgr.createUser("alice")) << "\n";
    std::cout << "Test3-DuplicatePermission: " << (!mgr.addPermissionToRole("admin", "read_all")) << "\n";

    // Test 4
    std::cout << "Test4-RevokeRole: " << (mgr.revokeRoleFromUser("alice", "admin")) << "\n";
    std::cout << "Test4-CheckPermAfterRevoke: " << (!mgr.userHasPermission("alice", "read_all")) << "\n";

    // Test 5
    std::cout << "Test5-AddRoleBack: " << (mgr.assignRoleToUser("alice", "admin")) << "\n";
    std::cout << "Test5-RemovePermFromRole: " << (mgr.removePermissionFromRole("admin", "write_all")) << "\n";
    std::cout << "Test5-CheckRemovedPerm: " << (!mgr.userHasPermission("alice", "write_all")) << "\n";

    // Display
    auto permsAlice = mgr.listUserPermissions("alice");
    std::cout << "AlicePerms:";
    for (const auto& p : permsAlice) std::cout << " " << p;
    std::cout << "\n";
    auto permsBob = mgr.listUserPermissions("bob");
    std::cout << "BobPerms:";
    for (const auto& p : permsBob) std::cout << " " << p;
    std::cout << "\n";
    return 0;
}