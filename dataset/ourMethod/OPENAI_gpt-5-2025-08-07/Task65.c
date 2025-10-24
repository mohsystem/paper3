/*
Secure file reader in C.

Features:
- Retrieves a filename from a command line argument (or runs 5 tests if no argument is provided).
- Validates filename format using a strict whitelist (no path separators, 1..255 length, allowed chars only).
- Anchors file access to a trusted base directory and uses openat with O_NOFOLLOW to avoid symlink traversal.
- Opens the file first, then validates type and size via fstat (prevents TOCTOU).
- Enforces a maximum file size limit and reads within bounds.
- Returns the file data from a function as a dynamically allocated buffer.
- Handles errors gracefully with clear, generic messages, without exposing internal details.

Build: cc -Wall -Wextra -O2 -std=c11 -D_POSIX_C_SOURCE=200809L task65.c -o task65
Run:
  - CLI mode: ./task65 filename
  - Test mode (5 cases): ./task65 (no arguments)
*/

#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/* Policy limits */
#define MAX_FILENAME_LEN 255
#define MAX_FILE_SIZE (1024 * 1024) /* 1 MiB */

typedef struct ReadResult {
    char *data;
    size_t size;
    int error; /* 0 on success, else errno-like code */
} ReadResult;

/* Safe addition check for size_t */
static int safe_add_size(size_t a, size_t b, size_t *out) {
#if defined(__has_builtin)
#  if __has_builtin(__builtin_add_overflow)
    return __builtin_add_overflow(a, b, out);
#  endif
#elif defined(__GNUC__) && (__GNUC__ >= 5)
    return __builtin_add_overflow(a, b, out);
#endif
    if (SIZE_MAX - a < b) return 1;
    *out = a + b;
    return 0;
}

/* Minimal, safe, fixed-format error copy to buffer */
static void set_error(char *errbuf, size_t errbufsz, const char *msg) {
    if (!errbuf || errbufsz == 0) return;
    /* Use snprintf to ensure null termination */
    (void)snprintf(errbuf, errbufsz, "%s", msg ? msg : "error");
}

/* Validate a filename: must be 1..255, only [A-Za-z0-9._-], not "." or "..", no path separators */
static int validate_filename(const char *name) {
    if (name == NULL) return 0;
    size_t len = strnlen(name, MAX_FILENAME_LEN + 1);
    if (len == 0 || len > MAX_FILENAME_LEN) return 0;
    if ((len == 1 && name[0] == '.') || (len == 2 && name[0] == '.' && name[1] == '.')) return 0;

    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)name[i];
        if (c == '/' || c == '\\') return 0; /* disallow path separators */
        if ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '.' || c == '_' || c == '-') {
            continue;
        }
        return 0; /* disallow all other characters */
    }
    return 1;
}

/* Open a base directory in a race-safe way */
static int open_base_dir(const char *base_path) {
    if (!base_path) {
        errno = EINVAL;
        return -1;
    }
    int flags = O_RDONLY | O_CLOEXEC;
#ifdef O_DIRECTORY
    flags |= O_DIRECTORY;
#endif
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
    int dfd = open(base_path, flags);
    return dfd; /* caller checks for -1 */
}

/* Reads a file securely from an anchored directory.
   Returns a heap buffer with file contents on success; caller must free(result.data). */
