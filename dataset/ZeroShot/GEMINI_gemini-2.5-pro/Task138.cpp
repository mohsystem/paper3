#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

/**
 * Manages user roles and permissions securely.
 * - Encapsulates data within private members.
 * - Uses const references for safe and efficient parameter passing.
 * - Validates input by checking for empty strings.
 * - Uses unordered_map and unordered_set for efficient lookups.
 */
class PermissionManager {
private:
    // Maps role names to a set of permission strings
    std::unordered_map<std::string, std::unordered_set<std::string>> rolesToPermissions;
    // Maps user names to a set of role strings
    std::unordered_map<std::string, std::unordered_set<std::string>> usersToRoles;

public:
    PermissionManager() = default;

    /**
     * Adds a new role to the system.
     * @param roleName The name of the role to add.
     * @return true if successful, false if input is invalid or role exists.
     */
    bool addRole(const std::string& roleName) {
        if (roleName.empty()) return false;
        // The insert method returns a pair; the .second is a bool indicating if insertion took place.
        return rolesToPermissions.insert({roleName, {}}).second;
    }

    /**
     * Adds a new user to the system.
     * @param userName The name of the user to add.
     * @return true if successful, false if input is invalid or user exists.
     */
    bool addUser(const std::string& userName) {
        if (userName.empty()) return false;
        return usersToRoles.insert({userName, {}}).second;
    }

    /**
     * Adds a permission to an existing role.
     * @param roleName The role to modify.
     * @param permission The permission to add.
     * @return true if successful, false if role doesn't exist or input is invalid.
     */
    bool addPermissionToRole(const std::string& roleName, const std::string& permission) {
        if (roleName.empty() || permission.empty()) return false;
        auto it = rolesToPermissions.find(roleName);
        if (it == rolesToPermissions.end()) {
            return false; // Role not found
        }
        it->second.insert(permission);
        return true;
    }

    /**
     * Assigns an existing role to an existing user.
     * @param userName The user to whom the role will be assigned.
     * @param roleName The role to assign.
     * @return true if successful, false if user or role doesn't exist or input is invalid.
     */
    bool assignRoleToUser(const std::string& userName, const std::string& roleName) {
        if (userName.empty() || roleName.empty()) return false;
        auto userIt = usersToRoles.find(userName);
        if (userIt == usersToRoles.end() || rolesToPermissions.find(roleName) == rolesToPermissions.end()) {
            return false; // User or role not found
        }
        userIt->second.insert(roleName);
        return true;
    }

    /**
     * Checks if a user has a specific permission.
     * @param userName The user to check.
     * @param permission The permission to check for.
     * @return true if the user has the permission through any of their roles, false otherwise.
     */
    bool checkPermission(const std::string& userName, const std::string& permission) {
        if (userName.empty() || permission.empty()) return false;

        auto userIt = usersToRoles.find(userName);
        if (userIt == usersToRoles.end()) {
            return false; // User not found
        }

        for (const auto& role : userIt->second) {
            auto roleIt = rolesToPermissions.find(role);
            if (roleIt != rolesToPermissions.end()) {
                if (roleIt->second.count(permission)) {
                    return true;
                }
            }
        }
        return false;
    }
};

int main() {
    PermissionManager manager;

    // Setup
    manager.addRole("admin");
    manager.addRole("editor");
    manager.addRole("viewer");

    manager.addPermissionToRole("admin", "read");
    manager.addPermissionToRole("admin", "write");
    manager.addPermissionToRole("admin", "delete");
    manager.addPermissionToRole("admin", "manage_users");

    manager.addPermissionToRole("editor", "read");
    manager.addPermissionToRole("editor", "write");

    manager.addPermissionToRole("viewer", "read");

    manager.addUser("alice");
    manager.addUser("bob");
    manager.addUser("charlie");

    manager.assignRoleToUser("alice", "admin");
    manager.assignRoleToUser("bob", "editor");
    manager.assignRoleToUser("charlie", "viewer");

    std::cout << "--- Running Test Cases ---" << std::endl;
    std::cout << std::boolalpha; // Print bools as "true" or "false"

    // Test Case 1: Admin has delete permission
    std::cout << "1. Alice has 'delete' permission: " << manager.checkPermission("alice", "delete") << std::endl;

    // Test Case 2: Editor has write permission
    std::cout << "2. Bob has 'write' permission: " << manager.checkPermission("bob", "write") << std::endl;

    // Test Case 3: Editor does NOT have delete permission
    std::cout << "3. Bob has 'delete' permission: " << manager.checkPermission("bob", "delete") << std::endl;

    // Test Case 4: Non-existent user
    std::cout << "4. Dave has 'read' permission: " << manager.checkPermission("dave", "read") << std::endl;
    
    // Test Case 5: User with multiple roles
    manager.assignRoleToUser("bob", "viewer"); // bob is now editor and viewer
    std::cout << "5. Bob (now editor & viewer) has 'read' permission: " << manager.checkPermission("bob", "read") << std::endl;

    std::cout << "--- Test Cases Finished ---" << std::endl;

    return 0;
}