#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define ROLE_GUEST 1
#define ROLE_USER  2
#define ROLE_ADMIN 4

#define MAX_ID_LEN 20

static const char* GENERIC_ERROR = "ERROR: Access denied or resource not found.";

typedef struct {
    const char* id;
    const char* content;
    int allowed_roles_mask;
} Resource;

static const Resource RESOURCES[] = {
    {"public", "Welcome to the public area.", ROLE_GUEST | ROLE_USER | ROLE_ADMIN},
    {"dashboard", "Admin control panel.", ROLE_ADMIN},
    {"profile", "User profile page.", ROLE_USER | ROLE_ADMIN},
    {"report2025", "Confidential report 2025.", ROLE_USER | ROLE_ADMIN},
    {"help", "Help and FAQs.", ROLE_GUEST | ROLE_USER | ROLE_ADMIN}
};

static const size_t RESOURCES_COUNT = sizeof(RESOURCES) / sizeof(RESOURCES[0]);

static int role_to_mask(const char* role) {
    if (role == NULL) return 0;
    if (strcmp(role, "guest") == 0) return ROLE_GUEST;
    if (strcmp(role, "user") == 0) return ROLE_USER;
    if (strcmp(role, "admin") == 0) return ROLE_ADMIN;
    return 0;
}

static bool is_valid_resource_id(const char* id) {
    if (id == NULL) return false;
    size_t len = strlen(id);
    if (len == 0 || len > MAX_ID_LEN) return false;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)id[i];
        if (!(isalnum(c) || c == '_' || c == '-')) return false;
    }
    return true;
}

static char* str_dup_safe(const char* s) {
    if (s == NULL) return NULL;
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, s, len);
    out[len] = '\0';
    return out;
}

char* access_resource(const char* role, const char* resource_id) {
    int role_mask = role_to_mask(role);
    if (role_mask == 0 || !is_valid_resource_id(resource_id)) {
        return str_dup_safe(GENERIC_ERROR);
    }
    for (size_t i = 0; i < RESOURCES_COUNT; i++) {
        if (strcmp(resource_id, RESOURCES[i].id) == 0) {
            if ((RESOURCES[i].allowed_roles_mask & role_mask) != 0) {
                return str_dup_safe(RESOURCES[i].content);
            } else {
                return str_dup_safe(GENERIC_ERROR);
            }
        }
    }
    return str_dup_safe(GENERIC_ERROR);
}

int main(void) {
    const char* roles[5] = {"admin", "guest", "user", "guest", "user"};
    const char* resources[5] = {"dashboard", "public", "report2025", "report2025", "bad$id"};

    for (int i = 0; i < 5; i++) {
        char* result = access_resource(roles[i], resources[i]);
        if (result != NULL) {
            printf("Test %d (%s, %s): %s\n", i + 1, roles[i], resources[i], result);
            free(result);
        } else {
            printf("Test %d (%s, %s): ERROR: allocation failure\n", i + 1, roles[i], resources[i]);
        }
    }
    return 0;
}