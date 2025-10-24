#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_USERS 100
#define MAX_NAME_LEN 64

typedef enum {
    PERM_READ   = 1u << 0,
    PERM_WRITE  = 1u << 1,
    PERM_DELETE = 1u << 2,
    PERM_EXEC   = 1u << 3,
    PERM_ADMIN  = 1u << 4
} PermissionMask;

typedef enum {
    ROLE_GUEST        = 1u << 0,
    ROLE_USER         = 1u << 1,
    ROLE_MODERATOR    = 1u << 2,
    ROLE_ADMINISTRATOR= 1u << 3
} RoleMask;

typedef struct {
    char username[MAX_NAME_LEN + 1];
    unsigned int rolesMask;        // bitmask of roles
    unsigned int directPermsMask;  // bitmask of direct permissions
    int in_use;
} User;

typedef struct {
    User users[MAX_USERS];
} PermissionManager;

static void sanitize_name(const char* input, char* out, size_t out_size) {
    if (!input || !out || out_size == 0) return;
    size_t out_idx = 0;
    // trim
    const char* start = input;
    const char* end = input + strlen(input);
    while (start < end && isspace((unsigned char)*start)) start++;
    while (end > start && isspace((unsigned char)*(end - 1))) end--;
    for (const char* p = start; p < end && out_idx + 1 < out_size && out_idx < MAX_NAME_LEN; ++p) {
        unsigned char c = (unsigned char)*p;
        if (isalnum(c) || c == '_' || c == '.' || c == '-') {
            out[out_idx++] = (char)c;
        }
    }
    out[out_idx] = '\0';
}

static unsigned int role_permissions(RoleMask role) {
    switch (role) {
        case ROLE_GUEST: return PERM_READ;
        case ROLE_USER: return PERM_READ | PERM_WRITE;
        case ROLE_MODERATOR: return PERM_READ | PERM_WRITE | PERM_DELETE;
        case ROLE_ADMINISTRATOR: return PERM_READ | PERM_WRITE | PERM_DELETE | PERM_EXEC | PERM_ADMIN;
        default: return 0;
    }
}

static void pm_init(PermissionManager* pm) {
    if (!pm) return;
    memset(pm, 0, sizeof(*pm));
}

static int pm_find_user_index(PermissionManager* pm, const char* rawUsername) {
    if (!pm || !rawUsername) return -1;
    char uname[MAX_NAME_LEN + 1];
    sanitize_name(rawUsername, uname, sizeof(uname));
    if (uname[0] == '\0') return -1;
    for (int i = 0; i < MAX_USERS; ++i) {
        if (pm->users[i].in_use && strcmp(pm->users[i].username, uname) == 0) {
            return i;
        }
    }
    return -1;
}

static int pm_create_user(PermissionManager* pm, const char* rawUsername) {
    if (!pm || !rawUsername) return 0;
    char uname[MAX_NAME_LEN + 1];
    sanitize_name(rawUsername, uname, sizeof(uname));
    if (uname[0] == '\0') return 0;
    if (pm_find_user_index(pm, uname) >= 0) return 0;
    for (int i = 0; i < MAX_USERS; ++i) {
        if (!pm->users[i].in_use) {
            memset(&pm->users[i], 0, sizeof(User));
            strncpy(pm->users[i].username, uname, MAX_NAME_LEN);
            pm->users[i].username[MAX_NAME_LEN] = '\0';
            pm->users[i].in_use = 1;
            return 1;
        }
    }
    return 0;
}

static int pm_delete_user(PermissionManager* pm, const char* rawUsername) {
    int idx = pm_find_user_index(pm, rawUsername);
    if (idx < 0) return 0;
    pm->users[idx].in_use = 0;
    memset(&pm->users[idx], 0, sizeof(User));
    return 1;
}

static int pm_assign_role(PermissionManager* pm, const char* rawUsername, RoleMask role) {
    int idx = pm_find_user_index(pm, rawUsername);
    if (idx < 0) return 0;
    if (pm->users[idx].rolesMask & role) return 0;
    pm->users[idx].rolesMask |= role;
    return 1;
}

static int pm_revoke_role(PermissionManager* pm, const char* rawUsername, RoleMask role) {
    int idx = pm_find_user_index(pm, rawUsername);
    if (idx < 0) return 0;
    if (!(pm->users[idx].rolesMask & role)) return 0;
    pm->users[idx].rolesMask &= ~role;
    return 1;
}

static int pm_grant_permission(PermissionManager* pm, const char* rawUsername, unsigned int permMask) {
    int idx = pm_find_user_index(pm, rawUsername);
    if (idx < 0) return 0;
    if (pm->users[idx].directPermsMask & permMask) return 0;
    pm->users[idx].directPermsMask |= permMask;
    return 1;
}

