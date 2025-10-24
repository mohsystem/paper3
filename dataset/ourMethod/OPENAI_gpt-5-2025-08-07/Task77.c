/*
Secure FTP/FTPS downloader in C using libcurl
- Connects to FTP/FTPS server with provided hostname, username, and password
- Downloads a specified remote file path to current directory
- Enforces secure patterns: TLS verification for FTPS, safe file writes (O_NOFOLLOW, O_EXCL), size limits, input validation, and sanitized filenames
- Usage:
    ./prog <host> <username> <password> <remote_path>
  Notes:
    - host may include scheme (ftp:// or ftps://). If absent, FTPS is assumed.
    - remote_path must be absolute or relative path on the server.
    - Password is not logged. It is zeroized in memory after use when possible.
- Build:
    cc -Wall -Wextra -Werror -O2 ftp_secure_download.c -o ftpget -lcurl
*/

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <dirent.h>
#include <curl/curl.h>
#include <limits.h>
#include <sys/time.h>

#ifndef AT_FDCWD
#define AT_FDCWD -100
#endif

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/* Limits and policies */
#define MAX_LOCAL_FILENAME_LEN 255
#define MAX_URL_LEN 4096
#define MAX_USERNAME_LEN 128
#define MAX_PASSWORD_LEN 256
#define MAX_HOST_LEN 2048
#define MAX_REMOTE_PATH_LEN 2048
#define MAX_DOWNLOAD_BYTES (100UL * 1024UL * 1024UL) /* 100 MB */
#define CONNECT_TIMEOUT_SECS 5L
#define OPERATION_TIMEOUT_SECS 600L

/* Error codes */
enum {
    RC_OK = 0,
    RC_USAGE = 1,
    RC_INPUT_INVALID = 2,
    RC_FILE_OPEN = 3,
    RC_NETWORK = 4,
    RC_WRITE = 5,
    RC_FINALIZE = 6,
    RC_INTERNAL = 7
};

static void secure_zero(void *v, size_t n) {
#if defined(__STDC_LIB_EXT1__)
    if (v && n) memset_s(v, n, 0, n);
#else
    if (!v || !n) return;
    volatile unsigned char *p = (volatile unsigned char *)v;
    while (n--) *p++ = 0;
#endif
}

static bool has_prefix(const char *s, const char *pfx) {
    if (!s || !pfx) return false;
    size_t ls = strlen(s), lp = strlen(pfx);
    return (ls >= lp) && (strncmp(s, pfx, lp) == 0);
}

static bool is_scheme_present(const char *host) {
    return has_prefix(host, "ftp://") || has_prefix(host, "ftps://");
}

static bool is_ftps_scheme(const char *host) {
    return has_prefix(host, "ftps://");
}

static bool is_safe_filename_char(int c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') ||
           c == '.' || c == '_' || c == '-';
}

/* Extract basename from remote path and sanitize for local filename */
static int sanitize_basename(const char *remote_path, char *out, size_t outsz) {
    if (!remote_path || !out || outsz == 0) return -1;

    const char *last = remote_path;
    const char *p = remote_path;
    while (*p) {
        if (*p == '/') last = p + 1;
        p++;
    }
    const char *base = last;
    if (*base == '\0') base = "download";

    size_t len = 0;
    for (size_t i = 0; base[i] != '\0' && len + 1 < outsz && len < MAX_LOCAL_FILENAME_LEN; i++) {
        char ch = base[i];
        if (is_safe_filename_char((unsigned char)ch)) {
            out[len++] = ch;
        } else {
            out[len++] = '_';
        }
    }
    if (len == 0) {
        if (outsz < 2) return -1;
        out[0] = 'f';
        out[1] = '\0';
        return 0;
    }
    out[len] = '\0';
    return 0;
}

/* Basic remote path validation to reduce traversal surprises on server side */
static bool validate_remote_path(const char *rp) {
    if (!rp) return false;
    size_t n = strnlen(rp, MAX_REMOTE_PATH_LEN + 1);
    if (n == 0 || n > MAX_REMOTE_PATH_LEN) return false;

    /* Reject overly suspicious traversal sequences */
    if (strstr(rp, "/../") != NULL) return false;
    if (has_prefix(rp, "../")) return false;
    if (strcmp(rp, "..") == 0) return false;
    if (strstr(rp, "/..") && rp[strlen(rp) - 1] == '.') return false;

    /* Avoid CRLF injection */
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)rp[i];
        if (c == '\r' || c == '\n') return false;
    }
    return true;
}

/* Encode path preserving '/' and encoding spaces/unsafe bytes */
static char hex_digit(unsigned v) { return (v < 10) ? (char)('0' + v) : (char)('A' + (v - 10)); }

