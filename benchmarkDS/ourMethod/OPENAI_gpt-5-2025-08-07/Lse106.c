#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>

// Secure random bytes via /dev/urandom
static bool secure_random_bytes(unsigned char* buf, size_t len) {
    int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC
#ifdef O_NOFOLLOW
        | O_NOFOLLOW
#endif
    );
    if (fd < 0) return false;
    size_t off = 0;
    while (off < len) {
        ssize_t n = read(fd, buf + off, len - off);
        if (n < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return false;
        }
        if (n == 0) {
            close(fd);
            return false;
        }
        off += (size_t)n;
    }
    close(fd);
    return true;
}

static bool is_valid_relname(const char* name) {
    if (name == NULL) return false;
    size_t len = strlen(name);
    if (len == 0 || len > 255) return false;
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) return false;
    for (size_t i = 0; i < len; i++) {
        char c = name[i];
        if (!((c >= 'a' && c <= 'z') ||
              (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') ||
              c == '.' || c == '-' || c == '_')) {
            return false;
        }
        if (c == '/' || c == '\\') return false;
    }
    return true;
}

// Returns a newly allocated string with the full path, or NULL on error.
// Caller must free the returned string.
char* write_secret_file(const char* base_dir, const char* filename) {
    if (!is_valid_relname(filename)) {
        return NULL;
    }

    const char* secret = "TOP-SECRET: Highly confidential data. Do not disclose.";
    size_t secret_len = strlen(secret);

    int dfd = open(base_dir, O_RDONLY | O_DIRECTORY | O_CLOEXEC
#ifdef O_NOFOLLOW
        | O_NOFOLLOW
#endif
    );
    if (dfd < 0) {
        return NULL;
    }

    struct stat ds;
    if (fstat(dfd, &ds) != 0 || !S_ISDIR(ds.st_mode)) {
        close(dfd);
        return NULL;
    }

    // Create random temp filename
    unsigned char rnd[12];
    if (!secure_random_bytes(rnd, sizeof(rnd))) {
        close(dfd);
        return NULL;
    }
    char hex[25] = {0};
    for (size_t i = 0; i < sizeof(rnd); ++i) {
        snprintf(&hex[i*2], 3, "%02x", rnd[i]);
    }
    char tmpname[64];
    snprintf(tmpname, sizeof(tmpname), ".tmp_%s.part", hex);

    int flags = O_CREAT | O_EXCL | O_WRONLY | O_CLOEXEC;
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif

    int tfd = openat(dfd, tmpname, flags, 0600);
    if (tfd < 0) {
        close(dfd);
        return NULL;
    }

    struct stat st;
    if (fstat(tfd, &st) != 0 || !S_ISREG(st.st_mode)) {
        close(tfd);
        close(dfd);
        return NULL;
    }

    // Write secret
    const char* p = secret;
    size_t left = secret_len;
    while (left > 0) {
        ssize_t n = write(tfd, p, left);
        if (n < 0) {
            if (errno == EINTR) continue;
            // cleanup
            close(tfd);
            unlinkat(dfd, tmpname, 0);
            close(dfd);
            return NULL;
        }
        p += n;
        left -= (size_t)n;
    }

    if (fsync(tfd) != 0) {
        close(tfd);
        unlinkat(dfd, tmpname, 0);
        close(dfd);
        return NULL;
    }

    if (fchmod(tfd, 0400) != 0) {
        close(tfd);
        unlinkat(dfd, tmpname, 0);
        close(dfd);
        return NULL;
    }

    if (close(tfd) != 0) {
        unlinkat(dfd, tmpname, 0);
        close(dfd);
        return NULL;
    }

    if (renameat(dfd, tmpname, dfd, filename) != 0) {
        unlinkat(dfd, tmpname, 0);
        close(dfd);
        return NULL;
    }

    (void)fsync(dfd);
    close(dfd);

    // Construct full path
    size_t blen = strlen(base_dir);
    size_t flen = strlen(filename);
    size_t need = blen + 1 + flen + 1;
    char* full = (char*)malloc(need);
    if (!full) return NULL;
    if (blen > 0 && base_dir[blen - 1] == '/') {
        snprintf(full, need, "%s%s", base_dir, filename);
    } else {
        snprintf(full, need, "%s/%s", base_dir, filename);
    }
    return full;
}

int main(void) {
    char tmpl[] = "/tmp/safe_base_XXXXXX";
    char* d = mkdtemp(tmpl);
    if (d == NULL) {
        printf("Operation failed\n");
        return 0;
    }

    const char* names[5] = {
        "secret1.txt",
        "secret2.txt",
        "alpha-3.log",
        "note_4.dat",
        "report5"
    };

    for (int i = 0; i < 5; i++) {
        char* path = write_secret_file(d, names[i]);
        if (path) {
            printf("%s\n", path);
            free(path);
        } else {
            printf("Operation failed\n");
        }
    }

    return 0;
}