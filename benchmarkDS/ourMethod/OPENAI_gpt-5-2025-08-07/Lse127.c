#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INPUT_BYTES 2048
#define MAX_USERNAME_CHARS 64

// Returns a heap-allocated string with the response; caller must free.
// On failure, returns NULL.
char* http_hello_page(FILE* request, FILE* response) {
    if (request == NULL || response == NULL) {
        return NULL;
    }

    // Read up to first newline or MAX_INPUT_BYTES
    char raw_buf[MAX_INPUT_BYTES + 1];
    size_t raw_len = 0;
    int ch;
    while (raw_len < MAX_INPUT_BYTES) {
        ch = fgetc(request);
        if (ch == EOF) break;
        if (ch == '\n' || ch == '\r') break;
        raw_buf[raw_len++] = (char)ch;
    }
    raw_buf[raw_len] = '\0';

    // Sanitize: allow [A-Za-z0-9 _.-'], collapse spaces, trim, and limit length
    char clean[MAX_USERNAME_CHARS + 1];
    size_t clen = 0;
    int lastWasSpace = 1; // to trim leading space
    for (size_t i = 0; i < raw_len && clen < MAX_USERNAME_CHARS; i++) {
        unsigned char c = (unsigned char)raw_buf[i];
        int allowed = (c >= 'A' && c <= 'Z') ||
                      (c >= 'a' && c <= 'z') ||
                      (c >= '0' && c <= '9') ||
                      c == '_' || c == '-' || c == '.' || c == '\'';
        if (allowed) {
            clean[clen++] = (char)c;
            lastWasSpace = 0;
        } else if (isspace(c)) {
            if (!lastWasSpace && clen < MAX_USERNAME_CHARS) {
                clean[clen++] = ' ';
            }
            lastWasSpace = 1;
        } else {
            // drop disallowed
        }
    }
    // Trim trailing space
    if (clen > 0 && clean[clen - 1] == ' ') {
        clen--;
    }
    clean[clen] = '\0';
    if (clen == 0) {
        strncpy(clean, "user", sizeof(clean) - 1);
        clean[sizeof(clean) - 1] = '\0';
        clen = strlen(clean);
    }

    // Compose response: "Hello " + username
    const char* prefix = "Hello ";
    size_t prefix_len = strlen(prefix);

    // Check for overflow; MAX_USERNAME_CHARS prevents this
    if (prefix_len > SIZE_MAX - (clen + 1)) {
        return NULL;
    }
    size_t total_len = prefix_len + clen; // without null
    char* responseBody = (char*)malloc(total_len + 1);
    if (!responseBody) {
        return NULL;
    }
    memcpy(responseBody, prefix, prefix_len);
    memcpy(responseBody + prefix_len, clean, clen);
    responseBody[total_len] = '\0';

    // Write to response
    if (total_len > 0) {
        size_t wrote = fwrite(responseBody, 1, total_len, response);
        (void)wrote; // ignore partial write in this context
        fflush(response);
    }

    return responseBody;
}

static void run_test(const char* input) {
    FILE* req = tmpfile();
    FILE* resp = tmpfile();
    if (!req || !resp) {
        fprintf(stderr, "Failed to create temp files\n");
        if (req) fclose(req);
        if (resp) fclose(resp);
        return;
    }

    if (input && strlen(input) > 0) {
        fwrite(input, 1, strlen(input), req);
    }
    fflush(req);
    rewind(req);

    char* result = http_hello_page(req, resp);

    fflush(resp);
    rewind(resp);

    char buffer[256];
    size_t n;
    printf("Returned: %s\n", result ? result : "(null)");

    printf("Written:  ");
    while ((n = fread(buffer, 1, sizeof(buffer), resp)) > 0) {
        fwrite(buffer, 1, n, stdout);
    }
    printf("\n---\n");

    free(result);
    fclose(req);
    fclose(resp);
}

int main(void) {
    run_test("Alice");
    run_test("  Bob   Smith  \r\n");
    run_test("Eve<script>alert(1)</script>");
    run_test("");
    char longInput[501];
    memset(longInput, 'X', 500);
    longInput[500] = '\0';
    run_test(longInput);
    return 0;
}