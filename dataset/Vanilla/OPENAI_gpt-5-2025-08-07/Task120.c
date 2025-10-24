#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* escape_html(const char* in) {
    if (!in) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    size_t len = strlen(in);
    size_t extra = 0;
    for (size_t i = 0; i < len; ++i) {
        switch (in[i]) {
            case '&': extra += 4; break;   /* &amp; => +4 */
            case '<': extra += 3; break;   /* &lt;  => +3 */
            case '>': extra += 3; break;   /* &gt;  => +3 */
            case '"': extra += 5; break;   /* &quot;=> +5 */
            case '\'': extra += 4; break;  /* &#39; => +4 */
            default: break;
        }
    }
    size_t out_len = len + extra;
    char* out = (char*)malloc(out_len + 1);
    if (!out) return NULL;

    size_t j = 0;
    for (size_t i = 0; i < len; ++i) {
        switch (in[i]) {
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
                out[j++] = in[i]; break;
        }
    }
    out[j] = '\0';
    return out;
}

char* display_on_web(const char* input) {
    const char* prefix = "<!doctype html><html><head><meta charset=\"utf-8\"><title>Echo</title></head><body><h1>Echo</h1><pre>";
    const char* suffix = "</pre></body></html>";
    char* escaped = escape_html(input ? input : "");
    if (!escaped) return NULL;

    size_t total = strlen(prefix) + strlen(escaped) + strlen(suffix);
    char* page = (char*)malloc(total + 1);
    if (!page) {
        free(escaped);
        return NULL;
    }
    strcpy(page, prefix);
    strcat(page, escaped);
    strcat(page, suffix);
    free(escaped);
    return page;
}

int main(void) {
    const char* tests[5] = {
        "Hello, world!",
        "<script>alert('xss')</script>",
        "Tom & Jerry",
        "Quotes: \"double\" and 'single'",
        "Line1\nLine2\tTabbed"
    };
    for (int i = 0; i < 5; ++i) {
        char* html = display_on_web(tests[i]);
        if (html) {
            printf("%s\n", html);
            free(html);
        }
    }
    return 0;
}