
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>

#define MAX_USERS 100
#define MAX_ROLES_PER_USER 10
#define MAX_PERMISSIONS_PER_USER 20
#define MAX_PERMISSIONS_PER_ROLE 10
#define MAX_STRING_LENGTH 50
#define HASH_LENGTH 65

typedef struct {
    char name[MAX_STRING_LENGTH];
    int count;
} StringSet;

typedef struct {
    char username[MAX_STRING_LENGTH];
    char passwordHash[HASH_LENGTH];
    char roles[MAX_ROLES_PER_USER][MAX_STRING_LENGTH];
    int roleCount;
    char permissions[MAX_PERMISSIONS_PER_USER][MAX_STRING_LENGTH];
    int permissionCount;
} User;

typedef struct {
    char name[MAX_STRING_LENGTH];
    char permissions[MAX_PERMISSIONS_PER_ROLE][MAX_STRING_LENGTH];
    int permissionCount;
} Role;

typedef struct {
    User users[MAX_USERS];
    int userCount;
    Role roles[10];
    int roleCount;
} PermissionManager;

void hashPassword(const char* password, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\\0';
}

void initializeDefaultRoles(PermissionManager* pm) {
    pm->roleCount = 3;
    
    strcpy(pm->roles[0].name, "ADMIN");
    pm->roles[0].permissionCount = 7;
    strcpy(pm->roles[0].permissions[0], "CREATE_USER");
    strcpy(pm->roles[0].permissions[1], "DELETE_USER");
    strcpy(pm->roles[0].permissions[2], "MODIFY_USER");
    strcpy(pm->roles[0].permissions[3], "READ_USER");
    strcpy(pm->roles[0].permissions[4], "CREATE_ROLE");
    strcpy(pm->roles[0].permissions[5], "DELETE_ROLE");
    strcpy(pm->roles[0].permissions[6], "ASSIGN_ROLE");
    
    strcpy(pm->roles[1].name, "MANAGER");
    pm->roles[1].permissionCount = 3;
    strcpy(pm->roles[1].permissions[0], "READ_USER");
    strcpy(pm->roles[1].permissions[1], "MODIFY_USER");
    strcpy(pm->roles[1].permissions[2], "ASSIGN_ROLE");
    
    strcpy(pm->roles[2].name, "USER");
    pm->roles[2].permissionCount = 1;
    strcpy(pm->roles[2].permissions[0], "READ_USER");
}

void initPermissionManager(PermissionManager* pm) {
    pm->userCount = 0;
    initializeDefaultRoles(pm);
}

int createUser(PermissionManager* pm, const char* username, const char* password) {
    for (int i = 0; i < pm->userCount; i++) {
        if (strcmp(pm->users[i].username, username) == 0) {
            return 0;
        }
    }
    
    if (pm->userCount >= MAX_USERS) {
        return 0;
    }
    
    User* user = &pm->users[pm->userCount];
    strcpy(user->username, username);
    hashPassword(password, user->passwordHash);
    user->roleCount = 0;
    user->permissionCount = 0;
    pm->userCount++;
    return 1;
}

int authenticateUser(PermissionManager* pm, const char* username, const char* password) {
    for (int i = 0; i < pm->userCount; i++) {
        if (strcmp(pm->users[i].username, username) == 0) {
            char hash[HASH_LENGTH];
            hashPassword(password, hash);
            return strcmp(pm->users[i].passwordHash, hash) == 0;
        }
    }
    return 0;
}

User* findUser(PermissionManager* pm, const char* username) {
    for (int i = 0; i < pm->userCount; i++) {
        if (strcmp(pm->users[i].username, username) == 0) {
            return &pm->users[i];
        }
    }
    return NULL;
}

Role* findRole(PermissionManager* pm, const char* roleName) {
    for (int i = 0; i < pm->roleCount; i++) {
        if (strcmp(pm->roles[i].name, roleName) == 0) {
            return &pm->roles[i];
        }
    }
    return NULL;
}

