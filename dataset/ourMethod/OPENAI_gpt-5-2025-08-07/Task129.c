/*
Secure file retriever in C using libcurl and POSIX-safe file handling.

Build (Linux/macOS):
  cc -Wall -Wextra -Werror -O2 task129.c -o task129 -lcurl

Run:
  ./task129

This program demonstrates secure downloading of files from a fixed HTTPS base URL,
based on a user-provided filename (validated and sanitized). It writes the file
to a local "downloads" directory using race-safe, symlink-safe APIs and enforces
size and time limits.
*/

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <curl/curl.h>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

/* Return codes */
#define DL_OK 0
#define DL_ERR_INPUT 1
#define DL_ERR_DIR 2
#define DL_ERR_OPEN 3
#define DL_ERR_NET 4
#define DL_ERR_IO 5
#define DL_ERR_SIZE 6
#define DL_ERR_INTERNAL 7

/* Limits */
#define FILENAME_MAX_LEN 64
#define URL_MAX_LEN 2048
#define MAX_DOWNLOAD_BYTES (10 * 1024 * 1024) /* 10 MiB */
#define CONNECT_TIMEOUT_SECS 2L
#define TOTAL_TIMEOUT_SECS 5L
#define MAX_REDIRECTS 3L

/* Validate HTTPS base URL (scheme and trailing slash) */
static int is_https_base_url(const char *url) {
    if (!url) return 0;
    size_t len = strnlen(url, URL_MAX_LEN + 1);
    if (len == 0 || len > URL_MAX_LEN) return 0;
    const char *prefix = "https://";
    size_t prelen = 8; /* including 's' and '://' */
    if (len <= prelen || strncmp(url, prefix, prelen) != 0) return 0;
    /* Must have at least one host character and end with '/' to form a directory-like base */
    if (url[len - 1] != '/') return 0;
    /* Ensure there is a host part between scheme and trailing slash */
    if (len <= prelen + 1) return 0;
    return 1;
}

/* Validate filename: 1..64 chars, only [A-Za-z0-9._-], no leading '.', no ".." substring */
static int validate_filename(const char *name) {
    if (!name) return 0;
    size_t len = strnlen(name, FILENAME_MAX_LEN + 1);
    if (len == 0 || len > FILENAME_MAX_LEN) return 0;
    if (name[0] == '.') return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)name[i];
        int ok = (c >= 'A' && c <= 'Z') ||
                 (c >= 'a' && c <= 'z') ||
                 (c >= '0' && c <= '9') ||
                 c == '.' || c == '_' || c == '-';
        if (!ok) return 0;
    }
    if (strstr(name, "..") != NULL) return 0;
    return 1;
}

