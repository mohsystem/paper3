#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char* escape_html_ascii(const char* in) {
    size_t len = strlen(in);
    size_t out_len = 0;
    for (size_t i = 0; i < len; ++i) {
        char c = in[i];
        switch (c) {
            case '&': out_len += 5; break;      // &amp;
            case '<': out_len += 4; break;      // &lt;
            case '>': out_len += 4; break;      // &gt;
            case '"': out_len += 6; break;      // &quot;
            case '\'': out_len += 6; break;     // &#x27;
            default: out_len += 1; break;
        }
    }
    char* out = (char*)malloc(out_len + 1);
    if (!out) return NULL;
    size_t pos = 0;
    for (size_t i = 0; i < len; ++i) {
        char c = in[i];
        const char* rep = NULL;
        switch (c) {
            case '&': rep = "&amp;"; break;
            case '<': rep = "&lt;"; break;
            case '>': rep = "&gt;"; break;
            case '"': rep = "&quot;"; break;
            case '\'': rep = "&#x27;"; break;
            default:
                out[pos++] = c;
                continue;
        }
        size_t rlen = strlen(rep);
        memcpy(out + pos, rep, rlen);
        pos += rlen;
    }
    out[pos] = '\0';
    return out;
}

char* http_hello_page(const char* requestBody) {
    const char* fallback = "Guest";
    const char* input = requestBody ? requestBody : fallback;

    char sanitized[257];
    size_t sidx = 0;
    for (size_t i = 0; input[i] != '\0' && sidx < 256; ++i) {
        unsigned char ch = (unsigned char)input[i];
        if (ch >= 0x20 && ch <= 0x7E && ch != 0x7F) {
            sanitized[sidx++] = (char)ch;
        }
    }
    sanitized[sidx] = '\0';

    char* escaped = escape_html_ascii(sanitized);
    if (!escaped) return NULL;

    const char* prefix =
        "<!doctype html>\n"
        "<html lang=\"en\">\n"
        "<head>\n"
        "<meta charset=\"utf-8\">\n"
        "<title>Hello</title>\n"
        "</head>\n"
        "<body>\n"
        "<h1>Hello, ";
    const char* suffix =
        "!</h1>\n"
        "</body>\n"
        "</html>\n";

    size_t total_len = strlen(prefix) + strlen(escaped) + strlen(suffix);
    char* outputbuf = (char*)malloc(total_len + 1);
    if (!outputbuf) {
        free(escaped);
        return NULL;
    }

    // "Write" to response buffer
    strcpy(outputbuf, prefix);
    strcat(outputbuf, escaped);
    strcat(outputbuf, suffix);

    free(escaped);
    // Simulate closing the response stream by returning the completed buffer
    return outputbuf;
}

int main(void) {
    const char* tests[5] = {
        "Alice",
        "<script>alert(1)</script>",
        "Bob_the_Builder_Bob_the_Builder_Bob_the_Builder_Bob_the_Builder_Bob_the_Builder_Bob_the_Builder_",
        NULL,
        "Eve\t\r\n\x7F"
    };
    for (int i = 0; i < 5; ++i) {
        char* res = http_hello_page(tests[i]);
        if (res) {
            printf("Test %d:\n%s\n\n", i + 1, res);
            free(res);
        } else {
            printf("Test %d:\n<error>\n\n", i + 1);
        }
    }
    return 0;
}