
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_USERS 100
#define MAX_ROLES 10
#define MAX_PERMISSIONS 20
#define MAX_NAME_LENGTH 50

typedef struct {
    char name[MAX_NAME_LENGTH];
    char permissions[MAX_PERMISSIONS][MAX_NAME_LENGTH];
    int permissionCount;
} Role;

typedef struct {
    char username[MAX_NAME_LENGTH];
    int roleIndex;
} User;

typedef struct {
    User users[MAX_USERS];
    int userCount;
    Role roles[MAX_ROLES];
    int roleCount;
} Task138;

void initializeRole(Role* role, const char* name) {
    strcpy(role->name, name);
    role->permissionCount = 0;
}

void addPermissionToRoleStruct(Role* role, const char* permission) {
    if (role->permissionCount < MAX_PERMISSIONS) {
        strcpy(role->permissions[role->permissionCount], permission);
        role->permissionCount++;
    }
}

bool roleHasPermission(const Role* role, const char* permission) {
    for (int i = 0; i < role->permissionCount; i++) {
        if (strcmp(role->permissions[i], permission) == 0) {
            return true;
        }
    }
    return false;
}

Task138* createTask138() {
    Task138* system = (Task138*)malloc(sizeof(Task138));
    system->userCount = 0;
    system->roleCount = 0;
    
    // Initialize default roles
    initializeRole(&system->roles[0], "admin");
    addPermissionToRoleStruct(&system->roles[0], "read");
    addPermissionToRoleStruct(&system->roles[0], "write");
    addPermissionToRoleStruct(&system->roles[0], "delete");
    addPermissionToRoleStruct(&system->roles[0], "execute");
    system->roleCount++;
    
    initializeRole(&system->roles[1], "editor");
    addPermissionToRoleStruct(&system->roles[1], "read");
    addPermissionToRoleStruct(&system->roles[1], "write");
    system->roleCount++;
    
    initializeRole(&system->roles[2], "viewer");
    addPermissionToRoleStruct(&system->roles[2], "read");
    system->roleCount++;
    
    return system;
}

int findRole(Task138* system, const char* roleName) {
    for (int i = 0; i < system->roleCount; i++) {
        if (strcmp(system->roles[i].name, roleName) == 0) {
            return i;
        }
    }
    return -1;
}

int findUser(Task138* system, const char* username) {
    for (int i = 0; i < system->userCount; i++) {
        if (strcmp(system->users[i].username, username) == 0) {
            return i;
        }
    }
    return -1;
}

bool addUser(Task138* system, const char* username, const char* roleName) {
    if (findUser(system, username) != -1) {
        return false;
    }
    int roleIndex = findRole(system, roleName);
    if (roleIndex == -1) {
        return false;
    }
    if (system->userCount >= MAX_USERS) {
        return false;
    }
    strcpy(system->users[system->userCount].username, username);
    system->users[system->userCount].roleIndex = roleIndex;
    system->userCount++;
    return true;
}

bool removeUser(Task138* system, const char* username) {
    int userIndex = findUser(system, username);
    if (userIndex == -1) {
        return false;
    }
    for (int i = userIndex; i < system->userCount - 1; i++) {
        system->users[i] = system->users[i + 1];
    }
    system->userCount--;
    return true;
}

bool hasPermission(Task138* system, const char* username, const char* permission) {
    int userIndex = findUser(system, username);
    if (userIndex == -1) {
        return false;
    }
    int roleIndex = system->users[userIndex].roleIndex;
    return roleHasPermission(&system->roles[roleIndex], permission);
}

bool assignRole(Task138* system, const char* username, const char* roleName) {
    int userIndex = findUser(system, username);
    int roleIndex = findRole(system, roleName);
    if (userIndex == -1 || roleIndex == -1) {
        return false;
    }
    system->users[userIndex].roleIndex = roleIndex;
    return true;
}

bool addPermissionToRole(Task138* system, const char* roleName, const char* permission) {
    int roleIndex = findRole(system, roleName);
    if (roleIndex == -1) {
        return false;
    }
    addPermissionToRoleStruct(&system->roles[roleIndex], permission);
    return true;
}

bool removePermissionFromRole(Task138* system, const char* roleName, const char* permission) {
    int roleIndex = findRole(system, roleName);
    if (roleIndex == -1) {
        return false;
    }
    Role* role = &system->roles[roleIndex];
    for (int i = 0; i < role->permissionCount; i++) {
        if (strcmp(role->permissions[i], permission) == 0) {
            for (int j = i; j < role->permissionCount - 1; j++) {
                strcpy(role->permissions[j], role->permissions[j + 1]);
            }
            role->permissionCount--;
            return true;
        }
    }
    return false;
}

const char* getUserRole(Task138* system, const char* username) {
    int userIndex = findUser(system, username);
    if (userIndex == -1) {
        return NULL;
    }
    return system->roles[system->users[userIndex].roleIndex].name;
}

void freeTask138(Task138* system) {
    free(system);
}

int main() {
    Task138* system = createTask138();
    
    // Test Case 1: Add users with different roles
    printf("Test Case 1: Add users\\n");
    printf("Add admin user: %d\\n", addUser(system, "john", "admin"));
    printf("Add editor user: %d\\n", addUser(system, "jane", "editor"));
    printf("Add viewer user: %d\\n", addUser(system, "bob", "viewer"));
    printf("\\n");
    
    // Test Case 2: Check permissions
    printf("Test Case 2: Check permissions\\n");
    printf("john has delete permission: %d\\n", hasPermission(system, "john", "delete"));
    printf("jane has delete permission: %d\\n", hasPermission(system, "jane", "delete"));
    printf("bob has read permission: %d\\n", hasPermission(system, "bob", "read"));
    printf("bob has write permission: %d\\n", hasPermission(system, "bob", "write"));
    printf("\\n");
    
    // Test Case 3: Get user role
    printf("Test Case 3: Get user roles\\n");
    printf("john's role: %s\\n", getUserRole(system, "john"));
    printf("jane's role: %s\\n", getUserRole(system, "jane"));
    printf("bob's role: %s\\n", getUserRole(system, "bob"));
    printf("\\n");
    
    // Test Case 4: Change user role
    printf("Test Case 4: Change user role\\n");
    printf("bob's current role: %s\\n", getUserRole(system, "bob"));
    printf("Assign editor role to bob: %d\\n", assignRole(system, "bob", "editor"));
    printf("bob's new role: %s\\n", getUserRole(system, "bob"));
    printf("bob now has write permission: %d\\n", hasPermission(system, "bob", "write"));
    printf("\\n");
    
    // Test Case 5: Add and remove permissions from role
    printf("Test Case 5: Modify role permissions\\n");
    printf("Add 'backup' permission to viewer: %d\\n", addPermissionToRole(system, "viewer", "backup"));
    addUser(system, "alice", "viewer");
    printf("alice has backup permission: %d\\n", hasPermission(system, "alice", "backup"));
    printf("Remove 'backup' permission from viewer: %d\\n", removePermissionFromRole(system, "viewer", "backup"));
    printf("alice still has backup permission: %d\\n", hasPermission(system, "alice", "backup"));
    
    freeTask138(system);
    return 0;
}
