#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_ID_LEN 32
#define MAX_RESOURCES 5

typedef struct {
    const char* username;
    const char* role;
} UserRole;

typedef struct {
    const char* role;
    const char* resources[MAX_RESOURCES];
    int count;
} RoleResources;

static const UserRole USER_ROLES[] = {
    {"alice", "admin"},
    {"bob", "editor"},
    {"carol", "viewer"},
    {"dave", "viewer"},
    {"eve", "editor"}
};
static const size_t USER_ROLES_COUNT = sizeof(USER_ROLES) / sizeof(USER_ROLES[0]);

static const char* ADMIN_RES[]  = {"config", "reports", "dashboard", "users", "logs"};
static const char* EDITOR_RES[] = {"reports", "dashboard"};
static const char* VIEWER_RES[] = {"dashboard"};

static const RoleResources ROLE_RESOURCES[] = {
    {"admin",  { "config", "reports", "dashboard", "users", "logs" }, 5},
    {"editor", { "reports", "dashboard", NULL, NULL, NULL }, 2},
    {"viewer", { "dashboard", NULL, NULL, NULL, NULL }, 1}
};
static const size_t ROLE_RESOURCES_COUNT = sizeof(ROLE_RESOURCES) / sizeof(ROLE_RESOURCES[0]);

static int canonicalize(const char* input, char* out, size_t outSize) {
    if (input == NULL || out == NULL || outSize < 1) return 0;

    // Trim leading/trailing whitespace
    const char* start = input;
    while (*start && isspace((unsigned char)*start)) start++;
    const char* end = input + strlen(input);
    while (end > start && isspace((unsigned char)*(end - 1))) end--;

    size_t len = (size_t)(end - start);
    if (len == 0 || len > MAX_ID_LEN) return 0;

    size_t j = 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char ch = (unsigned char)start[i];
        if (ch >= 'A' && ch <= 'Z') ch = (unsigned char)(ch + 32);
        if (!((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == '_' || ch == '-')) {
            return 0;
        }
        if (j + 1 >= outSize) return 0; // ensure space for null terminator
        out[j++] = (char)ch;
    }
    out[j] = '\0';
    return 1;
}

static const char* get_user_role(const char* username) {
    for (size_t i = 0; i < USER_ROLES_COUNT; ++i) {
        if (strcmp(USER_ROLES[i].username, username) == 0) {
            return USER_ROLES[i].role;
        }
    }
    return NULL;
}

static const RoleResources* get_role_resources(const char* role) {
    for (size_t i = 0; i < ROLE_RESOURCES_COUNT; ++i) {
        if (strcmp(ROLE_RESOURCES[i].role, role) == 0) {
            return &ROLE_RESOURCES[i];
        }
    }
    return NULL;
}

// Returns number of resources written to out; out is an array of strings with size cap 'max_out'.
// Each element of out should be a char buffer of size at least MAX_ID_LEN+1.
int get_accessible_resources(const char* username_in, char out[][MAX_ID_LEN + 1], int max_out) {
    char username[MAX_ID_LEN + 1];
    if (!canonicalize(username_in, username, sizeof(username))) {
        return 0;
    }
    const char* role = get_user_role(username);
    if (!role) return 0;
    const RoleResources* rr = get_role_resources(role);
    if (!rr) return 0;

    int written = 0;
    for (int i = 0; i < rr->count && written < max_out; ++i) {
        strncpy(out[written], rr->resources[i], MAX_ID_LEN);
        out[written][MAX_ID_LEN] = '\0';
        written++;
    }
    return written;
}

int can_access(const char* username_in, const char* resource_in) {
    char username[MAX_ID_LEN + 1];
    char resource[MAX_ID_LEN + 1];
    if (!canonicalize(username_in, username, sizeof(username))) return 0;
    if (!canonicalize(resource_in, resource, sizeof(resource))) return 0;

    const char* role = get_user_role(username);
    if (!role) return 0;
    const RoleResources* rr = get_role_resources(role);
    if (!rr) return 0;

    for (int i = 0; i < rr->count; ++i) {
        if (strcmp(rr->resources[i], resource) == 0) return 1;
    }
    return 0;
}

// Writes a message into out (size outSize). Returns 1 on success, 0 on failure.
int request_access(const char* username_in, const char* resource_in, char* out, size_t outSize) {
    if (!out || outSize == 0) return 0;
    int allowed = can_access(username_in, resource_in);
    int n = snprintf(out, outSize, "%s: %s -> %s",
                     allowed ? "GRANTED" : "DENIED",
                     username_in ? username_in : "(null)",
                     resource_in ? resource_in : "(null)");
    return (n >= 0 && (size_t)n < outSize) ? 1 : 0;
}

int main(void) {
    // 5 test cases
    const char* tests[5][2] = {
        {"alice", "logs"},
        {"bob", "config"},
        {"carol", "dashboard"},
        {"unknown", "reports"},
        {"eve", "reports"}
    };

    char msg[128];
    for (int i = 0; i < 5; ++i) {
        if (request_access(tests[i][0], tests[i][1], msg, sizeof msg)) {
            printf("%s\n", msg);
        } else {
            printf("Error processing request\n");
        }
    }

    const char* users[] = {"alice", "bob", "carol", "unknown", "eve"};
    for (size_t i = 0; i < sizeof(users)/sizeof(users[0]); ++i) {
        char resources[5][MAX_ID_LEN + 1];
        int count = get_accessible_resources(users[i], resources, 5);
        printf("Resources for %s: [", users[i]);
        for (int j = 0; j < count; ++j) {
            printf("%s%s", resources[j], (j + 1 < count) ? ", " : "");
        }
        printf("]\n");
    }

    return 0;
}