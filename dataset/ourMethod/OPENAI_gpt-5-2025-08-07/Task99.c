#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <regex.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

static int starts_with(const char *str, const char *prefix) {
    size_t lp = strlen(prefix);
    return strncmp(str, prefix, lp) == 0;
}

static int join_paths(char *out, size_t outsz, const char *base, const char *rel) {
    if (snprintf(out, outsz, "%s/%s", base, rel) >= (int)outsz) {
        return -1;
    }
    return 0;
}

static char* read_line_dynamic(FILE *f) {
    size_t cap = 256;
    size_t len = 0;
    char *buf = (char*)malloc(cap);
    if (!buf) return NULL;

    int c;
    while ((c = fgetc(f)) != EOF) {
        if (len + 1 >= cap) {
            size_t ncap = cap * 2;
            char *nb = (char*)realloc(buf, ncap);
            if (!nb) {
                free(buf);
                return NULL;
            }
            buf = nb;
            cap = ncap;
        }
        buf[len++] = (char)c;
        if (c == '\n') break;
    }
    if (len == 0 && c == EOF) {
        free(buf);
        return NULL;
    }
    buf[len] = '\0';
    return buf;
}

static int append_string(char **dst, size_t *cap, size_t *len, const char *s) {
    size_t sl = strlen(s);
    if (*len + sl + 1 > *cap) {
        size_t ncap = (*cap == 0 ? 256 : *cap);
        while (*len + sl + 1 > ncap) ncap *= 2;
        char *nb = (char*)realloc(*dst, ncap);
        if (!nb) return -1;
        *dst = nb;
        *cap = ncap;
    }
    memcpy(*dst + *len, s, sl);
    *len += sl;
    (*dst)[*len] = '\0';
    return 0;
}

// Returns a newly allocated string with matching lines separated by '\n'. Caller must free.
// On error, returns an allocated empty string "".
char* find_matching_lines(const char *base_dir, const char *rel_path) {
    if (!base_dir || !rel_path) {
        char *e = (char*)malloc(1); if (e) e[0]='\0'; return e;
    }
    if (rel_path[0] == '\0' || strlen(rel_path) > 4096) {
        char *e = (char*)malloc(1); if (e) e[0]='\0'; return e;
    }
#ifdef _WIN32
    // Simplistic absolute path check for Windows not fully implemented
    if ((strlen(rel_path) > 2 && rel_path[1] == ':') || rel_path[0] == '\\' || rel_path[0] == '/') {
        char *e = (char*)malloc(1); if (e) e[0]='\0'; return e;
    }
#else
    if (rel_path[0] == '/') {
        char *e = (char*)malloc(1); if (e) e[0]='\0'; return e;
    }
#endif

    char base_real[PATH_MAX];
    if (!realpath(base_dir, base_real)) {
        char *e = (char*)malloc(1); if (e) e[0]='\0'; return e;
    }

    struct stat st;
    if (stat(base_real, &st) != 0 || !S_ISDIR(st.st_mode)) {
        char *e = (char*)malloc(1); if (e) e[0]='\0'; return e;
    }

    char candidate[PATH_MAX];
    if (join_paths(candidate, sizeof(candidate), base_real, rel_path) != 0) {
        char *e = (char*)malloc(1); if (e) e[0]='\0'; return e;
    }

    struct stat lst;
    if (lstat(candidate, &lst) != 0) {
        char *e = (char*)malloc(1); if (e) e[0]='\0'; return e;
    }
    if (S_ISLNK(lst.st_mode)) {
        char *e = (char*)malloc(1); if (e) e[0]='\0'; return e;
    }
    if (!S_ISREG(lst.st_mode)) {
        char *e = (char*)malloc(1); if (e) e[0]='\0'; return e;
    }

    char resolved[PATH_MAX];
    if (!realpath(candidate, resolved)) {
        char *e = (char*)malloc(1); if (e) e[0]='\0'; return e;
    }

    size_t bl = strlen(base_real);
    if (!(starts_with(resolved, base_real) && (resolved[bl] == '/' || resolved[bl] == '\0'))) {
        char *e = (char*)malloc(1); if (e) e[0]='\0'; return e;
    }

    int flags = O_RDONLY;
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
    int fd = open(resolved, flags);
    if (fd < 0) {
        char *e = (char*)malloc(1); if (e) e[0]='\0'; return e;
    }

    struct stat fst;
    if (fstat(fd, &fst) != 0) {
        close(fd);
        char *e = (char*)malloc(1); if (e) e[0]='\0'; return e;
    }
    if (fst.st_ino != lst.st_ino || fst.st_dev != lst.st_dev) {
        close(fd);
        char *e = (char*)malloc(1); if (e) e[0]='\0'; return e;
    }

    FILE *f = fdopen(fd, "r");
    if (!f) {
        close(fd);
        char *e = (char*)malloc(1); if (e) e[0]='\0'; return e;
    }

    regex_t re;
    if (regcomp(&re, "^[0-9].*\\.$", REG_EXTENDED | REG_NOSUB) != 0) {
        fclose(f);
        char *e = (char*)malloc(1); if (e) e[0]='\0'; return e;
    }

    char *out = NULL;
    size_t cap = 0, len = 0;

    char *line;
    while ((line = read_line_dynamic(f)) != NULL) {
        size_t l = strlen(line);
        if (l > 0 && line[l - 1] == '\n') line[l - 1] = '\0';
        int r = regexec(&re, line, 0, NULL, 0);
        if (r == 0) {
            if (append_string(&out, &cap, &len, line) != 0) { free(line); break; }
            if (append_string(&out, &cap, &len, "\n") != 0) { free(line); break; }
        }
        free(line);
    }

    regfree(&re);
    fclose(f);

    if (!out) {
        out = (char*)malloc(1);
        if (out) out[0] = '\0';
        return out;
    }

    // Remove trailing newline if present
    if (len > 0 && out[len - 1] == '\n') {
        out[len - 1] = '\0';
    }
    return out;
}

