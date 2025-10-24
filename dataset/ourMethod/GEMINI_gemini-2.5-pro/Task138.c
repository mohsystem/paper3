#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// --- Data Structures ---

typedef struct {
    char* name;
    char** permissions;
    size_t num_permissions;
} Role;

typedef struct {
    char* username;
    Role** roles;
    size_t num_roles;
    size_t capacity_roles;
} User;

typedef struct {
    User** users;
    size_t num_users;
    size_t capacity_users;
    Role** roles;
    size_t num_roles;
    size_t capacity_roles;
} PermissionManager;

// --- Helper Functions ---

// strdup is not standard C, so provide a safe implementation
char* safe_strdup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char* new_s = (char*)malloc(len);
    if (new_s) {
        memcpy(new_s, s, len);
    }
    return new_s;
}

// Find functions to avoid code duplication
User* find_user(const PermissionManager* pm, const char* username) {
    if (!pm || !username) return NULL;
    for (size_t i = 0; i < pm->num_users; ++i) {
        if (strcmp(pm->users[i]->username, username) == 0) {
            return pm->users[i];
        }
    }
    return NULL;
}

Role* find_role(const PermissionManager* pm, const char* role_name) {
    if (!pm || !role_name) return NULL;
    for (size_t i = 0; i < pm->num_roles; ++i) {
        if (strcmp(pm->roles[i]->name, role_name) == 0) {
            return pm->roles[i];
        }
    }
    return NULL;
}

// --- Manager Lifecycle ---

void permission_manager_init(PermissionManager* pm) {
    if (!pm) return;
    pm->users = NULL;
    pm->num_users = 0;
    pm->capacity_users = 0;
    pm->roles = NULL;
    pm->num_roles = 0;
    pm->capacity_roles = 0;
}

void permission_manager_destroy(PermissionManager* pm) {
    if (!pm) return;
    for (size_t i = 0; i < pm->num_users; ++i) {
        free(pm->users[i]->username);
        free(pm->users[i]->roles);
        free(pm->users[i]);
    }
    free(pm->users);

    for (size_t i = 0; i < pm->num_roles; ++i) {
        for (size_t j = 0; j < pm->roles[i]->num_permissions; ++j) {
            free(pm->roles[i]->permissions[j]);
        }
        free(pm->roles[i]->permissions);
        free(pm->roles[i]->name);
        free(pm->roles[i]);
    }
    free(pm->roles);
    
    permission_manager_init(pm);
}

// --- Core Functionality ---

bool add_user(PermissionManager* pm, const char* username) {
    if (!pm || !username || strlen(username) == 0) {
        fprintf(stderr, "Error: Invalid input to add_user.\n");
        return false;
    }
    if (find_user(pm, username) != NULL) {
        fprintf(stderr, "Error: User '%s' already exists.\n", username);
        return false;
    }

    if (pm->num_users >= pm->capacity_users) {
        size_t new_capacity = (pm->capacity_users == 0) ? 4 : pm->capacity_users * 2;
        User** new_users = (User**)realloc(pm->users, new_capacity * sizeof(User*));
        if (!new_users) return false;
        pm->users = new_users;
        pm->capacity_users = new_capacity;
    }

    User* new_user = (User*)malloc(sizeof(User));
    if (!new_user) return false;
    
    new_user->username = safe_strdup(username);
    if (!new_user->username) {
        free(new_user);
        return false;
    }
    
    new_user->roles = NULL;
    new_user->num_roles = 0;
    new_user->capacity_roles = 0;
    
    pm->users[pm->num_users++] = new_user;
    return true;
}

bool add_role(PermissionManager* pm, const char* role_name, const char** permissions, size_t num_permissions) {
    if (!pm || !role_name || strlen(role_name) == 0 || !permissions || num_permissions == 0) {
         fprintf(stderr, "Error: Invalid input to add_role.\n");
        return false;
    }
    if (find_role(pm, role_name) != NULL) {
        fprintf(stderr, "Error: Role '%s' already exists.\n", role_name);
        return false;
    }

    if (pm->num_roles >= pm->capacity_roles) {
        size_t new_capacity = (pm->capacity_roles == 0) ? 4 : pm->capacity_roles * 2;
        Role** new_roles = (Role**)realloc(pm->roles, new_capacity * sizeof(Role*));
        if (!new_roles) return false;
        pm->roles = new_roles;
        pm->capacity_roles = new_capacity;
    }

    Role* new_role = (Role*)malloc(sizeof(Role));
    if (!new_role) return false;

    new_role->name = safe_strdup(role_name);
    if (!new_role->name) { free(new_role); return false; }
    
    new_role->permissions = (char**)malloc(num_permissions * sizeof(char*));
    if (!new_role->permissions) { free(new_role->name); free(new_role); return false; }
    
    new_role->num_permissions = 0;
    for (size_t i = 0; i < num_permissions; ++i) {
        new_role->permissions[i] = safe_strdup(permissions[i]);
        if (!new_role->permissions[i]) {
            for(size_t j=0; j<i; ++j) free(new_role->permissions[j]);
            free(new_role->permissions);
            free(new_role->name);
            free(new_role);
            return false;
        }
        new_role->num_permissions++;
    }
    
    pm->roles[pm->num_roles++] = new_role;
    return true;
}

