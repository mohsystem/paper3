#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

struct RoutePair {
    const char* key;
    const char* path;
};

static const struct RoutePair ROUTES[] = {
    {"home", "/home"},
    {"index", "/home"},
    {"main", "/home"},
    {"about", "/about"},
    {"contact", "/contact"},
    {"dashboard", "/dashboard"},
    {"settings", "/settings"},
    {"help", "/help"}
};

static size_t route_count(void) {
    return sizeof(ROUTES) / sizeof(ROUTES[0]);
}

const char* redirect_user(const char* input) {
    static const char* DEFAULT_HOME = "/home";
    static const char* ERROR_PAGE = "/error";

    if (input == NULL) {
        return DEFAULT_HOME;
    }

    size_t inlen = strlen(input);
    size_t start = 0;
    while (start < inlen && isspace((unsigned char)input[start])) start++;
    size_t end = inlen;
    while (end > start && isspace((unsigned char)input[end - 1])) end--;

    if (end <= start) {
        return ERROR_PAGE;
    }

    size_t len = end - start;
    if (len > 100) {
        return ERROR_PAGE;
    }

    static char buf[128];
    size_t j = 0;
    for (size_t i = 0; i < len && j < sizeof(buf) - 1; i++) {
        unsigned char ch = (unsigned char)input[start + i];
        buf[j++] = (char)tolower(ch);
    }
    buf[j] = '\0';

    if (strncmp(buf, "http:", 5) == 0 || strncmp(buf, "https:", 6) == 0 || strstr(buf, "://") != NULL || (buf[0] == '/' && buf[1] == '/')) {
        return ERROR_PAGE;
    }

    for (size_t i = 0; i < j; i++) {
        unsigned char ch = (unsigned char)buf[i];
        if (!((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == '-' || ch == '_')) {
            return ERROR_PAGE;
        }
    }

    size_t n = route_count();
    for (size_t i = 0; i < n; i++) {
        if (strcmp(ROUTES[i].key, buf) == 0) {
            return ROUTES[i].path;
        }
    }

    return ERROR_PAGE;
}

int main(void) {
    const char* tests[5] = {"home", "ABOUT", "Contact", "http://evil.com", "settings"};
    for (int i = 0; i < 5; i++) {
        printf("Input: %s -> %s\n", tests[i], redirect_user(tests[i]));
    }
    return 0;
}