static int pm_revoke_permission(PermissionManager* pm, const char* rawUsername, unsigned int permMask) {
    int idx = pm_find_user_index(pm, rawUsername);
    if (idx < 0) return 0;
    if (!(pm->users[idx].directPermsMask & permMask)) return 0;
    pm->users[idx].directPermsMask &= ~permMask;
    return 1;
}

static int pm_effective_permissions(PermissionManager* pm, const char* rawUsername, unsigned int* outMask) {
    if (!outMask) return 0;
    int idx = pm_find_user_index(pm, rawUsername);
    if (idx < 0) return 0;
    unsigned int eff = pm->users[idx].directPermsMask;
    unsigned int roles = pm->users[idx].rolesMask;
    if (roles & ROLE_GUEST) eff |= role_permissions(ROLE_GUEST);
    if (roles & ROLE_USER) eff |= role_permissions(ROLE_USER);
    if (roles & ROLE_MODERATOR) eff |= role_permissions(ROLE_MODERATOR);
    if (roles & ROLE_ADMINISTRATOR) eff |= role_permissions(ROLE_ADMINISTRATOR);
    *outMask = eff;
    return 1;
}

static int pm_has_permission(PermissionManager* pm, const char* rawUsername, unsigned int permMask) {
    unsigned int eff = 0;
    if (!pm_effective_permissions(pm, rawUsername, &eff)) return 0;
    return (eff & permMask) ? 1 : 0;
}

static void print_perms(unsigned int mask) {
    int first = 1;
    if (mask & PERM_READ) { printf("%sREAD", first ? "" : " "); first = 0; }
    if (mask & PERM_WRITE) { printf("%sWRITE", first ? "" : " "); first = 0; }
    if (mask & PERM_DELETE) { printf("%sDELETE", first ? "" : " "); first = 0; }
    if (mask & PERM_EXEC) { printf("%sEXECUTE", first ? "" : " "); first = 0; }
    if (mask & PERM_ADMIN) { printf("%sADMIN", first ? "" : " "); first = 0; }
    if (first) printf("(none)");
}

int main(void) {
    PermissionManager pm;
    pm_init(&pm);

    // Test 1: Create user and assign role USER; check READ true, DELETE false
    printf("Test 1:\n");
    printf("%s\n", pm_create_user(&pm, "alice") ? "1" : "0");
    printf("%s\n", pm_assign_role(&pm, "alice", ROLE_USER) ? "1" : "0");
    printf("%s\n", pm_has_permission(&pm, "alice", PERM_READ) ? "1" : "0");
    printf("%s\n", pm_has_permission(&pm, "alice", PERM_DELETE) ? "1" : "0");

    // Test 2: Grant direct WRITE and check
    printf("Test 2:\n");
    printf("%s\n", pm_grant_permission(&pm, "alice", PERM_WRITE) ? "1" : "0");
    printf("%s\n", pm_has_permission(&pm, "alice", PERM_WRITE) ? "1" : "0");
    unsigned int eff1 = 0;
    pm_effective_permissions(&pm, "alice", &eff1);
    print_perms(eff1); printf("\n");

    // Test 3: Admin role for bob
    printf("Test 3:\n");
    printf("%s\n", pm_create_user(&pm, "bob") ? "1" : "0");
    printf("%s\n", pm_assign_role(&pm, "bob", ROLE_ADMINISTRATOR) ? "1" : "0");
    printf("%s\n", pm_has_permission(&pm, "bob", PERM_ADMIN) ? "1" : "0");
    printf("%s\n", pm_has_permission(&pm, "bob", PERM_DELETE) ? "1" : "0");
    unsigned int eff2 = 0;
    pm_effective_permissions(&pm, "bob", &eff2);
    print_perms(eff2); printf("\n");

    // Test 4: Revoke permission
    printf("Test 4:\n");
    printf("%s\n", pm_revoke_permission(&pm, "alice", PERM_WRITE) ? "1" : "0");
    printf("%s\n", pm_has_permission(&pm, "alice", PERM_WRITE) ? "1" : "0");
    unsigned int eff3 = 0;
    pm_effective_permissions(&pm, "alice", &eff3);
    print_perms(eff3); printf("\n");

    // Test 5: Sanitize and duplicate handling
    printf("Test 5:\n");
    printf("%s\n", pm_create_user(&pm, "  Eve!!  ") ? "1" : "0");
    printf("%s\n", pm_create_user(&pm, "Eve") ? "1" : "0");
    // list users
    for (int i = 0; i < MAX_USERS; ++i) {
        if (pm.users[i].in_use) {
            printf("%s ", pm.users[i].username);
        }
    }
    printf("\n");

    return 0;
}