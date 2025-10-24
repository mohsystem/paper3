#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define POPEN _popen
#define PCLOSE _pclose
#else
#define POPEN popen
#define PCLOSE pclose
#endif

static char* quote_posix(const char* path) {
    size_t len = strlen(path);
    size_t maxlen = len * 4 + 3;
    char* out = (char*)malloc(maxlen);
    if (!out) return NULL;
    size_t pos = 0;
    out[pos++] = '\'';
    for (size_t i = 0; i < len; ++i) {
        if (path[i] == '\'') {
            out[pos++] = '\'';
            out[pos++] = '\\';
            out[pos++] = '\'';
            out[pos++] = '\'';
        } else {
            out[pos++] = path[i];
        }
    }
    out[pos++] = '\'';
    out[pos] = '\0';
    return out;
}

static char* quote_windows(const char* path) {
    size_t len = strlen(path);
    size_t maxlen = len * 2 + 3;
    char* out = (char*)malloc(maxlen);
    if (!out) return NULL;
    size_t pos = 0;
    out[pos++] = '"';
    for (size_t i = 0; i < len; ++i) {
        if (path[i] == '"') {
            out[pos++] = '"';
            out[pos++] = '"';
        } else {
            out[pos++] = path[i];
        }
    }
    out[pos++] = '"';
    out[pos] = '\0';
    return out;
}

char* display_file(const char* filename) {
    if (!filename) return NULL;

    char* cmd = NULL;
#ifdef _WIN32
    char* q = quote_windows(filename);
    if (!q) return NULL;
    const char* prefix = "cmd /c type ";
#else
    char* q = quote_posix(filename);
    if (!q) return NULL;
    const char* prefix = "cat ";
#endif

    size_t cmdlen = strlen(prefix) + strlen(q) + 1;
    cmd = (char*)malloc(cmdlen);
    if (!cmd) { free(q); return NULL; }
    strcpy(cmd, prefix);
    strcat(cmd, q);
    free(q);

    FILE* fp = POPEN(cmd, "rb");
    free(cmd);
    if (!fp) return NULL;

    size_t cap = 4096;
    size_t len = 0;
    char* out = (char*)malloc(cap);
    if (!out) { PCLOSE(fp); return NULL; }

    size_t n;
    char buffer[4096];
    while ((n = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        if (len + n + 1 > cap) {
            size_t newcap = (cap * 2 > len + n + 1) ? cap * 2 : (len + n + 1);
            char* tmp = (char*)realloc(out, newcap);
            if (!tmp) {
                free(out);
                PCLOSE(fp);
                return NULL;
            }
            cap = newcap;
            out = tmp;
        }
        memcpy(out + len, buffer, n);
        len += n;
    }
    PCLOSE(fp);
    out[len] = '\0';
    return out;
}

int main(void) {
    const char* files[5] = {
        "c_file1.txt",
        "c_empty.txt",
        "c multi line.txt",
        "c_unicode.txt",
        "c_numbers.txt"
    };
    const char* contents[5] = {
        "Hello from C\nSecond line\n",
        "",
        "Red\nGreen\nBlue\n",
        "こんにちは世界\nПривет мир\nHola mundo\n",
        "7\n8\n9\n10\n11\n"
    };

    for (int i = 0; i < 5; ++i) {
        FILE* f = fopen(files[i], "wb");
        if (f) {
            fwrite(contents[i], 1, strlen(contents[i]), f);
            fclose(f);
        }
    }

    for (int i = 0; i < 5; ++i) {
        printf("--- %s ---\n", files[i]);
        char* out = display_file(files[i]);
        if (out) {
            printf("%s", out);
            free(out);
        } else {
            printf("\n");
        }
    }
    return 0;
}