static ReadResult read_file_from_base(int dirfd, const char *filename, char *errbuf, size_t errbufsz) {
    ReadResult rr;
    rr.data = NULL;
    rr.size = 0;
    rr.error = 0;

    if (dirfd < 0 || filename == NULL) {
        rr.error = EINVAL;
        set_error(errbuf, errbufsz, "invalid parameters");
        return rr;
    }

    if (!validate_filename(filename)) {
        rr.error = EINVAL;
        set_error(errbuf, errbufsz, "invalid filename");
        return rr;
    }

    int fd = openat(dirfd, filename, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) {
        rr.error = errno;
        set_error(errbuf, errbufsz, "cannot open file");
        return rr;
    }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        rr.error = errno;
        set_error(errbuf, errbufsz, "cannot stat file");
        close(fd);
        return rr;
    }

    if (!S_ISREG(st.st_mode)) {
        rr.error = EISDIR; /* Not a regular file */
        set_error(errbuf, errbufsz, "not a regular file");
        close(fd);
        return rr;
    }

    if (st.st_size < 0) {
        rr.error = EIO;
        set_error(errbuf, errbufsz, "invalid file size");
        close(fd);
        return rr;
    }

    if ((uint64_t)st.st_size > (uint64_t)MAX_FILE_SIZE) {
        rr.error = EFBIG;
        set_error(errbuf, errbufsz, "file too large");
        close(fd);
        return rr;
    }

    size_t alloc_size = 0;
    if (safe_add_size((size_t)st.st_size, 1u, &alloc_size) != 0) {
        rr.error = EOVERFLOW;
        set_error(errbuf, errbufsz, "size overflow");
        close(fd);
        return rr;
    }

    char *buf = (char *)malloc(alloc_size);
    if (!buf) {
        rr.error = ENOMEM;
        set_error(errbuf, errbufsz, "out of memory");
        close(fd);
        return rr;
    }

    size_t to_read = (size_t)st.st_size;
    size_t off = 0;
    while (off < to_read) {
        size_t chunk = to_read - off;
        if (chunk > 1 << 20) chunk = 1 << 20; /* read in up to 1MiB chunks, though to_read is <=1MiB here */
        ssize_t n = read(fd, buf + off, chunk);
        if (n < 0) {
            rr.error = errno;
            set_error(errbuf, errbufsz, "read error");
            free(buf);
            close(fd);
            return rr;
        }
        if (n == 0) {
            /* EOF before expected size: accept what we have */
            break;
        }
        off += (size_t)n;
    }
    buf[off] = '\0'; /* NUL-terminate for convenience; data may be binary */

    rr.data = buf;
    rr.size = off;
    rr.error = 0;
    close(fd);
    return rr;
}

