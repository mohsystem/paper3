#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

static char* dup_string(const char* src) {
    if (!src) return NULL;
    size_t len = strlen(src);
    char* dst = (char*)malloc(len + 1);
    if (!dst) return NULL;
    memcpy(dst, src, len);
    dst[len] = '\0';
    return dst;
}

char* write_secret(const char* secret) {
    if (secret == NULL) {
        return NULL;
    }

    char tmpl[] = "/tmp/secret_XXXXXX";
    int fd = mkstemp(tmpl);
    if (fd == -1) {
        return NULL;
    }

    // Ensure owner read/write during write
    (void)fchmod(fd, 0600);

    size_t len = strlen(secret);
    size_t off = 0;
    while (off < len) {
        ssize_t n = write(fd, secret + off, len - off);
        if (n < 0) {
            int saved = errno;
            close(fd);
            unlink(tmpl);
            errno = saved;
            return NULL;
        }
        off += (size_t)n;
    }

    (void)fsync(fd);
    close(fd);

    // Set to read-only for the owner
    if (chmod(tmpl, 0400) == -1) {
        // ignore, best-effort
    }

    char* out = dup_string(tmpl);
    if (!out) {
        // If allocation fails, best effort to remove file reference from caller
        return NULL;
    }
    return out;
}

int main(void) {
    const char* tests[5] = {
        "super secret info #1",
        "super secret info #2",
        "super secret info #3",
        "super secret info #4",
        "super secret info #5"
    };

    for (int i = 0; i < 5; ++i) {
        char* path = write_secret(tests[i]);
        if (path) {
            printf("%s\n", path);
            free(path);
        } else {
            fprintf(stderr, "Error: failed to write secret for test %d\n", i + 1);
        }
    }
    return 0;
}