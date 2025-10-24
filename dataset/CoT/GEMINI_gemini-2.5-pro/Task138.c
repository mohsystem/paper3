#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_ROLES 10
#define MAX_USERS 50
#define MAX_PERMS_PER_ROLE 5
#define MAX_NAME_LEN 32

// Enum for type-safe permissions
typedef enum {
    READ,
    WRITE,
    DELETE,
    EXECUTE
} Permission;

// Struct to define a role
typedef struct {
    char name[MAX_NAME_LEN];
    Permission permissions[MAX_PERMS_PER_ROLE];
    int num_permissions;
} Role;

// Struct to define a user
typedef struct {
    char name[MAX_NAME_LEN];
    const Role* role; // Pointer to the user's role
} User;

// "Database" of roles and users
typedef struct {
    Role roles[MAX_ROLES];
    User users[MAX_USERS];
    int num_roles;
    int num_users;
} PermissionManager;

// Helper to find a role by name
const Role* find_role(const PermissionManager* manager, const char* roleName) {
    for (int i = 0; i < manager->num_roles; ++i) {
        if (strcmp(manager->roles[i].name, roleName) == 0) {
            return &manager->roles[i];
        }
    }
    return NULL;
}

// Helper to find a user by name
const User* find_user(const PermissionManager* manager, const char* userName) {
    for (int i = 0; i < manager->num_users; ++i) {
        if (strcmp(manager->users[i].name, userName) == 0) {
            return &manager->users[i];
        }
    }
    return NULL;
}

/**
 * @brief Checks if a user has a specific permission.
 * 
 * @param manager Pointer to the PermissionManager containing the data.
 * @param userName The name of the user.
 * @param permission The permission to check.
 * @return true if the user has the permission, false otherwise.
 */
bool check_permission(const PermissionManager* manager, const char* userName, Permission permission) {
    if (userName == NULL) {
        return false;
    }

    // 1. Find the user
    const User* user = find_user(manager, userName);
    if (user == NULL || user->role == NULL) {
        return false; // User not found or has no role
    }

    // 2. Get the role from the user
    const Role* role = user->role;

    // 3. Check if the permission is in the role's permissions list
    for (int i = 0; i < role->num_permissions; ++i) {
        if (role->permissions[i] == permission) {
            return true;
        }
    }

    return false;
}

int main() {
    PermissionManager manager = {0}; // Initialize all fields to zero/NULL

    // 1. Define roles
    // ADMIN role
    strcpy(manager.roles[0].name, "ADMIN");
    manager.roles[0].permissions[0] = READ;
    manager.roles[0].permissions[1] = WRITE;
    manager.roles[0].permissions[2] = DELETE;
    manager.roles[0].permissions[3] = EXECUTE;
    manager.roles[0].num_permissions = 4;
    manager.num_roles++;

    // EDITOR role
    strcpy(manager.roles[1].name, "EDITOR");
    manager.roles[1].permissions[0] = READ;
    manager.roles[1].permissions[1] = WRITE;
    manager.roles[1].num_permissions = 2;
    manager.num_roles++;

    // VIEWER role
    strcpy(manager.roles[2].name, "VIEWER");
    manager.roles[2].permissions[0] = READ;
    manager.roles[2].num_permissions = 1;
    manager.num_roles++;
    
    // 2. Define users and assign roles
    // alice
    strcpy(manager.users[0].name, "alice");
    manager.users[0].role = find_role(&manager, "ADMIN");
    manager.num_users++;

    // bob
    strcpy(manager.users[1].name, "bob");
    manager.users[1].role = find_role(&manager, "EDITOR");
    manager.num_users++;

    // charlie
    strcpy(manager.users[2].name, "charlie");
    manager.users[2].role = find_role(&manager, "VIEWER");
    manager.num_users++;

    printf("Running test cases...\n\n");

    // Test Case 1: Admin user checking an admin-only permission
    printf("Test 1: Does alice (ADMIN) have DELETE permission?\n");
    printf("Result: %s\n", check_permission(&manager, "alice", DELETE) ? "true" : "false"); // Expected: true

    // Test Case 2: Editor user checking an editor permission
    printf("\nTest 2: Does bob (EDITOR) have WRITE permission?\n");
    printf("Result: %s\n", check_permission(&manager, "bob", WRITE) ? "true" : "false"); // Expected: true

    // Test Case 3: Editor user checking an admin-only permission
    printf("\nTest 3: Does bob (EDITOR) have DELETE permission?\n");
    printf("Result: %s\n", check_permission(&manager, "bob", DELETE) ? "true" : "false"); // Expected: false

    // Test Case 4: Viewer user checking a read-only permission
    printf("\nTest 4: Does charlie (VIEWER) have READ permission?\n");
    printf("Result: %s\n", check_permission(&manager, "charlie", READ) ? "true" : "false"); // Expected: true

    // Test Case 5: Non-existent user checking any permission
    printf("\nTest 5: Does david (non-existent) have READ permission?\n");
    printf("Result: %s\n", check_permission(&manager, "david", READ) ? "true" : "false"); // Expected: false

    return 0;
}