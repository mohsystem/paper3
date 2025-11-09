/*
Secure TAR extraction and safe file operations in C.

Features:
- Securely extract .tar, .tar.gz, and .tar.bz2 archives to /tmp/unpack using libarchive.
- Strict path sanitization to prevent directory traversal and absolute path extraction.
- Avoids following symlinks for both directories and files using O_NOFOLLOW.
- Creates directories and files securely using openat/mkdirat anchored to a verified base directory.
- Atomic writes using a temp file and linkat to avoid TOCTOU overwrites (no replacement).
- Simple "with"-like context management via a callback that guarantees fclose.
- Safe file open/read with range checks and secure default permissions.
- Includes 5 test cases.

Build (example):
  gcc -Wall -Wextra -O2 secure_tar.c -larchive -o secure_tar

Note: Requires libarchive development headers and library.
*/

#define _GNU_SOURCE
#include <archive.h>
#include <archive_entry.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#ifdef __linux__
#include <sys/random.h>
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/* Limits to avoid resource exhaustion */
#define MAX_FILE_SIZE (10 * 1024 * 1024)       /* 10 MiB per file */
#define MAX_TOTAL_SIZE (50 * 1024 * 1024)      /* 50 MiB per archive */
#define IO_BUF_SIZE (64 * 1024)
#define DIR_MODE (0700)
#define FILE_MODE (0600)

/* Utility: secure random 64-bit */
static uint64_t secure_rand_u64(void) {
    uint64_t v = 0;
#ifdef __linux__
    ssize_t r = getrandom(&v, sizeof(v), 0);
    if (r == (ssize_t)sizeof(v)) return v;
#endif
    /* Fallback: /dev/urandom */
    FILE *ur = fopen("/dev/urandom", "rb");
    if (ur) {
        size_t n = fread(&v, 1, sizeof(v), ur);
        fclose(ur);
        if (n == sizeof(v)) return v;
    }
    /* Last resort (not cryptographic) */
    static bool seeded = false;
    if (!seeded) {
        unsigned s = (unsigned)(time(NULL) ^ (uintptr_t)&v ^ (uintptr_t)getpid());
        srand(s);
        seeded = true;
    }
    v = ((uint64_t)rand() << 32) ^ (uint64_t)rand();
    return v;
}

/* Path sanitization: reject absolute paths and any ".." segments; collapse "." and empty segments */
static int sanitize_relative_path(const char *in, char *out, size_t outsz) {
    if (!in || !out || outsz == 0) return -1;
    out[0] = '\0';
    size_t outlen = 0;
    const char *p = in;

    if (*p == '/') return -1; /* absolute not allowed */
    while (*p) {
        /* skip consecutive slashes */
        while (*p == '/') p++;
        if (*p == '\0') break;

        char seg[256];
        size_t s = 0;
        while (*p && *p != '/') {
            if (s + 1 >= sizeof(seg)) return -1; /* segment too long */
            seg[s++] = *p++;
        }
        seg[s] = '\0';

        if (s == 0 || (s == 1 && seg[0] == '.')) {
            continue; /* skip empty and "." */
        }
        if ((s == 2 && seg[0] == '.' && seg[1] == '.')) {
            return -1; /* directory traversal not allowed */
        }

        /* append to out with '/' if needed */
        size_t need = s + (outlen ? 1 : 0);
        if (outlen + need + 1 > outsz) return -1;
        if (outlen) out[outlen++] = '/';
        memcpy(out + outlen, seg, s);
        outlen += s;
        out[outlen] = '\0';
    }
    if (outlen == 0) return -1; /* resulting path empty */
    return 0;
}

