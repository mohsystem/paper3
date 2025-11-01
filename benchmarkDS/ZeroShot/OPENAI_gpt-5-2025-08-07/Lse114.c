#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

static char* print_format(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    va_list ap_copy;
    va_copy(ap_copy, ap);
    int n = vsnprintf(NULL, 0, fmt, ap_copy);
    va_end(ap_copy);
    if (n < 0) {
        va_end(ap);
        return NULL;
    }
    char* buf = (char*)malloc((size_t)n + 1);
    if (!buf) {
        va_end(ap);
        return NULL;
    }
    vsnprintf(buf, (size_t)n + 1, fmt, ap);
    va_end(ap);
    return buf;
}

static char* shell_quote(const char* s) {
    if (s == NULL || *s == '\0') {
        char* empty = (char*)malloc(3);
        if (!empty) return NULL;
        strcpy(empty, "''");
        return empty;
    }
    size_t len = strlen(s);
    size_t extra = 0;
    for (size_t i = 0; i < len; i++) {
        if (s[i] == '\'') {
            extra += 3; // '\'' expansion adds 3 extra chars beyond the one replaced
        }
    }
    size_t total = 2 + len + extra; // surrounding quotes + content
    char* out = (char*)malloc(total + 1);
    if (!out) return NULL;

    char* p = out;
    *p++ = '\'';
    for (size_t i = 0; i < len; i++) {
        if (s[i] == '\'') {
            memcpy(p, "'\\''", 4);
            p += 4;
        } else {
            *p++ = s[i];
        }
    }
    *p++ = '\'';
    *p = '\0';
    return out;
}

int run_ls(const char* path) {
    const char* base = "/bin/ls -la";
    char* cmd = NULL;
    if (path && *path) {
        char* q = shell_quote(path);
        if (!q) return -1;
        cmd = print_format("%s %s", base, q);
        free(q);
    } else {
        cmd = print_format("%s", base);
    }
    if (!cmd) return -1;
    int rc = system(cmd);
    free(cmd);
    return rc;
}

int main(void) {
    const char* tests[5] = { NULL, ".", "/", "/tmp", "no_such_file_xyz" };
    for (int i = 0; i < 5; i++) {
        const char* t = tests[i];
        printf("== C run_ls(%s) ==\n", t ? t : "null/empty");
        run_ls(t);
    }
    return 0;
}