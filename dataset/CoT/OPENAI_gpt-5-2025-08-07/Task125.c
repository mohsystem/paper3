#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/*
Chain-of-Through Step 1: Problem understanding
- Provide safe access to predefined resources based on user_id, role, resource_id.

Chain-of-Through Step 2: Security requirements
- Validate tokens (charset and length).
- Normalize role (uppercase) and enforce whitelist.
- No external IO or filesystem access.
*/

typedef struct {
    const char* id;
    const char* content;
    unsigned rolesMask; // bitmask: 1=ADMIN, 2=USER, 4=GUEST
} Resource;

enum { ROLE_ADMIN=1, ROLE_USER=2, ROLE_GUEST=4 };

static const Resource RESOURCES[] = {
    {"doc_public", "Public Document", ROLE_ADMIN|ROLE_USER|ROLE_GUEST},
    {"doc_user",   "User Document",   ROLE_ADMIN|ROLE_USER},
    {"doc_admin",  "Admin Secrets",   ROLE_ADMIN},
    {"img_banner", "Image Banner",    ROLE_ADMIN|ROLE_USER|ROLE_GUEST},
    {"report_q3",  "Q3 Report",       ROLE_ADMIN|ROLE_USER},
};
static const size_t RES_COUNT = sizeof(RESOURCES)/sizeof(RESOURCES[0]);

static int valid_token(const char* s) {
    if (s == NULL) return 0;
    size_t n = strlen(s);
    if (n == 0 || n > 32) return 0;
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)s[i];
        if (!(isalnum(c) || c=='_' || c=='-')) return 0;
    }
    return 1;
}

static unsigned role_mask_from(const char* role) {
    if (role == NULL) return 0;
    char buf[16];
    size_t n = strlen(role);
    if (n >= sizeof(buf)) return 0;
    for (size_t i = 0; i <= n; i++) {
        buf[i] = (char)toupper((unsigned char)role[i]);
    }
    if (strcmp(buf, "ADMIN") == 0) return ROLE_ADMIN;
    if (strcmp(buf, "USER") == 0)  return ROLE_USER;
    if (strcmp(buf, "GUEST") == 0) return ROLE_GUEST;
    return 0;
}

static const Resource* find_resource(const char* id) {
    for (size_t i = 0; i < RES_COUNT; i++) {
        if (strcmp(RESOURCES[i].id, id) == 0) return &RESOURCES[i];
    }
    return NULL;
}

/*
Chain-of-Through Step 3: Secure coding generation
- Access function: validate, authorize, return deterministic string.
Note: Returns pointer to static buffer (not thread-safe). Suitable for this demo use.
*/
const char* access_resource(const char* user_id, const char* role, const char* resource_id) {
    static char result[160];
    result[0] = '\0';

    if (!valid_token(user_id) || !valid_token(resource_id)) {
        snprintf(result, sizeof(result), "ERROR:INVALID_INPUT");
        return result;
    }
    unsigned rmask = role_mask_from(role);
    if (rmask == 0) {
        snprintf(result, sizeof(result), "ERROR:INVALID_INPUT");
        return result;
    }
    const Resource* r = find_resource(resource_id);
    if (!r) {
        snprintf(result, sizeof(result), "ERROR:NOT_FOUND");
        return result;
    }
    if ((r->rolesMask & rmask) == 0) {
        snprintf(result, sizeof(result), "ERROR:ACCESS_DENIED");
        return result;
    }
    // Construct success string
    snprintf(result, sizeof(result), "OK:%s:%s", r->id, r->content);
    return result;
}

/*
Chain-of-Through Step 4: Code review
- Validations and masks are correct; outputs bounded by snprintf.

Chain-of-Through Step 5: Secure code output
*/
int main(void) {
    // 5 test cases
    printf("%s\n", access_resource("alice","user","doc_user"));
    printf("%s\n", access_resource("bob","guest","doc_admin"));
    printf("%s\n", access_resource("carol","ADMIN","report_q3"));
    printf("%s\n", access_resource("","user","doc_public"));
    printf("%s\n", access_resource("dave","user","unknown"));
    return 0;
}