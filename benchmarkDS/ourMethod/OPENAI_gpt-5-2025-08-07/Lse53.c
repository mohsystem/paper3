#include <stdio.h>
#include <stdbool.h>

const char* get_redirect_target(bool is_logged_in, bool is_admin) {
    if (!is_logged_in) {
        return "/login";
    }
    if (is_admin) {
        return "/admin";
    }
    return "/user";
}

static void run_test(bool is_logged_in, bool is_admin) {
    const char* result = get_redirect_target(is_logged_in, is_admin);
    printf("is_logged_in=%s, is_admin=%s -> %s\n",
           is_logged_in ? "true" : "false",
           is_admin ? "true" : "false",
           result);
}

int main(void) {
    /* 5 test cases */
    run_test(false, false); /* /login */
    run_test(true, false);  /* /user */
    run_test(true, true);   /* /admin */
    run_test(false, true);  /* /login */
    run_test(true, true);   /* /admin */
    return 0;
}