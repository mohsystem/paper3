#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_PATH_LEN 2048
#define MAX_USERNAME_LEN 256

typedef char* (*route_func)(const char* username);

typedef struct {
    const char* pattern;
    route_func func;
} Route;

#define MAX_ROUTES 8

static Route g_routes[MAX_ROUTES];
static size_t g_route_count = 0;

static int register_route(const char* pattern, route_func func) {
    if (g_route_count >= MAX_ROUTES) {
        return -1;
    }
    g_routes[g_route_count].pattern = pattern;
    g_routes[g_route_count].func = func;
    g_route_count++;
    return 0;
}

static size_t safe_strlen_limit(const char* s, size_t max) {
    size_t n = 0;
    if (!s) return 0;
    while (n < max && s[n] != '\0') {
        n++;
    }
    return n;
}

static char* escape_html(const char* s) {
    if (!s) {
        char* z = (char*)malloc(1);
        if (z) z[0] = '\0';
        return z;
    }
    size_t in_len = safe_strlen_limit(s, MAX_USERNAME_LEN);
    // Worst-case expansion: '&' -> "&amp;" (5), '<' -> "&lt;" (4), '>' -> "&gt;" (4), '"' -> "&quot;" (6), '\'' -> "&#x27;" (6)
    // Use 6 as safe upper bound per char plus null
    size_t max_out = in_len * 6 + 1;
    char* out = (char*)malloc(max_out);
    if (!out) return NULL;

    size_t j = 0;
    for (size_t i = 0; i < in_len; i++) {
        char c = s[i];
        const char* rep = NULL;
        switch (c) {
            case '&': rep = "&amp;"; break;
            case '<': rep = "&lt;"; break;
            case '>': rep = "&gt;"; break;
            case '"': rep = "&quot;"; break;
            case '\'': rep = "&#x27;"; break;
            default: break;
        }
        if (rep) {
            size_t rep_len = strlen(rep);
            if (j + rep_len >= max_out) { break; }
            memcpy(out + j, rep, rep_len);
            j += rep_len;
        } else {
            if (j + 1 >= max_out) { break; }
            out[j++] = c;
        }
    }
    out[j] = '\0';
    return out;
}

static char* hello(const char* username) {
    if (!username) {
        char* msg = (char*)malloc(20);
        if (!msg) return NULL;
        snprintf(msg, 20, "400 Bad Request");
        return msg;
    }
    char* escaped = escape_html(username);
    if (!escaped) {
        char* msg = (char*)malloc(26);
        if (!msg) return NULL;
        snprintf(msg, 26, "500 Internal Server Error");
        return msg;
    }
    const char* prefix = "Hello ";
    size_t plen = strlen(prefix);
    size_t ulen = strlen(escaped);
    size_t total = plen + ulen + 1;
    char* result = (char*)malloc(total);
    if (!result) {
        free(escaped);
        char* msg = (char*)malloc(26);
        if (!msg) return NULL;
        snprintf(msg, 26, "500 Internal Server Error");
        return msg;
    }
    memcpy(result, prefix, plen);
    memcpy(result + plen, escaped, ulen);
    result[total - 1] = '\0';
    free(escaped);
    return result;
}

static char* handle_request(const char* path) {
    if (!path) {
        char* msg = (char*)malloc(16);
        if (!msg) return NULL;
        snprintf(msg, 16, "400 Bad Request");
        return msg;
    }
    size_t path_len = safe_strlen_limit(path, MAX_PATH_LEN + 1);
    if (path_len == 0 || path_len > MAX_PATH_LEN) {
        char* msg = (char*)malloc(16);
        if (!msg) return NULL;
        snprintf(msg, 16, "400 Bad Request");
        return msg;
    }

    for (size_t i = 0; i < g_route_count; i++) {
        const char* pattern = g_routes[i].pattern;
        route_func func = g_routes[i].func;

        if (strcmp(pattern, "/hello/{username}") == 0) {
            const char* prefix = "/hello/";
            size_t prefix_len = strlen(prefix);
            if (strncmp(path, prefix, prefix_len) != 0) {
                continue;
            }
            const char* username = path + prefix_len;
            if (*username == '\0' || strchr(username, '/') != NULL) {
                continue;
            }
            size_t user_len = safe_strlen_limit(username, MAX_USERNAME_LEN + 1);
            if (user_len == 0 || user_len > MAX_USERNAME_LEN) {
                char* msg = (char*)malloc(16);
                if (!msg) return NULL;
                snprintf(msg, 16, "400 Bad Request");
                return msg;
            }
            // Call handler
            char* resp = func(username);
            if (!resp) {
                char* msg = (char*)malloc(26);
                if (!msg) return NULL;
                snprintf(msg, 26, "500 Internal Server Error");
                return msg;
            }
            return resp;
        }
    }

    char* msg = (char*)malloc(14);
    if (!msg) return NULL;
    snprintf(msg, 14, "404 Not Found");
    return msg;
}

int main(void) {
    // Register route
    if (register_route("/hello/{username}", hello) != 0) {
        fprintf(stderr, "Failed to register route\n");
        return 1;
    }

    const char* tests[5] = {
        "/hello/Alice",
        "/hello/Bob<Admin>",
        "/hello/",
        "/hi/Alice",
        "/hello/AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    };

    for (size_t i = 0; i < 5; i++) {
        char* resp = handle_request(tests[i]);
        if (resp) {
            printf("URL: %s -> %s\n", tests[i], resp);
            free(resp);
        } else {
            printf("URL: %s -> (null)\n", tests[i]);
        }
    }

    return 0;
}