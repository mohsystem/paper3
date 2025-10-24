#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

enum { MAX_READ_SIZE = 1024 * 1024, MAX_COMPONENT_LEN = 255 };

static int set_error(char **err_out, const char *msg) {
    if (err_out) {
        size_t len = strlen(msg);
        char *buf = (char *)malloc(len + 1);
        if (buf) {
            memcpy(buf, msg, len + 1);
            *err_out = buf;
        }
    }
    return -1;
}

static int append_errno(char **err_out, const char *prefix) {
    if (err_out) {
        const char *sys = strerror(errno);
        size_t lp = strlen(prefix), ls = strlen(sys);
        char *buf = (char *)malloc(lp + 2 + ls + 1);
        if (buf) {
            memcpy(buf, prefix, lp);
            buf[lp] = ':';
            buf[lp + 1] = ' ';
            memcpy(buf + lp + 2, sys, ls + 1);
            *err_out = buf;
        }
    }
    return -1;
}

typedef struct {
    char **items;
    size_t count;
} StrList;

static void free_strlist(StrList *sl) {
    if (!sl) return;
    if (sl->items) {
        for (size_t i = 0; i < sl->count; ++i) {
            free(sl->items[i]);
        }
        free(sl->items);
    }
    sl->items = NULL;
    sl->count = 0;
}

static int is_safe_component(const char *s) {
    size_t len = strlen(s);
    if (len == 0 || len > MAX_COMPONENT_LEN) return 0;
    if ((len == 1 && s[0] == '.') || (len == 2 && s[0] == '.' && s[1] == '.')) return 0;
    for (size_t i = 0; i < len; ++i) {
        if (s[i] == '/') return 0;
    }
    return 1;
}

static int split_and_validate(const char *rel_path, StrList *out, char **err_out) {
    if (!rel_path || !*rel_path) return set_error(err_out, "Empty path not allowed");
    if (rel_path[0] == '/') return set_error(err_out, "Absolute paths are not allowed");

    out->items = NULL;
    out->count = 0;

    size_t cap = 8;
    char **arr = (char **)malloc(cap * sizeof(char *));
    if (!arr) return set_error(err_out, "Out of memory");

    size_t i = 0, start = 0;
    size_t len = strlen(rel_path);
    while (i <= len) {
        if (i == len || rel_path[i] == '/') {
            if (i == start) {
                free(arr);
                return set_error(err_out, "Empty path segment detected (e.g., double slashes)");
            }
            size_t seglen = i - start;
            char *seg = (char *)malloc(seglen + 1);
            if (!seg) {
                free(arr);
                return set_error(err_out, "Out of memory");
            }
            memcpy(seg, rel_path + start, seglen);
            seg[seglen] = '\0';
            if (!is_safe_component(seg)) {
                free(seg);
                free(arr);
                return set_error(err_out, "Unsafe path segment");
            }
            if (out->count == cap) {
                cap *= 2;
                char **tmp = (char **)realloc(arr, cap * sizeof(char *));
                if (!tmp) {
                    free(seg);
                    free(arr);
                    return set_error(err_out, "Out of memory");
                }
                arr = tmp;
            }
            arr[out->count++] = seg;
            start = i + 1;
        }
        i++;
    }
    if (rel_path[len - 1] == '/') {
        for (size_t k = 0; k < out->count; ++k) free(arr[k]);
        free(arr);
        return set_error(err_out, "Path cannot end with slash");
    }

    out->items = arr;
    return 0;
}

// Securely open file under base directory without following symlinks in any component
static int open_under_base(const char *base_dir, const char *rel_path, char **err_out) {
    if (!base_dir || !*base_dir) return set_error(err_out, "Base directory must not be empty");

    StrList comps = {0};
    if (split_and_validate(rel_path, &comps, err_out) != 0) {
        return -1;
    }

    int dirfd = open(base_dir, O_RDONLY | O_DIRECTORY);
    if (dirfd < 0) {
        free_strlist(&comps);
        return append_errno(err_out, "Failed to open base directory");
    }

    for (size_t i = 0; i + 1 < comps.count; ++i) {
        int nextfd = openat(dirfd, comps.items[i], O_RDONLY | O_DIRECTORY | O_NOFOLLOW);
        if (nextfd < 0) {
            int saved = errno;
            close(dirfd);
            free_strlist(&comps);
            errno = saved;
            return append_errno(err_out, "Failed to open directory segment");
        }
        close(dirfd);
        dirfd = nextfd;
    }

    int fd = openat(dirfd, comps.items[comps.count - 1], O_RDONLY | O_NOFOLLOW);
    int saved = errno;
    close(dirfd);
    free_strlist(&comps);
    errno = saved;
    if (fd < 0) {
        return append_errno(err_out, "Failed to open target file");
    }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        int saved2 = errno;
        close(fd);
        errno = saved2;
        return append_errno(err_out, "fstat failed");
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        return set_error(err_out, "Path is not a regular file");
    }
    return fd;
}