static void write_file(const char *path, const char * const *lines, size_t n) {
    char dirbuf[PATH_MAX];
    strncpy(dirbuf, path, sizeof(dirbuf) - 1);
    dirbuf[sizeof(dirbuf) - 1] = '\0';
    char *slash = strrchr(dirbuf, '/');
    if (slash) {
        *slash = '\0';
        if (dirbuf[0] != '\0') {
            mkdir(dirbuf, 0700);
        }
    }
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd >= 0) {
        FILE *f = fdopen(fd, "w");
        if (f) {
            for (size_t i = 0; i < n; ++i) {
                fputs(lines[i], f);
                if (i + 1 < n) fputc('\n', f);
            }
            fclose(f);
        } else {
            close(fd);
        }
    }
}

int main(void) {
    char base[PATH_MAX];
    snprintf(base, sizeof(base), "./task99_c_base");
    mkdir(base, 0700);

    char path1[PATH_MAX], path2[PATH_MAX], path3[PATH_MAX], path4[PATH_MAX];
    snprintf(path1, sizeof(path1), "%s/%s", base, "t1.txt");
    snprintf(path2, sizeof(path2), "%s/%s", base, "t2.txt");
    snprintf(path3, sizeof(path3), "%s/%s", base, "t3.txt");
    snprintf(path4, sizeof(path4), "%s/%s", base, "subdir/inner.txt");

    const char *f1[] = {"123 start and end.", "Nope", "9.", "0x23.", "abc."};
    const char *f2[] = {"A1.", "1 but no dot", "44.", "7!"};
    const char *f3[] = {"...", "1a.", "", "2."};
    const char *f4[] = {"777.", "x7."};

    write_file(path1, f1, sizeof(f1)/sizeof(f1[0]));
    write_file(path2, f2, sizeof(f2)/sizeof(f2[0]));
    write_file(path3, f3, sizeof(f3)/sizeof(f3[0]));
    write_file(path4, f4, sizeof(f4)/sizeof(f4[0]));
    mkdir("./task99_c_base/subdir", 0700);

    // 5 test cases
    char *res;

    res = find_matching_lines(base, "t1.txt");
    printf("=== Test1 t1.txt ===\n");
    if (res && res[0] != '\0') { printf("%s\n", res); } else { printf("(no matches)\n"); }
    free(res);

    res = find_matching_lines(base, "t2.txt");
    printf("=== Test2 t2.txt ===\n");
    if (res && res[0] != '\0') { printf("%s\n", res); } else { printf("(no matches)\n"); }
    free(res);

    res = find_matching_lines(base, "t3.txt");
    printf("=== Test3 t3.txt ===\n");
    if (res && res[0] != '\0') { printf("%s\n", res); } else { printf("(no matches)\n"); }
    free(res);

    res = find_matching_lines(base, "subdir"); // directory should be rejected
    printf("=== Test4 subdir (directory should be rejected) ===\n");
    if (res && res[0] != '\0') { printf("%s\n", res); } else { printf("(no matches)\n"); }
    free(res);

    res = find_matching_lines(base, "../t1.txt"); // traversal should be rejected
    printf("=== Test5 path traversal (should be rejected) ===\n");
    if (res && res[0] != '\0') { printf("%s\n", res); } else { printf("(no matches)\n"); }
    free(res);

    res = find_matching_lines(base, "subdir/inner.txt");
    printf("=== Extra nested file ===\n");
    if (res && res[0] != '\0') { printf("%s\n", res); } else { printf("(no matches)\n"); }
    free(res);

    return 0;
}