#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef __linux__
#include <sys/random.h>
#endif
#include <errno.h>

static int fill_random(unsigned char *buf, size_t len) {
#ifdef __linux__
    ssize_t r = getrandom(buf, len, 0);
    if (r == (ssize_t)len) return 0;
#endif
    int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
    if (fd < 0) return -1;
    size_t off = 0;
    while (off < len) {
        ssize_t n = read(fd, buf + off, len - off);
        if (n <= 0) { close(fd); return -1; }
        off += (size_t)n;
    }
    close(fd);
    return 0;
}

static void hex_encode(const unsigned char *in, size_t len, char *out, size_t outsz) {
    static const char *hex = "0123456789abcdef";
    size_t j = 0;
    for (size_t i = 0; i < len && (j + 1) < outsz; ++i) {
        unsigned char b = in[i];
        if (j + 2 >= outsz) break;
        out[j++] = hex[(b >> 4) & 0xF];
        out[j++] = hex[b & 0xF];
    }
    if (j < outsz) out[j] = '\0';
}

int write_important_config(const char *base_dir) {
    if (base_dir == NULL) return -1;
    size_t blen = strnlen(base_dir, 8192);
    if (blen == 0 || blen >= 4096) return -1;

    int dirfd = open(base_dir, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dirfd < 0) return -1;

    struct stat ds;
    if (fstat(dirfd, &ds) != 0 || !S_ISDIR(ds.st_mode)) {
        close(dirfd);
        return -1;
    }

    char tmpname[128];
    unsigned char rnd[8];
    int fd = -1;
    for (int i = 0; i < 10; ++i) {
        if (fill_random(rnd, sizeof(rnd)) != 0) {
            close(dirfd);
            return -1;
        }
        char hex[17];
        hex_encode(rnd, sizeof(rnd), hex, sizeof(hex));
        int n = snprintf(tmpname, sizeof(tmpname), ".impconf.%s.tmp", hex);
        if (n < 0 || (size_t)n >= sizeof(tmpname)) {
            close(dirfd);
            return -1;
        }
        fd = openat(dirfd, tmpname, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);
        if (fd >= 0) break;
        if (errno != EEXIST) {
            // Other errors: give up
            close(dirfd);
            return -1;
        }
    }
    if (fd < 0) {
        close(dirfd);
        return -1;
    }

    struct stat fs;
    if (fstat(fd, &fs) != 0 || !S_ISREG(fs.st_mode)) {
        close(fd);
        unlinkat(dirfd, tmpname, 0);
        close(dirfd);
        return -1;
    }

    const char *data = "important_config";
    size_t len = strlen(data);
    size_t off = 0;
    while (off < len) {
        ssize_t w = write(fd, data + off, len - off);
        if (w <= 0) {
            close(fd);
            unlinkat(dirfd, tmpname, 0);
            close(dirfd);
            return -1;
        }
        off += (size_t)w;
    }

    if (fsync(fd) != 0) {
        close(fd);
        unlinkat(dirfd, tmpname, 0);
        close(dirfd);
        return -1;
    }

    if (close(fd) != 0) {
        unlinkat(dirfd, tmpname, 0);
        close(dirfd);
        return -1;
    }

    if (renameat(dirfd, tmpname, dirfd, "important_config") != 0) {
        unlinkat(dirfd, tmpname, 0);
        close(dirfd);
        return -1;
    }

    (void)fsync(dirfd); // best effort
    close(dirfd);
    return 0;
}

int main(void) {
    const char *tests[5] = {
        ".",
        "c_out1",
        "c_out1/subdir",
        "c out 2",
        "c_out3"
    };
    for (int i = 0; i < 5; ++i) {
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "mkdir -p -- '%s'", tests[i]);
        int r = system(cmd);
        (void)r;
        int res = write_important_config(tests[i]);
        printf("%s: %s\n", tests[i], (res == 0 ? "ok" : "fail"));
    }
    return 0;
}