static char *percent_encode_path(const char *path) {
    if (!path) return NULL;
    size_t inlen = strnlen(path, MAX_REMOTE_PATH_LEN + 1);
    if (inlen == 0 || inlen > MAX_REMOTE_PATH_LEN) return NULL;

    /* Worst case every byte becomes %XX -> 3x + 1 for NUL */
    size_t maxout = inlen * 3 + 1;
    if (maxout > MAX_URL_LEN) return NULL;

    char *out = (char *)malloc(maxout);
    if (!out) return NULL;

    size_t j = 0;
    for (size_t i = 0; i < inlen; i++) {
        unsigned char c = (unsigned char)path[i];
        if (c == '/') {
            out[j++] = '/';
        } else if ((c >= 'a' && c <= 'z') ||
                   (c >= 'A' && c <= 'Z') ||
                   (c >= '0' && c <= '9') ||
                   c == '-' || c == '_' || c == '.' || c == '~') {
            out[j++] = (char)c;
        } else {
            if (j + 3 >= maxout) { free(out); return NULL; }
            out[j++] = '%';
            out[j++] = hex_digit((c >> 4) & 0xF);
            out[j++] = hex_digit(c & 0xF);
        }
    }
    out[j] = '\0';
    return out;
}

/* Build a URL for curl: scheme + host + '/' + encoded path */
static int build_url(const char *host, const char *remote_path, char *out, size_t outsz, int *out_is_ftps) {
    if (!host || !remote_path || !out || outsz == 0 || !out_is_ftps) return -1;

    size_t hostlen = strnlen(host, MAX_HOST_LEN + 1);
    if (hostlen == 0 || hostlen > MAX_HOST_LEN) return -1;

    char *enc_path = percent_encode_path(remote_path);
    if (!enc_path) return -1;

    const char *scheme;
    if (is_scheme_present(host)) {
        scheme = "";
        *out_is_ftps = is_ftps_scheme(host) ? 1 : 0;
    } else {
        scheme = "ftps://";
        *out_is_ftps = 1;
    }

    const char *host_body = host;
    if (is_scheme_present(host)) {
        host_body = host; /* already includes scheme */
    }

    if (scheme[0] != '\0') {
        /* scheme provided by us */
        int w = snprintf(out, outsz, "%s%s%s%s", scheme, host_body,
                         (enc_path[0] == '/') ? "" : "/", enc_path);
        free(enc_path);
        if (w < 0 || (size_t)w >= outsz) return -1;
        return 0;
    } else {
        /* host already includes scheme */
        int w = snprintf(out, outsz, "%s%s%s", host_body,
                         (enc_path[0] == '/') ? "" : "/", enc_path);
        free(enc_path);
        if (w < 0 || (size_t)w >= outsz) return -1;
        return 0;
    }
}

/* Random bytes from /dev/urandom */
static int get_random_bytes(void *buf, size_t n) {
    if (!buf || n == 0) return -1;
    int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
    if (fd < 0) return -1;
    size_t off = 0;
    while (off < n) {
        ssize_t r = read(fd, (unsigned char*)buf + off, n - off);
        if (r < 0) { if (errno == EINTR) continue; close(fd); return -1; }
        if (r == 0) { close(fd); return -1; }
        off += (size_t)r;
    }
    close(fd);
    return 0;
}

/* Create a temp file in dirfd with O_EXCL and no symlink following.
   outname must have capacity >= MAX_LOCAL_FILENAME_LEN + 32 */
static int create_temp_file(int dirfd, const char *base, char *outname, size_t outname_sz, int *outfd) {
    if (!base || !outname || outname_sz < 2 || !outfd) return -1;

    for (int attempt = 0; attempt < 16; attempt++) {
        unsigned char rnd[8];
        if (get_random_bytes(rnd, sizeof(rnd)) != 0) return -1;
        uint64_t val = 0;
        for (int i = 0; i < 8; i++) val = (val << 8) | rnd[i];

        int n = snprintf(outname, outname_sz, "%s.part.%016llx", base, (unsigned long long)val);
        if (n < 0 || (size_t)n >= outname_sz) return -1;

        int fd = openat(dirfd, outname, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);
        if (fd >= 0) {
            *outfd = fd;
            return 0;
        }
        if (errno == EEXIST) continue;
        if (errno == ELOOP) return -1;
        if (errno == ENOSPC || errno == EDQUOT) return -1;
        /* Otherwise retry a few times */
    }
    return -1;
}

static int fsync_dirfd(int dirfd) {
    if (dirfd < 0) return -1;
    if (fsync(dirfd) != 0) return -1;
    return 0;
}

struct dl_state {
    int fd;
    size_t total;
    size_t max_bytes;
    int error; /* 0 ok, else errno-like */
};

