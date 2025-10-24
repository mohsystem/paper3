#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_REL 512
#define MAX_STR 64

typedef struct {
    char role[MAX_STR];
    char perm[MAX_STR];
} RolePerm;

typedef struct {
    char user[MAX_STR];
    char perm[MAX_STR];
} UserPerm;

typedef struct {
    char user[MAX_STR];
    char role[MAX_STR];
} UserRole;

typedef struct {
    RolePerm rolePerms[MAX_REL];
    int rpCount;
    UserPerm userPerms[MAX_REL];
    int upCount;
    UserRole userRoles[MAX_REL];
    int urCount;
} PermissionSystem;

void ps_init(PermissionSystem* ps) {
    ps->rpCount = 0;
    ps->upCount = 0;
    ps->urCount = 0;
}

void add_user(PermissionSystem* ps, const char* user) {
    (void)ps; (void)user; /* no-op */
}

void add_role(PermissionSystem* ps, const char* role) {
    (void)ps; (void)role; /* no-op */
}

static int eq_str(const char* a, const char* b) {
    return strcmp(a, b) == 0;
}

void grant_role_to_user(PermissionSystem* ps, const char* user, const char* role) {
    for (int i = 0; i < ps->urCount; ++i) {
        if (eq_str(ps->userRoles[i].user, user) && eq_str(ps->userRoles[i].role, role)) return;
    }
    if (ps->urCount < MAX_REL) {
        strncpy(ps->userRoles[ps->urCount].user, user, MAX_STR - 1);
        ps->userRoles[ps->urCount].user[MAX_STR - 1] = '\0';
        strncpy(ps->userRoles[ps->urCount].role, role, MAX_STR - 1);
        ps->userRoles[ps->urCount].role[MAX_STR - 1] = '\0';
        ps->urCount++;
    }
}

void revoke_role_from_user(PermissionSystem* ps, const char* user, const char* role) {
    for (int i = 0; i < ps->urCount; ++i) {
        if (eq_str(ps->userRoles[i].user, user) && eq_str(ps->userRoles[i].role, role)) {
            for (int j = i + 1; j < ps->urCount; ++j) ps->userRoles[j - 1] = ps->userRoles[j];
            ps->urCount--;
            return;
        }
    }
}

void grant_permission_to_role(PermissionSystem* ps, const char* role, const char* perm) {
    for (int i = 0; i < ps->rpCount; ++i) {
        if (eq_str(ps->rolePerms[i].role, role) && eq_str(ps->rolePerms[i].perm, perm)) return;
    }
    if (ps->rpCount < MAX_REL) {
        strncpy(ps->rolePerms[ps->rpCount].role, role, MAX_STR - 1);
        ps->rolePerms[ps->rpCount].role[MAX_STR - 1] = '\0';
        strncpy(ps->rolePerms[ps->rpCount].perm, perm, MAX_STR - 1);
        ps->rolePerms[ps->rpCount].perm[MAX_STR - 1] = '\0';
        ps->rpCount++;
    }
}

void revoke_permission_from_role(PermissionSystem* ps, const char* role, const char* perm) {
    for (int i = 0; i < ps->rpCount; ++i) {
        if (eq_str(ps->rolePerms[i].role, role) && eq_str(ps->rolePerms[i].perm, perm)) {
            for (int j = i + 1; j < ps->rpCount; ++j) ps->rolePerms[j - 1] = ps->rolePerms[j];
            ps->rpCount--;
            return;
        }
    }
}

void grant_permission_to_user(PermissionSystem* ps, const char* user, const char* perm) {
    for (int i = 0; i < ps->upCount; ++i) {
        if (eq_str(ps->userPerms[i].user, user) && eq_str(ps->userPerms[i].perm, perm)) return;
    }
    if (ps->upCount < MAX_REL) {
        strncpy(ps->userPerms[ps->upCount].user, user, MAX_STR - 1);
        ps->userPerms[ps->upCount].user[MAX_STR - 1] = '\0';
        strncpy(ps->userPerms[ps->upCount].perm, perm, MAX_STR - 1);
        ps->userPerms[ps->upCount].perm[MAX_STR - 1] = '\0';
        ps->upCount++;
    }
}

void revoke_permission_from_user(PermissionSystem* ps, const char* user, const char* perm) {
    for (int i = 0; i < ps->upCount; ++i) {
        if (eq_str(ps->userPerms[i].user, user) && eq_str(ps->userPerms[i].perm, perm)) {
            for (int j = i + 1; j < ps->upCount; ++j) ps->userPerms[j - 1] = ps->userPerms[j];
            ps->upCount--;
            return;
        }
    }
}

int has_permission(PermissionSystem* ps, const char* user, const char* perm) {
    for (int i = 0; i < ps->upCount; ++i) {
        if (eq_str(ps->userPerms[i].user, user) && eq_str(ps->userPerms[i].perm, perm)) return 1;
    }
    for (int i = 0; i < ps->urCount; ++i) {
        if (eq_str(ps->userRoles[i].user, user)) {
            const char* role = ps->userRoles[i].role;
            for (int j = 0; j < ps->rpCount; ++j) {
                if (eq_str(ps->rolePerms[j].role, role) && eq_str(ps->rolePerms[j].perm, perm)) return 1;
            }
        }
    }
    return 0;
}

