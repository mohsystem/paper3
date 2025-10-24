#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Using bitmasks for permissions
typedef enum {
    PERM_NONE    = 0,
    PERM_READ    = 1 << 0, // 1
    PERM_WRITE   = 1 << 1, // 2
    PERM_EXECUTE = 1 << 2, // 4
    PERM_DELETE  = 1 << 3  // 8
} Permission;

// User structure
typedef struct {
    char* username;
    int permissions; // Bitmask of permissions
} User;

// Permission Manager structure with a dynamic array of users
typedef struct {
    User** users;
    int user_count;
    int capacity;
} PermissionManager;

// Function prototypes
PermissionManager* create_manager();
void destroy_manager(PermissionManager* manager);
User* find_user(PermissionManager* manager, const char* username);
void add_user(PermissionManager* manager, const char* username);
void grant_permission(PermissionManager* manager, const char* username, Permission perm);
void revoke_permission(PermissionManager* manager, const char* username, Permission perm);
int has_permission(PermissionManager* manager, const char* username, Permission perm);
void print_user_permissions(PermissionManager* manager, const char* username);

// Implementation
PermissionManager* create_manager() {
    PermissionManager* manager = (PermissionManager*)malloc(sizeof(PermissionManager));
    if (!manager) return NULL;
    manager->capacity = 10; // Initial capacity
    manager->user_count = 0;
    manager->users = (User**)malloc(manager->capacity * sizeof(User*));
    if (!manager->users) {
        free(manager);
        return NULL;
    }
    return manager;
}

void destroy_manager(PermissionManager* manager) {
    if (!manager) return;
    for (int i = 0; i < manager->user_count; i++) {
        free(manager->users[i]->username);
        free(manager->users[i]);
    }
    free(manager->users);
    free(manager);
}

User* find_user(PermissionManager* manager, const char* username) {
    for (int i = 0; i < manager->user_count; i++) {
        if (strcmp(manager->users[i]->username, username) == 0) {
            return manager->users[i];
        }
    }
    return NULL;
}

void add_user(PermissionManager* manager, const char* username) {
    if (find_user(manager, username) != NULL) {
        printf("User '%s' already exists.\n", username);
        return;
    }

    if (manager->user_count >= manager->capacity) {
        manager->capacity *= 2;
        User** new_users = (User**)realloc(manager->users, manager->capacity * sizeof(User*));
        if (!new_users) {
            printf("Error: Failed to reallocate memory for users.\n");
            return;
        }
        manager->users = new_users;
    }

    User* new_user = (User*)malloc(sizeof(User));
    if (!new_user) return;
    
    new_user->username = (char*)malloc(strlen(username) + 1);
    if(!new_user->username) {
        free(new_user);
        return;
    }
    strcpy(new_user->username, username);
    new_user->permissions = PERM_NONE;

    manager->users[manager->user_count++] = new_user;
    printf("User '%s' added.\n", username);
}

void grant_permission(PermissionManager* manager, const char* username, Permission perm) {
    User* user = find_user(manager, username);
    if (user) {
        user->permissions |= perm;
        printf("Permission granted to '%s'.\n", username);
    } else {
        printf("User '%s' not found.\n", username);
    }
}

void revoke_permission(PermissionManager* manager, const char* username, Permission perm) {
    User* user = find_user(manager, username);
    if (user) {
        user->permissions &= ~perm;
        printf("Permission revoked from '%s'.\n", username);
    } else {
        printf("User '%s' not found.\n", username);
    }
}

int has_permission(PermissionManager* manager, const char* username, Permission perm) {
    User* user = find_user(manager, username);
    if (user) {
        return (user->permissions & perm) == perm;
    }
    return 0; // False
}

void print_user_permissions(PermissionManager* manager, const char* username) {
    User* user = find_user(manager, username);
    if (user) {
        printf("User '%s' permissions: ", username);
        if (user->permissions == PERM_NONE) {
            printf("None");
        } else {
            int first = 1;
            if (user->permissions & PERM_READ) { printf("READ"); first = 0; }
            if (user->permissions & PERM_WRITE) { if(!first) printf(", "); printf("WRITE"); first = 0;}
            if (user->permissions & PERM_EXECUTE) { if(!first) printf(", "); printf("EXECUTE"); first = 0;}
            if (user->permissions & PERM_DELETE) { if(!first) printf(", "); printf("DELETE"); first = 0;}
        }
        printf("\n");
    } else {
        printf("User '%s' not found.\n", username);
    }
}


int main() {
    PermissionManager* manager = create_manager();

    printf("--- Test Case 1: Add users ---\n");
    add_user(manager, "alice");
    add_user(manager, "bob");
    printf("\n");

    printf("--- Test Case 2: Grant permissions to alice ---\n");
    grant_permission(manager, "alice", PERM_READ);
    grant_permission(manager, "alice", PERM_WRITE);
    print_user_permissions(manager, "alice");
    print_user_permissions(manager, "bob");
    printf("\n");

    printf("--- Test Case 3: Check alice's permissions ---\n");
    printf("Does alice have WRITE permission? %s\n", has_permission(manager, "alice", PERM_WRITE) ? "true" : "false");
    printf("Does alice have EXECUTE permission? %s\n", has_permission(manager, "alice", PERM_EXECUTE) ? "true" : "false");
    printf("\n");

    printf("--- Test Case 4: Revoke a permission from alice ---\n");
    revoke_permission(manager, "alice", PERM_WRITE);
    print_user_permissions(manager, "alice");
    printf("\n");
    
    printf("--- Test Case 5: Final permission checks ---\n");
    printf("Does alice have WRITE permission now? %s\n", has_permission(manager, "alice", PERM_WRITE) ? "true" : "false");
    printf("Does alice still have READ permission? %s\n", has_permission(manager, "alice", PERM_READ) ? "true" : "false");
    printf("Does bob have READ permission? %s\n", has_permission(manager, "bob", PERM_READ) ? "true" : "false");
    grant_permission(manager, "bob", PERM_EXECUTE);
    print_user_permissions(manager, "bob");
    
    destroy_manager(manager);

    return 0;
}