/* Robust write loop to fd */
static int write_all(int fd, const unsigned char *buf, size_t len) {
    size_t off = 0;
    while (off < len) {
        ssize_t w = write(fd, buf + off, len - off);
        if (w < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (w == 0) return -1;
        off += (size_t)w;
    }
    return 0;
}

static size_t curl_write_cb(char *ptr, size_t size, size_t nmemb, void *userdata) {
    struct dl_state *st = (struct dl_state *)userdata;
    if (!st || st->fd < 0) return 0;

    size_t bytes = size * nmemb;
    if (size != 0 && nmemb > SIZE_MAX / size) {
        st->error = EOVERFLOW;
        return 0;
    }
    if (st->total > st->max_bytes) {
        st->error = EFBIG;
        return 0;
    }
    if (bytes > 0) {
        if (st->total + bytes < st->total || st->total + bytes > st->max_bytes) {
            st->error = EFBIG;
            return 0;
        }
        if (write_all(st->fd, (const unsigned char *)ptr, bytes) != 0) {
            st->error = errno ? errno : EIO;
            return 0;
        }
        st->total += bytes;
    }
    return bytes;
}

/* Finalize safe write: fsync file, then create final name via linkat, unlink temp.
   If linkat fails due to filesystem constraints, fallback to renameat only if destination doesn't exist.
*/
static int finalize_file(int dirfd, int fd, const char *tmpname, const char *finalname) {
    if (fd < 0 || !tmpname || !finalname) return -1;

    if (fsync(fd) != 0) return -1;
    if (close(fd) != 0) return -1;

    /* Ensure final doesn't already exist by using linkat (will fail if exists) */
    if (linkat(dirfd, tmpname, dirfd, finalname, 0) == 0) {
        /* Remove temp link */
        if (unlinkat(dirfd, tmpname, 0) != 0) return -1;
        if (fsync_dirfd(dirfd) != 0) return -1;
        return 0;
    } else {
        if (errno == EEXIST) {
            /* Do not overwrite existing file */
            (void)unlinkat(dirfd, tmpname, 0);
            return -2;
        }
        /* Fallback: try to ensure destination doesn't exist, then renameat */
        struct stat st;
        if (fstatat(dirfd, finalname, &st, AT_SYMLINK_NOFOLLOW) == 0) {
            /* Exists: do not overwrite */
            (void)unlinkat(dirfd, tmpname, 0);
            return -2;
        }
        if (errno != ENOENT) {
            (void)unlinkat(dirfd, tmpname, 0);
            return -1;
        }
        if (renameat(dirfd, tmpname, dirfd, finalname) != 0) {
            (void)unlinkat(dirfd, tmpname, 0);
            return -1;
        }
        if (fsync_dirfd(dirfd) != 0) return -1;
        return 0;
    }
}

/* Core download function */
static int download_sensitive_file(const char *host, const char *username, const char *password, const char *remote_path) {
    if (!host || !username || !password || !remote_path) return RC_INPUT_INVALID;

    size_t ulen = strnlen(username, MAX_USERNAME_LEN + 1);
    size_t plen = strnlen(password, MAX_PASSWORD_LEN + 1);
    size_t hlen = strnlen(host, MAX_HOST_LEN + 1);
    size_t rlen = strnlen(remote_path, MAX_REMOTE_PATH_LEN + 1);
    if (ulen == 0 || ulen > MAX_USERNAME_LEN) return RC_INPUT_INVALID;
    if (plen > MAX_PASSWORD_LEN) return RC_INPUT_INVALID;
    if (hlen == 0 || hlen > MAX_HOST_LEN) return RC_INPUT_INVALID;
    if (rlen == 0 || rlen > MAX_REMOTE_PATH_LEN) return RC_INPUT_INVALID;
    if (!validate_remote_path(remote_path)) return RC_INPUT_INVALID;

    char local_base[MAX_LOCAL_FILENAME_LEN + 1];
    if (sanitize_basename(remote_path, local_base, sizeof(local_base)) != 0) return RC_INPUT_INVALID;

    /* Open current directory as anchor */
    int dirfd = open(".", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dirfd < 0) return RC_FILE_OPEN;

    char tmpname[MAX_LOCAL_FILENAME_LEN + 64];
    int data_fd = -1;
    if (create_temp_file(dirfd, local_base, tmpname, sizeof(tmpname), &data_fd) != 0) {
        close(dirfd);
        return RC_FILE_OPEN;
    }

    CURL *curl = NULL;
    CURLcode cres;
    int rc = RC_INTERNAL;
    struct dl_state st = { .fd = data_fd, .total = 0, .max_bytes = MAX_DOWNLOAD_BYTES, .error = 0 };

    char url[MAX_URL_LEN];
    int is_ftps = 0;
    if (build_url(host, remote_path, url, sizeof(url), &is_ftps) != 0) {
        rc = RC_INPUT_INVALID;
        goto cleanup;
    }

    curl = curl_easy_init();
    if (!curl) {
        rc = RC_INTERNAL;
        goto cleanup;
    }

    /* Restrict protocols */
    long protos = is_ftps ? CURLPROTO_FTPS : CURLPROTO_FTP;
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS, protos);
    curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, protos);

    /* URL and credentials */
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERNAME, username);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, password);

    /* Timeouts and safety */
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CONNECT_TIMEOUT_SECS);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, OPERATION_TIMEOUT_SECS);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

    /* Strong TLS policy for FTPS */
    if (is_ftps) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
