#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include <dirent.h>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

#define MAX_FILENAME_LEN 128
#define MAX_SUGGESTED_BASE_LEN 100
#define MAX_UPLOAD_SIZE (10 * 1024 * 1024) /* 10 MiB */
#define READ_BUF_SIZE 8192

/* Secure logging: constant formats, no sensitive data */
static void log_msg(const char *msg) {
    (void)fprintf(stderr, "%s\n", msg);
}

/* Portable explicit zero to avoid compiler optimizations */
static void secure_bzero(void *ptr, size_t len) {
#if defined(__STDC_LIB_EXT1__)
    memset_s(ptr, len, 0, len);
#else
    volatile unsigned char *p = (volatile unsigned char*)ptr;
    while (len--) { *p++ = 0; }
#endif
}

/* Rule#1: Validate filename (no slashes, traversal, only whitelisted chars) */
static int sanitize_filename(const char *input, char *out, size_t out_len) {
    if (!input || !out || out_len == 0) return -1;

    size_t in_len = strnlen(input, MAX_FILENAME_LEN + 1);
    if (in_len == 0 || in_len > MAX_FILENAME_LEN) return -1;

    size_t j = 0;
    bool has_dotdot = false;
    for (size_t i = 0; i < in_len; i++) {
        char c = input[i];
        if (c == '/' || c == '\\') {
            return -1; /* No path separators */
        }
        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '.' || c == '-' || c == '_') {
            if (j + 1 >= out_len) return -1;
            out[j++] = c;
        } else {
            return -1; /* reject any other char */
        }
    }
    if (j == 0) return -1;
    out[j] = '\0';

    /* reject sequences of ".." or "." only names that could be confusing */
    if (strcmp(out, ".") == 0 || strcmp(out, "..") == 0) return -1;
    if (strstr(out, "..") != NULL) has_dotdot = true;
    if (has_dotdot) return -1;

    return 0;
}

/* Get extension pointer within sanitized filename (returns pointer into buffer) */
static const char* get_extension(const char *san) {
    const char *last_dot = strrchr(san, '.');
    if (!last_dot || last_dot == san) return ""; /* no ext or hidden file name, treat as no ext */
    return last_dot; /* includes '.' */
}

/* Read 4 random bytes from /dev/urandom safely */
static int get_random_u32(uint32_t *out) {
    if (!out) return -1;
    int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
    if (fd >= 0) {
        uint32_t v = 0;
        ssize_t r = read(fd, &v, sizeof(v));
        int saved = errno;
        close(fd);
        errno = saved;
        if (r == (ssize_t)sizeof(v)) {
            *out = v;
            return 0;
        }
    }
    /* Fallback: not cryptographically strong, but acceptable for filename suffix */
    uint32_t v = (uint32_t)time(NULL) ^ (uint32_t)getpid();
    *out = v;
    return 0;
}

