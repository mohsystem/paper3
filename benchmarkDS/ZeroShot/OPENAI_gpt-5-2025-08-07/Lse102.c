#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* secure_open_and_write(const char* filename, const char* content) {
    struct stat lst;
    if (lstat(filename, &lst) == 0) {
        if (S_ISLNK(lst.st_mode)) {
            // Refuse to operate on symlinks
            return NULL;
        }
    }

    int flags = O_RDWR | O_CREAT;
#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif

    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH; // 0644
    int fd = open(filename, flags, mode);
    if (fd < 0) {
        return NULL;
    }

    // Enforce permissions (best effort)
    (void)fchmod(fd, mode);

    if (ftruncate(fd, 0) != 0) {
        close(fd);
        return NULL;
    }

    const char* buf = content;
    size_t len = strlen(content);
    size_t off = 0;
    while (off < len) {
        ssize_t n = write(fd, buf + off, len - off);
        if (n < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return NULL;
        }
        off += (size_t)n;
    }

    if (lseek(fd, 0, SEEK_SET) == (off_t)-1) {
        close(fd);
        return NULL;
    }

    size_t cap = 4096;
    size_t sz = 0;
    char* out = (char*)malloc(cap);
    if (!out) {
        close(fd);
        return NULL;
    }

    for (;;) {
        if (sz == cap) {
            size_t ncap = cap * 2;
            char* tmp = (char*)realloc(out, ncap);
            if (!tmp) {
                free(out);
                close(fd);
                return NULL;
            }
            out = tmp;
            cap = ncap;
        }
        ssize_t n = read(fd, out + sz, cap - sz);
        if (n < 0) {
            if (errno == EINTR) continue;
            free(out);
            close(fd);
            return NULL;
        }
        if (n == 0) break;
        sz += (size_t)n;
    }

    // Null-terminate
    if (sz == cap) {
        char* tmp = (char*)realloc(out, cap + 1);
        if (!tmp) {
            free(out);
            close(fd);
            return NULL;
        }
        out = tmp;
        cap += 1;
    }
    out[sz] = '\0';

    close(fd);
    return out;
}

int main(void) {
    const char* filename = "important_config";
    const char* content = "important_config";
    for (int i = 0; i < 5; ++i) {
        char* result = secure_open_and_write(filename, content);
        if (result) {
            printf("Test %d: %s\n", i + 1, result);
            free(result);
        } else {
            printf("Error\n");
        }
    }
    return 0;
}