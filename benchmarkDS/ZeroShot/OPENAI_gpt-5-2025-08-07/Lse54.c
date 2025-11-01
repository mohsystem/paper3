#include <stdio.h>
#include <string.h>
#include <stddef.h>

static const char* ADMIN_USERNAME = "admin";
static const char* ADMIN_PASSWORD = "AdminPassword!234";

const char* login() {
    return "<!DOCTYPE html>"
           "<html><head><meta charset=\"utf-8\"><title>Login</title></head>"
           "<body>"
           "<form action=\"/do_login\" method=\"post\" autocomplete=\"off\">"
           "<label for=\"username\">Username:</label>"
           "<input type=\"text\" id=\"username\" name=\"username\" required minlength=\"1\" maxlength=\"64\" />"
           "<label for=\"password\">Password:</label>"
           "<input type=\"password\" id=\"password\" name=\"password\" required minlength=\"1\" maxlength=\"128\" />"
           "<button type=\"submit\">Login</button>"
           "</form>"
           "</body></html>";
}

static int valid_username(const char* u) {
    if (u == NULL) return 0;
    size_t len = strlen(u);
    if (len < 1 || len > 64) return 0;
    for (size_t i = 0; i < len; ++i) {
        char c = u[i];
        int ok = (c >= 'A' && c <= 'Z')
              || (c >= 'a' && c <= 'z')
              || (c >= '0' && c <= '9')
              || c == '.' || c == '_' || c == '-';
        if (!ok) return 0;
    }
    return 1;
}

static int valid_password(const char* p) {
    if (p == NULL) return 0;
    size_t len = strlen(p);
    return (len >= 1 && len <= 128);
}

static int constant_time_equals(const char* a, const char* b) {
    if (a == NULL || b == NULL) {
        return (a == b) ? 1 : 0;
    }
    size_t la = strlen(a);
    size_t lb = strlen(b);
    size_t len = la > lb ? la : lb;
    unsigned char diff = 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char ai = (i < la) ? (unsigned char)a[i] : 0;
        unsigned char bi = (i < lb) ? (unsigned char)b[i] : 0;
        diff |= (unsigned char)(ai ^ bi);
    }
    return (diff == 0 && la == lb) ? 1 : 0;
}

const char* do_login(const char* username, const char* password) {
    if (!valid_username(username) || !valid_password(password)) {
        return "400 Bad Request";
    }
    int isAdminUser = constant_time_equals(username, ADMIN_USERNAME);
    int isAdminPass = constant_time_equals(password, ADMIN_PASSWORD);
    if (isAdminUser && isAdminPass) {
        return "REDIRECT:/admin_page";
    }
    return "401 Unauthorized";
}

int main(void) {
    // 5 test cases
    printf("TEST1 FORM:\n%s\n", login());
    printf("TEST2 ADMIN CORRECT:\n%s\n", do_login("admin", "AdminPassword!234"));
    printf("TEST3 ADMIN WRONG PASS:\n%s\n", do_login("admin", "wrong"));
    printf("TEST4 NON-ADMIN:\n%s\n", do_login("user", "AdminPassword!234"));
    printf("TEST5 BAD INPUT:\n%s\n", do_login("", ""));
    return 0;
}