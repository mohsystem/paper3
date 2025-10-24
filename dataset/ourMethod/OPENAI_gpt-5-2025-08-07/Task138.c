#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_NAME_LEN 64
#define MAX_USERS 256
#define MAX_ROLES 256
#define MAX_PERMS_PER_ROLE 256
#define MAX_ROLES_PER_USER 128

typedef struct {
    char name[MAX_NAME_LEN + 1];
    char perms[MAX_PERMS_PER_ROLE][MAX_NAME_LEN + 1];
    int perm_count;
} Role;

typedef struct {
    char name[MAX_NAME_LEN + 1];
    char roles[MAX_ROLES_PER_USER][MAX_NAME_LEN + 1];
    int role_count;
} User;

typedef struct {
    Role roles[MAX_ROLES];
    int role_count;
    User users[MAX_USERS];
    int user_count;
} AccessManager;

static bool valid_name(const char* s) {
    if (s == NULL) return false;
    size_t len = strlen(s);
    if (len == 0 || len > MAX_NAME_LEN) return false;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s[i];
        if (!(isalnum(c) || c == '_' || c == '-')) return false;
    }
    return true;
}

static int find_role(const AccessManager* m, const char* roleName) {
    for (int i = 0; i < m->role_count; i++) {
        if (strncmp(m->roles[i].name, roleName, MAX_NAME_LEN) == 0) return i;
    }
    return -1;
}

static int find_user(const AccessManager* m, const char* userName) {
    for (int i = 0; i < m->user_count; i++) {
        if (strncmp(m->users[i].name, userName, MAX_NAME_LEN) == 0) return i;
    }
    return -1;
}

static bool add_string_to_set(char arr[][MAX_NAME_LEN + 1], int* count, int max, const char* s) {
    for (int i = 0; i < *count; i++) {
        if (strncmp(arr[i], s, MAX_NAME_LEN) == 0) return false;
    }
    if (*count >= max) return false;
    snprintf(arr[*count], MAX_NAME_LEN + 1, "%s", s);
    (*count)++;
    return true;
}

static bool remove_string_from_set(char arr[][MAX_NAME_LEN + 1], int* count, const char* s) {
    for (int i = 0; i < *count; i++) {
        if (strncmp(arr[i], s, MAX_NAME_LEN) == 0) {
            // shift left
            for (int j = i + 1; j < *count; j++) {
                snprintf(arr[j - 1], MAX_NAME_LEN + 1, "%s", arr[j]);
            }
            (*count)--;
            return true;
        }
    }
    return false;
}

static void init_manager(AccessManager* m) {
    if (!m) return;
    m->role_count = 0;
    m->user_count = 0;
}

bool create_role(AccessManager* m, const char* roleName) {
    if (!m || !valid_name(roleName)) return false;
    if (m->role_count >= MAX_ROLES) return false;
    if (find_role(m, roleName) >= 0) return false;
    Role* r = &m->roles[m->role_count];
    snprintf(r->name, MAX_NAME_LEN + 1, "%s", roleName);
    r->perm_count = 0;
    m->role_count++;
    return true;
}

bool remove_role(AccessManager* m, const char* roleName) {
    if (!m || !valid_name(roleName)) return false;
    int idx = find_role(m, roleName);
    if (idx < 0) return false;
    // Remove role from users
    for (int u = 0; u < m->user_count; u++) {
        remove_string_from_set(m->users[u].roles, &m->users[u].role_count, roleName);
    }
    // Remove role by shifting
    for (int i = idx + 1; i < m->role_count; i++) {
        m->roles[i - 1] = m->roles[i];
    }
    m->role_count--;
    return true;
}

bool add_permission_to_role(AccessManager* m, const char* roleName, const char* perm) {
    if (!m || !valid_name(roleName) || !valid_name(perm)) return false;
    int idx = find_role(m, roleName);
    if (idx < 0) return false;
    Role* r = &m->roles[idx];
    if (r->perm_count >= MAX_PERMS_PER_ROLE) return false;
    return add_string_to_set(r->perms, &r->perm_count, MAX_PERMS_PER_ROLE, perm);
}