/* Open or create a directory securely; return dirfd or -1 */
static int open_or_create_dir(const char *dir) {
    if (!dir) return -1;
    /* Try to open existing directory */
    int dfd = open(dir, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dfd >= 0) {
        return dfd;
    }
    if (errno != ENOENT) {
        return -1;
    }
    /* Create with 0700 and try open again */
    if (mkdir(dir, 0700) != 0) {
        /* If created by another process just now, continue */
        if (!(errno == EEXIST)) {
            return -1;
        }
    }
    dfd = open(dir, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    return dfd;
}

/* Compose a file path string (for display only). out must have out_len bytes. */
static int compose_path(const char *dir, const char *filename, char *out, size_t out_len) {
    if (!dir || !filename || !out || out_len == 0) return 0;
    size_t dlen = strnlen(dir, 1024);
    size_t flen = strnlen(filename, FILENAME_MAX_LEN);
    if (dlen == 0 || flen == 0) return 0;
    int need_sep = (dir[dlen - 1] == '/') ? 0 : 1;
    size_t total = dlen + (size_t)need_sep + flen + 1;
    if (total > out_len) return 0;
    memcpy(out, dir, dlen);
    size_t pos = dlen;
    if (need_sep) {
        out[pos++] = '/';
    }
    memcpy(out + pos, filename, flen);
    pos += flen;
    out[pos] = '\0';
    return 1;
}

/* Build a full URL by appending a URL-escaped filename to base_url. Returns malloc'd string or NULL. */
static char* build_full_url(CURL *curlh, const char *base_url, const char *filename) {
    if (!curlh || !base_url || !filename) return NULL;
    char *escaped = curl_easy_escape(curlh, filename, 0);
    if (!escaped) return NULL;
    size_t blen = strnlen(base_url, URL_MAX_LEN);
    size_t elen = strnlen(escaped, URL_MAX_LEN);
    if (blen == 0 || elen == 0 || blen + elen >= URL_MAX_LEN) {
        curl_free(escaped);
        return NULL;
    }
    char *full = (char*)malloc(blen + elen + 1);
    if (!full) {
        curl_free(escaped);
        return NULL;
    }
    memcpy(full, base_url, blen);
    memcpy(full + blen, escaped, elen);
    full[blen + elen] = '\0';
    curl_free(escaped);
    return full;
}

struct WriteCtx {
    FILE *out;
    size_t written;
    size_t limit;
    int overflowed; /* 1 if exceeded limit */
    int io_error;   /* 1 if fwrite failed */
};

/* Write callback enforcing a maximum size and robust I/O */
static size_t write_cb(char *ptr, size_t size, size_t nmemb, void *userdata) {
    struct WriteCtx *ctx = (struct WriteCtx*)userdata;
    if (!ctx || !ctx->out) return 0;
    size_t chunk = size * nmemb;
    if (chunk == 0) return 0;

    if (ctx->written > ctx->limit) {
        ctx->overflowed = 1;
        return 0; /* Abort */
    }
    size_t remaining = ctx->limit - ctx->written;
    if (chunk > remaining) {
        /* Write only up to the limit and signal overflow */
        size_t to_write = remaining;
        if (to_write > 0) {
            size_t w = fwrite(ptr, 1, to_write, ctx->out);
            if (w != to_write) {
                ctx->io_error = 1;
                return 0;
            }
            ctx->written += w;
        }
        ctx->overflowed = 1;
        /* Returning less than asked causes libcurl to abort with CURLE_WRITE_ERROR */
        return 0;
    } else {
        size_t w = fwrite(ptr, 1, chunk, ctx->out);
        if (w != chunk) {
            ctx->io_error = 1;
            return 0;
        }
        ctx->written += w;
        return w;
    }
}

/* Perform the download securely with libcurl. Returns 0 on success, else error code. */
static int perform_download_https(const char *url, FILE *out, size_t max_bytes, long connect_timeout, long total_timeout) {
    if (!url || !out) return DL_ERR_INTERNAL;

    int rc = DL_ERR_NET;
    CURL *curl = curl_easy_init();
    if (!curl) return DL_ERR_INTERNAL;

    struct WriteCtx ctx;
    ctx.out = out;
    ctx.written = 0;
    ctx.limit = max_bytes;
    ctx.overflowed = 0;
    ctx.io_error = 0;

    /* Enforce HTTPS only */
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
#ifdef CURLOPT_REDIR_PROTOCOLS
    curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS);
#endif
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, MAX_REDIRECTS);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, connect_timeout);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, total_timeout);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
#ifdef CURLOPT_SSLVERSION
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
#endif
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "SecureRetriever/1.0");

    CURLcode res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
        if (ctx.overflowed) {
            rc = DL_ERR_SIZE;
        } else if (ctx.io_error) {
            rc = DL_ERR_IO;
        } else {
            rc = DL_OK;
        }
    } else {
        if (ctx.overflowed) rc = DL_ERR_SIZE;
        else if (ctx.io_error) rc = DL_ERR_IO;
        else rc = DL_ERR_NET;
    }

    curl_easy_cleanup(curl);
    return rc;
}

/* Create an output file securely in dirfd with the exact filename. Return fd or -1 on error. */
static int secure_create_output(int dirfd, const char *filename) {
    if (dirfd < 0 || !filename) return -1;
    /* O_NOFOLLOW prevents following a symlink at the final path component.
       O_EXCL ensures we do not overwrite an existing file. */
    int fd = openat(dirfd, filename, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);
    if (fd < 0) {
        return -1;
    }
    /* Validate the opened handle refers to a regular file */
    struct stat st;
    if (fstat(fd, &st) != 0) {
        int e = errno;
        close(fd);
        errno = e;
        return -1;
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        errno = EINVAL;
        return -1;
    }
    return fd;
}

