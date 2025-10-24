#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cctype>

class PermissionManager {
private:
    std::unordered_map<std::string, std::unordered_set<std::string>> rolesToPermissions;
    std::unordered_map<std::string, std::unordered_set<std::string>> usersToRoles;

    // Helper to check for empty/whitespace-only strings
    bool isStringBlank(const std::string& s) const {
        return s.empty() || std::all_of(s.begin(), s.end(), [](unsigned char c){ return std::isspace(c); });
    }

public:
    PermissionManager() = default;
    
    // Prevent copying to avoid issues with complex state
    PermissionManager(const PermissionManager&) = delete;
    PermissionManager& operator=(const PermissionManager&) = delete;

    bool addUser(const std::string& username) {
        if (isStringBlank(username)) {
            std::cerr << "Error: Username cannot be empty." << std::endl;
            return false;
        }
        if (usersToRoles.count(username)) {
            std::cerr << "Error: User '" << username << "' already exists." << std::endl;
            return false;
        }
        usersToRoles[username] = {};
        return true;
    }

    bool addRole(const std::string& roleName, const std::unordered_set<std::string>& permissions) {
        if (isStringBlank(roleName)) {
            std::cerr << "Error: Role name cannot be empty." << std::endl;
            return false;
        }
        if (permissions.empty()) {
            std::cerr << "Error: Permissions set cannot be empty." << std::endl;
            return false;
        }
        rolesToPermissions[roleName] = permissions;
        return true;
    }

    bool assignRoleToUser(const std::string& username, const std::string& roleName) {
        if (usersToRoles.find(username) == usersToRoles.end()) {
            std::cerr << "Error: User '" << username << "' not found." << std::endl;
            return false;
        }
        if (rolesToPermissions.find(roleName) == rolesToPermissions.end()) {
            std::cerr << "Error: Role '" << roleName << "' not found." << std::endl;
            return false;
        }
        usersToRoles[username].insert(roleName);
        return true;
    }

    bool checkPermission(const std::string& username, const std::string& permission) const {
        if (isStringBlank(permission)) {
            return false;
        }
        auto user_it = usersToRoles.find(username);
        if (user_it == usersToRoles.end()) {
            // Fail closed
            return false;
        }

        const auto& userRoles = user_it->second;
        for (const auto& roleName : userRoles) {
            auto role_it = rolesToPermissions.find(roleName);
            if (role_it != rolesToPermissions.end()) {
                const auto& permissions = role_it->second;
                if (permissions.count(permission)) {
                    return true;
                }
            }
        }

        return false;
    }
};

void run_tests() {
    PermissionManager pm;

    std::cout << "--- Test Case 1: Basic Setup and Permission Check ---" << std::endl;
    pm.addRole("admin", {"read", "write", "delete"});
    pm.addRole("editor", {"read", "write"});
    pm.addRole("viewer", {"read"});
    pm.addUser("alice");
    pm.addUser("bob");
    pm.assignRoleToUser("alice", "admin");
    pm.assignRoleToUser("bob", "editor");

    std::cout << "Does alice have 'delete' permission? " << std::boolalpha << pm.checkPermission("alice", "delete") << std::endl;
    std::cout << "Does bob have 'delete' permission? " << std::boolalpha << pm.checkPermission("bob", "delete") << std::endl;
    std::cout << "Does bob have 'write' permission? " << std::boolalpha << pm.checkPermission("bob", "write") << std::endl;
    std::cout << std::endl;

    std::cout << "--- Test Case 2: User with multiple roles ---" << std::endl;
    pm.addUser("charlie");
    pm.assignRoleToUser("charlie", "viewer");
    pm.assignRoleToUser("charlie", "editor");
    std::cout << "Does charlie have 'write' permission? " << std::boolalpha << pm.checkPermission("charlie", "write") << std::endl;
    std::cout << "Does charlie have 'read' permission? " << std::boolalpha << pm.checkPermission("charlie", "read") << std::endl;
    std::cout << std::endl;

    std::cout << "--- Test Case 3: Non-existent user, role, or permission ---" << std::endl;
    std::cout << "Does non_existent_user have 'read' permission? " << std::boolalpha << pm.checkPermission("non_existent_user", "read") << std::endl;
    std::cout << "Does alice have 'execute' permission? " << std::boolalpha << pm.checkPermission("alice", "execute") << std::endl;
    std::cout << "Assigning non-existent role: " << std::boolalpha << pm.assignRoleToUser("alice", "super_admin") << std::endl;
    std::cout << std::endl;

    std::cout << "--- Test Case 4: User with no roles ---" << std::endl;
    pm.addUser("dave");
    std::cout << "Does dave have 'read' permission? " << std::boolalpha << pm.checkPermission("dave", "read") << std::endl;
    std::cout << std::endl;
    
    std::cout << "--- Test Case 5: Invalid inputs ---" << std::endl;
    std::cout << "Adding user with empty name: " << std::boolalpha << pm.addUser("  ") << std::endl;
    std::cout << "Checking empty permission for alice: " << std::boolalpha << pm.checkPermission("alice", " ") << std::endl;
    std::cout << std::endl;
}

int main() {
    run_tests();
    return 0;
}