#ifdef CURL_SSLVERSION_TLSv1_3
        /* Set minimum TLSv1.2, allow up to 1.3 if available */
        curl_easy_setopt(curl, CURLOPT_SSLVERSION, (long)CURL_SSLVERSION_TLSv1_2);
#else
        curl_easy_setopt(curl, CURLOPT_SSLVERSION, (long)CURL_SSLVERSION_TLSv1_2);
#endif
    }

    /* Use passive mode by default (safer across firewalls) */
    curl_easy_setopt(curl, CURLOPT_FTP_USE_EPSV, 1L);

    /* Set write callback */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &st);

    cres = curl_easy_perform(curl);
    if (cres != CURLE_OK) {
        rc = RC_NETWORK;
        goto cleanup;
    }
    if (st.error != 0) {
        rc = RC_WRITE;
        goto cleanup;
    }

    /* Finalize file */
    {
        int fin = finalize_file(dirfd, data_fd, tmpname, local_base);
        data_fd = -1; /* finalized or closed inside finalize_file */
        if (fin == 0) {
            rc = RC_OK;
        } else if (fin == -2) {
            rc = RC_FINALIZE; /* target exists; do not overwrite */
        } else {
            rc = RC_FINALIZE;
        }
    }

cleanup:
    if (curl) curl_easy_cleanup(curl);

    /* Zeroize password buffer if we can (argv memory) */
    if (password) {
        size_t l = strnlen(password, MAX_PASSWORD_LEN);
        secure_zero((void *)password, l);
    }

    if (data_fd >= 0) {
        (void)close(data_fd);
    }
    if (dirfd >= 0) {
        /* If we failed before finalizing, clean up temp file */
        if (rc != RC_OK) {
            (void)unlinkat(dirfd, tmpname, 0);
        }
        (void)fsync_dirfd(dirfd); /* Best effort */
        (void)close(dirfd);
    }

    return rc;
}

/* Simple test harness: 5 test cases.
   When args are provided, run a single download.
   Tests avoid printing secrets and are expected to fail on typical environments without servers.
*/
static void run_tests(void) {
    struct {
        const char *desc;
        const char *host;
        const char *user;
        char passwd[32];
        const char *path;
        int expect_any; /* -1 means any result acceptable, else expected code */
    } tests[5] = {
        { "Invalid host (FTPS default)", "nonexistent.invalid", "user", "pass123!", "/secret/file.txt", -1 },
        { "Explicit FTPS scheme", "ftps://example.com", "u", "p", "/sensitive/data.bin", -1 },
        { "Reject traversal path", "ftps://example.com", "u", "p", "/../../etc/passwd", RC_INPUT_INVALID },
        { "Plain FTP scheme (insecure by user choice)", "ftp://example.com", "anonymous", "x", "/pub/readme.txt", -1 },
        { "Localhost likely no FTP", "127.0.0.1", "u", "p", "relative/file.txt", -1 }
    };

    for (int i = 0; i < 5; i++) {
        int rc = download_sensitive_file(tests[i].host, tests[i].user, tests[i].passwd, tests[i].path);
        /* Only print minimal info, no secrets */
        printf("Test %d (%s): rc=%d\n", i + 1, tests[i].desc, rc);
        /* password in test buffer is already zeroized by the function */
    }
}

int main(int argc, char **argv) {
    if (argc == 5) {
        const char *host = argv[1];
        const char *user = argv[2];
        const char *pass = argv[3];
        const char *rpath = argv[4];

        int rc = download_sensitive_file(host, user, pass, rpath);
        if (rc != RC_OK) {
            /* Minimal error reporting without leaking secrets */
            fprintf(stderr, "Download failed (rc=%d)\n", rc);
            return rc;
        }
        printf("Download succeeded\n");
        return 0;
    } else {
        /* Run internal tests if no/incorrect args */
        run_tests();
        fprintf(stderr, "Usage: %s <host> <username> <password> <remote_path>\n", argv[0]);
        return RC_USAGE;
    }
}