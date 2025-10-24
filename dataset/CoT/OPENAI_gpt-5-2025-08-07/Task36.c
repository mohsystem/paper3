/* Chain-of-Through process:
 * 1) Problem understanding: expose a secure function to read a user-specified file and return its contents; main prints it.
 * 2) Security requirements: avoid symlink traversal, ensure regular file, enforce max size, handle errors, avoid unsafe APIs.
 * 3) Secure coding generation: use lstat (no follow), check S_ISREG, size cap, use buffered reads, validate inputs, proper memory management.
 * 4) Code review: check all return values, prevent overflows, null-terminate buffers, free resources.
 * 5) Secure code output: final code with mitigations and 5 test cases.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>

#ifdef _WIN32
#include <io.h>
#define OPEN _open
#define READ _read
#define CLOSE _close
#define OFLAGS _O_BINARY | _O_RDONLY
#else
#define OPEN open
#define READ read
#define CLOSE close
#define OFLAGS O_RDONLY
#endif

static const size_t MAX_FILE_SIZE = 10u * 1024u * 1024u; // 10 MB

/* Secure file reader: accepts path, outputs allocated content via out_content, returns 0 on success. Caller must free(*out_content). */
int read_file_secure(const char* path, char** out_content) {
    if (!path || !out_content) return EINVAL;
    *out_content = NULL;

    struct stat st;
#ifdef _WIN32
    /* On Windows, _stat does not follow symlinks the same as POSIX; still check attributes as best effort. */
    if (stat(path, &st) != 0) return errno;
    /* Windows symlink detection is non-trivial; skip explicit symlink rejection on Windows. */
#else
    if (lstat(path, &st) != 0) return errno;
    if (S_ISLNK(st.st_mode)) return ELOOP; /* refusing to read symlink */
#endif

    if (!S_ISREG(st.st_mode)) return EISDIR; /* Not a regular file */
    if ((size_t)st.st_size > MAX_FILE_SIZE) return EFBIG;

    int fd = OPEN(path, OFLAGS);
    if (fd < 0) return errno;

    size_t cap = (st.st_size > 0) ? (size_t)st.st_size : 0;
    if (cap == 0) cap = 1; /* allocate at least 1 for null terminator */
    char* buf = (char*)malloc(cap + 1);
    if (!buf) {
        CLOSE(fd);
        return ENOMEM;
    }

    size_t total = 0;
    while (1) {
        if (total >= MAX_FILE_SIZE) {
            free(buf);
            CLOSE(fd);
            return EFBIG;
        }
        ssize_t r = READ(fd, buf + total, (cap - total) > 8192 ? 8192 : (cap - total));
        if (r < 0) {
            int e = errno;
            free(buf);
            CLOSE(fd);
            return e;
        } else if (r == 0) {
            break;
        } else {
            total += (size_t)r;
            if (total == cap) {
                size_t new_cap = cap * 2;
                if (new_cap < cap || new_cap > MAX_FILE_SIZE) new_cap = MAX_FILE_SIZE;
                char* tmp = (char*)realloc(buf, new_cap + 1);
                if (!tmp) {
                    free(buf);
                    CLOSE(fd);
                    return ENOMEM;
                }
                buf = tmp;
                cap = new_cap;
            }
        }
    }
    buf[total] = '\0';
    CLOSE(fd);
    *out_content = buf;
    return 0;
}

static int write_file(const char* path, const char* data) {
    FILE* f = fopen(path, "wb");
    if (!f) return errno;
    size_t len = data ? strlen(data) : 0;
    if (len && fwrite(data, 1, len, f) != len) {
        int e = ferror(f);
        fclose(f);
        return e ? e : EIO;
    }
    fclose(f);
    return 0;
}

int main(int argc, char** argv) {
    /* If a path is provided by user, read and print it. */
    if (argc > 1) {
        char* content = NULL;
        int rc = read_file_secure(argv[1], &content);
        if (rc == 0) {
            fputs(content, stdout);
            free(content);
        } else {
            fprintf(stderr, "Error: %s\n", strerror(rc));
        }
    }

    /* 5 test cases */
    char dir_template[] = "/tmp/task36_tests_c_XXXXXX";
#ifndef _WIN32
    char* d = mkdtemp(dir_template);
    if (!d) {
        fprintf(stderr, "Test setup error: %s\n", strerror(errno));
        return 1;
    }
    char t1[PATH_MAX], t2[PATH_MAX], t3[PATH_MAX], t4[PATH_MAX], t5[PATH_MAX];
    snprintf(t1, sizeof(t1), "%s/%s", d, "small.txt");
    snprintf(t2, sizeof(t2), "%s/%s", d, "empty.txt");
    snprintf(t3, sizeof(t3), "%s/%s", d, "unicode.txt");
    snprintf(t4, sizeof(t4), "%s/%s", d, "missing.txt");
    snprintf(t5, sizeof(t5), "%s/%s", d, "limit.txt");

    write_file(t1, "Hello\nWorld\n");         /* Test 1: small */
    write_file(t2, "");                        /* Test 2: empty */
    write_file(t3, "こんにちは\n\xF0\x9F\x99\x82\n"); /* Test 3: unicode (UTF-8) */
    /* Test 4: missing - do not create */
    {
        FILE* f = fopen(t5, "wb");             /* Test 5: 1KB */
        if (f) {
            for (int i = 0; i < 1024; ++i) fputc('A', f);
            fclose(f);
        }
    }

    const char* tests[5] = { t1, t2, t3, t4, t5 };
    for (int i = 0; i < 5; ++i) {
        printf("\n--- Reading: %s ---\n", tests[i]);
        char* content = NULL;
        int rc = read_file_secure(tests[i], &content);
        if (rc == 0) {
            fputs(content, stdout);
            free(content);
        } else {
            printf("Error: %s\n", strerror(rc));
        }
    }
#else
    /* On Windows, simple temp directory simulation */
    char t1[MAX_PATH] = "small_win.txt";
    char t2[MAX_PATH] = "empty_win.txt";
    char t3[MAX_PATH] = "unicode_win.txt";
    char t4[MAX_PATH] = "missing_win.txt";
    char t5[MAX_PATH] = "limit_win.txt";

    write_file(t1, "Hello\nWorld\n");
    write_file(t2, "");
    write_file(t3, "こんにちは\n");
    {
        FILE* f = fopen(t5, "wb");
        if (f) { for (int i = 0; i < 1024; ++i) fputc('A', f); fclose(f); }
    }

    const char* tests[5] = { t1, t2, t3, t4, t5 };
    for (int i = 0; i < 5; ++i) {
        printf("\n--- Reading: %s ---\n", tests[i]);
        char* content = NULL;
        int rc = read_file_secure(tests[i], &content);
        if (rc == 0) {
            fputs(content, stdout);
            free(content);
        } else {
            printf("Error: %s\n", strerror(rc));
        }
    }
#endif
    return 0;
}