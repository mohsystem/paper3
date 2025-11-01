#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif
#ifndef O_DIRECTORY
#define O_DIRECTORY 0
#endif

static bool validate_filename(const char* name) {
    if (!name) return false;
    size_t len = strnlen(name, 512);
    if (len == 0 || len > 255) return false;
    if ((strcmp(name, ".") == 0) || (strcmp(name, "..") == 0)) return false;
    for (size_t i = 0; i < len; ++i) {
        char c = name[i];
        if (c == '/' || c == '\\') return false;
        if (!((c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') ||
              c == '.' || c == '_' || c == '-')) {
            return false;
        }
    }
    return true;
}

static int open_dirfd_secure(const char* base_dir) {
    if (!base_dir) {
        errno = EINVAL;
        return -1;
    }
    int flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY | O_NOFOLLOW;
    int dirfd = open(base_dir, flags);
    if (dirfd < 0) return -1;
    struct stat st;
    if (fstat(dirfd, &st) != 0 || !S_ISDIR(st.st_mode)) {
        close(dirfd);
        errno = ENOTDIR;
        return -1;
    }
    return dirfd;
}

static bool read_urandom(unsigned char* buf, size_t len) {
    int ur = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
    if (ur < 0) return false;
    size_t off = 0;
    while (off < len) {
        ssize_t n = read(ur, buf + off, len - off);
        if (n < 0) {
            if (errno == EINTR) continue;
            close(ur);
            return false;
        }
        if (n == 0) break;
        off += (size_t)n;
    }
    close(ur);
    return off == len;
}

static bool gen_tmpname(char* out, size_t out_sz) {
    // .tmp. + 32 hex chars + NUL => need at least 38
    if (out_sz < 40) return false;
    unsigned char rnd[16];
    if (!read_urandom(rnd, sizeof(rnd))) {
        // fallback to rand
        srand((unsigned int)time(NULL) ^ (unsigned int)getpid());
        for (size_t i = 0; i < sizeof(rnd); ++i) rnd[i] = (unsigned char)(rand() & 0xFF);
    }
    static const char hexd[] = "0123456789abcdef";
    size_t pos = 0;
    const char prefix[] = ".tmp.";
    for (size_t i = 0; i < sizeof(prefix) - 1; ++i) out[pos++] = prefix[i];
    for (size_t i = 0; i < sizeof(rnd); ++i) {
        out[pos++] = hexd[(rnd[i] >> 4) & 0xF];
        out[pos++] = hexd[rnd[i] & 0xF];
    }
    out[pos] = '\0';
    return true;
}

static bool write_all(int fd, const void* buf, size_t len) {
    const unsigned char* p = (const unsigned char*)buf;
    size_t off = 0;
    while (off < len) {
        ssize_t n = write(fd, p + off, len - off);
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        off += (size_t)n;
    }
    return true;
}

int create_secret_file(const char* base_dir, const char* filename, const char* content) {
    if (!validate_filename(filename)) {
        return -1;
    }
    int dirfd = open_dirfd_secure(base_dir);
    if (dirfd < 0) {
        return -1;
    }

    char tmpname[64];
    if (!gen_tmpname(tmpname, sizeof(tmpname))) {
        close(dirfd);
        errno = EIO;
        return -1;
    }

    int flags = O_CREAT | O_EXCL | O_WRONLY | O_CLOEXEC | O_NOFOLLOW;
    mode_t mode = 0600;
    int fd = openat(dirfd, tmpname, flags, mode);
    if (fd < 0) {
        close(dirfd);
        return -1;
    }

    size_t content_len = content ? strnlen(content, 4096) : 0;
    if (content_len == 0 || content_len > 4096) {
        close(fd);
        close(dirfd);
        errno = EINVAL;
        return -1;
    }

    int ret = 0;
    if (!write_all(fd, content, content_len)) {
        ret = -1;
    }
    if (ret == 0 && fsync(fd) != 0) {
        ret = -1;
    }
    if (ret == 0 && fchmod(fd, 0600) != 0) {
        ret = -1;
    }
    if (ret == 0 && renameat(dirfd, tmpname, dirfd, filename) != 0) {
        ret = -1;
    }
    if (ret == 0 && fsync(dirfd) != 0) {
        ret = -1;
    }

    close(fd);
    close(dirfd);
    return ret;
}

int main(void) {
    const char* content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
    int overall = 0;
    for (int i = 1; i <= 5; ++i) {
        char templ[] = "/tmp/secure_secret_test_c_XXXXXX";
        char* dir = mkdtemp(templ);
        if (!dir) {
            printf("Test %d FAILED: mkdtemp\n", i);
            overall = 1;
            continue;
        }
        int rc = create_secret_file(dir, "secret.txt", content);
        if (rc == 0) {
            printf("Test %d OK: %s/secret.txt\n", i, dir);
        } else {
            printf("Test %d FAILED\n", i);
            overall = 1;
        }
    }
    return overall;
}