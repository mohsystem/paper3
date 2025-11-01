#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static char* escape_html(const char* input) {
    if (!input) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    // First pass: compute required length
    size_t len = 0;
    for (const char* p = input; *p; ++p) {
        switch (*p) {
            case '&': len += 5; break;      // &amp;
            case '<': len += 4; break;      // &lt;
            case '>': len += 4; break;      // &gt;
            case '"': len += 6; break;      // &quot;
            case '\'': len += 5; break;     // &#39;
            default: len += 1; break;
        }
        if (len > SIZE_MAX / 2) { // guard overflow
            return NULL;
        }
    }
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    char* w = out;
    for (const char* p = input; *p; ++p) {
        switch (*p) {
            case '&': memcpy(w, "&amp;", 5); w += 5; break;
            case '<': memcpy(w, "&lt;", 4); w += 4; break;
            case '>': memcpy(w, "&gt;", 4); w += 4; break;
            case '"': memcpy(w, "&quot;", 6); w += 6; break;
            case '\'': memcpy(w, "&#39;", 5); w += 5; break;
            default: *w++ = *p; break;
        }
    }
    *w = '\0';
    return out;
}

static char* hello(const char* username) {
    char* escaped = escape_html(username ? username : "");
    if (!escaped) return NULL;
    const char* prefix = "Hello ";
    size_t total = strlen(prefix) + strlen(escaped);
    char* out = (char*)malloc(total + 1);
    if (!out) {
        free(escaped);
        return NULL;
    }
    memcpy(out, prefix, strlen(prefix));
    memcpy(out + strlen(prefix), escaped, strlen(escaped));
    out[total] = '\0';
    free(escaped);
    return out;
}

static bool starts_with(const char* s, const char* prefix) {
    if (!s || !prefix) return false;
    size_t lp = strlen(prefix);
    return strncmp(s, prefix, lp) == 0;
}

// Returns a newly allocated string result or NULL; caller must free if not NULL
static char* navigate(const char* url) {
    const char* route_prefix = "/hello/";
    if (!url) return NULL;
    if (starts_with(url, route_prefix)) {
        const char* username = url + strlen(route_prefix);
        // username can be empty; pass as is
        return hello(username);
    }
    return NULL;
}

int main(void) {
    const char* tests[5] = {
        "/hello/World",
        "/hello/<script>alert(1)</script>",
        "/hello/O'Connor & Sons",
        "/hello/Tom \"T\"",
        "/hello/../path?x=1&y=2"
    };
    for (int i = 0; i < 5; ++i) {
        char* res = navigate(tests[i]);
        if (res) {
            printf("%s\n", res);
            free(res);
        } else {
            printf("404 Not Found\n");
        }
    }
    return 0;
}