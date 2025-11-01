#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef char* (*handler_fn)(const char*);

typedef struct {
    const char* path;
    handler_fn handler;
} Route;

static Route routes[16];
static int route_count = 0;

void register_route(const char* path, handler_fn handler) {
    if (route_count < 16) {
        routes[route_count].path = path;
        routes[route_count].handler = handler;
        route_count++;
    }
}

char* my_strdup(const char* s) {
    size_t n = strlen(s);
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    memcpy(out, s, n + 1);
    return out;
}

char* escape(const char* s) {
    size_t extra = 0;
    for (const char* p = s; *p; ++p) {
        switch (*p) {
            case '&': extra += 4; break;   /* &amp; -> +4 */
            case '<': extra += 3; break;   /* &lt;  -> +3 */
            case '>': extra += 3; break;   /* &gt;  -> +3 */
            case '"': extra += 5; break;   /* &quot;-> +5 */
            case '\'': extra += 5; break;  /* &#x27;-> +5 */
            default: break;
        }
    }
    size_t len = strlen(s);
    char* out = (char*)malloc(len + extra + 1);
    if (!out) return NULL;
    char* w = out;
    for (const char* p = s; *p; ++p) {
        switch (*p) {
            case '&': memcpy(w, "&amp;", 5); w += 5; break;
            case '<': memcpy(w, "&lt;", 4);  w += 4; break;
            case '>': memcpy(w, "&gt;", 4);  w += 4; break;
            case '"': memcpy(w, "&quot;", 6);w += 6; break;
            case '\'':memcpy(w, "&#x27;", 6);w += 6; break;
            default: *w++ = *p; break;
        }
    }
    *w = '\0';
    return out;
}

char* hello(const char* username);

#ifdef __GNUC__
static void hello_register(void) __attribute__((constructor));
static void hello_register(void) { register_route("/hello", hello); }
#endif

char* hello(const char* username) {
    char* esc = escape(username);
    if (!esc) return my_strdup("500 Internal Server Error");
    const char* prefix = "Hello ";
    size_t out_len = strlen(prefix) + strlen(esc) + 1;
    char* out = (char*)malloc(out_len);
    if (!out) {
        free(esc);
        return my_strdup("500 Internal Server Error");
    }
    strcpy(out, prefix);
    strcat(out, esc);
    free(esc);
    return out;
}

char* handle_request(const char* url) {
    const char* prefix = "/hello/";
    size_t prelen = strlen(prefix);
    if (strncmp(url, prefix, prelen) == 0) {
        const char* username = url + prelen;
        for (int i = 0; i < route_count; ++i) {
            if (strcmp(routes[i].path, "/hello") == 0) {
                return routes[i].handler(username);
            }
        }
    }
    return my_strdup("404 Not Found");
}

int main(void) {
#ifndef __GNUC__
    register_route("/hello", hello);
#endif
    const char* tests[] = {
        "/hello/world",
        "/hello/Alice&Bob",
        "/hello/Eve<Admin>",
        "/hello/O'Reilly",
        "/hello/Tom \"The Tiger\""
    };
    for (int i = 0; i < 5; ++i) {
        char* resp = handle_request(tests[i]);
        if (resp) {
            printf("%s\n", resp);
            free(resp);
        }
    }
    return 0;
}