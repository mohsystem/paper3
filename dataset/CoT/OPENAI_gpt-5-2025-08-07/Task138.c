/* Minimal high-level comments: Implementation includes input validation, normalization, and bounds-checked operations. */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_USERS 100
#define MAX_ROLES 100
#define MAX_PERMS 200
#define MAX_NAME_LEN 64
#define MAX_ASSIGN_ROLES 100
#define MAX_DIRECT_PERMS 200
#define MAX_ROLE_PERMS 200

typedef struct {
    char name[MAX_NAME_LEN + 1];
    char perms[MAX_ROLE_PERMS][MAX_NAME_LEN + 1];
    int perms_count;
} Role;

typedef struct {
    char name[MAX_NAME_LEN + 1];
    char direct_perms[MAX_DIRECT_PERMS][MAX_NAME_LEN + 1];
    int direct_perms_count;
    char role_names[MAX_ASSIGN_ROLES][MAX_NAME_LEN + 1];
    int role_count;
} User;

typedef struct {
    Role roles[MAX_ROLES];
    int roles_count;
    User users[MAX_USERS];
    int users_count;
} PermissionManager;

/* Utility: safe string duplication */
static char* str_dup(const char* s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    memcpy(out, s, n + 1);
    return out;
}

/* Trim and lowercase, validate allowed characters */
static int normalize(const char* in, char* out, size_t out_size) {
    if (!in || !out || out_size == 0) return 0;
    size_t len = strlen(in);
    size_t b = 0, e = len;
    while (b < e && isspace((unsigned char)in[b])) b++;
    while (e > b && isspace((unsigned char)in[e - 1])) e--;
    size_t n = e > b ? (e - b) : 0;
    if (n == 0 || n > MAX_NAME_LEN) return 0;
    if (n >= out_size) return 0;
    for (size_t i = 0; i < n; ++i) {
        char c = (char)tolower((unsigned char)in[b + i]);
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
              c == '_' || c == '.' || c == ':' || c == '-')) {
            return 0;
        }
        out[i] = c;
    }
    out[n] = '\0';
    return 1;
}

static int role_index(PermissionManager* pm, const char* role_norm) {
    for (int i = 0; i < pm->roles_count; ++i) {
        if (strcmp(pm->roles[i].name, role_norm) == 0) return i;
    }
    return -1;
}

static int user_index(PermissionManager* pm, const char* user_norm) {
    for (int i = 0; i < pm->users_count; ++i) {
        if (strcmp(pm->users[i].name, user_norm) == 0) return i;
    }
    return -1;
}

static int contains_str(char arr[][MAX_NAME_LEN + 1], int count, const char* s) {
    for (int i = 0; i < count; ++i) if (strcmp(arr[i], s) == 0) return 1;
    return 0;
}

static int add_str(char arr[][MAX_NAME_LEN + 1], int* count, int max_count, const char* s) {
    if (*count >= max_count) return 0;
    if (contains_str(arr, *count, s)) return 0;
    strncpy(arr[*count], s, MAX_NAME_LEN);
    arr[*count][MAX_NAME_LEN] = '\0';
    (*count)++;
    return 1;
}

static int remove_str(char arr[][MAX_NAME_LEN + 1], int* count, const char* s) {
    for (int i = 0; i < *count; ++i) {
        if (strcmp(arr[i], s) == 0) {
            for (int j = i + 1; j < *count; ++j) strcpy(arr[j - 1], arr[j]);
            (*count)--;
            return 1;
        }
    }
    return 0;
}

/* API functions */
int pm_init(PermissionManager* pm) {
    if (!pm) return 0;
    pm->roles_count = 0;
    pm->users_count = 0;
    return 1;
}

int pm_add_role(PermissionManager* pm, const char* role_name) {
    char r[MAX_NAME_LEN + 1];
    if (!normalize(role_name, r, sizeof(r))) return 0;
    if (role_index(pm, r) != -1) return 0;
    if (pm->roles_count >= MAX_ROLES) return 0;
    Role* role = &pm->roles[pm->roles_count++];
    strncpy(role->name, r, MAX_NAME_LEN);
    role->name[MAX_NAME_LEN] = '\0';
    role->perms_count = 0;
    return 1;
}

