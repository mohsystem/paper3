#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(p, m) _mkdir(p)
#else
#define MKDIR(p, m) mkdir(p, m)
#endif

#define MAX_FILE_SIZE (5u * 1024u * 1024u)

typedef struct {
    unsigned char* data;
    size_t size;
} Buffer;

static int is_allowed_name(const char* name) {
    if (!name || !*name) return 0;
    const char* p = name;
    while (*p) {
        if (!(isalnum((unsigned char)*p) || *p=='.' || *p=='_' || *p=='-')) return 0;
        p++;
    }
    const char* dot = strrchr(name, '.');
    if (!dot || dot == name) return 0;
    char ext[16];
    size_t len = strlen(dot + 1);
    if (len == 0 || len >= sizeof(ext)) return 0;
    for (size_t i = 0; i < len; ++i) ext[i] = (char)tolower((unsigned char)dot[1 + i]);
    ext[len] = '\0';
    const char* allowed[] = {"png","jpg","jpeg","gif","bmp","webp"};
    for (size_t i = 0; i < sizeof(allowed)/sizeof(allowed[0]); ++i) {
        if (strcmp(ext, allowed[i]) == 0) return 1;
    }
    return 0;
}

static int safe_join(const char* base, const char* name, char* out, size_t outsz) {
    if (snprintf(out, outsz, "%s/%s", base, name) >= (int)outsz) return 0;
    return 1;
}

static int starts_with_path(const char* base, const char* path) {
    size_t bl = strlen(base);
    if (strncmp(base, path, bl) != 0) return 0;
    if (path[bl] == '\0') return 1;
    if (path[bl] == '/') return 1;
    return 0;
}

static Buffer img(const char* file_param) {
    Buffer res = {NULL, 0};
    if (!is_allowed_name(file_param)) return res;

    const char* base_dir = "static/images";
    char base_abs[PATH_MAX];
    char cand_rel[PATH_MAX];
    char cand_abs[PATH_MAX];

    // Ensure base dir exists for use
    MKDIR("static", 0700);
    MKDIR(base_dir, 0700);

    if (!realpath(base_dir, base_abs)) {
        return res;
    }

    if (!safe_join(base_dir, file_param, cand_rel, sizeof(cand_rel))) {
        return res;
    }

    struct stat st;
    if (stat(cand_rel, &st) != 0 || !S_ISREG(st.st_mode)) {
        return res;
    }

    if (st.st_size < 0 || (unsigned long)st.st_size > MAX_FILE_SIZE) {
        return res;
    }

    if (!realpath(cand_rel, cand_abs)) {
        return res;
    }

    if (!starts_with_path(base_abs, cand_abs)) {
        return res;
    }

    FILE* f = fopen(cand_abs, "rb");
    if (!f) return res;

    size_t sz = (size_t)st.st_size;
    unsigned char* buf = (unsigned char*)malloc(sz ? sz : 1);
    if (!buf) {
        fclose(f);
        return res;
    }

    size_t rd = fread(buf, 1, sz, f);
    fclose(f);
    if (rd != sz) {
        free(buf);
        return res;
    }

    res.data = buf;
    res.size = sz;
    return res;
}

static void ensure_test_files() {
    MKDIR("static", 0700);
    MKDIR("static/images", 0700);

    const char* f1 = "static/images/test1.png";
    FILE* fp = fopen(f1, "rb");
    if (!fp) {
        fp = fopen(f1, "wb");
        if (fp) { const char* s = "PNGDATA"; fwrite(s, 1, strlen(s), fp); fclose(fp); }
    } else {
        fclose(fp);
    }

    const char* f2 = "static/images/test2.txt";
    fp = fopen(f2, "rb");
    if (!fp) {
        fp = fopen(f2, "wb");
        if (fp) { const char* s = "TEXTDATA"; fwrite(s, 1, strlen(s), fp); fclose(fp); }
    } else {
        fclose(fp);
    }
}

int main(void) {
    ensure_test_files();
    const char* tests[5] = {
        "test1.png",      // valid
        "../secret.txt",  // traversal
        "test2.txt",      // invalid extension
        "missing.jpg",    // missing
        "bad/evil.png"    // path separator (invalid name)
    };
    for (int i = 0; i < 5; ++i) {
        Buffer b = img(tests[i]);
        printf("%s -> %zu\n", tests[i], b.size);
        if (b.data) free(b.data);
    }
    return 0;
}