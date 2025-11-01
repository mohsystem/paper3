#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// Secure file write and read using openat, preventing traversal and symlink following.

static bool path_is_safe_relative(const char *rel) {
    if (rel == NULL || rel[0] == '\0') return false;
    if (rel[0] == '/') return false;
    const char *p = rel;
    while (*p) {
        if (p[0] == '.' && p[1] == '.' && (p[2] == '/' || p[2] == '\0')) return false;
        p++;
    }
    return true;
}

static bool ensure_dirfd(const char *base, int *dirfd_out) {
    int flags = O_RDONLY | O_DIRECTORY | O_CLOEXEC;
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
    int dfd = open(base, flags);
    if (dfd < 0) return false;
    *dirfd_out = dfd;
    return true;
}

static bool mkdirat_parents(int dirfd, const char *relpath) {
    char *path = strdup(relpath);
    if (!path) return false;
    char *p = path;
    while (*p) {
        if (*p == '/') {
            *p = '\0';
            if (path[0] != '\0') {
                if (mkdirat(dirfd, path, 0700) != 0 && errno != EEXIST) { free(path); return false; }
            }
            *p = '/';
        }
        p++;
    }
    if (path[0] != '\0') {
        if (mkdirat(dirfd, path, 0700) != 0 && errno != EEXIST) { free(path); return false; }
    }
    free(path);
    return true;
}

static void secure_zero(void *v, size_t n) {
#if defined(__STDC_LIB_EXT1__)
    memset_s(v, n, 0, n);
#else
    volatile unsigned char *p = (volatile unsigned char *)v;
    while (n--) *p++ = 0;
#endif
}

static char *rand_hex(size_t nbytes) {
    FILE *f = fopen("/dev/urandom", "rb");
    if (!f) return NULL;
    unsigned char *buf = (unsigned char *)malloc(nbytes);
    if (!buf) { fclose(f); return NULL; }
    if (fread(buf, 1, nbytes, f) != nbytes) { free(buf); fclose(f); return NULL; }
    fclose(f);
    static const char *hex = "0123456789abcdef";
    char *out = (char *)malloc(nbytes * 2 + 1);
    if (!out) { free(buf); return NULL; }
    for (size_t i = 0; i < nbytes; i++) {
        out[2 * i] = hex[buf[i] >> 4];
        out[2 * i + 1] = hex[buf[i] & 0x0F];
    }
    out[nbytes * 2] = '\0';
    secure_zero(buf, nbytes);
    free(buf);
    return out;
}

static bool secure_write_file(const char *base, const char *relpath, const char *content, char *err, size_t errlen) {
    if (!path_is_safe_relative(relpath)) { snprintf(err, errlen, "unsafe path"); return false; }
    int dirfd;
    if (!ensure_dirfd(base, &dirfd)) { snprintf(err, errlen, "open base failed"); return false; }

    // Create parents
    char *rel_dup = strdup(relpath);
    if (!rel_dup) { close(dirfd); snprintf(err, errlen, "oom"); return false; }
    char *last_slash = strrchr(rel_dup, '/');
    if (last_slash) {
        *last_slash = '\0';
        if (!mkdirat_parents(dirfd, rel_dup)) {
            free(rel_dup);
            close(dirfd);
            snprintf(err, errlen, "mkdirat failed");
            return false;
        }
    }
    free(rel_dup);

    // Temp file
    char *hex = rand_hex(8);
    if (!hex) { close(dirfd); snprintf(err, errlen, "rand failed"); return false; }
    char tmpname[512];
    const char *fname = strrchr(relpath, '/');
    fname = fname ? fname + 1 : relpath;
    snprintf(tmpname, sizeof tmpname, "%s.tmp.%s", fname, hex);
    free(hex);

    char parent[512];
    const char *slash = strrchr(relpath, '/');
    if (slash) {
        size_t plen = (size_t)(slash - relpath);
        if (plen >= sizeof parent) { close(dirfd); snprintf(err, errlen, "path too long"); return false; }
        memcpy(parent, relpath, plen);
        parent[plen] = '\0';
    } else {
        parent[0] = '\0';
    }

    char tmppath[1024];
    if (parent[0] != '\0') {
        if (snprintf(tmppath, sizeof tmppath, "%s/%s", parent, tmpname) >= (int)sizeof tmppath) {
            close(dirfd); snprintf(err, errlen, "path too long"); return false;
        }
    } else {
        if (snprintf(tmppath, sizeof tmppath, "%s", tmpname) >= (int)sizeof tmppath) {
            close(dirfd); snprintf(err, errlen, "path too long"); return false;
        }
    }

    int oflags = O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC;
#ifdef O_NOFOLLOW
    oflags |= O_NOFOLLOW;
#endif
    int fd = openat(dirfd, tmppath, oflags, 0600);
    if (fd < 0) {
        close(dirfd);
        snprintf(err, errlen, "open tmp fail");
        return false;
    }

    size_t len = strlen(content);
    const char *p = content;
    while (len > 0) {
        ssize_t w = write(fd, p, len > 65536 ? 65536 : len);
        if (w < 0) {
            close(fd);
            unlinkat(dirfd, tmppath, 0);
            close(dirfd);
            snprintf(err, errlen, "write fail");
            return false;
        }
        p += w;
        len -= (size_t)w;
    }
    if (fsync(fd) != 0) {
        close(fd);
        unlinkat(dirfd, tmppath, 0);
        close(dirfd);
        snprintf(err, errlen, "fsync fail");
        return false;
    }
    if (close(fd) != 0) {
        unlinkat(dirfd, tmppath, 0);
        close(dirfd);
        snprintf(err, errlen, "close fail");
        return false;
    }

    if (renameat(dirfd, tmppath, dirfd, relpath) != 0) {
        unlinkat(dirfd, tmppath, 0);
        close(dirfd);
        snprintf(err, errlen, "rename fail");
        return false;
    }

    close(dirfd);
    return true;
}

