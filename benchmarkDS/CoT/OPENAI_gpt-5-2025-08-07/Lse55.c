#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Step 1: Problem understanding - Simulate endpoints with sessions and RBAC.
// Step 2: Security requirements - Constant-time compares, secure tokens (/dev/urandom), session expiry.
// Step 3: Secure coding generation - Method checks, input validation, safe string handling.
// Step 4: Code review - No sensitive printing, avoid buffer overflows.
// Step 5: Secure code output - Final code.

#define SESSION_TTL_SEC (15 * 60)
#define TOKEN_BYTES 32
#define TOKEN_HEX_LEN (TOKEN_BYTES * 2)
#define MAX_SESSIONS 128

typedef struct {
    char token[TOKEN_HEX_LEN + 1];
    char user[32];
    char role[16];
    time_t expiry;
    int in_use;
} Session;

static Session SESSIONS[MAX_SESSIONS];

typedef struct {
    const char* user;
    const char* pass;
    const char* role;
} UserEntry;

static const UserEntry USERS[] = {
    {"user", "userpass", "user"},
    {"admin", "adminpass", "admin"}
};
static const size_t USERS_COUNT = sizeof(USERS)/sizeof(USERS[0]);

static const char* DUMMY_PASSWORD = "dummy_password_fallback_value";

static int constant_time_equals(const char* a, const char* b) {
    if (!a || !b) return 0;
    size_t la = strlen(a);
    size_t lb = strlen(b);
    size_t len = la > lb ? la : lb;
    unsigned char diff = (unsigned char)(la ^ lb);
    for (size_t i = 0; i < len; ++i) {
        unsigned char ac = (i < la) ? (unsigned char)a[i] : 0;
        unsigned char bc = (i < lb) ? (unsigned char)b[i] : 0;
        diff |= (unsigned char)(ac ^ bc);
    }
    return diff == 0;
}

static int get_random_bytes(unsigned char* out, size_t len) {
    FILE* f = fopen("/dev/urandom", "rb");
    if (f) {
        size_t r = fread(out, 1, len, f);
        fclose(f);
        if (r == len) return 1;
    }
    // Fallback (not ideal): use rand with srand from high-resolution time
    srand((unsigned int)time(NULL) ^ (unsigned int)clock());
    for (size_t i = 0; i < len; ++i) out[i] = (unsigned char)(rand() & 0xFF);
    return 1;
}

static void to_hex(const unsigned char* in, size_t len, char* out_hex) {
    static const char* HEX = "0123456789abcdef";
    for (size_t i = 0; i < len; ++i) {
        out_hex[2*i] = HEX[(in[i] >> 4) & 0xF];
        out_hex[2*i + 1] = HEX[in[i] & 0xF];
    }
    out_hex[2*len] = '\0';
}

static void generate_token(char* out_token_hex) {
    unsigned char buf[TOKEN_BYTES];
    get_random_bytes(buf, TOKEN_BYTES);
    to_hex(buf, TOKEN_BYTES, out_token_hex);
}

static UserEntry find_user(const char* username, int* found) {
    for (size_t i = 0; i < USERS_COUNT; ++i) {
        if (strcmp(USERS[i].user, username) == 0) {
            if (found) *found = 1;
            return USERS[i];
        }
    }
    if (found) *found = 0;
    UserEntry dummy = { "", DUMMY_PASSWORD, "user" };
    return dummy;
}

static Session* get_valid_session(const char* token) {
    if (!token) return NULL;
    time_t now = time(NULL);
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (SESSIONS[i].in_use && strcmp(SESSIONS[i].token, token) == 0) {
            if (SESSIONS[i].expiry < now) {
                SESSIONS[i].in_use = 0;
                return NULL;
            }
            return &SESSIONS[i];
        }
    }
    return NULL;
}

