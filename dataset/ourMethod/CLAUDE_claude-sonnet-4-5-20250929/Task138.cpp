
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <limits>

// Secure user permission management system
// All inputs are validated, no dynamic code execution, memory-safe operations

enum class Permission {
    READ,
    WRITE,
    EXECUTE,
    DELETE,
    ADMIN
};

// Convert permission enum to string for display
std::string permissionToString(Permission perm) {
    switch (perm) {
        case Permission::READ: return "READ";
        case Permission::WRITE: return "WRITE";
        case Permission::EXECUTE: return "EXECUTE";
        case Permission::DELETE: return "DELETE";
        case Permission::ADMIN: return "ADMIN";
        default: return "UNKNOWN";
    }
}

// Validate username: alphanumeric and underscore only, 3-32 chars
bool isValidUsername(const std::string& username) {
    if (username.empty() || username.length() < 3 || username.length() > 32) {
        return false;
    }
    
    for (char c : username) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') {
            return false;
        }
    }
    return true;
}

// Validate resource name: alphanumeric, underscore, hyphen, slash, 1-128 chars
bool isValidResource(const std::string& resource) {
    if (resource.empty() || resource.length() > 128) {
        return false;
    }
    
    for (char c : resource) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && 
            c != '_' && c != '-' && c != '/' && c != '.') {
            return false;
        }
    }
    return true;
}

class User {
private:
    std::string username;
    std::map<std::string, std::vector<Permission>> resourcePermissions;
    
public:
    explicit User(const std::string& name) {
        // Input validation: username must be safe
        if (!isValidUsername(name)) {
            throw std::invalid_argument("Invalid username format");
        }
        username = name;
    }
    
    std::string getUsername() const {
        return username;
    }
    
    // Grant permission to a resource
    bool grantPermission(const std::string& resource, Permission perm) {
        // Input validation: resource name must be safe
        if (!isValidResource(resource)) {
            return false;
        }
        
        auto& perms = resourcePermissions[resource];
        
        // Check if permission already exists
        if (std::find(perms.begin(), perms.end(), perm) == perms.end()) {
            perms.push_back(perm);
        }
        return true;
    }
    
    // Revoke permission from a resource
    bool revokePermission(const std::string& resource, Permission perm) {
        // Input validation
        if (!isValidResource(resource)) {
            return false;
        }
        
        auto it = resourcePermissions.find(resource);
        if (it != resourcePermissions.end()) {
            auto& perms = it->second;
            perms.erase(std::remove(perms.begin(), perms.end(), perm), perms.end());
            
            // Remove resource entry if no permissions left
            if (perms.empty()) {
                resourcePermissions.erase(it);
            }
            return true;
        }
        return false;
    }
    
    // Check if user has specific permission on resource
    bool hasPermission(const std::string& resource, Permission perm) const {
        // Input validation
        if (!isValidResource(resource)) {
            return false;
        }
        
        auto it = resourcePermissions.find(resource);
        if (it != resourcePermissions.end()) {
            const auto& perms = it->second;
            return std::find(perms.begin(), perms.end(), perm) != perms.end();
        }
        return false;
    }
    
    // Get all permissions for a resource
    std::vector<Permission> getPermissions(const std::string& resource) const {
        // Input validation
        if (!isValidResource(resource)) {
            return std::vector<Permission>();
        }
        
        auto it = resourcePermissions.find(resource);
        if (it != resourcePermissions.end()) {
            return it->second;
        }
        return std::vector<Permission>();
    }
    
    // List all resources user has access to
    std::vector<std::string> listResources() const {
        std::vector<std::string> resources;
        resources.reserve(resourcePermissions.size());
        
        for (const auto& pair : resourcePermissions) {
            resources.push_back(pair.first);
        }
        return resources;
    }
};

