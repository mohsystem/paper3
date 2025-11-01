#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* HELLO_TEMPLATE = "Hello, {{ username }}!";

static char* replace_username_in_template(const char* template_str, const char* username) {
    const char* placeholder = "{{ username }}";
    size_t tpl_len = strlen(template_str);
    size_t ph_len = strlen(placeholder);
    size_t uname_len = strlen(username);

    // Count occurrences
    size_t count = 0;
    for (const char* p = template_str; (p = strstr(p, placeholder)) != NULL; p += ph_len) {
        count++;
    }

    size_t out_len = tpl_len + count * (uname_len - ph_len);
    char* out = (char*)malloc(out_len + 1);
    if (!out) return NULL;

    const char* src = template_str;
    char* dst = out;
    while (*src) {
        const char* pos = strstr(src, placeholder);
        if (!pos) {
            size_t rem = strlen(src);
            memcpy(dst, src, rem);
            dst += rem;
            break;
        } else {
            size_t chunk = (size_t)(pos - src);
            memcpy(dst, src, chunk);
            dst += chunk;
            memcpy(dst, username, uname_len);
            dst += uname_len;
            src = pos + ph_len;
        }
    }
    *dst = '\0';
    return out;
}

// Public API: accept a URL path like "/hello/<username>", return rendered output (malloc'd string).
char* handle_request(const char* path) {
    const char* prefix = "/hello/";
    size_t prefix_len = strlen(prefix);
    if (strncmp(path, prefix, prefix_len) != 0) {
        char* not_found = (char*)malloc(14);
        if (not_found) strcpy(not_found, "404 Not Found");
        return not_found;
    }
    const char* rest = path + prefix_len;
    if (*rest == '\0' || strchr(rest, '/')) {
        char* not_found = (char*)malloc(14);
        if (not_found) strcpy(not_found, "404 Not Found");
        return not_found;
    }
    return replace_username_in_template(HELLO_TEMPLATE, rest);
}

int main(void) {
    const char* tests[] = {"Alice", "Bob", "Carol_123", "Dora", "Eve"};
    for (int i = 0; i < 5; ++i) {
        char url[256];
        snprintf(url, sizeof(url), "/hello/%s", tests[i]);
        char* response = handle_request(url);
        if (response) {
            printf("%s\n", response);
            free(response);
        }
    }
    return 0;
}