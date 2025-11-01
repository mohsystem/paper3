#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_INPUT_BYTES 1024
#define MAX_USERNAME_CHARS 100

static int is_allowed_char(unsigned char c) {
    if (c >= 'a' && c <= 'z') return 1;
    if (c >= 'A' && c <= 'Z') return 1;
    if (c >= '0' && c <= '9') return 1;
    if (c == ' ' || c == '_' || c == '-' || c == '.' || c == '@') return 1;
    return 0;
}

static char* sanitize_username(const char* s) {
    if (s == NULL) {
        char* anon = (char*)malloc(10);
        if (anon) strcpy(anon, "anonymous");
        return anon;
    }
    char tmp[MAX_USERNAME_CHARS + 1];
    size_t out_idx = 0;
    for (size_t i = 0; s[i] != '\0' && out_idx < MAX_USERNAME_CHARS; i++) {
        unsigned char c = (unsigned char)s[i];
        if (c == '\r' || c == '\n') continue;
        if (is_allowed_char(c)) {
            tmp[out_idx++] = (char)c;
        }
    }
    tmp[out_idx] = '\0';

    // trim spaces
    size_t start = 0;
    while (tmp[start] == ' ' && tmp[start] != '\0') start++;
    size_t end = strlen(tmp);
    while (end > start && tmp[end - 1] == ' ') end--;
    size_t len = (end > start) ? (end - start) : 0;

    if (len == 0) {
        char* anon = (char*)malloc(10);
        if (anon) strcpy(anon, "anonymous");
        return anon;
    }

    char* res = (char*)malloc(len + 1);
    if (!res) return NULL;
    memcpy(res, tmp + start, len);
    res[len] = '\0';
    return res;
}

// Returns a heap-allocated string with the response. Caller must free.
char* http_hello_page(FILE* request, FILE* response) {
    if (request == NULL || response == NULL) {
        return NULL;
    }

    char* inbuf = (char*)malloc(MAX_INPUT_BYTES + 1);
    if (!inbuf) return NULL;

    size_t total = fread(inbuf, 1, MAX_INPUT_BYTES, request);
    inbuf[total] = '\0';

    char* username = sanitize_username(inbuf);
    free(inbuf);
    if (!username) return NULL;

    const char* prefix = "hello ";
    size_t prefix_len = 6;
    size_t uname_len = strlen(username);

    if (uname_len > SIZE_MAX - prefix_len - 1) {
        free(username);
        return NULL;
    }

    size_t out_len = prefix_len + uname_len;
    char* out = (char*)malloc(out_len + 1);
    if (!out) {
        free(username);
        return NULL;
    }

    memcpy(out, prefix, prefix_len);
    memcpy(out + prefix_len, username, uname_len);
    out[out_len] = '\0';

    size_t written = fwrite(out, 1, out_len, response);
    (void)written; // ignore in this context
    fflush(response);

    free(username);
    return out;
}

// 5 test cases
int main(void) {
    const char* tests[5] = {
        "Alice",
        "Bob<script>alert(1)</script>",
        "Carol\r\nSet-Cookie: session=evil",
        "Δelta-用户",
        "Dave ../../etc/passwd\t\n"
    };

    for (int i = 0; i < 5; ++i) {
        FILE* req = tmpfile();
        FILE* resp = tmpfile();
        if (!req || !resp) {
            if (req) fclose(req);
            if (resp) fclose(resp);
            fprintf(stderr, "Failed to create temporary files\n");
            return 1;
        }

        fputs(tests[i], req);
        fflush(req);
        rewind(req);

        char* result = http_hello_page(req, resp);
        if (!result) {
            fprintf(stderr, "http_hello_page failed\n");
            fclose(req);
            fclose(resp);
            continue;
        }

        rewind(resp);
        char outbuf[256];
        size_t r = fread(outbuf, 1, sizeof(outbuf) - 1, resp);
        outbuf[r] = '\0';

        printf("Test %d: %s | ResponseStream: %s\n", i + 1, result, outbuf);

        free(result);
        fclose(req);
        fclose(resp);
    }

    return 0;
}