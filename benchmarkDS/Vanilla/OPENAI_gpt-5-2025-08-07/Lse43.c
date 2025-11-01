#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

char* read_from_safe(const char* filename) {
    if (!filename) return NULL;

    char combined[PATH_MAX];
    if (snprintf(combined, sizeof(combined), "/safe/%s", filename) >= (int)sizeof(combined)) {
        return NULL;
    }

    char resolved[PATH_MAX];
    if (!realpath(combined, resolved)) {
        return NULL;
    }

    const char* base = "/safe/";
    size_t base_len = strlen(base);
    if (strncmp(resolved, base, base_len) != 0) {
        return NULL;
    }

    FILE* f = fopen(resolved, "rb");
    if (!f) return NULL;

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    long sz = ftell(f);
    if (sz < 0) {
        fclose(f);
        return NULL;
    }
    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return NULL;
    }

    char* buf = (char*)malloc((size_t)sz + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    size_t readn = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    buf[readn] = '\0';
    return buf;
}

int main(void) {
    const char* tests[5] = {
        "example.txt",
        "notes.md",
        "data.json",
        "subdir/file.txt",
        "../etc/passwd"
    };
    for (int i = 0; i < 5; ++i) {
        printf("== %s ==\n", tests[i]);
        char* content = read_from_safe(tests[i]);
        if (content) {
            printf("%s\n\n", content);
            free(content);
        } else {
            printf("ERROR\n\n");
        }
    }
    return 0;
}