
#include <iostream>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

using namespace std;

class User {
private:
    string username;
    string passwordHash;
    unordered_set<string> roles;
    unordered_set<string> permissions;
    
    string hashPassword(const string& password) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)password.c_str(), password.length(), hash);
        
        stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << hex << setw(2) << setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    
public:
    User(const string& username, const string& password) 
        : username(username), passwordHash(hashPassword(password)) {}
    
    bool verifyPassword(const string& password) {
        return passwordHash == hashPassword(password);
    }
    
    void addRole(const string& role) {
        roles.insert(role);
    }
    
    void removeRole(const string& role) {
        roles.erase(role);
    }
    
    void addPermission(const string& permission) {
        permissions.insert(permission);
    }
    
    void removePermission(const string& permission) {
        permissions.erase(permission);
    }
    
    bool hasPermission(const string& permission) {
        return permissions.find(permission) != permissions.end();
    }
    
    bool hasRole(const string& role) {
        return roles.find(role) != roles.end();
    }
    
    string getUsername() const {
        return username;
    }
    
    unordered_set<string> getRoles() const {
        return roles;
    }
    
    unordered_set<string> getPermissions() const {
        return permissions;
    }
    
    void clearPermissions() {
        permissions.clear();
    }
};

class PermissionManager {
private:
    unordered_map<string, User*> users;
    unordered_map<string, unordered_set<string>> rolePermissions;
    
    void initializeDefaultRoles() {
        rolePermissions["ADMIN"] = {
            "CREATE_USER", "DELETE_USER", "MODIFY_USER", "READ_USER",
            "CREATE_ROLE", "DELETE_ROLE", "ASSIGN_ROLE"
        };
        rolePermissions["MANAGER"] = {
            "READ_USER", "MODIFY_USER", "ASSIGN_ROLE"
        };
        rolePermissions["USER"] = {
            "READ_USER"
        };
    }
    
    void recalculatePermissions(const string& username) {
        User* user = users[username];
        if (!user) return;
        
        unordered_set<string> allPermissions;
        for (const string& role : user->getRoles()) {
            if (rolePermissions.find(role) != rolePermissions.end()) {
                for (const string& perm : rolePermissions[role]) {
                    allPermissions.insert(perm);
                }
            }
        }
        
        user->clearPermissions();
        for (const string& perm : allPermissions) {
            user->addPermission(perm);
        }
    }
    
public:
    PermissionManager() {
        initializeDefaultRoles();
    }
    
    ~PermissionManager() {
        for (auto& pair : users) {
            delete pair.second;
        }
    }
    
    bool createUser(const string& username, const string& password) {
        if (users.find(username) != users.end()) {
            return false;
        }
        users[username] = new User(username, password);
        return true;
    }
    
    bool authenticateUser(const string& username, const string& password) {
        if (users.find(username) == users.end()) {
            return false;
        }
        return users[username]->verifyPassword(password);
    }
    
    bool assignRole(const string& username, const string& role) {
        if (users.find(username) == users.end() || 
            rolePermissions.find(role) == rolePermissions.end()) {
            return false;
        }
        
        User* user = users[username];
        user->addRole(role);
        
        for (const string& permission : rolePermissions[role]) {
            user->addPermission(permission);
        }
        return true;
    }
    
    bool revokeRole(const string& username, const string& role) {
        if (users.find(username) == users.end()) {
            return false;
        }
        users[username]->removeRole(role);
        recalculatePermissions(username);
        return true;
    }
    
    bool grantPermission(const string& username, const string& permission) {
        if (users.find(username) == users.end()) {
            return false;
        }
        users[username]->addPermission(permission);
        return true;
    }
    
    bool revokePermission(const string& username, const string& permission) {
        if (users.find(username) == users.end()) {
            return false;
        }
        users[username]->removePermission(permission);
        return true;
    }
    
    bool checkPermission(const string& username, const string& permission) {
        if (users.find(username) == users.end()) {
            return false;
        }
        return users[username]->hasPermission(permission);
    }
    
    unordered_set<string> getUserPermissions(const string& username) {
        if (users.find(username) == users.end()) {
            return unordered_set<string>();
        }
        return users[username]->getPermissions();
    }
    
    unordered_set<string> getUserRoles(const string& username) {
        if (users.find(username) == users.end()) {
            return unordered_set<string>();
        }
        return users[username]->getRoles();
    }
};

void printSet(const unordered_set<string>& s) {
    cout << "{ ";
    for (const string& item : s) {
        cout << item << " ";
    }
    cout << "}" << endl;
}

int main() {
    PermissionManager pm;
    
    // Test Case 1: Create users and authenticate
    cout << "Test Case 1: User Creation and Authentication" << endl;
    pm.createUser("alice", "password123");
    pm.createUser("bob", "securepass");
    cout << "Alice authentication: " << (pm.authenticateUser("alice", "password123") ? "true" : "false") << endl;
    cout << "Bob wrong password: " << (pm.authenticateUser("bob", "wrongpass") ? "true" : "false") << endl;
    cout << endl;
    
    // Test Case 2: Assign roles to users
    cout << "Test Case 2: Role Assignment" << endl;
    pm.assignRole("alice", "ADMIN");
    pm.assignRole("bob", "USER");
    cout << "Alice roles: ";
    printSet(pm.getUserRoles("alice"));
    cout << "Bob roles: ";
    printSet(pm.getUserRoles("bob"));
    cout << endl;
    
    // Test Case 3: Check permissions based on roles
    cout << "Test Case 3: Permission Checking" << endl;
    cout << "Alice has CREATE_USER: " << (pm.checkPermission("alice", "CREATE_USER") ? "true" : "false") << endl;
    cout << "Bob has CREATE_USER: " << (pm.checkPermission("bob", "CREATE_USER") ? "true" : "false") << endl;
    cout << "Bob has READ_USER: " << (pm.checkPermission("bob", "READ_USER") ? "true" : "false") << endl;
    cout << endl;
    
    // Test Case 4: Grant and revoke specific permissions
    cout << "Test Case 4: Grant and Revoke Permissions" << endl;
    pm.grantPermission("bob", "MODIFY_USER");
    cout << "Bob permissions after grant: ";
    printSet(pm.getUserPermissions("bob"));
    pm.revokePermission("bob", "MODIFY_USER");
    cout << "Bob permissions after revoke: ";
    printSet(pm.getUserPermissions("bob"));
    cout << endl;
    
    // Test Case 5: Multiple roles and permission inheritance
    cout << "Test Case 5: Multiple Roles" << endl;
    pm.createUser("charlie", "charlie123");
    pm.assignRole("charlie", "USER");
    pm.assignRole("charlie", "MANAGER");
    cout << "Charlie roles: ";
    printSet(pm.getUserRoles("charlie"));
    cout << "Charlie permissions: ";
    printSet(pm.getUserPermissions("charlie"));
    pm.revokeRole("charlie", "MANAGER");
    cout << "Charlie permissions after revoking MANAGER: ";
    printSet(pm.getUserPermissions("charlie"));
    
    return 0;
}
