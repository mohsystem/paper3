#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static const char* HELLO_HTML =
    "<!doctype html>\n"
    "<html lang=\"en\">\n"
    "<head>\n"
    "  <meta charset=\"utf-8\">\n"
    "  <title>Hello</title>\n"
    "</head>\n"
    "<body>\n"
    "  <h1>Hello, {{username}}</h1>\n"
    "</body>\n"
    "</html>\n";

static char* html_escape(const char* s) {
    if (!s) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    size_t len = strlen(s);
    size_t out_len = 0;
    for (size_t i = 0; i < len; i++) {
        switch (s[i]) {
            case '&': out_len += 5; break;     // &amp;
            case '<': out_len += 4; break;     // &lt;
            case '>': out_len += 4; break;     // &gt;
            case '"': out_len += 6; break;     // &quot;
            case '\'': out_len += 5; break;    // &#39;
            default: out_len += 1; break;
        }
        if (out_len > SIZE_MAX / 2) { // rudimentary overflow guard
            return NULL;
        }
    }
    char* out = (char*)malloc(out_len + 1);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        switch (s[i]) {
            case '&':
                memcpy(out + j, "&amp;", 5); j += 5; break;
            case '<':
                memcpy(out + j, "&lt;", 4); j += 4; break;
            case '>':
                memcpy(out + j, "&gt;", 4); j += 4; break;
            case '"':
                memcpy(out + j, "&quot;", 6); j += 6; break;
            case '\'':
                memcpy(out + j, "&#39;", 5); j += 5; break;
            default:
                out[j++] = s[i]; break;
        }
    }
    out[j] = '\0';
    return out;
}

char* render_hello(const char* username) {
    const char* placeholder = "{{username}}";
    size_t tpl_len = strlen(HELLO_HTML);
    size_t ph_len = strlen(placeholder);

    char* safe = html_escape(username ? username : "");
    if (!safe) return NULL;

    const char* pos = strstr(HELLO_HTML, placeholder);
    if (!pos) {
        // No placeholder; return copy of template
        char* out = (char*)malloc(tpl_len + 1);
        if (!out) { free(safe); return NULL; }
        memcpy(out, HELLO_HTML, tpl_len + 1);
        free(safe);
        return out;
    }

    size_t prefix_len = (size_t)(pos - HELLO_HTML);
    size_t safe_len = strlen(safe);
    size_t suffix_len = tpl_len - prefix_len - ph_len;

    // Check for overflow
    if (prefix_len > SIZE_MAX - safe_len) { free(safe); return NULL; }
    size_t tmp = prefix_len + safe_len;
    if (tmp > SIZE_MAX - suffix_len) { free(safe); return NULL; }
    size_t out_len = tmp + suffix_len;

    char* out = (char*)malloc(out_len + 1);
    if (!out) { free(safe); return NULL; }

    memcpy(out, HELLO_HTML, prefix_len);
    memcpy(out + prefix_len, safe, safe_len);
    memcpy(out + prefix_len + safe_len, pos + ph_len, suffix_len);
    out[out_len] = '\0';

    free(safe);
    return out;
}

int main(void) {
    const char* tests[5] = {
        "Alice",
        "Bob",
        "Eve<script>",
        "李雷",
        "O'Reilly & Co"
    };
    for (int i = 0; i < 5; i++) {
        char* rendered = render_hello(tests[i]);
        if (rendered) {
            printf("%s\n----\n", rendered);
            free(rendered);
        } else {
            fprintf(stderr, "Rendering failed for test %d\n", i);
        }
    }
    return 0;
}