/* Public API: download a given filename from base_url into base_dir. */
static int download_to_dir(const char *base_url,
                           const char *base_dir,
                           const char *filename,
                           size_t max_bytes,
                           long connect_timeout,
                           long total_timeout,
                           char *out_local_path,
                           size_t out_local_path_len) {
    if (!is_https_base_url(base_url)) return DL_ERR_INPUT;
    if (!validate_filename(filename)) return DL_ERR_INPUT;
    if (!base_dir) return DL_ERR_INPUT;

    int dirfd = open_or_create_dir(base_dir);
    if (dirfd < 0) {
        return DL_ERR_DIR;
    }

    /* Create output file securely */
    int fd = secure_create_output(dirfd, filename);
    if (fd < 0) {
        close(dirfd);
        return DL_ERR_OPEN;
    }

    FILE *f = fdopen(fd, "wb");
    if (!f) {
        int saved = errno;
        close(fd);
        /* Clean up the created file */
        (void)unlinkat(dirfd, filename, 0);
        close(dirfd);
        errno = saved;
        return DL_ERR_OPEN;
    }

    /* Setup curl and URL */
    CURL *curl_tmp = curl_easy_init();
    if (!curl_tmp) {
        fclose(f); /* also closes fd */
        (void)unlinkat(dirfd, filename, 0);
        close(dirfd);
        return DL_ERR_INTERNAL;
    }
    char *full_url = build_full_url(curl_tmp, base_url, filename);
    curl_easy_cleanup(curl_tmp);
    if (!full_url) {
        fclose(f);
        (void)unlinkat(dirfd, filename, 0);
        close(dirfd);
        return DL_ERR_INPUT;
    }

    int rc = perform_download_https(full_url, f, max_bytes, connect_timeout, total_timeout);

    /* Flush and fsync on success */
    if (rc == DL_OK) {
        if (fflush(f) != 0) {
            rc = DL_ERR_IO;
        } else {
            int ofd = fileno(f);
            if (ofd >= 0) {
                if (fsync(ofd) != 0) {
                    rc = DL_ERR_IO;
                }
            }
        }
    }

    free(full_url);
    int saved_errno = errno;
    fclose(f); /* close file */

    if (rc != DL_OK) {
        /* Remove partial file */
        (void)unlinkat(dirfd, filename, 0);
    } else {
        /* Optionally present the full local path to the caller */
        if (out_local_path && out_local_path_len > 0) {
            if (!compose_path(base_dir, filename, out_local_path, out_local_path_len)) {
                /* Not fatal to download result; just leave empty */
                if (out_local_path_len > 0) out_local_path[0] = '\0';
            }
        }
    }

    close(dirfd);
    errno = saved_errno;
    return rc;
}

/* Map result code to message */
static const char* result_str(int code) {
    switch (code) {
        case DL_OK: return "OK";
        case DL_ERR_INPUT: return "INPUT_ERROR";
        case DL_ERR_DIR: return "DIR_ERROR";
        case DL_ERR_OPEN: return "OPEN_ERROR";
        case DL_ERR_NET: return "NETWORK_ERROR";
        case DL_ERR_IO: return "IO_ERROR";
        case DL_ERR_SIZE: return "SIZE_LIMIT_EXCEEDED";
        default: return "INTERNAL_ERROR";
    }
}

/* Simple test harness with 5 cases */
int main(void) {
    /* Initialize libcurl once per process */
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0) {
        fprintf(stderr, "curl_global_init failed\n");
        return 1;
    }

    const char *base_url = "https://example.com/files/";
    const char *base_dir = "downloads";

    const char *tests[5] = {
        "readme.txt",           /* valid */
        "bad/../evil",          /* invalid: slash and traversal */
        "this_is_a_valid-name.1", /* valid */
        ".hidden",              /* invalid: starts with dot */
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", /* 64 'a' -> valid length edge */
    };

    for (int i = 0; i < 5; i++) {
        const char *fname = tests[i];
        char local_path[512];
        local_path[0] = '\0';
        int rc = download_to_dir(base_url, base_dir, fname,
                                 MAX_DOWNLOAD_BYTES,
                                 CONNECT_TIMEOUT_SECS,
                                 TOTAL_TIMEOUT_SECS,
                                 local_path, sizeof local_path);
        printf("Test %d: filename='%s' -> %s", i + 1, fname, result_str(rc));
        if (rc == DL_OK) {
            printf(" (saved to %s)", local_path[0] ? local_path : "(unknown)");
        }
        printf("\n");
    }

    curl_global_cleanup();
    return 0;
}