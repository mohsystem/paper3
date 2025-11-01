#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define TAR_BLOCK 512
#define MAX_FILE_SIZE 1000000000LL

static ssize_t read_full(int fd, void *buf, size_t n) {
    size_t off = 0;
    while (off < n) {
        ssize_t r = read(fd, (char*)buf + off, n - off);
        if (r == 0) break;
        if (r < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        off += (size_t)r;
    }
    return (ssize_t)off;
}

static int is_zero_block(const unsigned char *b) {
    for (size_t i = 0; i < TAR_BLOCK; ++i) {
        if (b[i] != 0) return 0;
    }
    return 1;
}

static void read_tar_string(const unsigned char* hdr, size_t off, size_t len, char* out, size_t outsz) {
    size_t end = off + len;
    size_t i = off;
    for (; i < end; ++i) if (hdr[i] == 0) break;
    size_t n = (i - off);
    if (n >= outsz) n = outsz - 1;
    memcpy(out, hdr + off, n);
    out[n] = '\0';
}

static int parse_octal(const unsigned char* hdr, size_t off, size_t len, long long* out) {
    *out = 0;
    size_t i = off;
    size_t end = off + len;
    while (i < end && (hdr[i] == 0 || hdr[i] == ' ')) i++;
    for (; i < end; ++i) {
        unsigned char c = hdr[i];
        if (c == 0 || c == ' ') break;
        if (c < '0' || c > '7') break;
        int digit = c - '0';
        if (*out > (LLONG_MAX >> 3)) return 0;
        *out = (*out << 3) + digit;
    }
    return 1;
}

static int is_safe_relpath(const char* p) {
    if (!p || p[0] == '\0') return 0;
    if (p[0] == '/') return 0;
    const char* s = p;
    while (*s) {
        const char* slash = strchr(s, '/');
        size_t len = slash ? (size_t)(slash - s) : strlen(s);
        if (len == 0) return 0;
        if ((len == 1 && s[0] == '.') || (len == 2 && s[0] == '.' && s[1] == '.')) return 0;
        if (!slash) break;
        s = slash + 1;
    }
    return 1;
}

static int mkdirs_at(int basefd, const char* rel_dir, int* out_fd) {
    int current = dup(basefd);
    if (current < 0) return -1;
    if (rel_dir[0] == '\0') { *out_fd = current; return 0; }

    char path[PATH_MAX];
    if (strlen(rel_dir) >= sizeof(path)) { close(current); errno = ENAMETOOLONG; return -1; }
    strcpy(path, rel_dir);
    char* s = path;
    while (*s) {
        char* slash = strchr(s, '/');
        if (slash) *slash = '\0';
        if (strcmp(s, ".") == 0 || strcmp(s, "..") == 0 || s[0] == '\0') { close(current); errno = EINVAL; return -1; }
        int nextfd = openat(current, s, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
        if (nextfd < 0) {
            if (errno != ENOENT) { close(current); return -1; }
            if (mkdirat(current, s, 0700) != 0) { close(current); return -1; }
            nextfd = openat(current, s, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
            if (nextfd < 0) { close(current); return -1; }
        }
        close(current);
        current = nextfd;
        if (!slash) break;
        s = slash + 1;
    }
    *out_fd = current;
    return 0;
}

static int create_parent_and_open_file(int basefd, const char* relpath, int* out_fd) {
    char path[PATH_MAX];
    if (strlen(relpath) >= sizeof(path)) { errno = ENAMETOOLONG; return -1; }
    strcpy(path, relpath);
    char* last_slash = strrchr(path, '/');
    int parentfd = -1;
    const char* fname = path;
    if (last_slash) {
        *last_slash = '\0';
        fname = last_slash + 1;
        if (mkdirs_at(basefd, path, &parentfd) != 0) return -1;
    } else {
        parentfd = dup(basefd);
        if (parentfd < 0) return -1;
    }
    if (fname[0] == '\0' || strcmp(fname, ".") == 0 || strcmp(fname, "..") == 0) { close(parentfd); errno = EINVAL; return -1; }
    int fd = openat(parentfd, fname, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
    int saved = errno;
    close(parentfd);
    errno = saved;
    if (fd < 0) return -1;
    struct stat st;
    if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode)) {
        int e = errno;
        close(fd);
        errno = e ? e : EINVAL;
        return -1;
    }
    *out_fd = fd;
    return 0;
}

static int extract_tar_secure_from_fd(int tarfd, int destfd) {
    unsigned char header[TAR_BLOCK];
    while (1) {
        ssize_t r = read_full(tarfd, header, TAR_BLOCK);
        if (r == 0) break;
        if (r < 0) return -1;
        if ((size_t)r < TAR_BLOCK) { errno = EINVAL; return -1; }
        if (is_zero_block(header)) {
            // possible second zero block
            r = read_full(tarfd, header, TAR_BLOCK);
            if (r < 0) return -1;
            if ((size_t)r < TAR_BLOCK || is_zero_block(header)) break;
            errno = EINVAL; return -1;
        }
        char name[101]; read_tar_string(header, 0, 100, name, sizeof name);
        char prefix[156]; read_tar_string(header, 345, 155, prefix, sizeof prefix);
        char rel[PATH_MAX];
        if (prefix[0] != '\0') {
            if ((strlen(prefix) + 1 + strlen(name)) >= sizeof(rel)) { errno = ENAMETOOLONG; return -1; }
            snprintf(rel, sizeof(rel), "%s/%s", prefix, name);
        } else {
            if (strlen(name) >= sizeof(rel)) { errno = ENAMETOOLONG; return -1; }
            strcpy(rel, name);
        }
        if (!is_safe_relpath(rel)) {
            long long size = 0;
            if (!parse_octal(header, 124, 12, &size) || size < 0) return -1;
            long long skip = size + ((TAR_BLOCK - (size % TAR_BLOCK)) % TAR_BLOCK);
            while (skip > 0) {
                char buf[4096];
                ssize_t toread = (ssize_t)((skip > (long long)sizeof(buf)) ? sizeof(buf) : skip);
                ssize_t nr = read(tarfd, buf, (size_t)toread);
                if (nr <= 0) return -1;
                skip -= nr;
            }
            continue;
        }
        unsigned char typeflag = header[156];
        long long fsize = 0;
        if (!parse_octal(header, 124, 12, &fsize) || fsize < 0 || fsize > MAX_FILE_SIZE) { errno = EINVAL; return -1; }

        if (typeflag == '5') {
            int dirfd = -1;
            if (mkdirs_at(destfd, rel, &dirfd) != 0) return -1;
            close(dirfd);
        } else if (typeflag == '0' || typeflag == 0) {
            int outfd = -1;
            if (create_parent_and_open_file(destfd, rel, &outfd) != 0) return -1;
            long long remaining = fsize;
            char buf[8192];
            while (remaining > 0) {
                ssize_t toread = (ssize_t)((remaining > (long long)sizeof(buf)) ? sizeof(buf) : remaining);
                ssize_t nr = read(tarfd, buf, (size_t)toread);
                if (nr <= 0) { close(outfd); return -1; }
                ssize_t off = 0;
                while (off < nr) {
                    ssize_t nw = write(outfd, buf + off, (size_t)(nr - off));
                    if (nw < 0) { if (errno == EINTR) continue; close(outfd); return -1; }
                    off += nw;
                }
                remaining -= nr;
            }
            fsync(outfd);
            close(outfd);
            long long pad = (TAR_BLOCK - (fsize % TAR_BLOCK)) % TAR_BLOCK;
            if (pad > 0) {
                char padbuf[512];
                if (read_full(tarfd, padbuf, (size_t)pad) < (ssize_t)pad) return -1;
            }
        } else {
            long long skip = fsize + ((TAR_BLOCK - (fsize % TAR_BLOCK)) % TAR_BLOCK);
            while (skip > 0) {
                char buf[4096];
                ssize_t toread = (ssize_t)((skip > (long long)sizeof(buf)) ? sizeof(buf) : skip);
                ssize_t nr = read(tarfd, buf, (size_t)toread);
                if (nr <= 0) return -1;
                skip -= nr;
            }
        }
    }
    fsync(destfd);
    return 0;
}

int extract_tar_secure(const char* archive_path, const char* dest_dir) {
    if (!archive_path || !dest_dir) return -1;
    int tarfd = open(archive_path, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (tarfd < 0) return -1;
    struct stat st;
    if (fstat(tarfd, &st) != 0 || !S_ISREG(st.st_mode)) {
        int e = errno;
        close(tarfd);
        errno = e ? e : EINVAL;
        return -1;
    }
    mkdir(dest_dir, 0700);
    int destfd = open(dest_dir, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (destfd < 0) {
        int e = errno;
        close(tarfd);
        errno = e;
        return -1;
    }
    int rc = extract_tar_secure_from_fd(tarfd, destfd);
    int e = errno;
    close(tarfd);
    fsync(destfd);
    close(destfd);
    errno = e;
    return rc;
}

int main(void) {
    const char* tests[5] = {
        "archive.zip",
        "./archive.zip",
        "/does/not/exist/archive.zip",
        "archive.zip",
        "archive.zip"
    };
    for (int i = 0; i < 5; ++i) {
        int rc = extract_tar_secure(tests[i], "/tmp/unpack");
        if (rc == 0) {
            printf("Test %d: OK\n", i + 1);
        } else {
            printf("Test %d: %s\n", i + 1, strerror(errno));
        }
    }
    return 0;
}