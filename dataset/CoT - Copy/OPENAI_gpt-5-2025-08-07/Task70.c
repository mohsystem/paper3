#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#define S_ISLNK(m) 0
#else
#include <unistd.h>
#endif

#define MAX_SIZE (10u * 1024u * 1024u) /* 10 MB */

/*
 Secure file-reading function:
 - Normalize path is platform-specific; rely on OS checks.
 - Reject symbolic links (where supported).
 - Ensure regular file and readable.
 - Enforce maximum size.
 - Stream read with growth-limited buffer to mitigate TOCTOU.
 Returns newly allocated NUL-terminated string on success; caller must free().
 Returns NULL on error.
*/
char* read_file(const char* path) {
    if (path == NULL) return NULL;

    struct stat st;
#if defined(_WIN32)
    if (_stat(path, &st) != 0) {
        return NULL;
    }
#else
    if (lstat(path, &st) != 0) {
        return NULL;
    }
#endif

#ifndef _WIN32
    if (S_ISLNK(st.st_mode)) {
        return NULL;
    }
#endif

    if (!S_ISREG(st.st_mode)) {
        return NULL;
    }

    /* Preliminary size check */
    if (st.st_size > (off_t)MAX_SIZE) {
        return NULL;
    }

    FILE* f = fopen(path, "rb");
    if (!f) {
        return NULL;
    }

    /* Read in chunks with cap */
    size_t cap = (st.st_size > 0 && st.st_size < (off_t)8192) ? (size_t)st.st_size + 1 : 8192;
    if (cap > MAX_SIZE + 1) cap = MAX_SIZE + 1;
    char* buf = (char*)malloc(cap);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    size_t len = 0;
    for (;;) {
        if (len + 8192 + 1 > cap) {
            size_t newcap = cap * 2;
            if (newcap > MAX_SIZE + 1) newcap = MAX_SIZE + 1;
            if (newcap <= cap) { /* overflow or cannot grow */
                free(buf);
                fclose(f);
                return NULL;
            }
            char* nb = (char*)realloc(buf, newcap);
            if (!nb) {
                free(buf);
                fclose(f);
                return NULL;
            }
            buf = nb;
            cap = newcap;
        }
        size_t to_read = 8192;
        if (cap - len - 1 < to_read) {
            to_read = cap - len - 1;
        }
        size_t r = fread(buf + len, 1, to_read, f);
        if (r == 0) {
            if (ferror(f)) {
                free(buf);
                fclose(f);
                return NULL;
            }
            break; /* EOF */
        }
        len += r;
        if (len > MAX_SIZE) {
            free(buf);
            fclose(f);
            return NULL;
        }
    }
    buf[len] = '\0';
    fclose(f);
    return buf;
}

/* Helper to create a temporary file with given content. Returns malloc'd path string, caller frees. */
static char* make_temp_with_content(const char* content) {
    char tmpl[] =
#ifdef _WIN32
        "task70_tmp_XXXXXX.txt";
#else
        "/tmp/task70_tmp_XXXXXX";
#endif

#ifdef _WIN32
    /* Windows fallback: create unique filename by appending a counter (not perfectly secure, but acceptable for test). */
    static int counter = 0;
    char pathbuf[512];
    for (int i = 0; i < 1000; ++i) {
        snprintf(pathbuf, sizeof(pathbuf), "task70_tmp_%d_%d.txt", (int)getpid(), counter++);
        FILE* f = fopen(pathbuf, "rb");
        if (f) { fclose(f); continue; } /* exists, try another */
        f = fopen(pathbuf, "wb");
        if (!f) continue;
        if (content) {
            fwrite(content, 1, strlen(content), f);
        }
        fclose(f);
        char* out = (char*)malloc(strlen(pathbuf) + 1);
        if (out) strcpy(out, pathbuf);
        return out;
    }
    return NULL;
#else
    int fd = mkstemp(tmpl);
    if (fd == -1) {
        return NULL;
    }
    FILE* f = fdopen(fd, "wb");
    if (!f) {
        close(fd);
        unlink(tmpl);
        return NULL;
    }
    if (content) {
        fwrite(content, 1, strlen(content), f);
    }
    fclose(f);
    char* out = (char*)malloc(strlen(tmpl) + 1);
    if (out) strcpy(out, tmpl);
    return out;
#endif
}

int main(int argc, char** argv) {
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            char* res = read_file(argv[i]);
            if (res) {
                fputs(res, stdout);
                size_t L = strlen(res);
                if (L == 0 || res[L - 1] != '\n') fputc('\n', stdout);
                free(res);
            } else {
                fputs("ERROR\n", stdout);
            }
        }
    } else {
        /* 5 test cases */
        char* t1 = make_temp_with_content("Hello from test 1\n");
        char* t2 = make_temp_with_content("Line1\nLine2\nLine3\n");
        char* t3 = make_temp_with_content(""); /* empty */
        char* t4 = make_temp_with_content("Unicode (UTF-8 may display differently): \xCE\xB1\xCE\xB2\xCE\xB3\n");
        const char* t5 = "this_path_should_not_exist_12345.txt";

        const char* tests[5] = { t1, t2, t3, t4, t5 };
        for (int i = 0; i < 5; ++i) {
            printf("Test %d:\n", i + 1);
            if (tests[i]) {
                char* res = read_file(tests[i]);
                if (res) {
                    fputs(res, stdout);
                    size_t L = strlen(res);
                    if (L == 0 || res[L - 1] != '\n') fputc('\n', stdout);
                    free(res);
                } else {
                    fputs("ERROR\n", stdout);
                }
            } else {
                fputs("ERROR\n", stdout);
            }
        }

        if (t1) { remove(t1); free(t1); }
        if (t2) { remove(t2); free(t2); }
        if (t3) { remove(t3); free(t3); }
        if (t4) { remove(t4); free(t4); }
    }
    return 0;
}