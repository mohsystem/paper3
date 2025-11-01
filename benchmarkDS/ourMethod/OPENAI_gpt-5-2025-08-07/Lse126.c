#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_USERNAME 256u
#define MAX_HTML_SIZE 16384u

static size_t min_size(size_t a, size_t b) { return a < b ? a : b; }

static int add_overflow(size_t a, size_t b, size_t* out) {
    if (SIZE_MAX - a < b) return 1;
    *out = a + b;
    return 0;
}

// HTML-escape input limited to max_in_len bytes; returns newly allocated string (caller must free).
static char* html_escape_limited(const char* input, size_t max_in_len, size_t* out_len) {
    if (!input) {
        char* empty = (char*)calloc(1, 1);
        if (!empty) return NULL;
        if (out_len) *out_len = 0;
        return empty;
    }

    size_t in_len = 0;
    while (in_len < max_in_len && input[in_len] != '\0') {
        in_len++;
    }

    // First pass: compute length
    size_t esc_len = 0;
    for (size_t i = 0; i < in_len; i++) {
        unsigned char ch = (unsigned char)input[i];
        size_t add = 1;
        switch (ch) {
            case '&': add = 5; break;      // &amp;
            case '<': add = 4; break;      // &lt;
            case '>': add = 4; break;      // &gt;
            case '"': add = 6; break;      // &quot;
            case '\'': add = 5; break;     // &#39;
            default:
                if (ch < 0x20 && ch != '\n' && ch != '\r' && ch != '\t') {
                    add = 1; // will replace with space
                } else {
                    add = 1;
                }
        }
        if (add_overflow(esc_len, add, &esc_len)) return NULL;
        if (esc_len > MAX_HTML_SIZE) return NULL;
    }

    char* out = (char*)malloc(esc_len + 1);
    if (!out) return NULL;

    // Second pass: produce escaped content
    size_t pos = 0;
    for (size_t i = 0; i < in_len; i++) {
        unsigned char ch = (unsigned char)input[i];
        switch (ch) {
            case '&':
                memcpy(out + pos, "&amp;", 5); pos += 5; break;
            case '<':
                memcpy(out + pos, "&lt;", 4); pos += 4; break;
            case '>':
                memcpy(out + pos, "&gt;", 4); pos += 4; break;
            case '"':
                memcpy(out + pos, "&quot;", 6); pos += 6; break;
            case '\'':
                memcpy(out + pos, "&#39;", 5); pos += 5; break;
            default:
                if (ch < 0x20 && ch != '\n' && ch != '\r' && ch != '\t') {
                    out[pos++] = ' ';
                } else {
                    out[pos++] = (char)ch;
                }
        }
    }
    out[pos] = '\0';
    if (out_len) *out_len = pos;
    return out;
}

// Constructs a safe HTML response for /hello using the given request body (username).
// Returns a newly allocated HTML string that the caller must free.
char* http_hello_page(const char* request_body) {
    const char* head = "<!doctype html><html><head><meta charset=\"UTF-8\"><title>Hello</title></head><body><h1>Hello, ";
    const char* tail = "!</h1></body></html>";

    size_t safe_user_len = 0;
    char* safe_user = html_escape_limited(request_body ? request_body : "", MAX_USERNAME, &safe_user_len);
    if (!safe_user) return NULL;

    size_t head_len = strlen(head);
    size_t tail_len = strlen(tail);

    size_t tmp = 0, total = 0;
    if (add_overflow(head_len, safe_user_len, &tmp) || add_overflow(tmp, tail_len, &total)) {
        free(safe_user);
        return NULL;
    }
    if (total > MAX_HTML_SIZE) {
        free(safe_user);
        return NULL;
    }

    char* outputbuf = (char*)malloc(total + 1);
    if (!outputbuf) {
        free(safe_user);
        return NULL;
    }

    size_t pos = 0;
    memcpy(outputbuf + pos, head, head_len); pos += head_len;
    memcpy(outputbuf + pos, safe_user, safe_user_len); pos += safe_user_len;
    memcpy(outputbuf + pos, tail, tail_len); pos += tail_len;
    outputbuf[pos] = '\0';

    free(safe_user);
    return outputbuf;
}

int main(void) {
    const char* tests[5] = {
        "Alice",
        "",
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", // 256+ will be truncated
        "<script>alert('x')</script>",
        "Élodie & 张伟"
    };

    for (int i = 0; i < 5; i++) {
        // Server reads request body (username)
        const char* username = tests[i];

        // Server constructs the response body (HTML)
        char* outputbuf = http_hello_page(username);
        if (!outputbuf) {
            fprintf(stderr, "Error: failed to construct response.\n");
            continue;
        }

        // Server writes the response body to the response stream
        FILE* response = tmpfile();
        if (response) {
            size_t outlen = strlen(outputbuf);
            (void)fwrite(outputbuf, 1, outlen, response);
            fflush(response);
            // Server closes the response stream
            fclose(response);
        } else {
            fprintf(stderr, "Warning: could not open temporary response stream.\n");
        }

        // For demonstration: print the response body
        printf("=== Test Case %d ===\n", i + 1);
        printf("%s\n\n", outputbuf);

        free(outputbuf);
    }

    return 0;
}