int pm_add_user(PermissionManager* pm, const char* username) {
    char u[MAX_NAME_LEN + 1];
    if (!normalize(username, u, sizeof(u))) return 0;
    if (user_index(pm, u) != -1) return 0;
    if (pm->users_count >= MAX_USERS) return 0;
    User* user = &pm->users[pm->users_count++];
    strncpy(user->name, u, MAX_NAME_LEN);
    user->name[MAX_NAME_LEN] = '\0';
    user->direct_perms_count = 0;
    user->role_count = 0;
    return 1;
}

int pm_add_permission_to_role(PermissionManager* pm, const char* role_name, const char* permission) {
    char r[MAX_NAME_LEN + 1], p[MAX_NAME_LEN + 1];
    if (!normalize(role_name, r, sizeof(r)) || !normalize(permission, p, sizeof(p))) return 0;
    int ri = role_index(pm, r);
    if (ri == -1) return 0;
    return add_str(pm->roles[ri].perms, &pm->roles[ri].perms_count, MAX_ROLE_PERMS, p);
}

int pm_remove_permission_from_role(PermissionManager* pm, const char* role_name, const char* permission) {
    char r[MAX_NAME_LEN + 1], p[MAX_NAME_LEN + 1];
    if (!normalize(role_name, r, sizeof(r)) || !normalize(permission, p, sizeof(p))) return 0;
    int ri = role_index(pm, r);
    if (ri == -1) return 0;
    return remove_str(pm->roles[ri].perms, &pm->roles[ri].perms_count, p);
}

int pm_grant_permission_to_user(PermissionManager* pm, const char* username, const char* permission) {
    char u[MAX_NAME_LEN + 1], p[MAX_NAME_LEN + 1];
    if (!normalize(username, u, sizeof(u)) || !normalize(permission, p, sizeof(p))) return 0;
    int ui = user_index(pm, u);
    if (ui == -1) return 0;
    return add_str(pm->users[ui].direct_perms, &pm->users[ui].direct_perms_count, MAX_DIRECT_PERMS, p);
}

int pm_revoke_permission_from_user(PermissionManager* pm, const char* username, const char* permission) {
    char u[MAX_NAME_LEN + 1], p[MAX_NAME_LEN + 1];
    if (!normalize(username, u, sizeof(u)) || !normalize(permission, p, sizeof(p))) return 0;
    int ui = user_index(pm, u);
    if (ui == -1) return 0;
    return remove_str(pm->users[ui].direct_perms, &pm->users[ui].direct_perms_count, p);
}

int pm_assign_role_to_user(PermissionManager* pm, const char* username, const char* role_name) {
    char u[MAX_NAME_LEN + 1], r[MAX_NAME_LEN + 1];
    if (!normalize(username, u, sizeof(u)) || !normalize(role_name, r, sizeof(r))) return 0;
    int ui = user_index(pm, u);
    int ri = role_index(pm, r);
    if (ui == -1 || ri == -1) return 0;
    return add_str(pm->users[ui].role_names, &pm->users[ui].role_count, MAX_ASSIGN_ROLES, r);
}

int pm_unassign_role_from_user(PermissionManager* pm, const char* username, const char* role_name) {
    char u[MAX_NAME_LEN + 1], r[MAX_NAME_LEN + 1];
    if (!normalize(username, u, sizeof(u)) || !normalize(role_name, r, sizeof(r))) return 0;
    int ui = user_index(pm, u);
    if (ui == -1) return 0;
    return remove_str(pm->users[ui].role_names, &pm->users[ui].role_count, r);
}

int pm_user_has_permission(PermissionManager* pm, const char* username, const char* permission) {
    char u[MAX_NAME_LEN + 1], p[MAX_NAME_LEN + 1];
    if (!normalize(username, u, sizeof(u)) || !normalize(permission, p, sizeof(p))) return 0;
    int ui = user_index(pm, u);
    if (ui == -1) return 0;
    if (contains_str(pm->users[ui].direct_perms, pm->users[ui].direct_perms_count, p)) return 1;
    for (int i = 0; i < pm->users[ui].role_count; ++i) {
        int ri = role_index(pm, pm->users[ui].role_names[i]);
        if (ri != -1 && contains_str(pm->roles[ri].perms, pm->roles[ri].perms_count, p)) return 1;
    }
    return 0;
}

