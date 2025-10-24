#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    const char* key;
    int value;
} IntEntry;

typedef struct {
    const char* key;
    const char* value;
} StrEntry;

static IntEntry ROLE_LEVEL[] = {
    {"guest", 0},
    {"user", 1},
    {"editor", 2},
    {"admin", 3}
};

static IntEntry RESOURCE_REQUIRED_LEVEL[] = {
    {"public:news", 0},
    {"content:view", 1},
    {"user:profile", 1},
    {"content:edit", 2},
    {"admin:dashboard", 3},
    {"reports:financial", 3}
};

static StrEntry RESOURCE_CONTENT[] = {
    {"public:news", "Top headlines for everyone"},
    {"content:view", "Catalog of articles"},
    {"user:profile", "Personal profile details"},
    {"content:edit", "Editor panel for articles"},
    {"admin:dashboard", "System metrics and controls"},
    {"reports:financial", "Quarterly financial report"}
};

static int roles_count() { return (int)(sizeof(ROLE_LEVEL) / sizeof(ROLE_LEVEL[0])); }
static int resources_req_count() { return (int)(sizeof(RESOURCE_REQUIRED_LEVEL) / sizeof(RESOURCE_REQUIRED_LEVEL[0])); }
static int resources_content_count() { return (int)(sizeof(RESOURCE_CONTENT) / sizeof(RESOURCE_CONTENT[0])); }

static void to_lower(const char* src, char* dst, size_t dst_size) {
    size_t i = 0;
    for (; src[i] != '\0' && i + 1 < dst_size; ++i) {
        char c = src[i];
        if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
        dst[i] = c;
    }
    dst[i] = '\0';
}

static int get_role_level(const char* role) {
    char lowered[64];
    to_lower(role, lowered, sizeof(lowered));
    for (int i = 0; i < roles_count(); ++i) {
        if (strcmp(ROLE_LEVEL[i].key, lowered) == 0) {
            return ROLE_LEVEL[i].value;
        }
    }
    return -1;
}

static int get_required_level(const char* resourceId) {
    for (int i = 0; i < resources_req_count(); ++i) {
        if (strcmp(RESOURCE_REQUIRED_LEVEL[i].key, resourceId) == 0) {
            return RESOURCE_REQUIRED_LEVEL[i].value;
        }
    }
    return -1;
}

static const char* get_resource_content(const char* resourceId) {
    for (int i = 0; i < resources_content_count(); ++i) {
        if (strcmp(RESOURCE_CONTENT[i].key, resourceId) == 0) {
            return RESOURCE_CONTENT[i].value;
        }
    }
    return "(no content)";
}

char* accessResource(const char* user, const char* role, const char* resourceId) {
    if (role == NULL || resourceId == NULL) {
        char* out = (char*)malloc(32);
        sprintf(out, "ACCESS DENIED: Invalid input");
        return out;
    }
    int userLevel = get_role_level(role);
    if (userLevel < 0) {
        char* out = (char*)malloc(30);
        sprintf(out, "ACCESS DENIED: Unknown role");
        return out;
    }
    int requiredLevel = get_required_level(resourceId);
    if (requiredLevel < 0) {
        char* out = (char*)malloc(34);
        sprintf(out, "ACCESS DENIED: Unknown resource");
        return out;
    }
    if (userLevel >= requiredLevel) {
        const char* content = get_resource_content(resourceId);
        size_t needed = strlen("ACCESS GRANTED: ") + strlen(resourceId) + strlen(" -> ") + strlen(content) + 1;
        char* out = (char*)malloc(needed);
        snprintf(out, needed, "ACCESS GRANTED: %s -> %s", resourceId, content);
        return out;
    } else {
        char* out = (char*)malloc(39);
        sprintf(out, "ACCESS DENIED: Insufficient permissions");
        return out;
    }
}

int main() {
    const char* tests[5][3] = {
        {"alice", "guest", "public:news"},
        {"bob", "user", "content:view"},
        {"cindy", "user", "admin:dashboard"},
        {"dave", "editor", "content:edit"},
        {"erica", "guest", "user:profile"}
    };
    for (int i = 0; i < 5; ++i) {
        char* res = accessResource(tests[i][0], tests[i][1], tests[i][2]);
        puts(res);
        free(res);
    }
    return 0;
}