bool assign_role_to_user(PermissionManager* pm, const char* username, const char* role_name) {
    User* user = find_user(pm, username);
    if (!user) {
        fprintf(stderr, "Error: User '%s' not found.\n", username ? username : "NULL");
        return false;
    }
    Role* role = find_role(pm, role_name);
    if (!role) {
        fprintf(stderr, "Error: Role '%s' not found.\n", role_name ? role_name : "NULL");
        return false;
    }

    for (size_t i = 0; i < user->num_roles; ++i) {
        if (user->roles[i] == role) return true;
    }

    if (user->num_roles >= user->capacity_roles) {
        size_t new_capacity = (user->capacity_roles == 0) ? 2 : user->capacity_roles * 2;
        Role** new_roles = (Role**)realloc(user->roles, new_capacity * sizeof(Role*));
        if (!new_roles) return false;
        user->roles = new_roles;
        user->capacity_roles = new_capacity;
    }
    
    user->roles[user->num_roles++] = role;
    return true;
}

bool check_permission(const PermissionManager* pm, const char* username, const char* permission) {
    if (!permission || strlen(permission) == 0) return false;
    
    User* user = find_user(pm, username);
    if (!user) return false; // Fail closed

    for (size_t i = 0; i < user->num_roles; ++i) {
        Role* role = user->roles[i];
        for (size_t j = 0; j < role->num_permissions; ++j) {
            if (strcmp(role->permissions[j], permission) == 0) {
                return true;
            }
        }
    }
    return false;
}

// --- Main with Test Cases ---

void run_tests() {
    PermissionManager pm;
    permission_manager_init(&pm);

    printf("--- Test Case 1: Basic Setup and Permission Check ---\n");
    const char* admin_perms[] = {"read", "write", "delete"};
    add_role(&pm, "admin", admin_perms, 3);
    const char* editor_perms[] = {"read", "write"};
    add_role(&pm, "editor", editor_perms, 2);
    const char* viewer_perms[] = {"read"};
    add_role(&pm, "viewer", viewer_perms, 1);
    
    add_user(&pm, "alice");
    add_user(&pm, "bob");
    assign_role_to_user(&pm, "alice", "admin");
    assign_role_to_user(&pm, "bob", "editor");

    printf("Does alice have 'delete' permission? %s\n", check_permission(&pm, "alice", "delete") ? "true" : "false");
    printf("Does bob have 'delete' permission? %s\n", check_permission(&pm, "bob", "delete") ? "true" : "false");
    printf("Does bob have 'write' permission? %s\n", check_permission(&pm, "bob", "write") ? "true" : "false");
    printf("\n");

    printf("--- Test Case 2: User with multiple roles ---\n");
    add_user(&pm, "charlie");
    assign_role_to_user(&pm, "charlie", "viewer");
    assign_role_to_user(&pm, "charlie", "editor");
    printf("Does charlie have 'write' permission? %s\n", check_permission(&pm, "charlie", "write") ? "true" : "false");
    printf("Does charlie have 'read' permission? %s\n", check_permission(&pm, "charlie", "read") ? "true" : "false");
    printf("\n");

    printf("--- Test Case 3: Non-existent user, role, or permission ---\n");
    printf("Does non_existent_user have 'read' permission? %s\n", check_permission(&pm, "non_existent_user", "read") ? "true" : "false");
    printf("Does alice have 'execute' permission? %s\n", check_permission(&pm, "alice", "execute") ? "true" : "false");
    printf("Assigning non-existent role: %s\n", assign_role_to_user(&pm, "alice", "super_admin") ? "true" : "false");
    printf("\n");

    printf("--- Test Case 4: User with no roles ---\n");
    add_user(&pm, "dave");
    printf("Does dave have 'read' permission? %s\n", check_permission(&pm, "dave", "read") ? "true" : "false");
    printf("\n");

    printf("--- Test Case 5: Invalid inputs ---\n");
    printf("Adding user with NULL name: %s\n", add_user(&pm, NULL) ? "true" : "false");
    printf("Adding user with empty name: %s\n", add_user(&pm, "") ? "true" : "false");
    printf("Checking NULL permission for alice: %s\n", check_permission(&pm, "alice", NULL) ? "true" : "false");
    printf("\n");

    permission_manager_destroy(&pm);
}

int main() {
    run_tests();
    return 0;
}