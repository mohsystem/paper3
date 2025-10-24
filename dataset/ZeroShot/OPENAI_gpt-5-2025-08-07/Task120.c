#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *HTML_PREFIX =
"<!DOCTYPE html>\n"
"<html lang=\"en\">\n"
"<head>\n"
"<meta charset=\"UTF-8\">\n"
"<meta http-equiv=\"Content-Security-Policy\" content=\"default-src 'none';\">\n"
"<title>Safe Echo</title>\n"
"</head>\n"
"<body>\n"
"<h1>Echo</h1>\n"
"<pre>";

static const char *HTML_SUFFIX =
"</pre>\n"
"</body>\n"
"</html>\n";

static size_t escaped_length(const char *s) {
    if (s == NULL) return 0;
    size_t len = 0;
    for (const unsigned char *p = (const unsigned char *)s; *p; ++p) {
        switch (*p) {
            case '&': len += 5; break;      // &amp;
            case '<': len += 4; break;      // &lt;
            case '>': len += 4; break;      // &gt;
            case '"': len += 6; break;      // &quot;
            case '\'': len += 5; break;     // &#x27;
            case '/': len += 6; break;      // &#x2F;
            default: len += 1; break;
        }
    }
    return len;
}

static void write_escaped(char *dst, const char *s) {
    if (s == NULL) return;
    while (*s) {
        switch (*s) {
            case '&': memcpy(dst, "&amp;", 5); dst += 5; break;
            case '<': memcpy(dst, "&lt;", 4); dst += 4; break;
            case '>': memcpy(dst, "&gt;", 4); dst += 4; break;
            case '"': memcpy(dst, "&quot;", 6); dst += 6; break;
            case '\'': memcpy(dst, "&#x27;", 5); dst += 5; break;
            case '/': memcpy(dst, "&#x2F;", 6); dst += 6; break;
            default: *dst++ = *s; break;
        }
        s++;
    }
}

char* display_user_input(const char *input) {
    size_t esc_len = escaped_length(input);
    size_t prefix_len = strlen(HTML_PREFIX);
    size_t suffix_len = strlen(HTML_SUFFIX);

    size_t total = prefix_len + esc_len + suffix_len;
    char *out = (char *)malloc(total + 1);
    if (!out) {
        return NULL;
    }

    char *ptr = out;
    memcpy(ptr, HTML_PREFIX, prefix_len); ptr += prefix_len;
    write_escaped(ptr, input); ptr += esc_len;
    memcpy(ptr, HTML_SUFFIX, suffix_len); ptr += suffix_len;
    *ptr = '\0';
    return out;
}

int main(void) {
    const char *tests[5] = {
        "Hello, world!",
        "<script>alert('XSS');</script>",
        "Tom & Jerry > Mickey & Minnie",
        "\"Quotes\" and 'single quotes' and /slashes/",
        "Unicode: Привет, 你好, مرحبا, 😀"
    };

    for (int i = 0; i < 5; ++i) {
        char *html = display_user_input(tests[i]);
        printf("----- Test %d -----\n", i + 1);
        if (html) {
            printf("%s\n", html);
            free(html);
        } else {
            printf("Error: memory allocation failed\n");
        }
    }
    return 0;
}