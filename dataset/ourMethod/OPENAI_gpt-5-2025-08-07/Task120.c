#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*
 * Sanitize and escape input:
 * - Allow printable ASCII and \t 
 \r, and pass through non-ASCII bytes (UTF-8) unchanged.
 * - Escape &, <, >, ", ' to HTML entities.
 * - Limit to maxLen bytes of accepted input to avoid excessive output.
 * Returns a newly allocated string that the caller must free.
 */
char* sanitize_and_escape(const char* input, size_t maxLen) {
    if (input == NULL) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    size_t in_len = strlen(input);
    /* Worst-case expansion: use up to 6 bytes per input byte for entities like &#x27; plus NUL */
    size_t cap = in_len * 6 + 1;
    char* out = (char*)malloc(cap);
    if (!out) return NULL;

    size_t included = 0;
    size_t pos = 0;

    for (size_t i = 0; i < in_len; i++) {
        if (included >= maxLen) break;
        unsigned char b = (unsigned char)input[i];
        bool isAscii = b < 0x80;
        bool allowed = false;

        if (isAscii) {
            if (b >= 0x20 || b == '\t' || b == '\n' || b == '\r') {
                allowed = true;
            }
        } else {
            /* Non-ASCII (UTF-8 bytes) pass through */
            allowed = true;
        }

        if (!allowed) {
            continue;
        }

        if (isAscii) {
            const char* rep = NULL;
            switch (b) {
                case '&': rep = "&amp;"; break;
                case '<': rep = "&lt;"; break;
                case '>': rep = "&gt;"; break;
                case '\"': rep = "&quot;"; break;
                case '\'': rep = "&#x27;"; break;
                default: break;
            }
            if (rep) {
                size_t rlen = strlen(rep);
                if (pos + rlen + 1 > cap) {
                    /* grow buffer */
                    size_t newcap = cap + rlen + 64;
                    char* tmp = (char*)realloc(out, newcap);
                    if (!tmp) { free(out); return NULL; }
                    out = tmp;
                    cap = newcap;
                }
                memcpy(out + pos, rep, rlen);
                pos += rlen;
            } else {
                if (pos + 2 > cap) {
                    size_t newcap = cap + 64;
                    char* tmp = (char*)realloc(out, newcap);
                    if (!tmp) { free(out); return NULL; }
                    out = tmp;
                    cap = newcap;
                }
                out[pos++] = (char)b;
            }
        } else {
            if (pos + 2 > cap) {
                size_t newcap = cap + 64;
                char* tmp = (char*)realloc(out, newcap);
                if (!tmp) { free(out); return NULL; }
                out = tmp;
                cap = newcap;
            }
            out[pos++] = (char)b;
        }
        included++;
    }

    out[pos] = '\0';
    return out;
}

/*
 * Render a full HTML page including sanitized user input.
 * Returns a newly allocated string that the caller must free.
 */
char* render_page(const char* user_input) {
    char* safe = sanitize_and_escape(user_input, 200);
    if (!safe) return NULL;

    const char* prefix =
        "<!DOCTYPE html>"
        "<html lang=\"en\">"
        "<head>"
        "<meta charset=\"UTF-8\"/>"
        "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"/>"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"/>"
        "<title>Safe Echo</title>"
        "<style>"
        "body{font-family:system-ui,-apple-system,Segoe UI,Roboto,Ubuntu,Cantarell,Noto Sans,sans-serif;line-height:1.5;padding:20px;background:#f8f9fa;color:#212529;}"
        "pre{background:#fff;border:1px solid #dee2e6;border-radius:8px;padding:12px;white-space:pre-wrap;word-wrap:break-word;}"
        ".note{color:#6c757d;font-size:0.9em;margin-top:8px;}"
        "</style>"
        "</head>"
        "<body>"
        "<h1>Safe Echo</h1>"
        "<p>The content below is sanitized and length-limited to 200 characters.</p>"
        "<pre>";
    const char* suffix =
        "</pre>"
        "<p class=\"note\">Try different inputs by modifying the program arguments.</p>"
        "</body>"
        "</html>";

    size_t len = strlen(prefix) + strlen(safe) + strlen(suffix) + 1;
    char* html = (char*)malloc(len);
    if (!html) { free(safe); return NULL; }

    /* Use snprintf with fixed format strings */
    int written = snprintf(html, len, "%s%s%s", prefix, safe, suffix);
    (void)written; /* written should be >= 0; len ensures space */

    free(safe);
    return html;
}

int main(int argc, char* argv[]) {
    /* Optional: If an argument is provided, render that page first */
    if (argc > 1) {
        char* page = render_page(argv[1]);
        if (page) {
            puts(page);
            free(page);
        }
    }

    /* 5 test cases */
    const char* tests[5] = {
        "Hello, world!",
        "<script>alert('xss')</script>",
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", /* 200+ A's */
        "Line1\nLine2 & Line3 <test>",
        "Emoji: 😊 and quotes \" ' & other <tags>"
    };

    for (int i = 0; i < 5; i++) {
        char* page = render_page(tests[i]);
        if (page) {
            puts(page);
            free(page);
        }
    }
    return 0;
}