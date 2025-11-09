/*
Secure PDF upload/download demo with MySQL persistence (mocked by default).

- Validates filename and content.
- Anchors file operations to a trusted "uploads" directory using openat.
- Writes via a temp file and atomic rename; fsyncs file and directory.
- Stores and retrieves relative paths in a database layer.
- Uses a mock DB by default to compile/run without external dependencies.
  Define USE_REAL_MYSQL and link against libmysqlclient to use a real MySQL server.
*/

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Optional: getrandom for secure randomness; fallback to /dev/urandom */
#if defined(__linux__)
#include <sys/syscall.h>
#include <linux/random.h>
#endif

/* ===== Configuration ===== */
#define UPLOAD_DIR "uploads"
#define MAX_PDF_SIZE (20u * 1024u * 1024u) /* 20 MB limit */
#define MAX_FILENAME_BASE 64u
#define MAX_STORED_PATH 192u
#define MAX_DOWNLOAD_SIZE (20u * 1024u * 1024u)

/* ===== Database Layer =====
   By default we build a mock DB that compiles without external deps.
   To use a real MySQL server, compile with -DUSE_REAL_MYSQL and link libmysqlclient.
*/
#ifdef USE_REAL_MYSQL
#include <mysql/mysql.h>
#else
/* Minimal mock types to satisfy signatures without external libs */
typedef struct {
    /* Very simple in-memory storage */
    char **paths;
    size_t count;
    size_t capacity;
} MYSQL;
#endif

/* ===== Utilities ===== */

static void secure_perror(const char *msg) {
    /* Use constant format string to avoid format string vulnerabilities */
    if (msg) {
        fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    } else {
        fprintf(stderr, "error: %s\n", strerror(errno));
    }
}

/* Read cryptographically secure random bytes */
static int get_secure_random(void *buf, size_t len) {
    if (buf == NULL || len == 0) return -1;
#if defined(__linux__)
    /* Try getrandom syscall (no blocking) */
    ssize_t got = 0;
    while ((size_t)got < len) {
        ssize_t r = syscall(SYS_getrandom, (char *)buf + got, len - (size_t)got, 0);
        if (r < 0) {
            if (errno == EINTR) continue;
            goto urandom_fallback;
        }
        got += r;
    }
    return 0;
urandom_fallback:
#endif
    /* Fallback to /dev/urandom */
    int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
    if (fd < 0) return -1;
    ssize_t off = 0;
    while ((size_t)off < len) {
        ssize_t r = read(fd, (char *)buf + off, len - (size_t)off);
        if (r < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return -1;
        }
        if (r == 0) break; /* shouldn't happen */
        off += r;
    }
    close(fd);
    return ((size_t)off == len) ? 0 : -1;
}

static int hex_encode(const uint8_t *in, size_t in_len, char *out, size_t out_size) {
    static const char hexd[16] = "0123456789abcdef";
    if (out_size < (in_len * 2 + 1)) return -1;
    for (size_t i = 0; i < in_len; i++) {
        out[i * 2]     = hexd[(in[i] >> 4) & 0xF];
        out[i * 2 + 1] = hexd[in[i] & 0xF];
    }
    out[in_len * 2] = '\0';
    return 0;
}

static bool ends_with_ci(const char *s, const char *sfx) {
    if (!s || !sfx) return false;
    size_t ls = strnlen(s, 4096);
    size_t lf = strnlen(sfx, 64);
    if (lf == 0 || lf > ls) return false;
    const char *p = s + (ls - lf);
    for (size_t i = 0; i < lf; i++) {
        if (tolower((unsigned char)p[i]) != tolower((unsigned char)sfx[i])) return false;
    }
    return true;
}

/* Validate simple ASCII, no control chars */
static bool is_safe_ascii(const char *s) {
    if (!s) return false;
    for (const unsigned char *p = (const unsigned char *)s; *p; ++p) {
        if (*p < 0x20 || *p > 0x7E) return false;
    }
    return true;
}

/* ===== File system helpers (anchor to uploads directory) ===== */

