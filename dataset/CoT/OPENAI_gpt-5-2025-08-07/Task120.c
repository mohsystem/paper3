/* 
Chain-of-Through Step 1: Problem understanding
- Return an HTML page safely embedding user input.

Step 2: Security requirements
- Escape special HTML characters to prevent XSS.
- Avoid buffer overflows via size calculation and checked allocation.
- Treat NULL input as empty.

Step 3: Secure coding generation
- Two-pass escaping for accurate allocation.
- Fixed HTML template with escaped content insertion.

Step 4: Code review
- Escapes &, <, >, ", ', /.
- malloc size checks and graceful failure.

Step 5: Secure code output
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* escape_html(const char* s) {
    if (s == NULL) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    // First pass: compute required length
    size_t len = 0;
    for (const char* p = s; *p; ++p) {
        switch (*p) {
            case '&': len += 5; break;   // &amp;
            case '<': len += 4; break;   // &lt;
            case '>': len += 4; break;   // &gt;
            case '"': len += 6; break;   // &quot;
            case '\'': len += 5; break;  // &#39;
            case '/': len += 5; break;   // &#47;
            default: len += 1; break;
        }
        // Basic guard against extreme sizes (optional)
        if (len > (size_t) (1024 * 1024 * 64)) { // 64MB safety cap
            return NULL;
        }
    }

    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;

    // Second pass: write escaped content
    char* w = out;
    for (const char* p = s; *p; ++p) {
        switch (*p) {
            case '&': memcpy(w, "&amp;", 5); w += 5; break;
            case '<': memcpy(w, "&lt;", 4); w += 4; break;
            case '>': memcpy(w, "&gt;", 4); w += 4; break;
            case '"': memcpy(w, "&quot;", 6); w += 6; break;
            case '\'': memcpy(w, "&#39;", 5); w += 5; break;
            case '/': memcpy(w, "&#47;", 5); w += 5; break;
            default: *w++ = *p; break;
        }
    }
    *w = '\0';
    return out;
}

char* display_user_input_on_web_page(const char* user_input) {
    const char* head = "<!doctype html><html lang=\"en\"><head>"
                       "<meta charset=\"utf-8\">"
                       "<meta http-equiv=\"Content-Security-Policy\" content=\"default-src 'none'; style-src 'self' 'unsafe-inline';\">"
                       "<title>Safe Echo</title></head><body>"
                       "<h1>User Input</h1><p>";
    const char* tail = "</p></body></html>";

    char* safe = escape_html(user_input);
    if (user_input && !safe) {
        // Allocation failed or guard triggered
        return NULL;
    }
    if (!safe) {
        // Treat as empty string if NULL and allocation failed above
        safe = (char*)malloc(1);
        if (!safe) return NULL;
        safe[0] = '\0';
    }

    size_t total = strlen(head) + strlen(safe) + strlen(tail);
    char* page = (char*)malloc(total + 1);
    if (!page) {
        free(safe);
        return NULL;
    }

    // Build final HTML
    memcpy(page, head, strlen(head));
    memcpy(page + strlen(head), safe, strlen(safe));
    memcpy(page + strlen(head) + strlen(safe), tail, strlen(tail));
    page[total] = '\0';

    free(safe);
    return page;
}

int main(void) {
    const char* tests[5] = {
        "Hello, world!",
        "<script>alert('xss')</script>",
        "Tom & Jerry > Spike",
        "\"quoted\" and 'single'",
        "こんにちは <b>世界</b> & emojis 😊"
    };

    for (int i = 0; i < 5; ++i) {
        char* page = display_user_input_on_web_page(tests[i]);
        printf("----- C Output -----\n");
        if (page) {
            printf("%s\n", page);
            free(page);
        } else {
            printf("Error generating page.\n");
        }
    }
    return 0;
}