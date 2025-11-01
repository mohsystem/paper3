#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* html_escape(const char* s) {
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
    const char* raw = (username == NULL || username[0] == '\0') ? "Guest" : username;
    char* safe = html_escape(raw);
    if (!safe) return NULL;

    const char* prefix = "<!DOCTYPE html>\n<html><head><meta charset=\"UTF-8\"><title>Hello</title></head><body><h1>Hello, ";
    const char* suffix = "!</h1><p>Welcome to the hello page.</p></body></html>";

    size_t total = strlen(prefix) + strlen(safe) + strlen(suffix) + 1;
    char* out = (char*)malloc(total);
    if (!out) {
        free(safe);
        return NULL;
    }
    strcpy(out, prefix);
    strcat(out, safe);
    strcat(out, suffix);
    free(safe);
    return out;
}

int main(void) {
    const char* tests[5] = {"Alice", "Bob & Co", "<script>", "", NULL};
    for (int i = 0; i < 5; ++i) {
        char* html = http_hello_page(tests[i]);
        printf("=== Test ===\n");
        if (html) {
            printf("%s\n", html);
            free(html);
        } else {
            printf("Allocation failed\n");
        }
    }
    return 0;
}