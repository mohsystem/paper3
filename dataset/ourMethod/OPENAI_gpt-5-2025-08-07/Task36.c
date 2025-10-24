#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

#define MAX_FILE_SIZE (10 * 1024 * 1024) /* 10 MB */

static int is_valid_filename(const char* name) {
    if (name == NULL) return 0;
    size_t len = strlen(name);
    if (len == 0 || len > 255) return 0;
    for (size_t i = 0; i < len; ++i) {
        char c = name[i];
        if (!((c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') ||
              c == '.' || c == '_' || c == '-')) {
            return 0;
        }
    }
    return 1;
}

static int canonicalize_dir(const char* dir, char outCanonical[PATH_MAX], char err[256]) {
    if (dir == NULL) {
        snprintf(err, 256, "Base directory is NULL");
        return 0;
    }
    if (strlen(dir) == 0 || strlen(dir) >= PATH_MAX) {
        snprintf(err, 256, "Invalid base directory length");
        return 0;
    }
    errno = 0;
    if (realpath(dir, outCanonical) == NULL) {
        snprintf(err, 256, "Failed to resolve base directory: %s", strerror(errno));
        return 0;
    }
    struct stat st;
    if (stat(outCanonical, &st) != 0) {
        snprintf(err, 256, "Failed to stat base directory: %s", strerror(errno));
        return 0;
    }
    if (!S_ISDIR(st.st_mode)) {
        snprintf(err, 256, "Base path is not a directory");
        return 0;
    }
    return 1;
}

/* Reads a file with a strict filename inside baseDir.
   On success returns 1 and allocates *out_buf with length *out_len (caller frees).
   On failure returns 0 and sets err. */
static int secure_read_file(const char* baseDir,
                            const char* userFilename,
                            unsigned char** out_buf,
                            size_t* out_len,
                            char err[256]) {
    if (out_buf == NULL || out_len == NULL || err == NULL) return 0;
    *out_buf = NULL;
    *out_len = 0;
    err[0] = '\0';

    if (!is_valid_filename(userFilename)) {
        snprintf(err, 256, "Invalid filename: only letters, digits, '.', '_', '-' allowed; length 1..255");
        return 0;
    }

    char canonBase[PATH_MAX];
    if (!canonicalize_dir(baseDir, canonBase, err)) {
        return 0;
    }

    char fullPath[PATH_MAX];
    size_t blen = strlen(canonBase);
    if (blen + 1 + strlen(userFilename) >= sizeof(fullPath)) {
        snprintf(err, 256, "Path too long");
        return 0;
    }
    strcpy(fullPath, canonBase);
    if (blen == 0 || fullPath[blen - 1] != '/') {
        fullPath[blen] = '/';
        fullPath[blen + 1] = '\0';
    }
    strncat(fullPath, userFilename, sizeof(fullPath) - strlen(fullPath) - 1);

    int fd = open(fullPath, O_RDONLY | O_NOFOLLOW);
    if (fd < 0) {
        snprintf(err, 256, "Open failed: %s", strerror(errno));
        return 0;
    }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        snprintf(err, 256, "fstat failed: %s", strerror(errno));
        close(fd);
        return 0;
    }
    if (!S_ISREG(st.st_mode)) {
        snprintf(err, 256, "Not a regular file");
        close(fd);
        return 0;
    }
    if (st.st_size > 0 && (unsigned long)st.st_size > MAX_FILE_SIZE) {
        snprintf(err, 256, "File too large (exceeds limit)");
        close(fd);
        return 0;
    }

    unsigned char* buf = NULL;
    size_t cap = 8192;
    size_t total = 0;
    buf = (unsigned char*)malloc(cap);
    if (buf == NULL) {
        snprintf(err, 256, "Memory allocation failed");
        close(fd);
        return 0;
    }

    for (;;) {
        if (total == cap) {
            size_t new_cap = cap * 2;
            if (new_cap > MAX_FILE_SIZE) new_cap = MAX_FILE_SIZE;
            if (new_cap == cap) {
                snprintf(err, 256, "File too large (exceeds limit)");
                free(buf);
                close(fd);
                return 0;
            }
            unsigned char* nbuf = (unsigned char*)realloc(buf, new_cap);
            if (nbuf == NULL) {
                snprintf(err, 256, "Memory reallocation failed");
                free(buf);
                close(fd);
                return 0;
            }
            buf = nbuf;
            cap = new_cap;
        }
        ssize_t n = read(fd, buf + total, cap - total);
        if (n < 0) {
            if (errno == EINTR) continue;
            snprintf(err, 256, "Read error: %s", strerror(errno));
            free(buf);
            close(fd);
            return 0;
        }
        if (n == 0) break; /* EOF */
        total += (size_t)n;
        if (total > MAX_FILE_SIZE) {
            snprintf(err, 256, "File too large (exceeds limit)");
            free(buf);
            close(fd);
            return 0;
        }
    }

    close(fd);
    *out_buf = buf;
    *out_len = total;
    return 1;
}