static int contains_str(char out[][MAX_STR], int count, const char* s) {
    for (int i = 0; i < count; ++i) if (eq_str(out[i], s)) return 1;
    return 0;
}

static int cmp_str_qsort(const void* a, const void* b) {
    const char* sa = *(const char* const*)a;
    const char* sb = *(const char* const*)b;
    return strcmp(sa, sb);
}

/* Fills out with permissions for user; returns count. max_out is capacity. */
int get_user_permissions(PermissionSystem* ps, const char* user, char out[][MAX_STR], int max_out) {
    int count = 0;

    for (int i = 0; i < ps->upCount && count < max_out; ++i) {
        if (eq_str(ps->userPerms[i].user, user)) {
            if (!contains_str(out, count, ps->userPerms[i].perm)) {
                strncpy(out[count], ps->userPerms[i].perm, MAX_STR - 1);
                out[count][MAX_STR - 1] = '\0';
                count++;
            }
        }
    }

    for (int i = 0; i < ps->urCount; ++i) {
        if (eq_str(ps->userRoles[i].user, user)) {
            const char* role = ps->userRoles[i].role;
            for (int j = 0; j < ps->rpCount && count < max_out; ++j) {
                if (eq_str(ps->rolePerms[j].role, role)) {
                    const char* perm = ps->rolePerms[j].perm;
                    if (!contains_str(out, count, perm)) {
                        strncpy(out[count], perm, MAX_STR - 1);
                        out[count][MAX_STR - 1] = '\0';
                        count++;
                    }
                }
            }
        }
    }

    // Sort permissions
    if (count > 1) {
        // Prepare array of pointers for qsort
        char* ptrs[MAX_REL];
        for (int i = 0; i < count; ++i) ptrs[i] = out[i];
        qsort(ptrs, count, sizeof(char*), cmp_str_qsort);
        // Reorder out according to sorted ptrs
        char tmp[MAX_REL][MAX_STR];
        for (int i = 0; i < count; ++i) strncpy(tmp[i], ptrs[i], MAX_STR);
        for (int i = 0; i < count; ++i) strncpy(out[i], tmp[i], MAX_STR);
    }

    return count;
}

static const char* bool_str(int v) { return v ? "true" : "false"; }

int main() {
    PermissionSystem ps;
    ps_init(&ps);

    // Test 1
    add_role(&ps, "admin");
    grant_permission_to_role(&ps, "admin", "read");
    grant_permission_to_role(&ps, "admin", "write");
    grant_permission_to_role(&ps, "admin", "delete");
    add_user(&ps, "alice");
    grant_role_to_user(&ps, "alice", "admin");
    printf("Test1 alice read: %s\n", bool_str(has_permission(&ps, "alice", "read")));
    printf("Test1 alice write: %s\n", bool_str(has_permission(&ps, "alice", "write")));
    printf("Test1 alice delete: %s\n", bool_str(has_permission(&ps, "alice", "delete")));

    // Test 2
    add_user(&ps, "bob");
    grant_permission_to_user(&ps, "bob", "read");
    printf("Test2 bob read: %s\n", bool_str(has_permission(&ps, "bob", "read")));
    printf("Test2 bob write: %s\n", bool_str(has_permission(&ps, "bob", "write")));

    // Test 3
    add_role(&ps, "editor");
    grant_permission_to_role(&ps, "editor", "read");
    grant_permission_to_role(&ps, "editor", "write");
    add_role(&ps, "viewer");
    grant_permission_to_role(&ps, "viewer", "read");
    add_user(&ps, "charlie");
    grant_role_to_user(&ps, "charlie", "editor");
    grant_role_to_user(&ps, "charlie", "viewer");
    printf("Test3 charlie read: %s\n", bool_str(has_permission(&ps, "charlie", "read")));
    printf("Test3 charlie write: %s\n", bool_str(has_permission(&ps, "charlie", "write")));

    // Test 4
    grant_role_to_user(&ps, "bob", "viewer");
    revoke_permission_from_user(&ps, "bob", "read");
    printf("Test4 bob read via role: %s\n", bool_str(has_permission(&ps, "bob", "read")));
    revoke_role_from_user(&ps, "bob", "viewer");
    printf("Test4 bob read after revoke role: %s\n", bool_str(has_permission(&ps, "bob", "read")));

    // Test 5
    add_user(&ps, "dana");
    grant_role_to_user(&ps, "dana", "editor");
    grant_permission_to_user(&ps, "dana", "export");
    char perms[64][MAX_STR];
    int cnt = get_user_permissions(&ps, "dana", perms, 64);
    printf("Test5 dana perms: [");
    for (int i = 0; i < cnt; ++i) {
        printf("%s", perms[i]);
        if (i + 1 < cnt) printf(", ");
    }
    printf("]\n");

    return 0;
}