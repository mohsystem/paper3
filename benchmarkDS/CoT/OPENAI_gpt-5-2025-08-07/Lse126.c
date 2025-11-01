#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* escape_html(const char* s) {
    if (!s) {
        char* w = (char*)malloc(6);
        if (w) strcpy(w, "World");
        return w;
    }
    size_t len = 0;
    for (const char* p = s; *p; ++p) {
        switch (*p) {
            case '&': len += 5; break;      // &amp;
            case '<': len += 4; break;      // &lt;
            case '>': len += 4; break;      // &gt;
            case '"': len += 6; break;      // &quot;
            case '\'': len += 5; break;     // &#39;
            default: len += 1; break;
        }
    }
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    char* q = out;
    for (const char* p = s; *p; ++p) {
        switch (*p) {
            case '&': memcpy(q, "&amp;", 5); q += 5; break;
            case '<': memcpy(q, "&lt;", 4); q += 4; break;
            case '>': memcpy(q, "&gt;", 4); q += 4; break;
            case '"': memcpy(q, "&quot;", 6); q += 6; break;
            case '\'': memcpy(q, "&#39;", 5); q += 5; break;
            default: *q++ = *p; break;
        }
    }
    *q = '\0';
    return out;
}

char* http_hello_page(const char* username) {
    const char* fallback = "World";
    int use_fallback = (username == NULL || username[0] == '\0');
    char* safe = use_fallback ? NULL : escape_html(username);
    if (use_fallback) {
        safe = escape_html(fallback);
    }
    if (!safe) return NULL;

    const char* prefix = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>Hello</title></head><body><h1>Hello, ";
    const char* suffix = "!</h1></body></html>";
    size_t total_len = strlen(prefix) + strlen(safe) + strlen(suffix);
    char* body = (char*)malloc(total_len + 1);
    if (!body) {
        free(safe);
        return NULL;
    }
    strcpy(body, prefix);
    strcat(body, safe);
    strcat(body, suffix);

    // Simulate writing to response stream by copying to a buffer that we then return
    // In real server code, you'd write to a socket/FILE* and then close it.
    free(safe);
    return body;
}

int main(void) {
    const char* tests[5] = {
        "Alice",
        "",
        "Bob & Co",
        "<script>alert(1)</script>",
        "O'Neil"
    };
    for (int i = 0; i < 5; ++i) {
        char* resp = http_hello_page(tests[i]);
        if (resp) {
            printf("%s\n", resp);
            free(resp);
        }
    }
    return 0;
}