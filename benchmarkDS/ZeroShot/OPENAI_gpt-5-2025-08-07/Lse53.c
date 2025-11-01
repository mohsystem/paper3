#include <stdio.h>

static const char* LOGIN_PATH = "/login";
static const char* USER_PATH  = "/user";
static const char* ADMIN_PATH = "/admin";

const char* secure_redirect(int is_logged_in, int is_admin) {
    if (!((is_logged_in == 0 || is_logged_in == 1) && (is_admin == 0 || is_admin == 1))) {
        return LOGIN_PATH;
    }
    if (is_logged_in == 0) {
        return LOGIN_PATH;
    }
    if (is_admin == 1) {
        return ADMIN_PATH;
    }
    return USER_PATH;
}

int main(void) {
    printf("%s\n", secure_redirect(0, 0)); // /login
    printf("%s\n", secure_redirect(1, 0)); // /user
    printf("%s\n", secure_redirect(1, 1)); // /admin
    printf("%s\n", secure_redirect(0, 1)); // /login
    printf("%s\n", secure_redirect(2, 1)); // /login (invalid input)
    return 0;
}