#include <stdio.h>

/*
 Steps 1-5: Simple and safe function. Returns string literals (static storage).
*/

const char* decide_redirect(int is_logged_in, int is_admin) {
    if (!is_logged_in) {
        return "/login";
    }
    if (is_admin) {
        return "/admin";
    }
    return "/user";
}

int main(void) {
    // 5 test cases
    struct Case { int logged_in; int admin; } tests[5] = {
        {0, 0}, // /login
        {0, 1}, // /login
        {1, 0}, // /user
        {1, 1}, // /admin
        {1, 0}  // /user
    };

    for (int i = 0; i < 5; ++i) {
        const char* result = decide_redirect(tests[i].logged_in, tests[i].admin);
        printf("Test %d (%s, %s): %s\n", i + 1,
               tests[i].logged_in ? "true" : "false",
               tests[i].admin ? "true" : "false",
               result);
    }
    return 0;
}