/* Split "path" into parent and leaf; returns 0 on success. Parent becomes "." if single segment. */
static int split_parent_leaf(const char *path, char *parent, size_t psz, char *leaf, size_t lsz) {
    if (!path || !parent || !leaf) return -1;
    size_t n = strnlen(path, PATH_MAX);
    if (n == 0 || n >= PATH_MAX) return -1;
    const char *last = strrchr(path, '/');
    if (!last) {
        if (psz < 2 || lsz <= n) return -1;
        memcpy(leaf, path, n);
        leaf[n] = '\0';
        parent[0] = '.';
        parent[1] = '\0';
        return 0;
    }
    size_t parent_len = (size_t)(last - path);
    size_t leaf_len = n - parent_len - 1;
    if (parent_len >= psz || leaf_len >= lsz || leaf_len == 0) return -1;
    memcpy(parent, path, parent_len);
    parent[parent_len] = '\0';
    memcpy(leaf, last + 1, leaf_len);
    leaf[leaf_len] = '\0';
    return 0;
}

/* Open "/" directory */
static int open_root_dir(void) {
    int fd = open("/", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    return fd;
}

/* Open or create an absolute directory path securely, rejecting symlinks on each segment. */
static int safe_open_or_create_abs_dir(const char *abs_path) {
    if (!abs_path || abs_path[0] != '/') return -1;
    int rootfd = open_root_dir();
    if (rootfd < 0) return -1;

    int curfd = rootfd; /* owns curfd */
    const char *p = abs_path;
    /* Skip leading slash */
    while (*p == '/') p++;
    if (*p == '\0') {
        /* path was "/" */
        return curfd;
    }

    char seg[256];
    while (*p) {
        /* Extract segment */
        size_t s = 0;
        while (*p && *p != '/') {
            if (s + 1 >= sizeof(seg)) { close(curfd); return -1; }
            seg[s++] = *p++;
        }
        seg[s] = '\0';
        while (*p == '/') p++;

        if (s == 0 || (s == 1 && seg[0] == '.')) {
            continue;
        }
        if (s == 2 && seg[0] == '.' && seg[1] == '.') {
            close(curfd);
            return -1;
        }

        /* Try open existing directory without following symlinks */
        int nextfd = openat(curfd, seg, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
        if (nextfd < 0) {
            if (errno != ENOENT) { close(curfd); return -1; }
            /* Create if missing */
            if (mkdirat(curfd, seg, DIR_MODE) != 0) {
                if (errno != EEXIST) { close(curfd); return -1; }
            }
            /* Open again */
            nextfd = openat(curfd, seg, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
            if (nextfd < 0) { close(curfd); return -1; }
        }
        close(curfd);
        curfd = nextfd;
    }
    return curfd;
}

/* Traverse to parent directory for a relative sanitized path; optionally create intermediate dirs. */
static int traverse_to_parent(int basefd, const char *rel_sanitized, bool create, int *out_parent_fd, char *leaf, size_t leafsz) {
    if (!rel_sanitized || !out_parent_fd || !leaf) return -1;
    char parent[PATH_MAX];
    if (split_parent_leaf(rel_sanitized, parent, sizeof(parent), leaf, leafsz) != 0) return -1;

    int curfd = dup(basefd);
    if (curfd < 0) return -1;

    if (strcmp(parent, ".") == 0) {
        *out_parent_fd = curfd;
        return 0;
    }

    const char *p = parent;
    char seg[256];
    while (*p) {
        while (*p == '/') p++;
        size_t s = 0;
        while (*p && *p != '/') {
            if (s + 1 >= sizeof(seg)) { close(curfd); return -1; }
            seg[s++] = *p++;
        }
        seg[s] = '\0';
        while (*p == '/') p++;

        if (s == 0 || (s == 1 && seg[0] == '.')) continue;
        if (s == 2 && seg[0] == '.' && seg[1] == '.') { close(curfd); return -1; }

        int nextfd = openat(curfd, seg, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
        if (nextfd < 0) {
            if (!create || errno != ENOENT) { close(curfd); return -1; }
            if (mkdirat(curfd, seg, DIR_MODE) != 0) {
                if (errno != EEXIST) { close(curfd); return -1; }
            }
            nextfd = openat(curfd, seg, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
            if (nextfd < 0) { close(curfd); return -1; }
        }
        close(curfd);
        curfd = nextfd;
    }

    *out_parent_fd = curfd;
    return 0;
}

/* Create a directory at basefd/rel_sanitized (entire path), creating parents as needed. */
static int ensure_dir_hierarchy_at(int basefd, const char *rel_sanitized) {
    if (!rel_sanitized) return -1;
    int parentfd = -1;
    char leaf[256];
    if (traverse_to_parent(basefd, rel_sanitized, true, &parentfd, leaf, sizeof(leaf)) != 0) return -1;

    int rc = 0;
    if (mkdirat(parentfd, leaf, DIR_MODE) != 0) {
        if (errno == EEXIST) {
            /* verify it's a directory and not a symlink */
            int dfd = openat(parentfd, leaf, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
            if (dfd < 0) rc = -1;
            else close(dfd);
        } else {
            rc = -1;
        }
    }
    close(parentfd);
    return rc;
}

/* Atomic write: create temp file in parentfd, write data from buffer, fsync, link as leaf if not existing, then unlink temp. */
static int safe_atomic_write_buf(int parentfd, const char *leaf, const uint8_t *data, size_t len) {
    if (!leaf || (!data && len != 0)) return -1;

    char tmpname[128];
    int tmpfd = -1;
    int tries = 0;

    for (tries = 0; tries < 8; ++tries) {
        uint64_t r = secure_rand_u64();
        int n = snprintf(tmpname, sizeof(tmpname), ".tmp.extract-%ld-%" PRIu64 ".part", (long)getpid(), (uint64_t)r);
        if (n <= 0 || (size_t)n >= sizeof(tmpname)) return -1;

        tmpfd = openat(parentfd, tmpname, O_CREAT | O_EXCL | O_WRONLY | O_CLOEXEC | O_NOFOLLOW, FILE_MODE);
        if (tmpfd >= 0) break;
        if (errno != EEXIST) return -1;
    }
    if (tmpfd < 0) return -1;

    size_t written = 0;
    while (written < len) {
        ssize_t w = write(tmpfd, data + written, len - written);
        if (w < 0) { close(tmpfd); unlinkat(parentfd, tmpname, 0); return -1; }
        written += (size_t)w;
    }
    if (fsync(tmpfd) != 0) { close(tmpfd); unlinkat(parentfd, tmpname, 0); return -1; }
    if (close(tmpfd) != 0) { unlinkat(parentfd, tmpname, 0); return -1; }

    /* atomic create without overwrite using linkat */
    if (linkat(parentfd, tmpname, parentfd, leaf, 0) != 0) {
        /* target exists or error */
        unlinkat(parentfd, tmpname, 0);
        return -1;
    }
    /* remove temp name; content persists via leaf hardlink */
    if (unlinkat(parentfd, tmpname, 0) != 0) {
        /* Not fatal for correctness, but clean up best-effort */
    }
    return 0;
}

/* Atomic write from libarchive stream into parentfd/leaf */
static int safe_atomic_write_from_archive(int parentfd, const char *leaf, struct archive *ar, size_t declared_size, size_t *out_written) {
    if (!leaf || !ar || !out_written) return -1;
    *out_written = 0;

    char tmpname[128];
    int tmpfd = -1;

    for (int tries = 0; tries < 8; ++tries) {
        uint64_t r = secure_rand_u64();
        int n = snprintf(tmpname, sizeof(tmpname), ".tmp.extract-%ld-%" PRIu64 ".part", (long)getpid(), (uint64_t)r);
        if (n <= 0 || (size_t)n >= sizeof(tmpname)) return -1;
        tmpfd = openat(parentfd, tmpname, O_CREAT | O_EXCL | O_WRONLY | O_CLOEXEC | O_NOFOLLOW, FILE_MODE);
        if (tmpfd >= 0) break;
        if (errno != EEXIST) return -1;
    }
    if (tmpfd < 0) return -1;

    uint8_t buf[IO_BUF_SIZE];
    size_t total = 0;

    while (1) {
        ssize_t r = archive_read_data(ar, buf, sizeof(buf));
        if (r == 0) break;  /* end of entry */
        if (r < 0) {
            close(tmpfd);
            unlinkat(parentfd, tmpname, 0);
            return -1;
        }
        size_t chunk = (size_t)r;
        total += chunk;
        if (total > MAX_FILE_SIZE) {
            close(tmpfd);
            unlinkat(parentfd, tmpname, 0);
            return -1;
        }
        size_t off = 0;
        while (off < chunk) {
            ssize_t w = write(tmpfd, buf + off, chunk - off);
            if (w <= 0) {
                close(tmpfd);
                unlinkat(parentfd, tmpname, 0);
                return -1;
            }
            off += (size_t)w;
        }
    }
    if (fsync(tmpfd) != 0) {
        close(tmpfd);
        unlinkat(parentfd, tmpname, 0);
        return -1;
    }
    if (close(tmpfd) != 0) {
        unlinkat(parentfd, tmpname, 0);
        return -1;
    }

    /* Atomic create without overwrite */
    if (linkat(parentfd, tmpname, parentfd, leaf, 0) != 0) {
        unlinkat(parentfd, tmpname, 0);
        return -1;
    }
    if (unlinkat(parentfd, tmpname, 0) != 0) {
        /* ignore */
    }
    *out_written = total;
    (void)declared_size; /* we enforce our own limit; declared size may be unreliable */
    return 0;
}

/* Extract tar archive to dest_dir (absolute path, will be created) securely. Returns 0 on success. */
int extract_tar_secure(const char *archive_path, const char *dest_dir) {
    if (!archive_path || !dest_dir || dest_dir[0] != '/') return -1;

    int basefd = safe_open_or_create_abs_dir(dest_dir);
    if (basefd < 0) return -1;

    struct archive *a = archive_read_new();
    if (!a) { close(basefd); return -1; }

    archive_read_support_filter_all(a);
    archive_read_support_format_tar(a);

    if (archive_read_open_filename(a, archive_path, 10240) != ARCHIVE_OK) {
        archive_read_free(a);
        close(basefd);
        return -1;
    }

    size_t total_written = 0;
    struct archive_entry *entry = NULL;
    int rc = 0;

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        const char *epath = archive_entry_pathname(entry);
        if (!epath) { rc = -1; break; }

        char rel[PATH_MAX];
        if (sanitize_relative_path(epath, rel, sizeof(rel)) != 0) {
            /* Reject unsafe path; skip entry */
            archive_read_data_skip(a);
            continue;
        }

        mode_t type = archive_entry_filetype(entry);
        if (type == AE_IFDIR) {
            if (ensure_dir_hierarchy_at(basefd, rel) != 0) { rc = -1; break; }
            continue;
        } else if (type == AE_IFREG) {
            /* Ensure parent exists */
            int parentfd = -1;
            char leaf[256];
            if (traverse_to_parent(basefd, rel, true, &parentfd, leaf, sizeof(leaf)) != 0) { rc = -1; break; }

            size_t declared = (size_t)archive_entry_size(entry);
            size_t written = 0;
            if (safe_atomic_write_from_archive(parentfd, leaf, a, declared, &written) != 0) {
                close(parentfd);
                rc = -1; break;
            }
            close(parentfd);
            total_written += written;
            if (total_written > MAX_TOTAL_SIZE) { rc = -1; break; }
            continue;
        } else {
            /* Skip symlinks, hardlinks, device files, etc. */
            archive_read_data_skip(a);
            continue;
        }
    }

    if (archive_read_close(a) != ARCHIVE_OK) rc = -1;
    archive_read_free(a);
    close(basefd);
    return rc;
}

/* Create tar archive with given entries and compression: "gz", "bz2", or "none". Returns 0 on success. */
struct TarEntry {
    const char *path;     /* relative path inside archive */
    const uint8_t *data;  /* NULL for directories */
    size_t len;           /* data length */
    bool is_dir;          /* true if directory entry */
};

int create_tar_archive(const char *out_path, const char *compression, const struct TarEntry *entries, size_t n_entries) {
    if (!out_path || !entries || n_entries == 0) return -1;
    struct archive *a = archive_write_new();
    if (!a) return -1;

    /* compression filter */
    if (compression && strcmp(compression, "gz") == 0) {
        if (archive_write_add_filter_gzip(a) != ARCHIVE_OK) { archive_write_free(a); return -1; }
    } else if (compression && strcmp(compression, "bz2") == 0) {
        if (archive_write_add_filter_bzip2(a) != ARCHIVE_OK) { archive_write_free(a); return -1; }
    } else {
        if (archive_write_add_filter_none(a) != ARCHIVE_OK) { archive_write_free(a); return -1; }
    }

    if (archive_write_set_format_pax_restricted(a) != ARCHIVE_OK) { archive_write_free(a); return -1; }

    if (archive_write_open_filename(a, out_path) != ARCHIVE_OK) { archive_write_free(a); return -1; }

    for (size_t i = 0; i < n_entries; ++i) {
        const struct TarEntry *e = &entries[i];
        if (!e->path) { archive_write_close(a); archive_write_free(a); return -1; }

        /* We allow unsafe paths in the archive for testing; extraction will sanitize. */
        struct archive_entry *ent = archive_entry_new();
        if (!ent) { archive_write_close(a); archive_write_free(a); return -1; }

        archive_entry_set_pathname(ent, e->path);

        if (e->is_dir) {
            archive_entry_set_filetype(ent, AE_IFDIR);
            archive_entry_set_perm(ent, DIR_MODE);
            archive_entry_set_size(ent, 0);
            if (archive_write_header(a, ent) != ARCHIVE_OK) {
                archive_entry_free(ent); archive_write_close(a); archive_write_free(a); return -1;
            }
        } else {
            archive_entry_set_filetype(ent, AE_IFREG);
            archive_entry_set_perm(ent, FILE_MODE);
            archive_entry_set_size(ent, (la_int64_t)e->len);
            if (archive_write_header(a, ent) != ARCHIVE_OK) {
                archive_entry_free(ent); archive_write_close(a); archive_write_free(a); return -1;
            }
            if (e->data && e->len > 0) {
                if (archive_write_data(a, e->data, e->len) < 0) {
                    archive_entry_free(ent); archive_write_close(a); archive_write_free(a); return -1;
                }
            }
        }
        archive_entry_free(ent);
    }

    if (archive_write_close(a) != ARCHIVE_OK) { archive_write_free(a); return -1; }
    archive_write_free(a);
    return 0;
}

/* Safe open (read-only) of a file within base_dir and relative sanitized path to a FILE*; caller provides relpath (unsanitized), returns FILE* via out. */
static int safe_fopen_ro_within(const char *base_dir, const char *relpath, FILE **out) {
    if (!base_dir || !relpath || !out) return -1;
    *out = NULL;

    char rel[PATH_MAX];
    if (sanitize_relative_path(relpath, rel, sizeof(rel)) != 0) return -1;

    int basefd = safe_open_or_create_abs_dir(base_dir);
    if (basefd < 0) return -1;

    int parentfd = -1;
    char leaf[256];
    if (traverse_to_parent(basefd, rel, false, &parentfd, leaf, sizeof(leaf)) != 0) {
        close(basefd);
        return -1;
    }

    int fd = openat(parentfd, leaf, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    close(parentfd);
    close(basefd);
    if (fd < 0) return -1;

    FILE *f = fdopen(fd, "rb");
    if (!f) {
        close(fd);
        return -1;
    }
    *out = f;
    return 0;
}

/* With-like pattern: open file read-only, pass FILE* to callback, ensure fclose. */
typedef int (*WithFileCallback)(FILE *f, void *ctx);

int with_file_readonly(const char *base_dir, const char *relpath, WithFileCallback cb, void *ctx) {
    if (!cb) return -1;
    FILE *f = NULL;
    if (safe_fopen_ro_within(base_dir, relpath, &f) != 0) return -1;
    int rc = cb(f, ctx);
    fclose(f);
    return rc;
}

/* Safe atomic write a text file within base_dir at relpath */
int safe_write_text_file(const char *base_dir, const char *relpath, const char *content) {
    if (!base_dir || !relpath || !content) return -1;

    char rel[PATH_MAX];
    if (sanitize_relative_path(relpath, rel, sizeof(rel)) != 0) return -1;

    int basefd = safe_open_or_create_abs_dir(base_dir);
    if (basefd < 0) return -1;

    int parentfd = -1;
    char leaf[256];
    if (traverse_to_parent(basefd, rel, true, &parentfd, leaf, sizeof(leaf)) != 0) {
        close(basefd);
        return -1;
    }

    int rc = safe_atomic_write_buf(parentfd, leaf, (const uint8_t *)content, strnlen(content, MAX_FILE_SIZE + 1));
    close(parentfd);
    close(basefd);
    return rc;
}

/* Callback to count lines in a file; returns number of lines via ctx (size_t*). */
static int count_lines_cb(FILE *f, void *ctx) {
    if (!f || !ctx) return -1;
    size_t *out = (size_t *)ctx;
    *out = 0;
    char buf[4096];
    while (fgets(buf, sizeof(buf), f)) {
        for (size_t i = 0; buf[i] != '\0'; ++i) {
            if (buf[i] == '\n') (*out)++;
        }
    }
    if (ferror(f)) return -1;
    return 0;
}

/* Test utilities */
static void print_result(const char *label, int rc) {
    printf("%s: %s\n", label, rc == 0 ? "OK" : "FAIL");
}

/* MAIN with 5 test cases */
int main(void) {
    /* Ensure /tmp/unpack exists securely */
    const char *base = "/tmp/unpack";
    int basefd = safe_open_or_create_abs_dir(base);
    if (basefd < 0) {
        printf("Failed to create base directory\n");
        return 1;
    }
    close(basefd);

    int rc;

    /* Test 1: Create and extract a gzipped tar with safe paths */
    {
        const char *tar1 = "/tmp/test1.tar.gz";
        const uint8_t data1[] = "hello\nworld\n";
        const uint8_t data2[] = "secure\nextraction\n";
        struct TarEntry entries[] = {
            { "foo", NULL, 0, true },
            { "foo/hello.txt", data1, sizeof(data1) - 1, false },
            { "bar", NULL, 0, true },
            { "bar/baz.txt", data2, sizeof(data2) - 1, false },
        };
        rc = create_tar_archive(tar1, "gz", entries, sizeof(entries)/sizeof(entries[0]));
        print_result("Create tar.gz (Test1)", rc);
        rc = extract_tar_secure(tar1, "/tmp/unpack/test1");
        print_result("Extract tar.gz to /tmp/unpack/test1 (Test1)", rc);

        size_t lines = 0;
        rc = with_file_readonly("/tmp/unpack/test1", "foo/hello.txt", count_lines_cb, &lines);
        if (rc == 0) {
            printf("Lines in foo/hello.txt: %zu\n", lines);
        } else {
            printf("Failed to read foo/hello.txt\n");
        }
    }

    /* Test 2: Create and extract a bzip2 tar with safe paths */
    {
        const char *tar2 = "/tmp/test2.tar.bz2";
        const uint8_t data1[] = "line1\nline2\nline3\n";
        struct TarEntry entries[] = {
            { "docs", NULL, 0, true },
            { "docs/readme.txt", data1, sizeof(data1) - 1, false },
        };
        rc = create_tar_archive(tar2, "bz2", entries, sizeof(entries)/sizeof(entries[0]));
        print_result("Create tar.bz2 (Test2)", rc);
        rc = extract_tar_secure(tar2, "/tmp/unpack/test2");
        print_result("Extract tar.bz2 to /tmp/unpack/test2 (Test2)", rc);

        size_t lines = 0;
        rc = with_file_readonly("/tmp/unpack/test2", "docs/readme.txt", count_lines_cb, &lines);
        if (rc == 0) {
            printf("Lines in docs/readme.txt: %zu\n", lines);
        } else {
            printf("Failed to read docs/readme.txt\n");
        }
    }

    /* Test 3: Archive with directory traversal entry (should be skipped by extractor) */
    {
        const char *tar3 = "/tmp/test3.tar.gz";
        const uint8_t safe_data[] = "safe\n";
        const uint8_t evil_data[] = "evil\n";
        struct TarEntry entries[] = {
            { "good/file.txt", safe_data, sizeof(safe_data) - 1, false },
            { "../evil.txt", evil_data, sizeof(evil_data) - 1, false }, /* traversal */
            { "nested/../alsoevil.txt", evil_data, sizeof(evil_data) - 1, false }, /* normalized traversal */
        };
        rc = create_tar_archive(tar3, "gz", entries, sizeof(entries)/sizeof(entries[0]));
        print_result("Create tar.gz with traversal entries (Test3)", rc);
        rc = extract_tar_secure(tar3, "/tmp/unpack/test3");
        print_result("Extract tar.gz to /tmp/unpack/test3 (Test3)", rc);

        /* Verify safe file exists, traversal ones should not */
        size_t lines = 0;
        rc = with_file_readonly("/tmp/unpack/test3", "good/file.txt", count_lines_cb, &lines);
        printf("Read good/file.txt (Test3): %s; lines=%zu\n", rc == 0 ? "OK" : "FAIL", rc == 0 ? lines : 0U);
        rc = with_file_readonly("/tmp/unpack/test3", "evil.txt", count_lines_cb, &lines);
        printf("Read evil.txt (should fail): %s\n", rc == 0 ? "UNEXPECTED OK" : "EXPECTED FAIL");
        rc = with_file_readonly("/tmp/unpack/test3", "alsoevil.txt", count_lines_cb, &lines);
        printf("Read alsoevil.txt (should fail): %s\n", rc == 0 ? "UNEXPECTED OK" : "EXPECTED FAIL");
    }

    /* Test 4: Archive with absolute path entry (should be skipped) */
    {
        const char *tar4 = "/tmp/test4.tar.gz";
        const uint8_t abs_data[] = "abs\n";
        struct TarEntry entries[] = {
            { "/abs.txt", abs_data, sizeof(abs_data) - 1, false }, /* absolute => skip */
            { "ok.txt", abs_data, sizeof(abs_data) - 1, false },
        };
        rc = create_tar_archive(tar4, "gz", entries, sizeof(entries)/sizeof(entries[0]));
        print_result("Create tar.gz with absolute path (Test4)", rc);
        rc = extract_tar_secure(tar4, "/tmp/unpack/test4");
        print_result("Extract tar.gz to /tmp/unpack/test4 (Test4)", rc);

        size_t lines = 0;
        rc = with_file_readonly("/tmp/unpack/test4", "ok.txt", count_lines_cb, &lines);
        printf("Read ok.txt (Test4): %s; lines=%zu\n", rc == 0 ? "OK" : "FAIL", rc == 0 ? lines : 0U);
        rc = with_file_readonly("/tmp/unpack/test4", "abs.txt", count_lines_cb, &lines);
        printf("Read abs.txt (should fail): %s\n", rc == 0 ? "UNEXPECTED OK" : "EXPECTED FAIL");
    }

    /* Test 5: Safe write and with-like read */
    {
        const char *dir = "/tmp/unpack/test5";
        int dfd = safe_open_or_create_abs_dir(dir);
        if (dfd >= 0) close(dfd);
        const char *content = "alpha\nbeta\ngamma\n";
        rc = safe_write_text_file(dir, "notes.txt", content);
        print_result("Safe atomic write notes.txt (Test5)", rc);

        size_t lines = 0;
        rc = with_file_readonly(dir, "notes.txt", count_lines_cb, &lines);
        printf("Read notes.txt (Test5): %s; lines=%zu\n", rc == 0 ? "OK" : "FAIL", rc == 0 ? lines : 0U);
    }

    return 0;
}