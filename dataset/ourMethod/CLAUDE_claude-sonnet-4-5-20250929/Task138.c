
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/* Secure user permission management system in C */
/* All inputs validated, bounds-checked, memory-safe */

#define MAX_USERNAME_LEN 32
#define MIN_USERNAME_LEN 3
#define MAX_RESOURCE_LEN 128
#define MAX_PERMISSIONS 100
#define MAX_USERS 1000

typedef enum {
    PERM_READ = 1,
    PERM_WRITE = 2,
    PERM_EXECUTE = 4,
    PERM_DELETE = 8,
    PERM_ADMIN = 16
} Permission;

/* Resource permission entry */
typedef struct {
    char resource[MAX_RESOURCE_LEN + 1];
    int permissions; /* Bitmask of permissions */
} ResourcePermission;

/* User structure */
typedef struct {
    char username[MAX_USERNAME_LEN + 1];
    ResourcePermission* resources;
    size_t resource_count;
    size_t resource_capacity;
} User;

/* Permission manager */
typedef struct {
    User* users;
    size_t user_count;
    size_t user_capacity;
} PermissionManager;

/* Validate username: alphanumeric and underscore only, 3-32 chars */
bool is_valid_username(const char* username) {
    if (username == NULL) {
        return false;
    }
    
    size_t len = strnlen(username, MAX_USERNAME_LEN + 1);
    if (len < MIN_USERNAME_LEN || len > MAX_USERNAME_LEN) {
        return false;
    }
    
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)username[i];
        if (!isalnum(c) && c != '_') {
            return false;
        }
    }
    return true;
}

/* Validate resource name */
bool is_valid_resource(const char* resource) {
    if (resource == NULL) {
        return false;
    }
    
    size_t len = strnlen(resource, MAX_RESOURCE_LEN + 1);
    if (len == 0 || len > MAX_RESOURCE_LEN) {
        return false;
    }
    
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)resource[i];
        if (!isalnum(c) && c != '_' && c != '-' && c != '/' && c != '.') {
            return false;
        }
    }
    return true;
}

/* Convert permission to string */
const char* permission_to_string(Permission perm) {
    switch (perm) {
        case PERM_READ: return "READ";
        case PERM_WRITE: return "WRITE";
        case PERM_EXECUTE: return "EXECUTE";
        case PERM_DELETE: return "DELETE";
        case PERM_ADMIN: return "ADMIN";
        default: return "UNKNOWN";
    }
}

/* Initialize permission manager */
PermissionManager* pm_create(void) {
    PermissionManager* pm = (PermissionManager*)calloc(1, sizeof(PermissionManager));
    if (pm == NULL) {
        return NULL;
    }
    
    pm->user_capacity = 10;
    pm->users = (User*)calloc(pm->user_capacity, sizeof(User));
    if (pm->users == NULL) {
        free(pm);
        return NULL;
    }
    
    pm->user_count = 0;
    return pm;
}

/* Free user resources */
void user_free(User* user) {
    if (user != NULL && user->resources != NULL) {
        free(user->resources);
        user->resources = NULL;
        user->resource_count = 0;
        user->resource_capacity = 0;
    }
}

/* Free permission manager */
void pm_destroy(PermissionManager* pm) {
    if (pm == NULL) {
        return;
    }
    
    if (pm->users != NULL) {
        for (size_t i = 0; i < pm->user_count; i++) {
            user_free(&pm->users[i]);
        }
        free(pm->users);
        pm->users = NULL;
    }
    
    free(pm);
}

/* Find user by username */
User* pm_find_user(PermissionManager* pm, const char* username) {
    if (pm == NULL || username == NULL) {
        return NULL;
    }
    
    for (size_t i = 0; i < pm->user_count; i++) {
        if (strncmp(pm->users[i].username, username, MAX_USERNAME_LEN) == 0) {
            return &pm->users[i];
        }
    }
    return NULL;
}

