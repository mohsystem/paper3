#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char* read_all(FILE* f) {
    size_t cap = 128;
    size_t len = 0;
    char* buf = (char*)malloc(cap + 1);
    if (!buf) return NULL;
    int c;
    while ((c = fgetc(f)) != EOF) {
        if (len + 1 >= cap) {
            cap *= 2;
            char* nb = (char*)realloc(buf, cap + 1);
            if (!nb) { free(buf); return NULL; }
            buf = nb;
        }
        buf[len++] = (char)c;
    }
    buf[len] = '\0';
    return buf;
}

static char* trim_inplace(char* s) {
    if (!s) return s;
    char* start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    char* end = start + strlen(start);
    while (end > start && isspace((unsigned char)end[-1])) end--;
    *end = '\0';
    if (start != s) memmove(s, start, (size_t)(end - start + 1));
    return s;
}

char* http_hello_page(FILE* request, FILE* response) {
    char* uname = read_all(request);
    if (!uname) return NULL;
    trim_inplace(uname);
    const char* prefix = "Hello ";
    size_t msglen = strlen(prefix) + strlen(uname);
    char* message = (char*)malloc(msglen + 1);
    if (!message) { free(uname); return NULL; }
    strcpy(message, prefix);
    strcat(message, uname);
    fputs(message, response);
    free(uname);
    return message; /* Caller is responsible for free() */
}

int main(void) {
    const char* usernames[5] = {"Alice", "Bob", "Charlie", "", "Eve Adams"};
    for (int i = 0; i < 5; ++i) {
        FILE* req = tmpfile();
        FILE* resp = tmpfile();
        if (!req || !resp) {
            if (req) fclose(req);
            if (resp) fclose(resp);
            fprintf(stderr, "Failed to open temporary files\n");
            return 1;
        }
        fputs(usernames[i], req);
        rewind(req);

        char* result = http_hello_page(req, resp);
        rewind(resp);
        char* resp_content = read_all(resp);

        printf("Returned: %s\n", result ? result : "(null)");
        printf("Response Stream: %s\n", resp_content ? resp_content : "(null)");

        free(result);
        free(resp_content);
        fclose(req);
        fclose(resp);
    }
    return 0;
}