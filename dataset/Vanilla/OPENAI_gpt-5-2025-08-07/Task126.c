#include <stdio.h>
#include <string.h>
#include <ctype.h>

const char* redirect(const char* input) {
    static char buf[256];
    if (input == NULL) return "/home";

    // Trim leading spaces
    const char* start = input;
    while (*start && isspace((unsigned char)*start)) start++;

    // If all spaces
    if (*start == '\0') return "/home";

    // Find end (last non-space)
    const char* end = start + strlen(start) - 1;
    while (end >= start && isspace((unsigned char)*end)) end--;

    size_t len = (size_t)(end - start + 1);
    if (len > sizeof(buf) - 1) len = sizeof(buf) - 1;

    // Copy and lowercase
    for (size_t i = 0; i < len; i++) {
        buf[i] = (char)tolower((unsigned char)start[i]);
    }
    buf[len] = '\0';

    // Check routes
    if (strncmp(buf, "profile", 7) == 0) return "/user/profile";
    if (strcmp(buf, "home") == 0) return "/home";
    if (strcmp(buf, "login") == 0) return "/auth/login";
    if (strcmp(buf, "logout") == 0) return "/auth/logout";
    if (strcmp(buf, "admin") == 0) return "/admin/dashboard";
    if (strcmp(buf, "help") == 0) return "/support/help";
    if (strcmp(buf, "settings") == 0) return "/user/settings";

    return "/error/404";
}

int main(void) {
    const char* tests[5] = {
        "home",
        "LOGIN",
        "Admin ",
        "profile?id=42",
        "unknown"
    };

    for (int i = 0; i < 5; i++) {
        printf("%s -> %s\n", tests[i], redirect(tests[i]));
    }

    return 0;
}