/* Create a new user */
bool pm_create_user(PermissionManager* pm, const char* username) {
    if (pm == NULL || !is_valid_username(username)) {
        return false;
    }
    
    /* Check maximum users limit */
    if (pm->user_count >= MAX_USERS) {
        return false;
    }
    
    /* Check if user already exists */
    if (pm_find_user(pm, username) != NULL) {
        return false;
    }
    
    /* Expand array if needed */
    if (pm->user_count >= pm->user_capacity) {
        size_t new_capacity = pm->user_capacity * 2;
        if (new_capacity > MAX_USERS) {
            new_capacity = MAX_USERS;
        }
        
        User* new_users = (User*)realloc(pm->users, new_capacity * sizeof(User));
        if (new_users == NULL) {
            return false;
        }
        
        pm->users = new_users;
        pm->user_capacity = new_capacity;
    }
    
    /* Initialize new user */
    User* user = &pm->users[pm->user_count];
    memset(user, 0, sizeof(User));
    strncpy(user->username, username, MAX_USERNAME_LEN);
    user->username[MAX_USERNAME_LEN] = '\\0';
    
    user->resource_capacity = 10;
    user->resources = (ResourcePermission*)calloc(user->resource_capacity, 
                                                   sizeof(ResourcePermission));
    if (user->resources == NULL) {
        return false;
    }
    
    pm->user_count++;
    return true;
}

/* Delete a user */
bool pm_delete_user(PermissionManager* pm, const char* username) {
    if (pm == NULL || !is_valid_username(username)) {
        return false;
    }
    
    for (size_t i = 0; i < pm->user_count; i++) {
        if (strncmp(pm->users[i].username, username, MAX_USERNAME_LEN) == 0) {
            user_free(&pm->users[i]);
            
            /* Move last user to this position */
            if (i < pm->user_count - 1) {
                pm->users[i] = pm->users[pm->user_count - 1];
            }
            pm->user_count--;
            return true;
        }
    }
    return false;
}

/* Find resource permission entry for user */
ResourcePermission* user_find_resource(User* user, const char* resource) {
    if (user == NULL || resource == NULL) {
        return NULL;
    }
    
    for (size_t i = 0; i < user->resource_count; i++) {
        if (strncmp(user->resources[i].resource, resource, MAX_RESOURCE_LEN) == 0) {
            return &user->resources[i];
        }
    }
    return NULL;
}

/* Grant permission to user for resource */
bool pm_grant_permission(PermissionManager* pm, const char* username, 
                        const char* resource, Permission perm) {
    if (pm == NULL || !is_valid_username(username) || !is_valid_resource(resource)) {
        return false;
    }
    
    User* user = pm_find_user(pm, username);
    if (user == NULL) {
        return false;
    }
    
    ResourcePermission* rp = user_find_resource(user, resource);
    
    if (rp != NULL) {
        /* Resource exists, add permission */
        rp->permissions |= perm;
        return true;
    }
    
    /* Check maximum permissions limit */
    if (user->resource_count >= MAX_PERMISSIONS) {
        return false;
    }
    
    /* Expand array if needed */
    if (user->resource_count >= user->resource_capacity) {
        size_t new_capacity = user->resource_capacity * 2;
        if (new_capacity > MAX_PERMISSIONS) {
            new_capacity = MAX_PERMISSIONS;
        }
        
        ResourcePermission* new_resources = (ResourcePermission*)realloc(
            user->resources, new_capacity * sizeof(ResourcePermission));
        if (new_resources == NULL) {
            return false;
        }
        
        user->resources = new_resources;
        user->resource_capacity = new_capacity;
    }
    
    /* Add new resource permission */
    rp = &user->resources[user->resource_count];
    memset(rp, 0, sizeof(ResourcePermission));
    strncpy(rp->resource, resource, MAX_RESOURCE_LEN);
    rp->resource[MAX_RESOURCE_LEN] = '\\0';
    rp->permissions = perm;
    
    user->resource_count++;
    return true;
}

/* Revoke permission from user for resource */
bool pm_revoke_permission(PermissionManager* pm, const char* username,
                         const char* resource, Permission perm) {
    if (pm == NULL || !is_valid_username(username) || !is_valid_resource(resource)) {
        return false;
    }
    
    User* user = pm_find_user(pm, username);
    if (user == NULL) {
        return false;
    }
    
    ResourcePermission* rp = user_find_resource(user, resource);
    if (rp != NULL) {
        rp->permissions &= ~perm;
        return true;
    }
    
    return false;
}

/* Check if user has permission */
bool pm_check_permission(PermissionManager* pm, const char* username,
                        const char* resource, Permission perm) {
    if (pm == NULL || !is_valid_username(username) || !is_valid_resource(resource)) {
        return false;
    }
    
    User* user = pm_find_user(pm, username);
    if (user == NULL) {
        return false;
    }
    
    ResourcePermission* rp = user_find_resource(user, resource);
    if (rp != NULL) {
        return (rp->permissions & perm) != 0;
    }
    
    return false;
}

