#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>

// Enum for type-safe permissions
enum class Permission {
    READ,
    WRITE,
    DELETE,
    EXECUTE
};

// Helper to convert Permission enum to string for printing
std::string permissionToString(Permission p) {
    switch (p) {
        case Permission::READ: return "READ";
        case Permission::WRITE: return "WRITE";
        case Permission::DELETE: return "DELETE";
        case Permission::EXECUTE: return "EXECUTE";
        default: return "UNKNOWN";
    }
}

class PermissionManager {
private:
    std::map<std::string, std::set<Permission>> roles; // Role name -> Set of permissions
    std::map<std::string, std::string> users;          // User name -> Role name

public:
    // Add a new role with a set of permissions
    void addRole(const std::string& roleName, const std::set<Permission>& permissions) {
        if (!roleName.empty()) {
            roles[roleName] = permissions;
        }
    }

    // Assign a role to a user
    void assignRoleToUser(const std::string& userName, const std::string& roleName) {
        // Ensure the role exists before assigning
        if (!userName.empty() && roles.count(roleName)) {
            users[userName] = roleName;
        }
    }

    /**
     * @brief Checks if a user has a specific permission.
     * 
     * @param userName The name of the user.
     * @param permission The permission to check.
     * @return true if the user has the permission, false otherwise.
     */
    bool checkPermission(const std::string& userName, Permission permission) const {
        // 1. Find the user's role
        auto user_it = users.find(userName);
        if (user_it == users.end()) {
            // User does not exist or has no role
            return false;
        }
        const std::string& roleName = user_it->second;

        // 2. Get the permissions for that role
        auto role_it = roles.find(roleName);
        if (role_it == roles.end()) {
            // Role assigned to user does not exist in roles map
            return false;
        }
        const std::set<Permission>& permissions = role_it->second;

        // 3. Check if the permission is in the set
        return permissions.count(permission) > 0;
    }
};

int main() {
    PermissionManager manager;

    // 1. Define roles and their permissions
    manager.addRole("ADMIN", {Permission::READ, Permission::WRITE, Permission::DELETE, Permission::EXECUTE});
    manager.addRole("EDITOR", {Permission::READ, Permission::WRITE});
    manager.addRole("VIEWER", {Permission::READ});

    // 2. Create users and assign roles
    manager.assignRoleToUser("alice", "ADMIN");
    manager.assignRoleToUser("bob", "EDITOR");
    manager.assignRoleToUser("charlie", "VIEWER");

    std::cout << "Running test cases...\n" << std::endl;
    std::cout << std::boolalpha; // Print bools as "true" or "false"

    // Test Case 1: Admin user checking an admin-only permission
    std::cout << "Test 1: Does alice (ADMIN) have DELETE permission?" << std::endl;
    std::cout << "Result: " << manager.checkPermission("alice", Permission::DELETE) << std::endl; // Expected: true

    // Test Case 2: Editor user checking an editor permission
    std::cout << "\nTest 2: Does bob (EDITOR) have WRITE permission?" << std::endl;
    std::cout << "Result: " << manager.checkPermission("bob", Permission::WRITE) << std::endl; // Expected: true

    // Test Case 3: Editor user checking an admin-only permission
    std::cout << "\nTest 3: Does bob (EDITOR) have DELETE permission?" << std::endl;
    std::cout << "Result: " << manager.checkPermission("bob", Permission::DELETE) << std::endl; // Expected: false

    // Test Case 4: Viewer user checking a read-only permission
    std::cout << "\nTest 4: Does charlie (VIEWER) have READ permission?" << std::endl;
    std::cout << "Result: " << manager.checkPermission("charlie", Permission::READ) << std::endl; // Expected: true

    // Test Case 5: Non-existent user checking any permission
    std::cout << "\nTest 5: Does david (non-existent) have READ permission?" << std::endl;
    std::cout << "Result: " << manager.checkPermission("david", Permission::READ) << std::endl; // Expected: false

    return 0;
}