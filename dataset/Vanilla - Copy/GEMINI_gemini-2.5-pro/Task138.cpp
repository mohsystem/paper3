#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>

// Enum class for type-safe permissions
enum class Permission {
    READ, WRITE, EXECUTE, DELETE
};

// Helper function to convert Permission enum to string for printing
std::string permissionToString(Permission p) {
    switch (p) {
        case Permission::READ:    return "READ";
        case Permission::WRITE:   return "WRITE";
        case Permission::EXECUTE: return "EXECUTE";
        case Permission::DELETE:  return "DELETE";
        default:                  return "UNKNOWN";
    }
}

class User {
private:
    std::string username;
    std::set<Permission> permissions;

public:
    User(const std::string& name) : username(name) {}

    // Default constructor for map compatibility
    User() : username("") {}

    void grantPermission(Permission p) {
        permissions.insert(p);
    }

    void revokePermission(Permission p) {
        permissions.erase(p);
    }

    bool hasPermission(Permission p) const {
        return permissions.count(p) > 0;
    }

    const std::set<Permission>& getPermissions() const {
        return permissions;
    }
};

class PermissionManager {
private:
    std::map<std::string, User> users;

public:
    void addUser(const std::string& username) {
        if (users.find(username) == users.end()) {
            users[username] = User(username);
            std::cout << "User '" << username << "' added." << std::endl;
        } else {
            std::cout << "User '" << username << "' already exists." << std::endl;
        }
    }

    void grantPermission(const std::string& username, Permission p) {
        auto it = users.find(username);
        if (it != users.end()) {
            it->second.grantPermission(p);
            std::cout << "Permission " << permissionToString(p) << " granted to '" << username << "'." << std::endl;
        } else {
            std::cout << "User '" << username << "' not found." << std::endl;
        }
    }

    void revokePermission(const std::string& username, Permission p) {
        auto it = users.find(username);
        if (it != users.end()) {
            it->second.revokePermission(p);
            std::cout << "Permission " << permissionToString(p) << " revoked from '" << username << "'." << std::endl;
        } else {
            std::cout << "User '" << username << "' not found." << std::endl;
        }
    }

    bool hasPermission(const std::string& username, Permission p) {
        auto it = users.find(username);
        if (it != users.end()) {
            return it->second.hasPermission(p);
        }
        return false;
    }

    void printUserPermissions(const std::string& username) {
        auto it = users.find(username);
        if (it != users.end()) {
            const auto& perms = it->second.getPermissions();
            if (perms.empty()) {
                std::cout << "User '" << username << "' has no permissions." << std::endl;
            } else {
                std::cout << "User '" << username << "' permissions: ";
                bool first = true;
                for (const auto& p : perms) {
                    if (!first) {
                        std::cout << ", ";
                    }
                    std::cout << permissionToString(p);
                    first = false;
                }
                std::cout << std::endl;
            }
        } else {
            std::cout << "User '" << username << "' not found." << std::endl;
        }
    }
};

int main() {
    PermissionManager manager;

    std::cout << "--- Test Case 1: Add users ---" << std::endl;
    manager.addUser("alice");
    manager.addUser("bob");
    std::cout << std::endl;

    std::cout << "--- Test Case 2: Grant permissions to alice ---" << std::endl;
    manager.grantPermission("alice", Permission::READ);
    manager.grantPermission("alice", Permission::WRITE);
    manager.printUserPermissions("alice");
    manager.printUserPermissions("bob");
    std::cout << std::endl;

    std::cout << "--- Test Case 3: Check alice's permissions ---" << std::endl;
    std::cout << "Does alice have WRITE permission? " << (manager.hasPermission("alice", Permission::WRITE) ? "true" : "false") << std::endl;
    std::cout << "Does alice have EXECUTE permission? " << (manager.hasPermission("alice", Permission::EXECUTE) ? "true" : "false") << std::endl;
    std::cout << std::endl;

    std::cout << "--- Test Case 4: Revoke a permission from alice ---" << std::endl;
    manager.revokePermission("alice", Permission::WRITE);
    manager.printUserPermissions("alice");
    std::cout << std::endl;
    
    std::cout << "--- Test Case 5: Final permission checks ---" << std::endl;
    std::cout << "Does alice have WRITE permission now? " << (manager.hasPermission("alice", Permission::WRITE) ? "true" : "false") << std::endl;
    std::cout << "Does alice still have READ permission? " << (manager.hasPermission("alice", Permission::READ) ? "true" : "false") << std::endl;
    std::cout << "Does bob have READ permission? " << (manager.hasPermission("bob", Permission::READ) ? "true" : "false") << std::endl;
    manager.grantPermission("bob", Permission::EXECUTE);
    manager.printUserPermissions("bob");

    return 0;
}