/* Get user's permissions for resource */\nint pm_get_permissions(PermissionManager* pm, const char* username,\n                      const char* resource) {\n    if (pm == NULL || !is_valid_username(username) || !is_valid_resource(resource)) {\n        return 0;\n    }\n    \n    User* user = pm_find_user(pm, username);\n    if (user == NULL) {\n        return 0;\n    }\n    \n    ResourcePermission* rp = user_find_resource(user, resource);\n    if (rp != NULL) {\n        return rp->permissions;\n    }\n    \n    return 0;\n}\n\nint main(void) {\n    PermissionManager* pm = pm_create();\n    if (pm == NULL) {\n        fprintf(stderr, "Failed to create permission manager\\n");\n        return 1;\n    }\n    \n    /* Test Case 1: Create users and grant basic permissions */\n    printf("Test Case 1: Create users and grant permissions\\n");\n    pm_create_user(pm, "alice");\n    pm_create_user(pm, "bob");\n    pm_grant_permission(pm, "alice", "/docs/report.txt", PERM_READ);\n    pm_grant_permission(pm, "alice", "/docs/report.txt", PERM_WRITE);\n    pm_grant_permission(pm, "bob", "/docs/report.txt", PERM_READ);\n    \n    printf("Alice has READ on /docs/report.txt: %s\\n",\n           pm_check_permission(pm, "alice", "/docs/report.txt", PERM_READ) ? "Yes" : "No");\n    printf("Bob has WRITE on /docs/report.txt: %s\\n\\n",\n           pm_check_permission(pm, "bob", "/docs/report.txt", PERM_WRITE) ? "Yes" : "No");\n    \n    /* Test Case 2: Revoke permissions */\n    printf("Test Case 2: Revoke permissions\\n");\n    pm_revoke_permission(pm, "alice", "/docs/report.txt", PERM_WRITE);\n    printf("Alice has WRITE after revoke: %s\\n",\n           pm_check_permission(pm, "alice", "/docs/report.txt", PERM_WRITE) ? "Yes" : "No");\n    printf("Alice has READ after revoke: %s\\n\\n",\n           pm_check_permission(pm, "alice", "/docs/report.txt", PERM_READ) ? "Yes" : "No");\n    \n    /* Test Case 3: Admin permissions */\n    printf("Test Case 3: Admin permissions\\n");\n    pm_create_user(pm, "admin_user");\n    pm_grant_permission(pm, "admin_user", "/system/config", PERM_ADMIN);\n    pm_grant_permission(pm, "admin_user", "/system/config", PERM_READ);\n    pm_grant_permission(pm, "admin_user", "/system/config", PERM_WRITE);\n    \n    int admin_perms = pm_get_permissions(pm, "admin_user", "/system/config");\n    printf("Admin user permissions on /system/config: ");\n    if (admin_perms & PERM_READ) printf("READ ");\n    if (admin_perms & PERM_WRITE) printf("WRITE ");\n    if (admin_perms & PERM_EXECUTE) printf("EXECUTE ");\n    if (admin_perms & PERM_DELETE) printf("DELETE ");\n    if (admin_perms & PERM_ADMIN) printf("ADMIN ");\n    printf("\\n\\n");\n    \n    /* Test Case 4: Invalid input handling */\n    printf("Test Case 4: Invalid input handling\\n");\n    bool result1 = pm_create_user(pm, "a"); /* Too short */\n    bool result2 = pm_create_user(pm, "user@invalid"); /* Invalid character */\n    char long_resource[300];\n    memset(long_resource, 'x', sizeof(long_resource) - 1);\n    long_resource[sizeof(long_resource) - 1] = '\\0';\n    bool result3 = pm_grant_permission(pm, "alice", long_resource, PERM_READ);\n    \n    printf("Create user 'a': %s\\n", result1 ? "Success" : "Failed (expected)");\n    printf("Create user 'user@invalid': %s\
", result2 ? "Success" : "Failed (expected)");\n    printf("Grant permission to 299-char resource: %s\
\
", result3 ? "Success" : "Failed (expected)");\n    \n    /* Test Case 5: Delete user */\n    printf("Test Case 5: Delete user\
");\n    printf("Users before delete: alice, bob, admin_user\
");\n    pm_delete_user(pm, "bob");\n    printf("After deleting bob, can check his permissions: %s\
",\n           pm_check_permission(pm, "bob", "/docs/report.txt", PERM_READ) ? "Yes" : "No (expected)");
    
    /* Cleanup: free all allocated memory */
    pm_destroy(pm);
    
    return 0;
}