int hasPermission(User* user, const char* permission) {
    for (int i = 0; i < user->permissionCount; i++) {
        if (strcmp(user->permissions[i], permission) == 0) {
            return 1;
        }
    }
    return 0;
}

int assignRole(PermissionManager* pm, const char* username, const char* roleName) {
    User* user = findUser(pm, username);
    Role* role = findRole(pm, roleName);
    
    if (!user || !role || user->roleCount >= MAX_ROLES_PER_USER) {
        return 0;
    }
    
    strcpy(user->roles[user->roleCount], roleName);
    user->roleCount++;
    
    for (int i = 0; i < role->permissionCount; i++) {
        if (!hasPermission(user, role->permissions[i]) && 
            user->permissionCount < MAX_PERMISSIONS_PER_USER) {
            strcpy(user->permissions[user->permissionCount], role->permissions[i]);
            user->permissionCount++;
        }
    }
    return 1;
}

int checkPermission(PermissionManager* pm, const char* username, const char* permission) {
    User* user = findUser(pm, username);
    if (!user) return 0;
    return hasPermission(user, permission);
}

void printUserRoles(PermissionManager* pm, const char* username) {
    User* user = findUser(pm, username);
    if (!user) {
        printf("{ }\\n");
        return;
    }
    printf("{ ");
    for (int i = 0; i < user->roleCount; i++) {
        printf("%s ", user->roles[i]);
    }
    printf("}\\n");
}

void printUserPermissions(PermissionManager* pm, const char* username) {
    User* user = findUser(pm, username);
    if (!user) {
        printf("{ }\\n");
        return;
    }
    printf("{ ");
    for (int i = 0; i < user->permissionCount; i++) {
        printf("%s ", user->permissions[i]);
    }
    printf("}\\n");
}

int main() {
    PermissionManager pm;
    initPermissionManager(&pm);
    
    printf("Test Case 1: User Creation and Authentication\\n");
    createUser(&pm, "alice", "password123");
    createUser(&pm, "bob", "securepass");
    printf("Alice authentication: %s\\n", authenticateUser(&pm, "alice", "password123") ? "true" : "false");
    printf("Bob wrong password: %s\\n", authenticateUser(&pm, "bob", "wrongpass") ? "true" : "false");
    printf("\\n");
    
    printf("Test Case 2: Role Assignment\\n");
    assignRole(&pm, "alice", "ADMIN");
    assignRole(&pm, "bob", "USER");
    printf("Alice roles: ");
    printUserRoles(&pm, "alice");
    printf("Bob roles: ");
    printUserRoles(&pm, "bob");
    printf("\\n");
    
    printf("Test Case 3: Permission Checking\\n");
    printf("Alice has CREATE_USER: %s\\n", checkPermission(&pm, "alice", "CREATE_USER") ? "true" : "false");
    printf("Bob has CREATE_USER: %s\\n", checkPermission(&pm, "bob", "CREATE_USER") ? "true" : "false");
    printf("Bob has READ_USER: %s\\n", checkPermission(&pm, "bob", "READ_USER") ? "true" : "false");
    printf("\\n");
    
    printf("Test Case 4: Multiple Users\\n");
    createUser(&pm, "charlie", "charlie123");
    assignRole(&pm, "charlie", "MANAGER");
    printf("Charlie roles: ");
    printUserRoles(&pm, "charlie");
    printf("Charlie permissions: ");
    printUserPermissions(&pm, "charlie");
    printf("\\n");
    
    printf("Test Case 5: Permission Verification\\n");
    printf("Charlie has MODIFY_USER: %s\\n", checkPermission(&pm, "charlie", "MODIFY_USER") ? "true" : "false");
    printf("Charlie has DELETE_USER: %s\\n", checkPermission(&pm, "charlie", "DELETE_USER") ? "true" : "false");
    
    return 0;
}
