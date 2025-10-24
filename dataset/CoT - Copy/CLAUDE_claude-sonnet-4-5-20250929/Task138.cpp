
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <regex>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <cstring>
#include <memory>

class Task138 {
public:
    static const int MAX_USERNAME_LENGTH = 50;
    static const int MIN_PASSWORD_LENGTH = 8;
    static const int MAX_LOGIN_ATTEMPTS = 3;
    
    enum class Permission {
        READ,
        WRITE,
        DELETE_PERM,
        ADMIN
    };
    
    struct PermissionHash {
        size_t operator()(const Permission& p) const {
            return static_cast<size_t>(p);
        }
    };
    
    class User {
    private:
        std::string username;
        std::string passwordHash;
        std::string salt;
        std::unordered_set<Permission, PermissionHash> permissions;
        
    public:
        User(const std::string& uname, const std::string& password, 
             const std::unordered_set<Permission, PermissionHash>& perms) {
            if (!isValidUsername(uname)) {
                throw std::invalid_argument("Invalid username format");
            }
            if (!isValidPassword(password)) {
                throw std::invalid_argument("Password too short");
            }
            
            username = uname;
            salt = generateSalt();
            passwordHash = hashPassword(password, salt);
            permissions = perms;
        }
        
        std::string getUsername() const { return username; }
        
        std::unordered_set<Permission, PermissionHash> getPermissions() const {
            return permissions;
        }
        
        bool verifyPassword(const std::string& password) const {
            if (password.empty()) return false;
            std::string hash = hashPassword(password, salt);
            return hash == passwordHash;
        }
        
        void addPermission(Permission perm) {
            permissions.insert(perm);
        }
        
        void removePermission(Permission perm) {
            permissions.erase(perm);
        }
    };
    
    class PermissionManager {
    private:
        std::unordered_map<std::string, std::unique_ptr<User>> users;
        std::unordered_map<std::string, int> loginAttempts;
        
    public:
        bool addUser(const std::string& username, const std::string& password,
                    const std::unordered_set<Permission, PermissionHash>& permissions) {
            try {
                if (username.empty() || password.empty()) {
                    return false;
                }
                
                if (users.find(username) != users.end()) {
                    return false;
                }
                
                users[username] = std::make_unique<User>(username, password, permissions);
                return true;
            } catch (...) {
                return false;
            }
        }
        
        bool authenticate(const std::string& username, const std::string& password) {
            try {
                if (username.empty() || password.empty()) {
                    return false;
                }
                
                if (loginAttempts[username] >= MAX_LOGIN_ATTEMPTS) {
                    return false;
                }
                
                auto it = users.find(username);
                if (it == users.end()) {
                    recordFailedAttempt(username);
                    return false;
                }
                
                if (it->second->verifyPassword(password)) {
                    loginAttempts.erase(username);
                    return true;
                } else {
                    recordFailedAttempt(username);
                    return false;
                }
            } catch (...) {
                return false;
            }
        }
        
        bool hasPermission(const std::string& username, Permission permission) {
            auto it = users.find(username);
            if (it == users.end()) {
                return false;
            }
            
            auto perms = it->second->getPermissions();
            return perms.find(permission) != perms.end() || 
                   perms.find(Permission::ADMIN) != perms.end();
        }
        
        bool grantPermission(const std::string& adminUsername, 
                           const std::string& targetUsername, 
                           Permission permission) {
            if (!hasPermission(adminUsername, Permission::ADMIN)) {
                return false;
            }
            
            auto it = users.find(targetUsername);
            if (it == users.end()) {
                return false;
            }
            
            it->second->addPermission(permission);
            return true;
        }
        
        bool revokePermission(const std::string& adminUsername,
                            const std::string& targetUsername,
                            Permission permission) {
            if (!hasPermission(adminUsername, Permission::ADMIN)) {
                return false;
            }
            
            auto it = users.find(targetUsername);
            if (it == users.end()) {
                return false;
            }
            
            it->second->removePermission(permission);
            return true;
        }
        
    private:
        void recordFailedAttempt(const std::string& username) {
            loginAttempts[username]++;
        }
    };
    
private:
    static bool isValidUsername(const std::string& username) {
        if (username.empty() || username.length() > MAX_USERNAME_LENGTH) {
            return false;
        }
        std::regex pattern("^[a-zA-Z0-9_]{3,50}$");
        return std::regex_match(username, pattern);
    }
    
    static bool isValidPassword(const std::string& password) {
        return !password.empty() && password.length() >= MIN_PASSWORD_LENGTH;
    }
    
    static std::string generateSalt() {
        unsigned char salt[16];
        RAND_bytes(salt, 16);
        
        std::string result;
        for (int i = 0; i < 16; i++) {
            char buf[3];
            snprintf(buf, sizeof(buf), "%02x", salt[i]);
            result += buf;
        }
        return result;
    }
    
    static std::string hashPassword(const std::string& password, const std::string& salt) {
        std::string combined = salt + password;
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256(reinterpret_cast<const unsigned char*>(combined.c_str()), 
               combined.length(), hash);
        
        std::string result;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            char buf[3];
            snprintf(buf, sizeof(buf), "%02x", hash[i]);
            result += buf;
        }
        return result;
    }
};

int main() {
    Task138::PermissionManager manager;
    
    // Test Case 1: Add admin user
    std::unordered_set<Task138::Permission, Task138::PermissionHash> adminPerms = {
        Task138::Permission::ADMIN
    };
    bool result1 = manager.addUser("admin", "Admin@1234", adminPerms);
    std::cout << "Test 1 - Add admin user: " << (result1 ? "PASSED" : "FAILED") << std::endl;
    
    // Test Case 2: Add regular user with READ permission
    std::unordered_set<Task138::Permission, Task138::PermissionHash> readPerms = {
        Task138::Permission::READ
    };
    bool result2 = manager.addUser("user1", "User@12345", readPerms);
    std::cout << "Test 2 - Add regular user: " << (result2 ? "PASSED" : "FAILED") << std::endl;
    
    // Test Case 3: Authenticate user
    bool result3 = manager.authenticate("admin", "Admin@1234");
    std::cout << "Test 3 - Authenticate admin: " << (result3 ? "PASSED" : "FAILED") << std::endl;
    
    // Test Case 4: Check permission
    bool result4 = manager.hasPermission("user1", Task138::Permission::READ);
    std::cout << "Test 4 - User1 has READ permission: " << (result4 ? "PASSED" : "FAILED") << std::endl;
    
    // Test Case 5: Grant permission
    bool result5 = manager.grantPermission("admin", "user1", Task138::Permission::WRITE);
    bool hasWrite = manager.hasPermission("user1", Task138::Permission::WRITE);
    std::cout << "Test 5 - Admin grants WRITE to user1: " 
              << (result5 && hasWrite ? "PASSED" : "FAILED") << std::endl;
    
    return 0;
}
