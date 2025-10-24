#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_ROLES 10
#define MAX_USERS 20
#define MAX_PERMS_PER_ROLE 10
#define MAX_ROLES_PER_USER 5
#define MAX_NAME_LEN 50

// --- Data Structures ---

typedef struct {
    char name[MAX_NAME_LEN];
    char permissions[MAX_PERMS_PER_ROLE][MAX_NAME_LEN];
    int num_permissions;
} Role;

typedef struct {
    char name[MAX_NAME_LEN];
    int role_indices[MAX_ROLES_PER_USER];
    int num_roles;
} User;

// --- Global State (acting as the manager) ---
// This simple model uses fixed-size global arrays. A more robust implementation
// would use dynamic memory allocation.
Role all_roles[MAX_ROLES];
int num_roles = 0;
User all_users[MAX_USERS];
int num_users = 0;

// --- Helper Functions ---

// Securely copies a string, ensuring null termination.
void secure_strcpy(char* dest, const char* src, size_t dest_size) {
    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';
}

// Finds a user by name and returns their index. Returns -1 if not found.
int find_user_index(const char* user_name) {
    for (int i = 0; i < num_users; ++i) {
        if (strcmp(all_users[i].name, user_name) == 0) {
            return i;
        }
    }
    return -1;
}

// Finds a role by name and returns its index. Returns -1 if not found.
int find_role_index(const char* role_name) {
    for (int i = 0; i < num_roles; ++i) {
        if (strcmp(all_roles[i].name, role_name) == 0) {
            return i;
        }
    }
    return -1;
}

// --- Core API Functions ---

bool add_role(const char* role_name) {
    if (role_name == NULL || role_name[0] == '\0') return false;
    if (num_roles >= MAX_ROLES) return false; // Out of space
    if (find_role_index(role_name) != -1) return false; // Already exists

    secure_strcpy(all_roles[num_roles].name, role_name, MAX_NAME_LEN);
    all_roles[num_roles].num_permissions = 0;
    num_roles++;
    return true;
}

bool add_user(const char* user_name) {
    if (user_name == NULL || user_name[0] == '\0') return false;
    if (num_users >= MAX_USERS) return false; // Out of space
    if (find_user_index(user_name) != -1) return false; // Already exists

    secure_strcpy(all_users[num_users].name, user_name, MAX_NAME_LEN);
    all_users[num_users].num_roles = 0;
    num_users++;
    return true;
}

bool add_permission_to_role(const char* role_name, const char* permission) {
    if (role_name == NULL || permission == NULL || role_name[0] == '\0' || permission[0] == '\0') return false;
    
    int role_idx = find_role_index(role_name);
    if (role_idx == -1) return false; // Role not found

    Role* role = &all_roles[role_idx];
    if (role->num_permissions >= MAX_PERMS_PER_ROLE) return false; // Out of space

    // Check if permission already exists for this role
    for (int i = 0; i < role->num_permissions; ++i) {
        if (strcmp(role->permissions[i], permission) == 0) return true; // Already exists, considered success
    }

    secure_strcpy(role->permissions[role->num_permissions], permission, MAX_NAME_LEN);
    role->num_permissions++;
    return true;
}

bool assign_role_to_user(const char* user_name, const char* role_name) {
    if (user_name == NULL || role_name == NULL || user_name[0] == '\0' || role_name[0] == '\0') return false;

    int user_idx = find_user_index(user_name);
    int role_idx = find_role_index(role_name);
    if (user_idx == -1 || role_idx == -1) return false; // User or role not found

    User* user = &all_users[user_idx];
    if (user->num_roles >= MAX_ROLES_PER_USER) return false; // Out of space

    // Check if role is already assigned
    for (int i = 0; i < user->num_roles; ++i) {
        if (user->role_indices[i] == role_idx) return true; // Already assigned, considered success
    }

    user->role_indices[user->num_roles] = role_idx;
    user->num_roles++;
    return true;
}

bool check_permission(const char* user_name, const char* permission) {
    if (user_name == NULL || permission == NULL || user_name[0] == '\0' || permission[0] == '\0') return false;

    int user_idx = find_user_index(user_name);
    if (user_idx == -1) return false; // User not found

    User* user = &all_users[user_idx];
    for (int i = 0; i < user->num_roles; ++i) {
        int role_idx = user->role_indices[i];
        Role* role = &all_roles[role_idx];
        for (int j = 0; j < role->num_permissions; ++j) {
            if (strcmp(role->permissions[j], permission) == 0) {
                return true;
            }
        }
    }
    return false;
}

// --- Main Function with Test Cases ---

int main() {
    // Setup
    add_role("admin");
    add_role("editor");
    add_role("viewer");

    add_permission_to_role("admin", "read");
    add_permission_to_role("admin", "write");
    add_permission_to_role("admin", "delete");
    add_permission_to_role("admin", "manage_users");

    add_permission_to_role("editor", "read");
    add_permission_to_role("editor", "write");

    add_permission_to_role("viewer", "read");

    add_user("alice");
    add_user("bob");
    add_user("charlie");

    assign_role_to_user("alice", "admin");
    assign_role_to_user("bob", "editor");
    assign_role_to_user("charlie", "viewer");

    printf("--- Running Test Cases ---\n");

    // Test Case 1: Admin has delete permission
    printf("1. Alice has 'delete' permission: %s\n", check_permission("alice", "delete") ? "true" : "false");

    // Test Case 2: Editor has write permission
    printf("2. Bob has 'write' permission: %s\n", check_permission("bob", "write") ? "true" : "false");

    // Test Case 3: Editor does NOT have delete permission
    printf("3. Bob has 'delete' permission: %s\n", check_permission("bob", "delete") ? "true" : "false");

    // Test Case 4: Non-existent user
    printf("4. Dave has 'read' permission: %s\n", check_permission("dave", "read") ? "true" : "false");
    
    // Test Case 5: User with multiple roles
    assign_role_to_user("bob", "viewer"); // bob is now editor and viewer
    printf("5. Bob (now editor & viewer) has 'read' permission: %s\n", check_permission("bob", "read") ? "true" : "false");
    
    printf("--- Test Cases Finished ---\n");

    return 0;
}