static int ensure_uploads_dir(const char *base_dir) {
    if (!base_dir || !is_safe_ascii(base_dir)) {
        errno = EINVAL;
        return -1;
    }
    /* Create directory if not exists. 0700 permissions. */
    if (mkdir(base_dir, 0700) != 0) {
        if (errno != EEXIST) {
            secure_perror("mkdir uploads");
            return -1;
        }
    }
    /* Open directory securely to anchor further operations */
    int dirfd = open(base_dir, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (dirfd < 0) {
        secure_perror("open uploads dir");
        return -1;
    }
    struct stat st;
    if (fstat(dirfd, &st) != 0) {
        secure_perror("fstat uploads dir");
        close(dirfd);
        return -1;
    }
    if (!S_ISDIR(st.st_mode)) {
        /* Not a directory – refuse */
        close(dirfd);
        errno = ENOTDIR;
        return -1;
    }
    return dirfd;
}

/* ===== Filename sanitization ===== */

static int sanitize_pdf_filename(const char *original_name, char *out_base, size_t out_base_size) {
    if (!original_name || !out_base || out_base_size == 0) {
        errno = EINVAL;
        return -1;
    }

    /* Must end with .pdf (case-insensitive) */
    if (!ends_with_ci(original_name, ".pdf")) {
        errno = EINVAL;
        return -1;
    }

    /* Extract basename (after last / or \) */
    const char *last_slash = strrchr(original_name, '/');
    const char *last_bslash = strrchr(original_name, '\\');
    const char *base = original_name;
    if (last_slash && last_bslash) {
        base = (last_slash > last_bslash) ? last_slash + 1 : last_bslash + 1;
    } else if (last_slash) {
        base = last_slash + 1;
    } else if (last_bslash) {
        base = last_bslash + 1;
    }

    /* Remove extension; find last dot. */
    size_t base_len = strnlen(base, 512);
    if (base_len == 0) {
        errno = EINVAL;
        return -1;
    }
    const char *last_dot = NULL;
    for (const char *p = base; *p; ++p) {
        if (*p == '.') last_dot = p;
    }
    size_t name_len = (last_dot && (strcasecmp(last_dot, ".pdf") == 0))
                        ? (size_t)(last_dot - base)
                        : base_len;

    /* Sanitize: allow [A-Za-z0-9._-], drop others; no empty name */
    char temp[MAX_FILENAME_BASE + 1];
    size_t w = 0;
    for (size_t i = 0; i < name_len && w < MAX_FILENAME_BASE; i++) {
        unsigned char c = (unsigned char)base[i];
        if (isalnum(c) || c == '.' || c == '_' || c == '-') {
            temp[w++] = (char)c;
        } else if (c == ' ') {
            temp[w++] = '-';
        } else {
            /* drop disallowed character */
        }
    }
    if (w == 0) {
        /* default fallback */
        const char *def = "document";
        size_t dlen = strnlen(def, MAX_FILENAME_BASE);
        if (dlen >= out_base_size) {
            errno = ENAMETOOLONG;
            return -1;
        }
        memcpy(out_base, def, dlen + 1);
        return 0;
    }
    temp[w] = '\0';

    /* Copy to out_base */
    if (w + 1 > out_base_size) {
        errno = ENAMETOOLONG;
        return -1;
    }
    memcpy(out_base, temp, w + 1);
    return 0;
}

/* Validate stored relative path from DB: only [A-Za-z0-9._-] plus enforced .pdf; no slashes. */
static int validate_stored_relpath(const char *rel, char *out, size_t out_size) {
    if (!rel || !out || out_size == 0) {
        errno = EINVAL;
        return -1;
    }
    size_t n = strnlen(rel, MAX_STORED_PATH + 1);
    if (n == 0 || n > MAX_STORED_PATH) {
        errno = EINVAL;
        return -1;
    }
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)rel[i];
        if (!(isalnum(c) || c == '.' || c == '_' || c == '-')) {
            errno = EINVAL;
            return -1;
        }
    }
    if (!ends_with_ci(rel, ".pdf")) {
        errno = EINVAL;
        return -1;
    }
    if (memchr(rel, '/', n) || memchr(rel, '\\', n)) {
        errno = EINVAL;
        return -1;
    }
    if (n + 1 > out_size) {
        errno = ENAMETOOLONG;
        return -1;
    }
    memcpy(out, rel, n + 1);
    return 0;
}

/* ===== Database API ===== */

#ifdef USE_REAL_MYSQL

static int db_connect(MYSQL **out, const char *host, const char *user, const char *pass,
                      const char *db, unsigned int port, const char *unix_socket) {
    if (!out) return -1;
    *out = NULL;
    MYSQL *conn = mysql_init(NULL);
    if (!conn) return -1;
    /* Enforce secure defaults */
    my_bool reconnect = 0;
    mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);
