#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MIN_USERNAME_LEN 1
#define MAX_USERNAME_LEN 64
#define MIN_PASSWORD_LEN 12
#define MAX_PASSWORD_LEN 128

static int valid_username(const char* u) {
    if (u == NULL) return 0;
    size_t len = strlen(u);
    if (len < MIN_USERNAME_LEN || len > MAX_USERNAME_LEN) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)u[i];
        if (c > 0x7F) return 0;
        if (!(isalnum(c) || c == '_' || c == '-' || c == '.' || c == '@')) return 0;
    }
    return 1;
}

static int is_special_char(char c) {
    const char* specials = "!@#$%^&*()_+-=[]{}|;:',.<>/?`~\"\\";
    for (const char* p = specials; *p; ++p) {
        if (*p == c) return 1;
    }
    return 0;
}

static int valid_password_policy(const char* p) {
    if (p == NULL) return 0;
    size_t len = strlen(p);
    if (len < MIN_PASSWORD_LEN || len > MAX_PASSWORD_LEN) return 0;
    int hasU = 0, hasL = 0, hasD = 0, hasS = 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)p[i];
        if (c > 0x7F) return 0;
        if (isupper(c)) hasU = 1;
        else if (islower(c)) hasL = 1;
        else if (isdigit(c)) hasD = 1;
        else {
            if (!is_special_char((char)c)) return 0;
            hasS = 1;
        }
    }
    return hasU && hasL && hasD && hasS;
}

static int const_time_eq(const char* a, const char* b) {
    if (a == NULL || b == NULL) return 0;
    size_t la = strlen(a), lb = strlen(b);
    unsigned char diff = (unsigned char)(la ^ lb);
    size_t len = la < lb ? la : lb;
    for (size_t i = 0; i < len; i++) {
        diff |= (unsigned char)(a[i] ^ b[i]);
    }
    return diff == 0;
}

static int set_env(const char* key, const char* val) {
#if defined(_WIN32)
    size_t needed = strlen(key) + strlen(val) + 2;
    char* buf = (char*)malloc(needed);
    if (!buf) return 0;
    snprintf(buf, needed, "%s=%s", key, val);
    // _putenv takes ownership of the memory in some implementations; still we may leak for portability.
    return _putenv(buf) == 0;
#else
    // Use putenv for portability without relying on setenv
    size_t needed = strlen(key) + strlen(val) + 2;
    char* buf = (char*)malloc(needed);
    if (!buf) return 0;
    snprintf(buf, needed, "%s=%s", key, val);
    // putenv may use the pointer directly; do not free.
    return putenv(buf) == 0;
#endif
}

const char* login(void) {
    return "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Login</title></head>"
           "<body><h1>Login</h1>"
           "<form method=\"POST\" action=\"/do_login\">"
           "<label>Username: <input type=\"text\" name=\"username\" maxlength=\"64\" required></label><br>"
           "<label>Password: <input type=\"password\" name=\"password\" minlength=\"12\" maxlength=\"128\" required></label><br>"
           "<button type=\"submit\">Login</button>"
           "</form></body></html>";
}

const char* do_login(const char* username, const char* password) {
    if (!valid_username(username) || !valid_password_policy(password)) {
        return "INVALID_INPUT";
    }
    const char* env_user = getenv("ADMIN_USER");
    const char* env_pass = getenv("ADMIN_PASSWORD");
    const char* admin_user = (env_user && env_user[0] != '\0') ? env_user : "admin";
    if (env_pass == NULL || env_pass[0] == '\0') {
        return "SERVER_NOT_CONFIGURED";
    }
    if (!valid_password_policy(env_pass)) {
        return "SERVER_NOT_CONFIGURED";
    }
    if (!const_time_eq(username, admin_user)) {
        return "ACCESS_DENIED";
    }
    if (const_time_eq(password, env_pass)) {
        return "REDIRECT:/admin_page";
    }
    return "ACCESS_DENIED";
}

int main(void) {
    // Configure environment for testing
    set_env("ADMIN_USER", "admin");
    set_env("ADMIN_PASSWORD", "StrongPassw0rd!X");

    printf("TEST1 login():\n%s\n\n", login());

    printf("TEST2 do_login(admin, correct):\n%s\n\n", do_login("admin", "StrongPassw0rd!X"));

    printf("TEST3 do_login(admin, wrong password):\n%s\n\n", do_login("admin", "WrongPass123!"));

    printf("TEST4 do_login(notadmin, correct password):\n%s\n\n", do_login("notadmin", "StrongPassw0rd!X"));

    printf("TEST5 do_login(invalid user, short password):\n%s\n", do_login("bad user", "shortPwd!"));

    return 0;
}