/* Utility to write a test file securely inside dirfd (for test harness) */
static int write_file_in_dir(int dirfd, const char *name, const char *content, size_t len) {
    if (!validate_filename(name)) return -1;
    int fd = openat(dirfd, name, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
    if (fd < 0) return -1;
    size_t off = 0;
    while (off < len) {
        ssize_t n = write(fd, content + off, len - off);
        if (n < 0) {
            int e = errno;
            (void)close(fd);
            errno = e;
            return -1;
        }
        off += (size_t)n;
    }
    if (close(fd) != 0) return -1;
    return 0;
}

/* Utility to create/ensure a subdirectory inside dirfd (for test harness) */
static int mkdir_in_dir(int dirfd, const char *name, mode_t mode) {
    if (!validate_filename(name)) return -1;
#if defined(__linux__) || defined(__APPLE__)
    if (mkdirat(dirfd, name, mode) == 0) return 0;
    if (errno == EEXIST) return 0;
    return -1;
#else
    /* Fallback: derive path (used only in tests) */
    char path[PATH_MAX];
    if (snprintf(path, sizeof(path), "%s", name) >= (int)sizeof(path)) {
        errno = ENAMETOOLONG;
        return -1;
    }
    if (mkdir(path, mode) == 0) return 0;
    if (errno == EEXIST) return 0;
    return -1;
#endif
}

/* Create a large file of target_size bytes (sparse if possible) */
static int create_large_file_in_dir(int dirfd, const char *name, size_t target_size) {
    if (!validate_filename(name)) return -1;
    int fd = openat(dirfd, name, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
    if (fd < 0) return -1;

#if defined(F_TRUNCATE) || defined(__linux__) || defined(__APPLE__)
    if (ftruncate(fd, (off_t)target_size) != 0) {
        int e = errno;
        (void)close(fd);
        errno = e;
        return -1;
    }
#else
    /* Portable write of zeros (may be slow but only in test harness) */
    const size_t chunk = 8192;
    char *zeros = (char *)calloc(1, chunk);
    if (!zeros) {
        (void)close(fd);
        errno = ENOMEM;
        return -1;
    }
    size_t remaining = target_size;
    while (remaining > 0) {
        size_t to_write = remaining < chunk ? remaining : chunk;
        ssize_t n = write(fd, zeros, to_write);
        if (n < 0) {
            int e = errno;
            free(zeros);
            (void)close(fd);
            errno = e;
            return -1;
        }
        remaining -= (size_t)n;
    }
    free(zeros);
#endif
    if (close(fd) != 0) return -1;
    return 0;
}

/* Test harness: creates a sandbox directory and 5 test cases */
static void run_tests(void) {
    const char *sandbox = "sandbox_task65";
    int basefd = open_base_dir(".");
    if (basefd < 0) {
        fprintf(stderr, "[TEST] Cannot open current directory: %s\n", strerror(errno));
        return;
    }

    /* Create sandbox directory (relative to current directory) */
#if defined(__linux__) || defined(__APPLE__)
    if (mkdirat(basefd, sandbox, 0700) != 0 && errno != EEXIST) {
        fprintf(stderr, "[TEST] Cannot create sandbox: %s\n", strerror(errno));
        close(basefd);
        return;
    }
#else
    if (mkdir(sandbox, 0700) != 0 && errno != EEXIST) {
        fprintf(stderr, "[TEST] Cannot create sandbox: %s\n", strerror(errno));
        close(basefd);
        return;
    }
#endif

    int sbfd = openat(basefd, sandbox, O_RDONLY | O_CLOEXEC | O_NOFOLLOW
#ifdef O_DIRECTORY
                      | O_DIRECTORY
#endif
    );
    if (sbfd < 0) {
        fprintf(stderr, "[TEST] Cannot open sandbox: %s\n", strerror(errno));
        close(basefd);
        return;
    }

    /* Prepare files */
    (void)write_file_in_dir(sbfd, "hello.txt", "Hello, world!\n", strlen("Hello, world!\n"));
    (void)write_file_in_dir(sbfd, "unicode.txt", "Sample-123_ABC-xyz.\n", strlen("Sample-123_ABC-xyz.\n"));
    (void)write_file_in_dir(sbfd, "empty.txt", "", 0);
    (void)mkdir_in_dir(sbfd, "subdir", 0700);
    (void)create_large_file_in_dir(sbfd, "too_big.bin", MAX_FILE_SIZE + 1024);

    /* 5 test cases */
    const char *tests[5] = {
        "hello.txt",            /* valid, should succeed */
        "bad/../evil.txt",      /* invalid due to slash */
        "nosuch.txt",           /* non-existent */
        "subdir",               /* directory, not regular file */
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaffffffffffffffffffff.txt" /* >255 len -> invalid */
    };

    for (int i = 0; i < 5; i++) {
        const char *fname = tests[i];
        char err[128];
        ReadResult rr = read_file_from_base(sbfd, fname, err, sizeof(err));
        if (rr.error == 0) {
            /* Print a preview of the data */
            size_t preview = rr.size < 40 ? rr.size : 40;
            printf("[TEST %d] %s: OK, %zu bytes. Preview: \"", i + 1, fname, rr.size);
            for (size_t j = 0; j < preview; j++) {
                unsigned char c = (unsigned char)rr.data[j];
                if (c >= 32 && c <= 126) putchar(c);
                else if (c == '\n') printf("\\n");
                else if (c == '\r') printf("\\r");
                else if (c == '\t') printf("\\t");
                else printf("\\x%02X", c);
            }
            printf("\"\n");
            free(rr.data);
        } else {
            printf("[TEST %d] %s: ERROR (%d) %s\n", i + 1, fname, rr.error, err);
        }
    }

    close(sbfd);
    close(basefd);
}

int main(int argc, char **argv) {
    if (argc > 1) {
        /* CLI mode: read file from current directory anchor */
        const char *filename = argv[1];
        int dirfd = open_base_dir(".");
        if (dirfd < 0) {
            fprintf(stderr, "Error: cannot open base directory: %s\n", strerror(errno));
            return 1;
        }
        char err[128];
        ReadResult rr = read_file_from_base(dirfd, filename, err, sizeof(err));
        close(dirfd);
        if (rr.error != 0) {
            fprintf(stderr, "Error: %s (code %d)\n", err, rr.error);
            return 2;
        }
        /* Write data to stdout */
        size_t off = 0;
        while (off < rr.size) {
            ssize_t n = write(STDOUT_FILENO, rr.data + off, rr.size - off);
            if (n < 0) {
                fprintf(stderr, "Error: failed to write output\n");
                free(rr.data);
                return 3;
            }
            off += (size_t)n;
        }
        free(rr.data);
        return 0;
    } else {
        /* Test mode */
        run_tests();
        return 0;
    }
}