static void store_session(const char* token, const char* user, const char* role) {
    time_t now = time(NULL);
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (!SESSIONS[i].in_use) {
            strncpy(SESSIONS[i].token, token, TOKEN_HEX_LEN);
            SESSIONS[i].token[TOKEN_HEX_LEN] = '\0';
            strncpy(SESSIONS[i].user, user, sizeof(SESSIONS[i].user) - 1);
            SESSIONS[i].user[sizeof(SESSIONS[i].user) - 1] = '\0';
            strncpy(SESSIONS[i].role, role, sizeof(SESSIONS[i].role) - 1);
            SESSIONS[i].role[sizeof(SESSIONS[i].role) - 1] = '\0';
            SESSIONS[i].expiry = now + SESSION_TTL_SEC;
            SESSIONS[i].in_use = 1;
            return;
        }
    }
}

// Helper to duplicate string safely
static char* str_dup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, s, len + 1);
    return out;
}

// The first function: login (GET)
const char* login(const char* method) {
    if (!method || strcmp(method, "GET") != 0) {
        return "405 Method Not Allowed";
    }
    return "Login Page: Please provide username and password via do_login (POST).";
}

// The second function: do_login (POST) - returns token (caller should free) or NULL on failure
char* do_login(const char* method, const char* username, const char* password) {
    if (!method || strcmp(method, "POST") != 0) return NULL;
    if (!username || !password) {
        constant_time_equals(password ? password : "", DUMMY_PASSWORD);
        return NULL;
    }
    int found = 0;
    UserEntry u = find_user(username, &found);
    const char* to_compare = u.pass ? u.pass : DUMMY_PASSWORD;
    int ok = constant_time_equals(password, to_compare) && found;
    if (!ok) return NULL;
    char token[TOKEN_HEX_LEN + 1];
    generate_token(token);
    store_session(token, found ? u.user : "user", found ? u.role : "user");
    return str_dup(token);
}

// The third function: index (GET)
char* index_page(const char* method, const char* sessionToken) {
    if (!method || strcmp(method, "GET") != 0) {
        return str_dup("405 Method Not Allowed");
    }
    Session* s = get_valid_session(sessionToken);
    if (!s) return str_dup("Welcome, guest. Please login.");
    char buf[128];
    snprintf(buf, sizeof(buf), "Welcome, %s!", s->user);
    return str_dup(buf);
}

// The fourth function: user_page (GET)
char* user_page(const char* method, const char* sessionToken) {
    if (!method || strcmp(method, "GET") != 0) {
        return str_dup("405 Method Not Allowed");
    }
    Session* s = get_valid_session(sessionToken);
    if (!s) return str_dup("401 Unauthorized");
    char buf[128];
    snprintf(buf, sizeof(buf), "User Page for %s", s->user);
    return str_dup(buf);
}

// The fifth function: admin_page (GET)
char* admin_page(const char* method, const char* sessionToken) {
    if (!method || strcmp(method, "GET") != 0) {
        return str_dup("405 Method Not Allowed");
    }
    Session* s = get_valid_session(sessionToken);
    if (!s) return str_dup("401 Unauthorized");
    if (strcmp(s->role, "admin") != 0) return str_dup("403 Forbidden");
    char buf[128];
    snprintf(buf, sizeof(buf), "Admin Page for %s", s->user);
    return str_dup(buf);
}

int main(void) {
    // 5 test cases
    printf("Test1: %s\n", login("GET"));
    char* userToken = do_login("POST", "user", "userpass");
    printf("Test2: user login token: %s\n", userToken ? "true" : "false");
    char* up = user_page("GET", userToken);
    printf("Test3: user page with user token: %s\n", up);
    free(up);
    char* ap_user = admin_page("GET", userToken);
    printf("Test4: admin page with user token: %s\n", ap_user);
    free(ap_user);
    char* adminToken = do_login("POST", "admin", "adminpass");
    char* ap_admin = admin_page("GET", adminToken);
    printf("Test5: admin page with admin token: %s\n", ap_admin);
    free(ap_admin);
    free(userToken);
    free(adminToken);
    return 0;
}