/* Test helpers */
static int make_dir(const char* path) {
    if (mkdir(path, 0700) == 0) return 1;
    if (errno == EEXIST) return 1;
    return 0;
}

static int write_file_atomic(const char* path, const char* content, char err[256]) {
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW, 0600);
    if (fd < 0) {
        snprintf(err, 256, "Create failed: %s", strerror(errno));
        return 0;
    }
    size_t len = strlen(content);
    size_t total = 0;
    while (total < len) {
        ssize_t n = write(fd, content + total, len - total);
        if (n < 0) {
            if (errno == EINTR) continue;
            snprintf(err, 256, "Write failed: %s", strerror(errno));
            close(fd);
            return 0;
        }
        total += (size_t)n;
    }
    if (fsync(fd) != 0) {
        snprintf(err, 256, "fsync failed: %s", strerror(errno));
        close(fd);
        return 0;
    }
    if (close(fd) != 0) {
        snprintf(err, 256, "close failed: %s", strerror(errno));
        return 0;
    }
    return 1;
}

static int path_join(const char* a, const char* b, char out[PATH_MAX]) {
    if (a == NULL || b == NULL) return 0;
    size_t alen = strlen(a);
    if (alen + 1 + strlen(b) >= PATH_MAX) return 0;
    strcpy(out, a);
    if (alen == 0 || out[alen - 1] != '/') {
        out[alen] = '/';
        out[alen + 1] = '\0';
    }
    strncat(out, b, PATH_MAX - strlen(out) - 1);
    return 1;
}

static int make_temp_base_dir(char out[PATH_MAX]) {
    char tmpl[] = "/tmp/secure_read_c_XXXXXX";
    char* res = mkdtemp(tmpl);
    if (res == NULL) return 0;
    strncpy(out, res, PATH_MAX - 1);
    out[PATH_MAX - 1] = '\0';
    return 1;
}

int main(void) {
    char base[PATH_MAX];
    if (!make_temp_base_dir(base)) {
        fprintf(stderr, "Failed to create temp base directory\n");
        return 1;
    }

    char err[256];
    char p[PATH_MAX];

    /* Create test files and directories */
    if (path_join(base, "file1.txt", p)) {
        write_file_atomic(p, "Hello from C file1\nC-Line2\n", err);
    }
    if (path_join(base, "adir", p)) {
        make_dir(p);
    }
    if (path_join(base, "subdir", p)) {
        make_dir(p);
    }
    if (path_join(base, "subdir_nested.txt", p)) {
        write_file_atomic(p, "This is a separate file (name suggests nested)\n", err);
    }
    /* Symlink (ignore failure on systems without permissions) */
    char link_src[PATH_MAX], link_dst[PATH_MAX];
    if (path_join(base, "file1.txt", link_src) && path_join(base, "link_to_file1", link_dst)) {
        symlink(link_src, link_dst);
    }

    struct {
        const char* filename;
        const char* desc;
    } tests[5] = {
        { "file1.txt", "Valid existing file" },
        { "nonexistent.txt", "Non-existent file" },
        { "link_to_file1", "Symlink should be refused" },
        { "adir", "Directory should be refused" },
        { "../etc_passwd", "Path traversal attempt (invalid filename)" }
    };

    for (int i = 0; i < 5; ++i) {
        unsigned char* buf = NULL;
        size_t len = 0;
        char e[256] = {0};
        int ok = secure_read_file(base, tests[i].filename, &buf, &len, e);
        printf("Test: %s [%s]\n", tests[i].desc, tests[i].filename);
        if (ok) {
            printf("----- BEGIN CONTENT -----\n");
            if (len > 0) {
                fwrite(buf, 1, len, stdout);
                if (buf[len - 1] != '\n') fputc('\n', stdout);
            }
            printf("------ END CONTENT ------\n");
        } else {
            printf("Error: %s\n", e);
        }
        printf("=========================\n");
        free(buf);
    }

    return 0;
}