/* Returns dynamically allocated array of strings and sets out_count. Caller must free each string and the array. */
char** pm_get_effective_permissions(PermissionManager* pm, const char* username, int* out_count) {
    if (out_count) *out_count = 0;
    char u[MAX_NAME_LEN + 1];
    if (!normalize(username, u, sizeof(u))) return NULL;
    int ui = user_index(pm, u);
    if (ui == -1) return NULL;

    /* Use a simple temporary list to avoid duplicates */
    char** list = (char**)malloc((MAX_DIRECT_PERMS + MAX_ROLE_PERMS) * sizeof(char*));
    if (!list) return NULL;
    int count = 0;

    /* Add direct perms */
    for (int i = 0; i < pm->users[ui].direct_perms_count; ++i) {
        list[count++] = str_dup(pm->users[ui].direct_perms[i]);
    }
    /* Add role perms without duplicates */
    for (int r = 0; r < pm->users[ui].role_count; ++r) {
        int ri = role_index(pm, pm->users[ui].role_names[r]);
        if (ri == -1) continue;
        for (int p = 0; p < pm->roles[ri].perms_count; ++p) {
            const char* perm = pm->roles[ri].perms[p];
            int exists = 0;
            for (int k = 0; k < count; ++k) {
                if (strcmp(list[k], perm) == 0) { exists = 1; break; }
            }
            if (!exists) list[count++] = str_dup(perm);
        }
    }
    if (out_count) *out_count = count;
    return list;
}

/* For testing */
int main(void) {
    PermissionManager pm;
    pm_init(&pm);

    /* Test Case 1 */
    pm_add_role(&pm, "admin");
    pm_add_permission_to_role(&pm, "admin", "read");
    pm_add_permission_to_role(&pm, "admin", "write");
    pm_add_permission_to_role(&pm, "admin", "delete");
    pm_add_user(&pm, "alice");
    pm_assign_role_to_user(&pm, "alice", "admin");
    printf("TC1 alice has delete: %s\n", pm_user_has_permission(&pm, "alice", "delete") ? "true" : "false");

    /* Test Case 2 */
    pm_add_role(&pm, "editor");
    pm_add_permission_to_role(&pm, "editor", "write");
    pm_add_user(&pm, "bob");
    pm_grant_permission_to_user(&pm, "Bob", "READ"); /* Case/whitespace normalized */
    printf("TC2 bob has write before role: %s\n", pm_user_has_permission(&pm, "bob", "write") ? "true" : "false");
    pm_assign_role_to_user(&pm, "bob", "editor");
    printf("TC2 bob has write after role: %s\n", pm_user_has_permission(&pm, "bob", "write") ? "true" : "false");

    /* Test Case 3 */
    pm_remove_permission_from_role(&pm, "admin", "delete");
    printf("TC3 alice has delete after removal from role: %s\n", pm_user_has_permission(&pm, "alice", "delete") ? "true" : "false");

    /* Test Case 4 */
    pm_grant_permission_to_user(&pm, "alice", "delete");
    printf("TC4 alice has delete after direct grant: %s\n", pm_user_has_permission(&pm, "alice", "delete") ? "true" : "false");

    /* Test Case 5 */
    printf("TC5 unknown user has read: %s\n", pm_user_has_permission(&pm, "charlie", "read") ? "true" : "false");
    printf("TC5 adding duplicate role 'admin': %s\n", pm_add_role(&pm, "admin") ? "true" : "false");
    int count = 0;
    char** perms = pm_get_effective_permissions(&pm, "alice", &count);
    printf("TC5 alice effective perms: [");
    if (perms) {
        /* simple sort */
        for (int i = 0; i < count - 1; ++i) {
            for (int j = i + 1; j < count; ++j) {
                if (strcmp(perms[i], perms[j]) > 0) {
                    char* tmp = perms[i]; perms[i] = perms[j]; perms[j] = tmp;
                }
            }
        }
        for (int i = 0; i < count; ++i) {
            printf("%s", perms[i]);
            if (i + 1 < count) printf(", ");
        }
    }
    printf("]\n");
    if (perms) {
        for (int i = 0; i < count; ++i) free(perms[i]);
        free(perms);
    }

    return 0;
}