#if defined(MYSQL_OPT_TLS_VERSION)
    mysql_options(conn, MYSQL_OPT_TLS_VERSION, "TLSv1.2,TLSv1.3");
#endif
    if (!mysql_real_connect(conn, host, user, pass, db, port, unix_socket, 0)) {
        mysql_close(conn);
        return -1;
    }
    *out = conn;
    return 0;
}

static void db_disconnect(MYSQL *conn) {
    if (conn) mysql_close(conn);
}

static int db_insert_path(MYSQL *conn, const char *path, unsigned long *out_id) {
    if (!conn || !path || !out_id) return -1;
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt) return -1;
    const char *sql = "INSERT INTO documents (path) VALUES (?)";
    if (mysql_stmt_prepare(stmt, sql, (unsigned long)strlen(sql)) != 0) {
        mysql_stmt_close(stmt);
        return -1;
    }
    MYSQL_BIND bind[1];
    memset(bind, 0, sizeof(bind));
    unsigned long path_len = (unsigned long)strnlen(path, MAX_STORED_PATH);
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char *)path;
    bind[0].buffer_length = path_len;
    bind[0].length = &path_len;
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        mysql_stmt_close(stmt);
        return -1;
    }
    if (mysql_stmt_execute(stmt) != 0) {
        mysql_stmt_close(stmt);
        return -1;
    }
    *out_id = (unsigned long)mysql_insert_id(conn);
    mysql_stmt_close(stmt);
    return 0;
}

static int db_get_path_by_id(MYSQL *conn, unsigned long id, char *out_path, size_t out_size) {
    if (!conn || !out_path || out_size == 0) return -1;
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt) return -1;
    const char *sql = "SELECT path FROM documents WHERE id = ?";
    if (mysql_stmt_prepare(stmt, sql, (unsigned long)strlen(sql)) != 0) {
        mysql_stmt_close(stmt);
        return -1;
    }
    MYSQL_BIND bind[1];
    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    int id_i = (int)id;
    bind[0].buffer = &id_i;
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        mysql_stmt_close(stmt);
        return -1;
    }
    if (mysql_stmt_execute(stmt) != 0) {
        mysql_stmt_close(stmt);
        return -1;
    }
    MYSQL_BIND result[1];
    memset(result, 0, sizeof(result));
    unsigned long len = 0;
    result[0].buffer_type = MYSQL_TYPE_STRING;
    result[0].buffer = out_path;
    result[0].buffer_length = (unsigned long)out_size - 1;
    result[0].length = &len;
    if (mysql_stmt_bind_result(stmt, result) != 0) {
        mysql_stmt_close(stmt);
        return -1;
    }
    int fetch_rc = mysql_stmt_fetch(stmt);
    if (fetch_rc != 0 && fetch_rc != MYSQL_DATA_TRUNCATED) {
        mysql_stmt_close(stmt);
        errno = ENOENT;
        return -1;
    }
    if (len >= out_size) {
        out_path[out_size - 1] = '\0';
    } else {
        out_path[len] = '\0';
    }
    mysql_stmt_close(stmt);
    return 0;
}

#else /* Mock DB */

static int db_connect(MYSQL **out, const char *host, const char *user, const char *pass,
                      const char *db, unsigned int port, const char *unix_socket) {
    (void)host; (void)user; (void)pass; (void)db; (void)port; (void)unix_socket;
    if (!out) return -1;
    MYSQL *m = (MYSQL *)calloc(1, sizeof(MYSQL));
    if (!m) return -1;
    m->capacity = 16;
    m->paths = (char **)calloc(m->capacity, sizeof(char *));
    if (!m->paths) {
        free(m);
        return -1;
    }
    m->count = 0;
    *out = m;
    return 0;
}
static void db_disconnect(MYSQL *conn) {
    if (!conn) return;
    for (size_t i = 0; i < conn->count; i++) {
        free(conn->paths[i]);
    }
    free(conn->paths);
    free(conn);
}
static int db_insert_path(MYSQL *conn, const char *path, unsigned long *out_id) {
    if (!conn || !path || !out_id) return -1;
    size_t n = strnlen(path, MAX_STORED_PATH + 1);
    if (n == 0 || n > MAX_STORED_PATH) return -1;
    if (conn->count == conn->capacity) {
        size_t nc = conn->capacity * 2;
        char **np = (char **)realloc(conn->paths, nc * sizeof(char *));
        if (!np) return -1;
        conn->paths = np;
        conn->capacity = nc;
    }
    conn->paths[conn->count] = (char *)malloc(n + 1);
    if (!conn->paths[conn->count]) return -1;
    memcpy(conn->paths[conn->count], path, n + 1);
    conn->count++;
    *out_id = (unsigned long)conn->count; /* 1-based IDs */
    return 0;
}
static int db_get_path_by_id(MYSQL *conn, unsigned long id, char *out_path, size_t out_size) {
    if (!conn || id == 0) return -1;
    size_t idx = (size_t)(id - 1);
    if (idx >= conn->count) {
        errno = ENOENT;
        return -1;
    }
    size_t n = strnlen(conn->paths[idx], MAX_STORED_PATH);
    if (n + 1 > out_size) {
        errno = ENAMETOOLONG;
        return -1;
    }
    memcpy(out_path, conn->paths[idx], n + 1);
    return 0;
}