bool remove_permission_from_role(AccessManager* m, const char* roleName, const char* perm) {
    if (!m || !valid_name(roleName) || !valid_name(perm)) return false;
    int idx = find_role(m, roleName);
    if (idx < 0) return false;
    Role* r = &m->roles[idx];
    return remove_string_from_set(r->perms, &r->perm_count, perm);
}

bool create_user(AccessManager* m, const char* userName) {
    if (!m || !valid_name(userName)) return false;
    if (m->user_count >= MAX_USERS) return false;
    if (find_user(m, userName) >= 0) return false;
    User* u = &m->users[m->user_count];
    snprintf(u->name, MAX_NAME_LEN + 1, "%s", userName);
    u->role_count = 0;
    m->user_count++;
    return true;
}

bool delete_user(AccessManager* m, const char* userName) {
    if (!m || !valid_name(userName)) return false;
    int idx = find_user(m, userName);
    if (idx < 0) return false;
    for (int i = idx + 1; i < m->user_count; i++) {
        m->users[i - 1] = m->users[i];
    }
    m->user_count--;
    return true;
}

bool assign_role_to_user(AccessManager* m, const char* userName, const char* roleName) {
    if (!m || !valid_name(userName) || !valid_name(roleName)) return false;
    int uidx = find_user(m, userName);
    int ridx = find_role(m, roleName);
    if (uidx < 0 || ridx < 0) return false;
    User* u = &m->users[uidx];
    if (u->role_count >= MAX_ROLES_PER_USER) return false;
    return add_string_to_set(u->roles, &u->role_count, MAX_ROLES_PER_USER, roleName);
}

bool revoke_role_from_user(AccessManager* m, const char* userName, const char* roleName) {
    if (!m || !valid_name(userName) || !valid_name(roleName)) return false;
    int uidx = find_user(m, userName);
    if (uidx < 0) return false;
    User* u = &m->users[uidx];
    return remove_string_from_set(u->roles, &u->role_count, roleName);
}

static bool role_has_permission(const Role* r, const char* perm) {
    for (int i = 0; i < r->perm_count; i++) {
        if (strncmp(r->perms[i], perm, MAX_NAME_LEN) == 0) return true;
    }
    return false;
}

bool user_has_permission(const AccessManager* m, const char* userName, const char* perm) {
    if (!m || !valid_name(userName) || !valid_name(perm)) return false;
    int uidx = find_user(m, userName);
    if (uidx < 0) return false;
    const User* u = &m->users[uidx];
    for (int i = 0; i < u->role_count; i++) {
        int ridx = find_role(m, u->roles[i]);
        if (ridx >= 0 && role_has_permission(&m->roles[ridx], perm)) {
            return true;
        }
    }
    return false;
}

