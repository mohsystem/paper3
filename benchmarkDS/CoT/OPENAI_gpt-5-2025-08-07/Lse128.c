#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char* trim_copy(const char* s) {
    const char* start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    const char* end = start + strlen(start);
    while (end > start && isspace((unsigned char)*(end - 1))) end--;
    size_t len = (size_t)(end - start);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, start, len);
    out[len] = '\0';
    return out;
}

char* http_hello_page(FILE* request, FILE* response) {
    if (!request || !response) return NULL;

    size_t cap = 64, len = 0;
    char* buf = (char*)malloc(cap);
    if (!buf) return NULL;

    int ch;
    while ((ch = fgetc(request)) != EOF) {
        if (len + 1 >= cap) {
            cap *= 2;
            char* tmp = (char*)realloc(buf, cap);
            if (!tmp) { free(buf); return NULL; }
            buf = tmp;
        }
        buf[len++] = (char)ch;
    }
    if (buf) buf[len] = '\0';

    char* username = trim_copy(buf ? buf : "");
    free(buf);
    if (!username) return NULL;

    const char* prefix = "hello ";
    size_t msg_len = strlen(prefix) + strlen(username);
    char* message = (char*)malloc(msg_len + 1);
    if (!message) { free(username); return NULL; }

    strcpy(message, prefix);
    strcat(message, username);

    fprintf(response, "%s", message);
    fflush(response);

    free(username);
    return message;
}

static void print_file_content(FILE* f) {
    int c;
    while ((c = fgetc(f)) != EOF) {
        putchar(c);
    }
    putchar('\n');
}

int main(void) {
    // Test case 1
    FILE* req1 = tmpfile();
    FILE* resp1 = tmpfile();
    fputs("Alice", req1);
    rewind(req1);
    char* out1 = http_hello_page(req1, resp1);
    rewind(resp1);
    print_file_content(resp1);
    free(out1);
    fclose(req1);
    fclose(resp1);

    // Test case 2
    FILE* req2 = tmpfile();
    FILE* resp2 = tmpfile();
    fputs("Bob", req2);
    rewind(req2);
    char* out2 = http_hello_page(req2, resp2);
    rewind(resp2);
    print_file_content(resp2);
    free(out2);
    fclose(req2);
    fclose(resp2);

    // Test case 3
    FILE* req3 = tmpfile();
    FILE* resp3 = tmpfile();
    fputs("   Charlie   ", req3);
    rewind(req3);
    char* out3 = http_hello_page(req3, resp3);
    rewind(resp3);
    print_file_content(resp3);
    free(out3);
    fclose(req3);
    fclose(resp3);

    // Test case 4 (empty)
    FILE* req4 = tmpfile();
    FILE* resp4 = tmpfile();
    rewind(req4);
    char* out4 = http_hello_page(req4, resp4);
    rewind(resp4);
    print_file_content(resp4);
    free(out4);
    fclose(req4);
    fclose(resp4);

    // Test case 5 (with newline)
    FILE* req5 = tmpfile();
    FILE* resp5 = tmpfile();
    fputs("Dana\n", req5);
    rewind(req5);
    char* out5 = http_hello_page(req5, resp5);
    rewind(resp5);
    print_file_content(resp5);
    free(out5);
    fclose(req5);
    fclose(resp5);

    return 0;
}