static bool secure_read_file(const char *base, const char *relpath, char **out, size_t *outlen, char *err, size_t errlen) {
    if (!path_is_safe_relative(relpath)) { snprintf(err, errlen, "unsafe path"); return false; }
    int dirfd;
    if (!ensure_dirfd(base, &dirfd)) { snprintf(err, errlen, "open base failed"); return false; }

    int flags = O_RDONLY | O_CLOEXEC;
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
    int fd = openat(dirfd, relpath, flags);
    if (fd < 0) { close(dirfd); snprintf(err, errlen, "open fail"); return false; }

    struct stat st{};
    if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode)) { close(fd); close(dirfd); snprintf(err, errlen, "not regular"); return false; }
    if (st.st_size < 0 || st.st_size > 10 * 1024 * 1024) { close(fd); close(dirfd); snprintf(err, errlen, "too large"); return false; }

    size_t cap = (size_t)st.st_size;
    char *buf = (char *)malloc(cap + 1);
    if (!buf) { close(fd); close(dirfd); snprintf(err, errlen, "oom"); return false; }

    size_t pos = 0;
    while (pos < (size_t)st.st_size) {
        ssize_t r = read(fd, buf + pos, (size_t)st.st_size - pos);
        if (r < 0) { free(buf); close(fd); close(dirfd); snprintf(err, errlen, "read fail"); return false; }
        if (r == 0) break;
        pos += (size_t)r;
    }
    buf[pos] = '\0';
    *out = buf;
    *outlen = pos;

    close(fd);
    close(dirfd);
    return true;
}

int main(void) {
    const char *base = "/tmp/unpack_c";
    mkdir(base, 0700);

    // Test 1: write a.txt
    {
        char err[64];
        bool ok = secure_write_file(base, "a.txt", "hello c", err, sizeof err);
        printf("Test1 write a.txt: %s\n", ok ? "OK" : err);
    }

    // Test 2: write nested sub/b.txt
    {
        char err[64];
        bool ok = secure_write_file(base, "sub/b.txt", "nested content", err, sizeof err);
        printf("Test2 write sub/b.txt: %s\n", ok ? "OK" : err);
    }

    // Test 3: traversal attempt ../evil.txt
    {
        char err[64];
        bool ok = secure_write_file(base, "../evil.txt", "nope", err, sizeof err);
        printf("Test3 traversal: %s\n", ok ? "UNEXPECTED OK" : "Blocked");
    }

    // Test 4: absolute path attempt
    {
        char err[64];
        bool ok = secure_write_file(base, "/etc/passwd", "nope", err, sizeof err);
        printf("Test4 absolute: %s\n", ok ? "UNEXPECTED OK" : "Blocked");
    }

    // Test 5: read back sub/b.txt
    {
        char err[64];
        char *data = NULL;
        size_t len = 0;
        bool ok = secure_read_file(base, "sub/b.txt", &data, &len, err, sizeof err);
        if (ok) {
            printf("Test5 read sub/b.txt: OK len=%zu\n", len);
            free(data);
        } else {
            printf("Test5 read sub/b.txt: %s\n", err);
        }
    }

    return 0;
}