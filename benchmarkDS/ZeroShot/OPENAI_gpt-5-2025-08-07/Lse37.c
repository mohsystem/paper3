#include <stdio.h>
#include <string.h>
#include <stddef.h>

#define MAX_USERNAME_LEN 64
#define MAX_PASSWORD_LEN 128

typedef struct {
    const char* username;
    const char* password;
} UserRecord;

static const UserRecord DB[] = {
    {"alice", "StrongPass!42"},
    {"bob", "s3cr3t"},
    {"charlie", "passw0rd"},
    {"diana", "Qwerty!234"}
};
static const size_t DB_COUNT = sizeof(DB) / sizeof(DB[0]);
static const char* DUMMY_SECRET = "dummy_password_for_timing";

static size_t safe_strnlen(const char* s, size_t maxlen) {
    if (!s) return 0;
    size_t i = 0;
    for (; i < maxlen && s[i] != '\0'; ++i) { }
    return i;
}

static int constant_time_equals(const char* a, const char* b) {
    size_t la = safe_strnlen(a, MAX_PASSWORD_LEN);
    size_t lb = safe_strnlen(b, MAX_PASSWORD_LEN);
    size_t maxlen = la > lb ? la : lb;
    unsigned int diff = (la == lb) ? 0u : 1u;
    for (size_t i = 0; i < maxlen; ++i) {
        unsigned char ca = (i < la) ? (unsigned char)a[i] : 0;
        unsigned char cb = (i < lb) ? (unsigned char)b[i] : 0;
        diff |= (unsigned int)(ca ^ cb);
    }
    return diff == 0;
}

const char* handle_login(const char* username, const char* password) {
    // Basic validation
    if (username == NULL || password == NULL) {
        return "cant_login";
    }

    // Simulate internal error
    if (strcmp(username, "__simulate_error__") == 0) {
        return "internal_server_error";
    }

    size_t ulen = safe_strnlen(username, MAX_USERNAME_LEN + 1);
    size_t plen = safe_strnlen(password, MAX_PASSWORD_LEN + 1);
    if (ulen == 0 || ulen > MAX_USERNAME_LEN || plen > MAX_PASSWORD_LEN) {
        return "cant_login";
    }

    // Look up user
    const char* stored = NULL;
    for (size_t i = 0; i < DB_COUNT; ++i) {
        // Compare usernames exactly
        if (safe_strnlen(DB[i].username, MAX_USERNAME_LEN + 1) == ulen &&
            strncmp(DB[i].username, username, ulen) == 0) {
            stored = DB[i].password;
            break;
        }
    }

    if (stored == NULL) {
        // Mitigate user enumeration by constant-time compare with dummy
        (void)constant_time_equals(password, DUMMY_SECRET);
        return "cant_login";
    }

    int ok = constant_time_equals(password, stored);
    return ok ? "success" : "cant_login";
}

int main(void) {
    // 5 test cases
    printf("%s\n", handle_login("alice", "StrongPass!42"));  // success
    printf("%s\n", handle_login("alice", "wrong"));          // cant_login
    printf("%s\n", handle_login("unknown", "whatever"));     // cant_login
    printf("%s\n", handle_login("", "somepass"));            // cant_login
    printf("%s\n", handle_login("__simulate_error__", "x")); // internal_server_error
    return 0;
}