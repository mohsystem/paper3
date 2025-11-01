#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char* read_all(FILE* f) {
    if (!f) return NULL;
    size_t cap = 128;
    size_t len = 0;
    char* buf = (char*)malloc(cap);
    if (!buf) return NULL;
    int c;
    while ((c = fgetc(f)) != EOF) {
        if (len + 1 >= cap) {
            cap *= 2;
            char* nb = (char*)realloc(buf, cap);
            if (!nb) { free(buf); return NULL; }
            buf = nb;
        }
        buf[len++] = (char)c;
    }
    if (len + 1 >= cap) {
        char* nb = (char*)realloc(buf, len + 1);
        if (!nb) { free(buf); return NULL; }
        buf = nb;
    }
    buf[len] = '\0';
    return buf;
}

static char* trim_copy(const char* s) {
    if (!s) return strdup("");
    const char* start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    const char* end = s + strlen(s);
    while (end > start && isspace((unsigned char)*(end - 1))) end--;
    size_t len = (size_t)(end - start);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, start, len);
    out[len] = '\0';
    return out;
}

char* http_hello_page(FILE* request, FILE* response) {
    char* req_content = read_all(request);
    if (!req_content) req_content = strdup("");
    char* username = trim_copy(req_content);
    size_t out_len = strlen("Hello ") + (username ? strlen(username) : 0);
    char* out = (char*)malloc(out_len + 1);
    if (!out) {
        free(req_content);
        free(username);
        return NULL;
    }
    strcpy(out, "Hello ");
    if (username) strcat(out, username);
    fputs(out, response);
    fflush(response);
    free(req_content);
    free(username);
    return out;
}

int main(void) {
    const char* tests[5] = {"Alice", "  Bob  ", "", "Bob123", "世界"};
    for (int i = 0; i < 5; ++i) {
        FILE* req = tmpfile();
        FILE* res = tmpfile();
        if (!req || !res) return 1;
        fputs(tests[i], req);
        fflush(req);
        fseek(req, 0, SEEK_SET);

        char* ret = http_hello_page(req, res);

        fseek(res, 0, SEEK_SET);
        char* written = read_all(res);

        printf("Test %d: return=%s, response=%s\n", i + 1, ret ? ret : "(null)", written ? written : "(null)");

        free(ret);
        free(written);
        fclose(req);
        fclose(res);
    }
    return 0;
}