static int read_all_fd(int fd, char **content_out, size_t *len_out, char **err_out) {
    if (!content_out || !len_out) return set_error(err_out, "Invalid output pointers");

    struct stat st;
    if (fstat(fd, &st) != 0) {
        return append_errno(err_out, "fstat failed");
    }
    if (st.st_size < 0) return set_error(err_out, "Invalid file size");
    if ((unsigned long long)st.st_size > (unsigned long long)MAX_READ_SIZE) {
        return set_error(err_out, "File too large");
    }

    size_t total = (size_t)st.st_size;
    char *buf = (char *)malloc(total + 1);
    if (!buf) return set_error(err_out, "Out of memory");

    size_t off = 0;
    while (off < total) {
        ssize_t r = read(fd, buf + off, total - off);
        if (r < 0) {
            if (errno == EINTR) continue;
            int saved = errno;
            free(buf);
            errno = saved;
            return append_errno(err_out, "Read error");
        }
        if (r == 0) {
            free(buf);
            return set_error(err_out, "Unexpected end of file");
        }
        off += (size_t)r;
    }
    buf[total] = '\0';
    *content_out = buf;
    *len_out = total;
    return 0;
}

// Public API: read file under base
static int read_file_under_base(const char *base_dir, const char *rel_path, char **content_out, size_t *len_out, char **err_out) {
    int fd = open_under_base(base_dir, rel_path, err_out);
    if (fd < 0) return -1;
    int rc = read_all_fd(fd, content_out, len_out, err_out);
    int saved = errno;
    close(fd);
    errno = saved;
    return rc;
}

// Test helper to write files
static int write_file(const char *path, const char *data, size_t len) {
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) return -1;
    size_t off = 0;
    while (off < len) {
        ssize_t w = write(fd, data + off, len - off);
        if (w < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return -1;
        }
        off += (size_t)w;
    }
    return close(fd);
}

int main(int argc, char *argv[]) {
    if (argc >= 3) {
        const char *base = argv[1];
        const char *rel = argv[2];
        char *content = NULL, *err = NULL;
        size_t len = 0;
        if (read_file_under_base(base, rel, &content, &len, &err) == 0) {
            if (content && len > 0) {
                fwrite(content, 1, len, stdout);
            }
            free(content);
            return 0;
        } else {
            if (err) {
                fprintf(stderr, "Error: %s\n", err);
                free(err);
            } else {
                fprintf(stderr, "Error: unknown\n");
            }
            return 1;
        }
    }

    // Self-tests (5 test cases)
    char tmpl[] = "/tmp/task70_c.XXXXXX";
    char *dir = mkdtemp(tmpl);
    if (!dir) {
        fprintf(stderr, "Failed to create temp dir\n");
        return 1;
    }

    char path_a[512], path_subdir[512], path_b[512], path_link[512];
    snprintf(path_a, sizeof(path_a), "%s/%s", dir, "a.txt");
    snprintf(path_subdir, sizeof(path_subdir), "%s/%s", dir, "subdir");
    snprintf(path_b, sizeof(path_b), "%s/%s", path_subdir, "b.txt");
    snprintf(path_link, sizeof(path_link), "%s/%s", dir, "link_to_a");

    if (mkdir(path_subdir, 0700) != 0) {
        fprintf(stderr, "mkdir failed: %s\n", strerror(errno));
        return 1;
    }
    const char *msgA = "Hello Secure C\n";
    if (write_file(path_a, msgA, strlen(msgA)) != 0) {
        fprintf(stderr, "write a.txt failed\n");
        return 1;
    }
    const char *msgB = "In subdir\n";
    if (write_file(path_b, msgB, strlen(msgB)) != 0) {
        fprintf(stderr, "write b.txt failed\n");
        return 1;
    }
    // Create symlink if possible; ignore error if not supported
    symlink("a.txt", path_link);

    // Test 1: valid read
    {
        char *content = NULL, *err = NULL;
        size_t len = 0;
        int rc = read_file_under_base(dir, "a.txt", &content, &len, &err);
        printf("TEST1 valid: %s\n", rc == 0 ? "OK" : "FAIL");
        if (rc == 0) {
            fwrite(content, 1, len, stdout);
            free(content);
        } else {
            if (err) { printf("ERR: %s\n", err); free(err); }
        }
    }
    // Test 2: absolute path
    {
        char *content = NULL, *err = NULL;
        size_t len = 0;
        int rc = read_file_under_base(dir, "/etc/passwd", &content, &len, &err);
        printf("TEST2 absolute: %s\n", rc == 0 ? "UNEXPECTED_OK" : "EXPECTED_FAIL");
        if (rc != 0 && err) { printf("ERR: %s\n", err); free(err); }
        free(content);
    }
    // Test 3: traversal
    {
        char *content = NULL, *err = NULL;
        size_t len = 0;
        int rc = read_file_under_base(dir, "../a.txt", &content, &len, &err);
        printf("TEST3 traversal: %s\n", rc == 0 ? "UNEXPECTED_OK" : "EXPECTED_FAIL");
        if (rc != 0 && err) { printf("ERR: %s\n", err); free(err); }
        free(content);
    }
    // Test 4: symlink
    {
        char *content = NULL, *err = NULL;
        size_t len = 0;
        int rc = read_file_under_base(dir, "link_to_a", &content, &len, &err);
        printf("TEST4 symlink: %s\n", rc == 0 ? "UNEXPECTED_OK" : "EXPECTED_FAIL");
        if (rc != 0 && err) { printf("ERR: %s\n", err); free(err); }
        free(content);
    }
    // Test 5: directory
    {
        char *content = NULL, *err = NULL;
        size_t len = 0;
        int rc = read_file_under_base(dir, "subdir", &content, &len, &err);
        printf("TEST5 directory: %s\n", rc == 0 ? "UNEXPECTED_OK" : "EXPECTED_FAIL");
        if (rc != 0 && err) { printf("ERR: %s\n", err); free(err); }
        free(content);
    }

    return 0;
}