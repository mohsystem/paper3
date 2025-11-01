#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char* read_all(FILE* f) {
    if (!f) return NULL;
    size_t cap = 1024;
    char* buf = (char*)malloc(cap);
    if (!buf) return NULL;
    size_t len = 0;
    char tmp[1024];
    size_t n;
    while ((n = fread(tmp, 1, sizeof(tmp), f)) > 0) {
        if (len + n + 1 > cap) {
            size_t newcap = cap * 2;
            while (len + n + 1 > newcap) newcap *= 2;
            char* nb = (char*)realloc(buf, newcap);
            if (!nb) {
                free(buf);
                return NULL;
            }
            buf = nb;
            cap = newcap;
        }
        memcpy(buf + len, tmp, n);
        len += n;
    }
    buf[len] = '\0';
    return buf;
}

static char* trim_new(const char* s) {
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
    if (!request || !response) return NULL;
    char* req_data = read_all(request);
    if (!req_data) {
        const char* msg = "hello ";
        fputs(msg, response);
        fflush(response);
        char* out = (char*)malloc(strlen(msg) + 1);
        if (out) strcpy(out, msg);
        return out;
    }
    char* username = trim_new(req_data);
    free(req_data);
    if (!username) return NULL;

    const char* prefix = "hello ";
    size_t prelen = strlen(prefix);
    size_t ulen = strlen(username);
    char* message = (char*)malloc(prelen + ulen + 1);
    if (!message) {
        free(username);
        return NULL;
    }
    memcpy(message, prefix, prelen);
    memcpy(message + prelen, username, ulen);
    message[prelen + ulen] = '\0';

    fputs(message, response);
    fflush(response);

    free(username);
    return message;
}

int main(void) {
    const char* names[5] = {"", "Alice", "Bob", "  Charlie  ", "Dana_Smith"};
    for (int i = 0; i < 5; ++i) {
        FILE* req = tmpfile();
        FILE* resp = tmpfile();
        if (!req || !resp) return 1;
        fputs(names[i], req);
        fflush(req);
        rewind(req);

        char* out = http_hello_page(req, resp);
        if (out) {
            printf("%s\n", out);
            free(out);
        }

        fclose(req);
        fclose(resp);
    }
    return 0;
}