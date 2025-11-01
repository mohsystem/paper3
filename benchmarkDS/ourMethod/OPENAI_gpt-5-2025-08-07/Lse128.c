#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_INPUT_BYTES 4096
#define MAX_USERNAME_LEN 64

static void sanitize_username(const char *in, size_t in_len, char *out, size_t out_size) {
    size_t j = 0;
    if (out_size == 0) return;
    for (size_t i = 0; i < in_len && j + 1 < out_size; i++) {
        unsigned char c = (unsigned char)in[i];
        if (c >= 0x20 && c <= 0x7E) { /* printable ASCII */
            if (isalnum(c) || c == ' ' || c == '_' || c == '-' || c == '.') {
                out[j++] = (char)c;
                if (j >= MAX_USERNAME_LEN) break;
            }
        }
    }
    /* Trim spaces */
    size_t start = 0;
    while (start < j && out[start] == ' ') start++;
    size_t end = j;
    while (end > start && out[end - 1] == ' ') end--;
    if (start > 0 && end > start) {
        memmove(out, out + start, end - start);
        j = end - start;
    } else {
        j = (end > start) ? (end - start) : 0;
    }
    out[j] = '\0';
}

void http_hello_page(FILE *request, FILE *response) {
    if (request == NULL || response == NULL) {
        return;
    }
    char inbuf[MAX_INPUT_BYTES + 1];
    size_t total = 0;
    while (total < MAX_INPUT_BYTES) {
        size_t r = fread(inbuf + total, 1, MAX_INPUT_BYTES - total, request);
        total += r;
        if (r == 0) break;
        if (feof(request)) break;
        if (ferror(request)) break;
    }
    inbuf[total] = '\0';

    /* Use only first line (stop at CR or LF) */
    size_t first_len = 0;
    for (; first_len < total; first_len++) {
        if (inbuf[first_len] == '\r' || inbuf[first_len] == '\n') break;
    }

    char username[MAX_USERNAME_LEN + 1];
    sanitize_username(inbuf, first_len, username, sizeof(username));
    if (username[0] == '\0') {
        strncpy(username, "user", sizeof(username) - 1);
        username[sizeof(username) - 1] = '\0';
    }

    char outbuf[6 + MAX_USERNAME_LEN + 1]; /* "hello " + username + NUL */
    int n = snprintf(outbuf, sizeof(outbuf), "hello %s", username);
    if (n < 0) return;

    size_t to_write = (size_t)n;
    size_t written = 0;
    while (written < to_write) {
        size_t w = fwrite(outbuf + written, 1, to_write - written, response);
        if (w == 0) break;
        written += w;
    }
    fflush(response);
}

static void run_test(const char *input) {
    FILE *req = tmpfile();
    FILE *resp = tmpfile();
    if (!req || !resp) {
        if (req) fclose(req);
        if (resp) fclose(resp);
        fprintf(stderr, "Failed to create temp files\n");
        return;
    }

    if (input && *input) {
        size_t len = strlen(input);
        if (fwrite(input, 1, len, req) != len) {
            fprintf(stderr, "Failed to write to request\n");
        }
    }
    fflush(req);
    fseek(req, 0, SEEK_SET);

    http_hello_page(req, resp);

    fflush(resp);
    fseek(resp, 0, SEEK_SET);

    char readbuf[256];
    size_t r = fread(readbuf, 1, sizeof(readbuf) - 1, resp);
    readbuf[r] = '\0';
    printf("%s\n", readbuf);

    fclose(req);
    fclose(resp);
}

int main(void) {
    /* Test 1 */
    run_test("alice");
    /* Test 2 */
    run_test("Bob-123");
    /* Test 3: injection attempt */
    run_test("eve\r\nSet-Cookie: x=1");
    /* Test 4: long input */
    char *longname = (char*)malloc(201);
    if (longname) {
        memset(longname, 'x', 200);
        longname[200] = '\0';
        run_test(longname);
        free(longname);
    } else {
        run_test("x");
    }
    /* Test 5: special chars and unicode (filtered) */
    run_test("张伟/..\\;|$");
    return 0;
}