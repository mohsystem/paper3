
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>

using namespace std;

class Role {
private:
    string name;
    unordered_set<string> permissions;

public:
    Role(const string& name) : name(name) {}
    
    void addPermission(const string& permission) {
        permissions.insert(permission);
    }
    
    bool removePermission(const string& permission) {
        return permissions.erase(permission) > 0;
    }
    
    bool hasPermission(const string& permission) const {
        return permissions.find(permission) != permissions.end();
    }
    
    unordered_set<string> getPermissions() const {
        return permissions;
    }
    
    string getName() const {
        return name;
    }
};

class User {
private:
    string username;
    shared_ptr<Role> role;

public:
    User(const string& username, shared_ptr<Role> role) 
        : username(username), role(role) {}
    
    bool hasPermission(const string& permission) const {
        return role->hasPermission(permission);
    }
    
    unordered_set<string> getPermissions() const {
        return role->getPermissions();
    }
    
    shared_ptr<Role> getRole() const {
        return role;
    }
    
    void setRole(shared_ptr<Role> newRole) {
        role = newRole;
    }
};

class Task138 {
private:
    unordered_map<string, unique_ptr<User>> users;
    unordered_map<string, shared_ptr<Role>> roles;
    
    void initializeDefaultRoles() {
        auto admin = make_shared<Role>("admin");
        admin->addPermission("read");
        admin->addPermission("write");
        admin->addPermission("delete");
        admin->addPermission("execute");
        roles["admin"] = admin;
        
        auto editor = make_shared<Role>("editor");
        editor->addPermission("read");
        editor->addPermission("write");
        roles["editor"] = editor;
        
        auto viewer = make_shared<Role>("viewer");
        viewer->addPermission("read");
        roles["viewer"] = viewer;
    }

public:
    Task138() {
        initializeDefaultRoles();
    }
    
    bool addUser(const string& username, const string& roleName) {
        if (users.find(username) != users.end()) {
            return false;
        }
        auto roleIt = roles.find(roleName);
        if (roleIt == roles.end()) {
            return false;
        }
        users[username] = make_unique<User>(username, roleIt->second);
        return true;
    }
    
    bool removeUser(const string& username) {
        return users.erase(username) > 0;
    }
    
    bool hasPermission(const string& username, const string& permission) {
        auto it = users.find(username);
        if (it == users.end()) {
            return false;
        }
        return it->second->hasPermission(permission);
    }
    
    bool assignRole(const string& username, const string& roleName) {
        auto userIt = users.find(username);
        auto roleIt = roles.find(roleName);
        if (userIt == users.end() || roleIt == roles.end()) {
            return false;
        }
        userIt->second->setRole(roleIt->second);
        return true;
    }
    
    unordered_set<string> getUserPermissions(const string& username) {
        auto it = users.find(username);
        if (it == users.end()) {
            return unordered_set<string>();
        }
        return it->second->getPermissions();
    }
    
    bool addPermissionToRole(const string& roleName, const string& permission) {
        auto it = roles.find(roleName);
        if (it == roles.end()) {
            return false;
        }
        it->second->addPermission(permission);
        return true;
    }
    
    bool removePermissionFromRole(const string& roleName, const string& permission) {
        auto it = roles.find(roleName);
        if (it == roles.end()) {
            return false;
        }
        return it->second->removePermission(permission);
    }
    
    string getUserRole(const string& username) {
        auto it = users.find(username);
        if (it == users.end()) {
            return "";
        }
        return it->second->getRole()->getName();
    }
};

int main() {
    Task138 system;
    
    // Test Case 1: Add users with different roles
    cout << "Test Case 1: Add users" << endl;
    cout << "Add admin user: " << system.addUser("john", "admin") << endl;
    cout << "Add editor user: " << system.addUser("jane", "editor") << endl;
    cout << "Add viewer user: " << system.addUser("bob", "viewer") << endl;
    cout << endl;
    
    // Test Case 2: Check permissions
    cout << "Test Case 2: Check permissions" << endl;
    cout << "john has delete permission: " << system.hasPermission("john", "delete") << endl;
    cout << "jane has delete permission: " << system.hasPermission("jane", "delete") << endl;
    cout << "bob has read permission: " << system.hasPermission("bob", "read") << endl;
    cout << "bob has write permission: " << system.hasPermission("bob", "write") << endl;
    cout << endl;
    
    // Test Case 3: Get user permissions
    cout << "Test Case 3: Get user permissions" << endl;
    cout << "john's permissions: ";
    for (const auto& perm : system.getUserPermissions("john")) {
        cout << perm << " ";
    }
    cout << endl;
    cout << "jane's permissions: ";
    for (const auto& perm : system.getUserPermissions("jane")) {
        cout << perm << " ";
    }
    cout << endl;
    cout << "bob's permissions: ";
    for (const auto& perm : system.getUserPermissions("bob")) {
        cout << perm << " ";
    }
    cout << endl << endl;
    
    // Test Case 4: Change user role
    cout << "Test Case 4: Change user role" << endl;
    cout << "bob's current role: " << system.getUserRole("bob") << endl;
    cout << "Assign editor role to bob: " << system.assignRole("bob", "editor") << endl;
    cout << "bob's new role: " << system.getUserRole("bob") << endl;
    cout << "bob now has write permission: " << system.hasPermission("bob", "write") << endl;
    cout << endl;
    
    // Test Case 5: Add and remove permissions from role
    cout << "Test Case 5: Modify role permissions" << endl;
    cout << "Add 'backup' permission to viewer: " << system.addPermissionToRole("viewer", "backup") << endl;
    system.addUser("alice", "viewer");
    cout << "alice has backup permission: " << system.hasPermission("alice", "backup") << endl;
    cout << "Remove 'backup' permission from viewer: " << system.removePermissionFromRole("viewer", "backup") << endl;
    cout << "alice still has backup permission: " << system.hasPermission("alice", "backup") << endl;
    
    return 0;
}