#endif

/* ===== Core Business Logic ===== */

/*
upload_pdf:
- Validates filename and content type (magic).
- Generates a randomized unique name: <sanitized>_<hex>.pdf
- Writes via a temp file within uploads dir, fsyncs, and atomically renames.
- Stores relative path in DB and returns document ID and stored path.

Returns 0 on success, non-zero on failure.
*/
static int upload_pdf(MYSQL *db,
                      int uploads_dirfd,
                      const char *original_name,
                      const unsigned char *pdf_data,
                      size_t pdf_len,
                      unsigned long *out_doc_id,
                      char *out_relpath,
                      size_t out_relpath_size) {
    if (!db || uploads_dirfd < 0 || !original_name || !pdf_data || !out_doc_id || !out_relpath) {
        errno = EINVAL;
        return -1;
    }
    if (pdf_len == 0 || pdf_len > MAX_PDF_SIZE) {
        errno = EFBIG;
        return -1;
    }
    /* Validate magic header */
    if (pdf_len < 5 || memcmp(pdf_data, "%PDF-", 5) != 0) {
        errno = EINVAL;
        return -1;
    }
    /* Sanitize base filename */
    char base[MAX_FILENAME_BASE + 1];
    if (sanitize_pdf_filename(original_name, base, sizeof(base)) != 0) {
        return -1;
    }
    /* Create random suffix */
    uint8_t randb[8];
    if (get_secure_random(randb, sizeof(randb)) != 0) {
        return -1;
    }
    char hexsuf[sizeof(randb) * 2 + 1];
    if (hex_encode(randb, sizeof(randb), hexsuf, sizeof(hexsuf)) != 0) {
        return -1;
    }
    /* Compose final and temp names */
    char final_name[MAX_STORED_PATH];
    char tmp_name[MAX_STORED_PATH];
    int s1 = snprintf(final_name, sizeof(final_name), "%s_%s.pdf", base, hexsuf);
    if (s1 <= 0 || (size_t)s1 >= sizeof(final_name)) {
        errno = ENAMETOOLONG;
        return -1;
    }
    int s2 = snprintf(tmp_name, sizeof(tmp_name), "%s_%s.pdf.tmp", base, hexsuf);
    if (s2 <= 0 || (size_t)s2 >= sizeof(tmp_name)) {
        errno = ENAMETOOLONG;
        return -1;
    }

    /* Create temp file (no TOCTOU; O_EXCL ensures not pre-existing), no symlink follow */
    int fd = openat(uploads_dirfd, tmp_name, O_CREAT | O_EXCL | O_WRONLY | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (fd < 0) {
        secure_perror("openat temp");
        return -1;
    }

    /* Write the content */
    size_t written = 0;
    while (written < pdf_len) {
        ssize_t w = write(fd, pdf_data + written, pdf_len - written);
        if (w < 0) {
            if (errno == EINTR) continue;
            secure_perror("write");
            close(fd);
            /* Best effort cleanup */
            unlinkat(uploads_dirfd, tmp_name, 0);
            return -1;
        }
        if (w == 0) break;
        written += (size_t)w;
    }
    if (written != pdf_len) {
        /* Short write */
        close(fd);
        unlinkat(uploads_dirfd, tmp_name, 0);
        errno = EIO;
        return -1;
    }
    /* Flush to disk */
    if (fsync(fd) != 0) {
        secure_perror("fsync file");
        close(fd);
        unlinkat(uploads_dirfd, tmp_name, 0);
        return -1;
    }
    if (close(fd) != 0) {
        secure_perror("close file");
        unlinkat(uploads_dirfd, tmp_name, 0);
        return -1;
    }

    /* Atomic rename to final name within same directory */
#if defined(__linux__) && defined(RENAME_NOREPLACE)
    if (renameat2(uploads_dirfd, tmp_name, uploads_dirfd, final_name, RENAME_NOREPLACE) != 0) {
        secure_perror("renameat2");
        unlinkat(uploads_dirfd, tmp_name, 0);
        return -1;
    }
#else
    /* Ensure target doesn't exist; if exists, fail closed. */
    int exists_fd = openat(uploads_dirfd, final_name, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (exists_fd >= 0) {
        close(exists_fd);
        unlinkat(uploads_dirfd, tmp_name, 0);
        errno = EEXIST;
        return -1;
    }
    if (renameat(uploads_dirfd, tmp_name, uploads_dirfd, final_name) != 0) {
        secure_perror("renameat");
        unlinkat(uploads_dirfd, tmp_name, 0);
        return -1;
    }
#endif

    /* fsync directory to persist entry */
    if (fsync(uploads_dirfd) != 0) {
        secure_perror("fsync dir");
        /* File is already in place; continue but report failure conservatively */
        return -1;
    }

    /* Store relative path in DB */
    unsigned long new_id = 0;
    if (db_insert_path(db, final_name, &new_id) != 0) {
        /* Do not leak details; caller can decide to unlink file if needed */
        errno = EIO;
        return -1;
    }

    /* Output values */
    *out_doc_id = new_id;
    size_t flen = strnlen(final_name, sizeof(final_name));
    if (flen + 1 > out_relpath_size) {
        errno = ENAMETOOLONG;
        return -1;
    }
    memcpy(out_relpath, final_name, flen + 1);
    return 0;
}

/*
download_pdf:
- Reads the stored relative path from DB by id.
- Validates it's a safe, normalized filename.
- Opens via openat anchored to uploads dir without following symlinks.
- Validates regular file and size limit; allocates buffer and reads to memory.

Returns 0 on success, non-zero on failure. Caller must free(*out_data) on success.
*/
static int download_pdf(MYSQL *db,
                        int uploads_dirfd,
                        unsigned long doc_id,
                        unsigned char **out_data,
                        size_t *out_len) {
    if (!db || uploads_dirfd < 0 || !out_data || !out_len) {
        errno = EINVAL;
        return -1;
    }
    char rel[MAX_STORED_PATH];
    if (db_get_path_by_id(db, doc_id, rel, sizeof(rel)) != 0) {
        errno = ENOENT;
        return -1;
    }
    char safe_rel[MAX_STORED_PATH];
    if (validate_stored_relpath(rel, safe_rel, sizeof(safe_rel)) != 0) {
        return -1;
    }

    int fd = openat(uploads_dirfd, safe_rel, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) {
        secure_perror("openat read");
        return -1;
    }
    struct stat st;
    if (fstat(fd, &st) != 0) {
        secure_perror("fstat");
        close(fd);
        return -1;
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        errno = EINVAL;
        return -1;
    }
    if (st.st_size < 0 || (size_t)st.st_size > MAX_DOWNLOAD_SIZE) {
        close(fd);
        errno = EFBIG;
        return -1;
    }

    size_t fsize = (size_t)st.st_size;
    unsigned char *buf = (unsigned char *)malloc(fsize ? fsize : 1);
    if (!buf) {
        close(fd);
        return -1;
    }
    size_t off = 0;
    while (off < fsize) {
        ssize_t r = read(fd, buf + off, fsize - off);
        if (r < 0) {
            if (errno == EINTR) continue;
            secure_perror("read");
            free(buf);
            close(fd);
            return -1;
        }
        if (r == 0) break;
        off += (size_t)r;
    }
    close(fd);
    if (off != fsize) {
        free(buf);
        errno = EIO;
        return -1;
    }
    *out_data = buf;
    *out_len = fsize;
    return 0;
}

/* ===== Test Harness (5 test cases) ===== */

static unsigned char *make_sample_pdf(size_t *out_len) {
    /* Minimal valid-looking PDF content */
    const char *content =
        "%PDF-1.7\n"
        "1 0 obj\n<< /Type /Catalog /Pages 2 0 R >>\nendobj\n"
        "2 0 obj\n<< /Type /Pages /Count 1 /Kids [3 0 R] >>\nendobj\n"
        "3 0 obj\n<< /Type /Page /Parent 2 0 R /MediaBox [0 0 200 200] >>\nendobj\n"
        "trailer\n<< /Root 1 0 R >>\n%%EOF\n";
    size_t len = strlen(content);
    unsigned char *buf = (unsigned char *)malloc(len);
    if (!buf) return NULL;
    memcpy(buf, content, len);
    if (out_len) *out_len = len;
    return buf;
}

static void run_test(MYSQL *db, int dirfd, const char *name, const unsigned char *data, size_t len) {
    unsigned long id = 0;
    char relpath[MAX_STORED_PATH];
    int rc = upload_pdf(db, dirfd, name, data, len, &id, relpath, sizeof(relpath));
    if (rc == 0) {
        printf("Upload OK: id=%lu, relpath=%s\n", id, relpath);
        unsigned char *dl = NULL;
        size_t dllen = 0;
        if (download_pdf(db, dirfd, id, &dl, &dllen) == 0) {
            printf("Download OK: id=%lu, size=%zu\n", id, dllen);
            /* Optional: verify magic again */
            if (dllen >= 5 && memcmp(dl, "%PDF-", 5) == 0) {
                printf("Downloaded content verified for id=%lu\n", id);
            } else {
                printf("Downloaded content failed magic check for id=%lu\n", id);
            }
            free(dl);
        } else {
            printf("Download failed for id=%lu\n", id);
        }
    } else {
        printf("Upload failed for name=%s\n", name ? name : "(null)");
    }
}

int main(void) {
    /* Initialize uploads directory */
    int dirfd = ensure_uploads_dir(UPLOAD_DIR);
    if (dirfd < 0) {
        fprintf(stderr, "Failed to initialize uploads directory\n");
        return 1;
    }

    /* Connect to DB (mock by default) */
    MYSQL *db = NULL;
#ifdef USE_REAL_MYSQL
    /* Read credentials from environment; do not hardcode secrets */
    const char *host = getenv("MYSQL_HOST");
    const char *user = getenv("MYSQL_USER");
    const char *pass = getenv("MYSQL_PASS");
    const char *dbn  = getenv("MYSQL_DB");
    unsigned int port = 0;
    const char *port_s = getenv("MYSQL_PORT");
    if (port_s && *port_s) port = (unsigned int)strtoul(port_s, NULL, 10);
    if (db_connect(&db, host ? host : "localhost", user, pass, dbn, port, NULL) != 0) {
        fprintf(stderr, "DB connection failed (check environment variables)\n");
        close(dirfd);
        return 1;
    }
#else
    if (db_connect(&db, NULL, NULL, NULL, NULL, 0, NULL) != 0) {
        fprintf(stderr, "Mock DB init failed\n");
        close(dirfd);
        return 1;
    }
#endif

    /* Prepare sample PDF data */
    size_t pdf_len = 0;
    unsigned char *pdf_data = make_sample_pdf(&pdf_len);
    if (!pdf_data) {
        fprintf(stderr, "Failed to allocate sample PDF\n");
        db_disconnect(db);
        close(dirfd);
        return 1;
    }

    /* Test case 1: Valid */
    run_test(db, dirfd, "report.pdf", pdf_data, pdf_len);

    /* Test case 2: Wrong extension */
    run_test(db, dirfd, "report.txt", pdf_data, pdf_len);

    /* Test case 3: Uppercase extension */
    run_test(db, dirfd, "FILE.PDF", pdf_data, pdf_len);

    /* Test case 4: Path traversal attempts */
    run_test(db, dirfd, "../evil.pdf", pdf_data, pdf_len);
    run_test(db, dirfd, "foo/..\\evil.pdf", pdf_data, pdf_len);

    /* Test case 5: Very long & odd characters */
    char longname[512];
    memset(longname, 'a', sizeof(longname));
    if (sizeof(longname) >= 5) {
        longname[sizeof(longname)-5] = '.';
        longname[sizeof(longname)-4] = 'p';
        longname[sizeof(longname)-3] = 'd';
        longname[sizeof(longname)-2] = 'f';
        longname[sizeof(longname)-1] = '\0';
    } else {
        snprintf(longname, sizeof(longname), "a.pdf");
    }
    run_test(db, dirfd, longname, pdf_data, pdf_len);

    /* Cleanup */
    free(pdf_data);
    db_disconnect(db);
    close(dirfd);
    return 0;
}