/* Create (if not exists) and open a directory securely, return dirfd */
static int open_secure_dir(const char *path) {
    if (!path) return -1;

    struct stat st;
    if (stat(path, &st) != 0) {
        if (errno == ENOENT) {
            if (mkdir(path, 0700) != 0) {
                log_msg("Error: cannot create directory");
                return -1;
            }
        } else {
            log_msg("Error: cannot stat directory");
            return -1;
        }
    }
    int dirfd = open(path, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (dirfd < 0) {
        log_msg("Error: cannot open directory");
        return -1;
    }
    if (fstat(dirfd, &st) != 0) {
        log_msg("Error: cannot fstat directory");
        close(dirfd);
        return -1;
    }
    if (!S_ISDIR(st.st_mode)) {
        log_msg("Error: path is not a directory");
        close(dirfd);
        return -1;
    }
    return dirfd;
}

/* Open a source file securely for reading */
static int open_source_file(const char *path, off_t *opt_size) {
    if (!path) return -1;
    int fd = open(path, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) {
        log_msg("Error: cannot open source file");
        return -1;
    }
    struct stat st;
    if (fstat(fd, &st) != 0) {
        log_msg("Error: cannot fstat source file");
        close(fd);
        return -1;
    }
    if (!S_ISREG(st.st_mode)) {
        log_msg("Error: source is not a regular file");
        close(fd);
        return -1;
    }
    if (opt_size) *opt_size = st.st_size;
    return fd;
}

/* Generate unique safe filename: base + "_" + 8hex + ext */
static int generate_unique_name(const char *san_base, char *out, size_t out_len) {
    if (!san_base || !out || out_len == 0) return -1;

    /* Split base and extension */
    const char *ext = get_extension(san_base);
    size_t base_len = (size_t)(ext > san_base ? (size_t)(ext - san_base) : strlen(san_base));
    if (base_len > MAX_SUGGESTED_BASE_LEN) base_len = MAX_SUGGESTED_BASE_LEN;

    uint32_t rnd = 0;
    if (get_random_u32(&rnd) != 0) return -1;

    char suffix[9];
    /* 8 hex chars */
    (void)snprintf(suffix, sizeof(suffix), "%08x", rnd);

    size_t ext_len = strlen(ext);
    /* Format: base + "_" + suffix + ext */
    size_t needed = base_len + 1 + 8 + ext_len + 1;
    if (needed > out_len) return -1;

    memcpy(out, san_base, base_len);
    out[base_len] = '_';
    memcpy(out + base_len + 1, suffix, 8);
    memcpy(out + base_len + 1 + 8, ext, ext_len);
    out[base_len + 1 + 8 + ext_len] = '\0';
    return 0;
}

/* Write stream from source_fd to a new file dirfd/final_name safely, up to max_size */
static int save_stream_to_new_file(int dirfd, const char *final_name, int source_fd, size_t max_size, size_t *bytes_written_out) {
    if (!final_name || dirfd < 0 || source_fd < 0) return -1;

    int dst_fd = openat(dirfd, final_name, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);
    if (dst_fd < 0) {
        log_msg("Error: cannot create destination file");
        return -1;
    }

    struct stat st;
    if (fstat(dst_fd, &st) != 0) {
        log_msg("Error: cannot fstat destination file");
        close(dst_fd);
        (void)unlinkat(dirfd, final_name, 0);
        return -1;
    }
    if (!S_ISREG(st.st_mode)) {
        log_msg("Error: destination is not a regular file");
        close(dst_fd);
        (void)unlinkat(dirfd, final_name, 0);
        return -1;
    }

    unsigned char buf[READ_BUF_SIZE];
    size_t total = 0;
    while (1) {
        ssize_t r = read(source_fd, buf, sizeof(buf));
        if (r < 0) {
            log_msg("Error: read failed");
            close(dst_fd);
            (void)unlinkat(dirfd, final_name, 0);
            return -1;
        }
        if (r == 0) break;
        if ((size_t)r > SIZE_MAX - total) {
            log_msg("Error: size overflow");
            close(dst_fd);
            (void)unlinkat(dirfd, final_name, 0);
            return -1;
        }
        total += (size_t)r;
        if (total > max_size) {
            log_msg("Error: file too large");
            close(dst_fd);
            (void)unlinkat(dirfd, final_name, 0);
            return -1;
        }
        size_t off = 0;
        while (off < (size_t)r) {
            ssize_t w = write(dst_fd, buf + off, (size_t)r - off);
            if (w < 0) {
                log_msg("Error: write failed");
                close(dst_fd);
                (void)unlinkat(dirfd, final_name, 0);
                return -1;
            }
            off += (size_t)w;
        }
    }

    if (fsync(dst_fd) != 0) {
        log_msg("Error: fsync failed");
        close(dst_fd);
        (void)unlinkat(dirfd, final_name, 0);
        return -1;
    }

    int saved_errno = 0;
    if (close(dst_fd) != 0) {
        saved_errno = errno;
        (void)unlinkat(dirfd, final_name, 0);
        errno = saved_errno;
        log_msg("Error: close failed");
        return -1;
    }

    if (bytes_written_out) *bytes_written_out = total;
    return 0;
}

/* Fsync directory to persist the new entry */
static int fsync_dirfd(int dirfd) {
    if (dirfd < 0) return -1;
    if (fsync(dirfd) != 0) {
        log_msg("Error: directory fsync failed");
        return -1;
    }
    return 0;
}

/* High-level upload from source_path to base_dir using suggested filename.
   Returns 0 on success and writes the final stored filename into out_saved_name. */
static int upload_file_from_path(const char *base_dir, const char *source_path, const char *suggested_name,
                                 char *out_saved_name, size_t out_saved_name_len, size_t max_size, size_t *out_bytes) {
    if (!base_dir || !source_path || !suggested_name || !out_saved_name || out_saved_name_len == 0) return -1;

    char san[MAX_FILENAME_LEN + 1];
    if (sanitize_filename(suggested_name, san, sizeof(san)) != 0) {
        log_msg("Error: invalid filename");
        return -1;
    }

    char final_name[MAX_FILENAME_LEN + 32]; /* room for suffix */
    if (generate_unique_name(san, final_name, sizeof(final_name)) != 0) {
        log_msg("Error: cannot generate unique name");
        return -1;
    }

    int dirfd = open_secure_dir(base_dir);
    if (dirfd < 0) {
        log_msg("Error: base dir open failed");
        return -1;
    }

    off_t src_size = 0;
    int src_fd = open_source_file(source_path, &src_size);
    if (src_fd < 0) {
        close(dirfd);
        return -1;
    }

    /* Optional pre-check on size if known and positive */
    if (src_size > 0 && (size_t)src_size > max_size) {
        log_msg("Error: source file too large");
        close(src_fd);
        close(dirfd);
        return -1;
    }

    size_t bytes_written = 0;
    if (save_stream_to_new_file(dirfd, final_name, src_fd, max_size, &bytes_written) != 0) {
        close(src_fd);
        close(dirfd);
        return -1;
    }

    if (fsync_dirfd(dirfd) != 0) {
        /* Best-effort: cannot ensure durability; keep file but report error */
        close(src_fd);
        close(dirfd);
        return -1;
    }

    if (close(src_fd) != 0) {
        log_msg("Error: closing source failed");
        close(dirfd);
        return -1;
    }

    if (strlen(final_name) + 1 > out_saved_name_len) {
        log_msg("Error: output buffer too small");
        close(dirfd);
        return -1;
    }
    (void)snprintf(out_saved_name, out_saved_name_len, "%s", final_name);
    if (out_bytes) *out_bytes = bytes_written;

    if (close(dirfd) != 0) {
        log_msg("Error: closing dir failed");
        return -1;
    }

    return 0;
}

/* Utility to create a file with specific content size in a directory (dirfd) */
static int create_file_at(int dirfd, const char *name, const unsigned char *data, size_t len) {
    if (!name) return -1;
    int fd = openat(dirfd, name, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);
    if (fd < 0) return -1;
    size_t off = 0;
    while (off < len) {
        ssize_t w = write(fd, data + off, len - off);
        if (w < 0) {
            int se = errno;
            close(fd);
            (void)unlinkat(dirfd, name, 0);
            errno = se;
            return -1;
        }
        off += (size_t)w;
    }
    if (fsync(fd) != 0) {
        int se = errno;
        close(fd);
        (void)unlinkat(dirfd, name, 0);
        errno = se;
        return -1;
    }
    int rc = close(fd);
    return rc == 0 ? 0 : -1;
}

/* Utility to create a big file of size bytes (filled with 'A') */
static int create_big_file_at(int dirfd, const char *name, size_t size) {
    if (!name) return -1;
    int fd = openat(dirfd, name, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);
    if (fd < 0) return -1;
    unsigned char buf[4096];
    memset(buf, 'A', sizeof(buf));
    size_t remaining = size;
    while (remaining > 0) {
        size_t chunk = remaining < sizeof(buf) ? remaining : sizeof(buf);
        ssize_t w = write(fd, buf, chunk);
        if (w < 0) {
            int se = errno;
            close(fd);
            (void)unlinkat(dirfd, name, 0);
            errno = se;
            return -1;
        }
        remaining -= (size_t)w;
    }
    if (fsync(fd) != 0) {
        int se = errno;
        close(fd);
        (void)unlinkat(dirfd, name, 0);
        errno = se;
        return -1;
    }
    int rc = close(fd);
    return rc == 0 ? 0 : -1;
}

/* Utility to read entire file size via fstat for verification */
static ssize_t get_file_size_at(int dirfd, const char *name) {
    int fd = openat(dirfd, name, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) return -1;
    struct stat st;
    if (fstat(fd, &st) != 0) {
        int se = errno;
        close(fd);
        errno = se;
        return -1;
    }
    off_t sz = st.st_size;
    close(fd);
    if (sz < 0) return -1;
    return (ssize_t)sz;
}

/* Test harness: 5 test cases */
int main(void) {
    /* Prepare temp base directory */
    char base_template[] = "/tmp/upload_c_XXXXXX";
    char *tmp_base = mkdtemp(base_template);
    if (!tmp_base) {
        log_msg("Fatal: cannot create temp base");
        return 1;
    }

    /* Create subdirs: src and uploads */
    char src_dir[PATH_MAX];
    char upload_dir[PATH_MAX];
    if (snprintf(src_dir, sizeof(src_dir), "%s/src", tmp_base) >= (int)sizeof(src_dir) ||
        snprintf(upload_dir, sizeof(upload_dir), "%s/uploads", tmp_base) >= (int)sizeof(upload_dir)) {
        log_msg("Fatal: path too long");
        return 1;
    }
    if (mkdir(src_dir, 0700) != 0) {
        log_msg("Fatal: cannot create src dir");
        return 1;
    }
    if (mkdir(upload_dir, 0700) != 0) {
        log_msg("Fatal: cannot create upload dir");
        return 1;
    }

    int src_dirfd = open_secure_dir(src_dir);
    int up_dirfd  = open_secure_dir(upload_dir);
    if (src_dirfd < 0 || up_dirfd < 0) {
        log_msg("Fatal: cannot open dirs");
        if (src_dirfd >= 0) close(src_dirfd);
        if (up_dirfd >= 0) close(up_dirfd);
        return 1;
    }

    /* Prepare source files */
    const unsigned char hello_data[] = "Hello, secure upload!\n";
    if (create_file_at(src_dirfd, "hello.txt", hello_data, sizeof(hello_data) - 1) != 0) {
        log_msg("Fatal: cannot create hello.txt");
        close(src_dirfd); close(up_dirfd);
        return 1;
    }

    size_t big_size = MAX_UPLOAD_SIZE + 1024;
    if (create_big_file_at(src_dirfd, "big.bin", big_size) != 0) {
        log_msg("Fatal: cannot create big.bin");
        close(src_dirfd); close(up_dirfd);
        return 1;
    }

    /* Create a symlink source pointing to hello.txt (to be rejected) */
    if (symlinkat("hello.txt", src_dirfd, "hello_link.txt") != 0) {
        log_msg("Warning: cannot create symlink, test may be skipped");
    }

    close(src_dirfd);
    close(up_dirfd);

    /* Helper to build paths */
    char src_hello[PATH_MAX];
    char src_big[PATH_MAX];
    char src_link[PATH_MAX];

    (void)snprintf(src_hello, sizeof(src_hello), "%s/src/hello.txt", tmp_base);
    (void)snprintf(src_big, sizeof(src_big), "%s/src/big.bin", tmp_base);
    (void)snprintf(src_link, sizeof(src_link), "%s/src/hello_link.txt", tmp_base);

    /* Test case 1: Valid upload */
    {
        char saved_name[256];
        size_t bytes = 0;
        int rc = upload_file_from_path(upload_dir, src_hello, "greeting.txt", saved_name, sizeof(saved_name), MAX_UPLOAD_SIZE, &bytes);
        printf("Test1 valid upload: rc=%d, bytes=%zu, saved_name=%s\n", rc, bytes, rc == 0 ? saved_name : "(none)");

        if (rc == 0) {
            /* Verify size */
            int udirfd = open_secure_dir(upload_dir);
            if (udirfd >= 0) {
                ssize_t sz = get_file_size_at(udirfd, saved_name);
                printf("Test1 verify size: sz=%zd\n", (ssize_t)sz);
                close(udirfd);
            }
        }
    }

    /* Test case 2: Invalid filename (traversal) */
    {
        char saved_name[256];
        size_t bytes = 0;
        int rc = upload_file_from_path(upload_dir, src_hello, "../evil.txt", saved_name, sizeof(saved_name), MAX_UPLOAD_SIZE, &bytes);
        printf("Test2 invalid filename traversal: rc=%d\n", rc);
    }

    /* Test case 3: Symlink source (should be rejected) */
    {
        char saved_name[256];
        size_t bytes = 0;
        int rc = upload_file_from_path(upload_dir, src_link, "link_upload.txt", saved_name, sizeof(saved_name), MAX_UPLOAD_SIZE, &bytes);
        printf("Test3 symlink source rejected: rc=%d\n", rc);
    }

    /* Test case 4: File too large */
    {
        char saved_name[256];
        size_t bytes = 0;
        int rc = upload_file_from_path(upload_dir, src_big, "big_upload.bin", saved_name, sizeof(saved_name), MAX_UPLOAD_SIZE, &bytes);
        printf("Test4 too large rejected: rc=%d\n", rc);
    }

    /* Test case 5: Invalid characters in filename */
    {
        char saved_name[256];
        size_t bytes = 0;
        int rc = upload_file_from_path(upload_dir, src_hello, "bad*name?.txt", saved_name, sizeof(saved_name), MAX_UPLOAD_SIZE, &bytes);
        printf("Test5 invalid characters rejected: rc=%d\n", rc);
    }

    /* Clean sensitive buffers (none here) */
    return 0;
}