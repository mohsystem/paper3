#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static int has_forbidden_segments(const char* p) {
    if (p == NULL) return 1;
    if (*p == '/') return 1;         // absolute path not allowed
#ifdef _WIN32
    if (strchr(p, '\\')) return 1;   // backslashes not allowed
#endif
    // Reject any ".." segment
    const char* s = p;
    while (*s) {
        if (s[0] == '.' && s[1] == '.' && (s[2] == '/' || s[2] == '\0')) return 1;
        if (s[0] == '/' && s[1] == '.' && s[2] == '.' && (s[3] == '/' || s[3] == '\0')) return 1;
        s++;
    }
    return 0;
}

char* read_safe_file(const char* requestedPath) {
    if (requestedPath == NULL) return NULL;
    if (has_forbidden_segments(requestedPath)) {
        return NULL;
    }

    char path[PATH_MAX];
    const char* base = "/safe";
    size_t base_len = strlen(base);

    // Build normalized path: /safe + '/' + sanitized segments
    // Normalize by removing empty and '.' segments and joining with '/'
    char normalized[PATH_MAX];
    size_t nlen = 0;
    // start with base
    if (base_len + 1 >= sizeof(path)) return NULL;
    strcpy(normalized, base);
    nlen = base_len;

    const char* p = requestedPath;
    while (*p) {
        // skip delimiters
        while (*p == '/') p++;
        if (!*p) break;
        // extract segment
        const char* start = p;
        while (*p && *p != '/') p++;
        size_t seglen = (size_t)(p - start);
        if (seglen == 0) continue;
        if (seglen == 1 && start[0] == '.') continue;
        if (seglen == 2 && start[0] == '.' && start[1] == '.') return NULL; // already checked, double ensure

        if (nlen + 1 + seglen >= sizeof(normalized)) return NULL;
        if (normalized[nlen - 1] != '/') {
            normalized[nlen++] = '/';
        }
        memcpy(normalized + nlen, start, seglen);
        nlen += seglen;
        normalized[nlen] = '\0';
    }

    // Use the normalized path
    strncpy(path, normalized, sizeof(path) - 1);
    path[sizeof(path) - 1] = '\0';

    FILE* f = fopen(path, "rb");
    if (!f) {
        return NULL;
    }

    size_t cap = 4096;
    size_t len = 0;
    char* buf = (char*)malloc(cap + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    for (;;) {
        if (len + 1024 > cap) {
            size_t newcap = cap * 2;
            char* nb = (char*)realloc(buf, newcap + 1);
            if (!nb) {
                free(buf);
                fclose(f);
                return NULL;
            }
            buf = nb;
            cap = newcap;
        }
        size_t n = fread(buf + len, 1, 1024, f);
        len += n;
        if (n < 1024) {
            if (feof(f)) break;
            // read error
            free(buf);
            fclose(f);
            return NULL;
        }
    }
    buf[len] = '\0';
    fclose(f);
    return buf;
}

int main(void) {
    const char* tests[5] = {
        "example.txt",
        "subdir/data.txt",
        "../etc/passwd",
        "/etc/passwd",
        "nonexistent.txt"
    };

    for (int i = 0; i < 5; ++i) {
        const char* t = tests[i];
        char* content = read_safe_file(t);
        if (content) {
            printf("[%s] CONTENT:\n%s\n", t, content);
            free(content);
        } else {
            printf("[%s] ERROR: Unable to read file (access denied or not found)\n", t);
        }
        printf("----\n");
    }
    return 0;
}