// Returns a heap-allocated comma-separated permission list. Caller must free.
char* list_user_permissions(const AccessManager* m, const char* userName) {
    if (!m || !valid_name(userName)) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    int uidx = find_user(m, userName);
    if (uidx < 0) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    // Collect unique permissions in a temporary array
    const int MAX_TOTAL_PERMS = MAX_ROLES_PER_USER * MAX_PERMS_PER_ROLE;
    char (*perms)[MAX_NAME_LEN + 1] = (char (*)[MAX_NAME_LEN + 1])calloc((size_t)MAX_TOTAL_PERMS, sizeof(*perms));
    int perm_count = 0;
    if (!perms) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    const User* u = &m->users[uidx];
    for (int i = 0; i < u->role_count; i++) {
        int ridx = find_role(m, u->roles[i]);
        if (ridx < 0) continue;
        const Role* r = &m->roles[ridx];
        for (int p = 0; p < r->perm_count; p++) {
            // add unique
            bool exists = false;
            for (int k = 0; k < perm_count; k++) {
                if (strncmp(perms[k], r->perms[p], MAX_NAME_LEN) == 0) {
                    exists = true; break;
                }
            }
            if (!exists && perm_count < MAX_TOTAL_PERMS) {
                snprintf(perms[perm_count], MAX_NAME_LEN + 1, "%s", r->perms[p]);
                perm_count++;
            }
        }
    }
    // Compute buffer length
    size_t total_len = 0;
    for (int i = 0; i < perm_count; i++) {
        total_len += strlen(perms[i]);
        if (i + 1 < perm_count) total_len += 1; // comma
    }
    char* out = (char*)malloc(total_len + 1);
    if (!out) {
        free(perms);
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    size_t pos = 0;
    for (int i = 0; i < perm_count; i++) {
        size_t l = strlen(perms[i]);
        memcpy(out + pos, perms[i], l);
        pos += l;
        if (i + 1 < perm_count) {
            out[pos++] = ',';
        }
    }
    out[pos] = '\0';
    free(perms);
    return out;
}

int main(void) {
    AccessManager mgr;
    init_manager(&mgr);

    // Test 1: Create roles, permissions, users, assignments, checks
    printf("Test1-CreateRoles: %d\n", (create_role(&mgr, "admin") && create_role(&mgr, "editor")) ? 1 : 0);
    printf("Test1-AddPermsAdmin: %d\n", (add_permission_to_role(&mgr, "admin", "read_all")
           && add_permission_to_role(&mgr, "admin", "write_all")
           && add_permission_to_role(&mgr, "admin", "manage_users")) ? 1 : 0);
    printf("Test1-AddPermsEditor: %d\n", (add_permission_to_role(&mgr, "editor", "read_articles")
           && add_permission_to_role(&mgr, "editor", "edit_articles")) ? 1 : 0);
    printf("Test1-CreateUsers: %d\n", (create_user(&mgr, "alice") && create_user(&mgr, "bob")) ? 1 : 0);
    printf("Test1-AssignRoles: %d\n", (assign_role_to_user(&mgr, "alice", "admin")
           && assign_role_to_user(&mgr, "bob", "editor")) ? 1 : 0);
    printf("Test1-CheckPerms: %d\n", (user_has_permission(&mgr, "alice", "manage_users")
           && !user_has_permission(&mgr, "bob", "manage_users")) ? 1 : 0);

    // Test 2: Invalid names
    printf("Test2-InvalidRoleName: %d\n", !create_role(&mgr, "invalid role") ? 1 : 0);
    printf("Test2-InvalidUserName: %d\n", !create_user(&mgr, "") ? 1 : 0);
    printf("Test2-InvalidPermToRole: %d\n", !add_permission_to_role(&mgr, "admin", "bad perm") ? 1 : 0);

    // Test 3: Duplicates
    printf("Test3-DuplicateRole: %d\n", !create_role(&mgr, "admin") ? 1 : 0);
    printf("Test3-DuplicateUser: %d\n", !create_user(&mgr, "alice") ? 1 : 0);
    printf("Test3-DuplicatePermission: %d\n", !add_permission_to_role(&mgr, "admin", "read_all") ? 1 : 0);

    // Test 4: Revoke role
    printf("Test4-RevokeRole: %d\n", revoke_role_from_user(&mgr, "alice", "admin") ? 1 : 0);
    printf("Test4-CheckPermAfterRevoke: %d\n", !user_has_permission(&mgr, "alice", "read_all") ? 1 : 0);

    // Test 5: Remove permission and verify
    printf("Test5-AddRoleBack: %d\n", assign_role_to_user(&mgr, "alice", "admin") ? 1 : 0);
    printf("Test5-RemovePermFromRole: %d\n", remove_permission_from_role(&mgr, "admin", "write_all") ? 1 : 0);
    printf("Test5-CheckRemovedPerm: %d\n", !user_has_permission(&mgr, "alice", "write_all") ? 1 : 0);

    char* alicePerms = list_user_permissions(&mgr, "alice");
    char* bobPerms = list_user_permissions(&mgr, "bob");
    printf("AlicePerms: %s\n", alicePerms);
    printf("BobPerms: %s\n", bobPerms);
    free(alicePerms);
    free(bobPerms);

    return 0;
}