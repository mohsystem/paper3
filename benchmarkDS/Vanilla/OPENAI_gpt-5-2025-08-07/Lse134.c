#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* escape_html(const char* s) {
    if (!s) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    size_t len = 0;
    for (const char* p = s; *p; ++p) {
        switch (*p) {
            case '&': len += 5; break;   /* &amp; */
            case '<': len += 4; break;   /* &lt; */
            case '>': len += 4; break;   /* &gt; */
            case '"': len += 6; break;   /* &quot; */
            case '\'': len += 6; break;  /* &#x27; */
            default: len += 1; break;
        }
    }
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    char* w = out;
    for (const char* p = s; *p; ++p) {
        switch (*p) {
            case '&': memcpy(w, "&amp;", 5); w += 5; break;
            case '<': memcpy(w, "&lt;", 4); w += 4; break;
            case '>': memcpy(w, "&gt;", 4); w += 4; break;
            case '"': memcpy(w, "&quot;", 6); w += 6; break;
            case '\'': memcpy(w, "&#x27;", 6); w += 6; break;
            default: *w++ = *p; break;
        }
    }
    *w = '\0';
    return out;
}

char* hello(const char* username) {
    char* esc = escape_html(username);
    if (!esc) return NULL;
    const char* prefix = "Hello ";
    size_t total = strlen(prefix) + strlen(esc);
    char* res = (char*)malloc(total + 1);
    if (!res) {
        free(esc);
        return NULL;
    }
    strcpy(res, prefix);
    strcat(res, esc);
    free(esc);
    return res;
}

static char* str_dup_local(const char* s) {
    size_t n = strlen(s);
    char* d = (char*)malloc(n + 1);
    if (d) memcpy(d, s, n + 1);
    return d;
}

char* handle_request(const char* url) {
    const char* prefix = "/hello/";
    size_t prelen = strlen(prefix);
    if (strncmp(url, prefix, prelen) == 0) {
        const char* username = url + prelen;
        return hello(username);
    }
    return str_dup_local("404 Not Found");
}

int main(void) {
    const char* tests[5] = {
        "/hello/Alice",
        "/hello/Bob & Co",
        "/hello/User<Name>",
        "/hello/Quote\"User'",
        "/hello/Greater>Less<"
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