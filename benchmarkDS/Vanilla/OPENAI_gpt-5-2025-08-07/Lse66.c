#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(p, m) _mkdir(p)
#else
#include <unistd.h>
#define MKDIR(p, m) mkdir(p, m)
#endif

static int is_pdf(const unsigned char* data, size_t len) {
    return len >= 4 && data[0] == '%' && data[1] == 'P' && data[2] == 'D' && data[3] == 'F';
}

static char* sanitize_filename(const char* name) {
    const char* last = name;
    for (const char* p = name; *p; ++p) {
        if (*p == '\\' || *p == '/') last = p + 1;
    }
    size_t n = strlen(last);
    char* out = (char*)malloc(n + 5); // possible ".pdf" append + null
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < n; ++i) {
        unsigned char c = (unsigned char)last[i];
        if (isalnum(c) || c == '.' || c == '_' || c == '-') out[j++] = (char)c;
        else out[j++] = '_';
    }
    out[j] = '\0';
    size_t L = strlen(out);
    int has_pdf = 0;
    if (L >= 4) {
        char a = tolower((unsigned char)out[L-4]);
        char b = tolower((unsigned char)out[L-3]);
        char c = tolower((unsigned char)out[L-2]);
        char d = tolower((unsigned char)out[L-1]);
        has_pdf = (a == '.' && b == 'p' && c == 'd' && d == 'f');
    }
    if (!has_pdf) {
        strcat(out, ".pdf");
    }
    return out;
}

static int file_exists(const char* path) {
    struct stat st;
    return stat(path, &st) == 0;
}

static char* join_path(const char* dir, const char* name) {
    size_t L1 = strlen(dir), L2 = strlen(name);
    size_t need = L1 + 1 + L2 + 1;
    char* r = (char*)malloc(need);
    if (!r) return NULL;
#ifdef _WIN32
    snprintf(r, need, "%s\\%s", dir, name);
#else
    snprintf(r, need, "%s/%s", dir, name);
#endif
    return r;
}

static char* save_pdf(const unsigned char* data, size_t len, const char* filename) {
    if (!data || !filename) {
        return NULL;
    }
    if (!is_pdf(data, len)) {
        return NULL;
    }
    char* sanitized = sanitize_filename(filename);
    if (!sanitized) return NULL;
    size_t L = strlen(sanitized);
    if (L < 4 || strcasecmp(sanitized + L - 4, ".pdf") != 0) {
        free(sanitized);
        return NULL;
    }

    // ensure uploads directory
#ifdef _WIN32
    MKDIR("uploads");
#else
    MKDIR("uploads", 0755);
#endif

    char* target = join_path("uploads", sanitized);
    if (!target) { free(sanitized); return NULL; }
    if (file_exists(target)) {
        char* base = (char*)malloc(L - 4 + 1);
        if (!base) { free(sanitized); free(target); return NULL; }
        memcpy(base, sanitized, L - 4);
        base[L - 4] = '\0';
        int i = 1;
        free(target);
        while (1) {
            size_t need = strlen("uploads") + 1 + strlen(base) + 1 + 10 + 4 + 1;
            target = (char*)malloc(need);
            if (!target) { free(sanitized); free(base); return NULL; }
#ifdef _WIN32
            snprintf(target, need, "uploads\\%s_%d.pdf", base, i);
#else
            snprintf(target, need, "uploads/%s_%d.pdf", base, i);
#endif
            if (!file_exists(target)) {
                break;
            }
            free(target);
            i++;
        }
        free(base);
    }

    FILE* f = fopen(target, "wb");
    if (!f) {
        free(sanitized);
        free(target);
        return NULL;
    }
    size_t wrote = fwrite(data, 1, len, f);
    fclose(f);
    free(sanitized);
    if (wrote != len) {
        remove(target);
        free(target);
        return NULL;
    }
    return target; // caller must free
}

int main(void) {
    const unsigned char valid[] = {'%', 'P', 'D', 'F', '-', '1', '.', '7', '\n'};
    const unsigned char invalidSig[] = {'X', 'X', 'X', 'X'};
    char* p;

    p = save_pdf(valid, sizeof(valid), "report.pdf");
    if (p) { printf("Test1: %s\n", p); free(p); } else { printf("Test1 failed\n"); }

    p = save_pdf(invalidSig, sizeof(invalidSig), "bad.pdf");
    if (p) { printf("Test2: %s\n", p); free(p); } else { printf("Test2 failed\n"); }

    p = save_pdf(valid, sizeof(valid), "notes.txt");
    if (p) { printf("Test3: %s\n", p); free(p); } else { printf("Test3 failed\n"); }

    p = save_pdf(valid, sizeof(valid), "../../escape.pdf");
    if (p) { printf("Test4: %s\n", p); free(p); } else { printf("Test4 failed\n"); }

    char* p1 = save_pdf(valid, sizeof(valid), "duplicate.pdf");
    char* p2 = save_pdf(valid, sizeof(valid), "duplicate.pdf");
    if (p1 && p2) { printf("Test5: %s and %s\n", p1, p2); }
    else { printf("Test5 failed\n"); }
    if (p1) free(p1);
    if (p2) free(p2);

    // Note: Minimal file-saving logic and tests are implemented.
    // A full HTTP server is not included here.
    return 0;
}