class PermissionManager {
private:
    std::map<std::string, std::unique_ptr<User>> users;
    static const size_t MAX_USERS = 10000; // Prevent excessive memory usage
    
public:
    // Create a new user
    bool createUser(const std::string& username) {
        // Input validation
        if (!isValidUsername(username)) {
            return false;
        }
        
        // Check maximum users limit
        if (users.size() >= MAX_USERS) {
            return false;
        }
        
        // Check if user already exists
        if (users.find(username) != users.end()) {
            return false;
        }
        
        try {
            users[username] = std::make_unique<User>(username);
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }
    
    // Delete a user
    bool deleteUser(const std::string& username) {
        // Input validation
        if (!isValidUsername(username)) {
            return false;
        }
        
        auto it = users.find(username);
        if (it != users.end()) {
            users.erase(it);
            return true;
        }
        return false;
    }
    
    // Grant permission to user for resource
    bool grantPermission(const std::string& username, const std::string& resource, 
                        Permission perm) {
        // Input validation
        if (!isValidUsername(username) || !isValidResource(resource)) {
            return false;
        }
        
        auto it = users.find(username);
        if (it != users.end()) {
            return it->second->grantPermission(resource, perm);
        }
        return false;
    }
    
    // Revoke permission from user for resource
    bool revokePermission(const std::string& username, const std::string& resource, 
                         Permission perm) {
        // Input validation
        if (!isValidUsername(username) || !isValidResource(resource)) {
            return false;
        }
        
        auto it = users.find(username);
        if (it != users.end()) {
            return it->second->revokePermission(resource, perm);
        }
        return false;
    }
    
    // Check if user has permission
    bool checkPermission(const std::string& username, const std::string& resource, 
                        Permission perm) const {
        // Input validation
        if (!isValidUsername(username) || !isValidResource(resource)) {
            return false;
        }
        
        auto it = users.find(username);
        if (it != users.end()) {
            return it->second->hasPermission(resource, perm);
        }
        return false;
    }
    
    // Get user's permissions for resource\n    std::vector<Permission> getUserPermissions(const std::string& username, \n                                              const std::string& resource) const {\n        // Input validation\n        if (!isValidUsername(username) || !isValidResource(resource)) {\n            return std::vector<Permission>();\n        }\n        \n        auto it = users.find(username);\n        if (it != users.end()) {\n            return it->second->getPermissions(resource);\n        }\n        return std::vector<Permission>();\n    }\n    \n    // List all users\n    std::vector<std::string> listUsers() const {\n        std::vector<std::string> userList;\n        userList.reserve(users.size());\n        \n        for (const auto& pair : users) {\n            userList.push_back(pair.first);\n        }\n        return userList;\n    }\n};\n\nint main() {\n    try {\n        PermissionManager manager;\n        \n        // Test Case 1: Create users and grant basic permissions\n        std::cout << "Test Case 1: Create users and grant permissions\\n";\n        manager.createUser("alice");\n        manager.createUser("bob");\n        manager.grantPermission("alice", "/docs/report.txt", Permission::READ);\n        manager.grantPermission("alice", "/docs/report.txt", Permission::WRITE);\n        manager.grantPermission("bob", "/docs/report.txt", Permission::READ);\n        \n        std::cout << "Alice has READ on /docs/report.txt: " \n                  << (manager.checkPermission("alice", "/docs/report.txt", Permission::READ) ? "Yes" : "No") << "\\n";\n        std::cout << "Bob has WRITE on /docs/report.txt: " \n                  << (manager.checkPermission("bob", "/docs/report.txt", Permission::WRITE) ? "Yes" : "No") << "\\n\\n";\n        \n        // Test Case 2: Revoke permissions\n        std::cout << "Test Case 2: Revoke permissions\\n";\n        manager.revokePermission("alice", "/docs/report.txt", Permission::WRITE);\n        std::cout << "Alice has WRITE after revoke: " \n                  << (manager.checkPermission("alice", "/docs/report.txt", Permission::WRITE) ? "Yes" : "No") << "\\n";\n        std::cout << "Alice has READ after revoke: " \n                  << (manager.checkPermission("alice", "/docs/report.txt", Permission::READ) ? "Yes" : "No") << "\\n\\n";\n        \n        // Test Case 3: Admin permissions\n        std::cout << "Test Case 3: Admin permissions\\n";\n        manager.createUser("admin_user");\n        manager.grantPermission("admin_user", "/system/config", Permission::ADMIN);\n        manager.grantPermission("admin_user", "/system/config", Permission::READ);\n        manager.grantPermission("admin_user", "/system/config", Permission::WRITE);\n        \n        auto adminPerms = manager.getUserPermissions("admin_user", "/system/config");\n        std::cout << "Admin user permissions on /system/config: ";\n        for (const auto& perm : adminPerms) {\n            std::cout << permissionToString(perm) << " ";\n        }\n        std::cout << "\\n\\n";\n        \n        // Test Case 4: Invalid input handling\n        std::cout << "Test Case 4: Invalid input handling\\n";\n        bool result1 = manager.createUser("a"); // Too short\n        bool result2 = manager.createUser("user@invalid"); // Invalid character\n        bool result3 = manager.grantPermission("alice", std::string(200, 'x'), Permission::READ); // Too long\n        \n        std::cout << "Create user 'a': " << (result1 ? "Success" : "Failed (expected)") << "\\n";\n        std::cout << "Create user 'user@invalid': " << (result2 ? "Success" : "Failed (expected)") << "\
";\n        std::cout << "Grant permission to 200-char resource: " << (result3 ? "Success" : "Failed (expected)") << "\
\
";\n        \n        // Test Case 5: List operations and delete user\n        std::cout << "Test Case 5: List operations and delete user\
";\n        auto users = manager.listUsers();\n        std::cout << "All users: ";\n        for (const auto& user : users) {\n            std::cout << user << " ";\n        }\n        std::cout << "\
";\n        \n        manager.deleteUser("bob");\n        std::cout << "After deleting bob, can check his permissions: " \n                  << (manager.checkPermission("bob", "/docs/report.txt", Permission::READ) ? "Yes" : "No (expected)") << "\
";\n        \n        return 0;\n        \n    } catch (const std::exception& e) {\n        std::cerr << "Error: " << e.what() << "\
";
        return 1;
    }
}
