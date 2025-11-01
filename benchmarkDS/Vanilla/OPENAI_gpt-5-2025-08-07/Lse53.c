#include <stdio.h>
#include <stdbool.h>

const char* get_redirect(bool is_logged_in, bool is_admin) {
    if (!is_logged_in) return "login";
    if (is_admin) return "admin";
    return "user";
}

int main() {
    printf("%s\n", get_redirect(false, false)); // Expected: login
    printf("%s\n", get_redirect(false, true));  // Expected: login
    printf("%s\n", get_redirect(true, false));  // Expected: user
    printf("%s\n", get_redirect(true, true));   // Expected: admin
    printf("%s\n", get_redirect(true, false));  // Expected: user
    return 0;
}