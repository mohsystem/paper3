/*
Simple and secure HTTP upload handler in C.

- Parses a minimal HTTP POST /upload with multipart/form-data
- Validates headers, boundary, filename, and size
- Stores uploaded file safely into a local uploads directory using openat with O_NOFOLLOW and atomic linkat
- Implements 5 self-contained test cases that simulate HTTP requests
- Not a network server; focuses on secure parsing and file writing primitives
*/

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#ifdef __linux__
#include <sys/random.h>
#endif

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

#ifndef AT_FDCWD
#define AT_FDCWD -100
#endif

#define MAX_REQUEST_SIZE       (6u * 1024u * 1024u)  /* 6 MB total HTTP request limit */
#define MAX_UPLOAD_SIZE        (5u * 1024u * 1024u)  /* 5 MB file payload limit */
#define MAX_HEADER_SECTION     (16u * 1024u)         /* 16 KB header limit */
#define MAX_BOUNDARY_LEN       70u                   /* per RFC, but we restrict further */
#define MAX_FILENAME_LEN       64u                   /* allowed file name length */
#define RESP_BUF_CAP           (8u * 1024u)          /* response buffer capacity */
#define MAX_PART_HEADERS       (8u * 1024u)          /* 8 KB part header limit */
#define DEFAULT_UPLOAD_DIR     "uploads"

typedef struct {
    const uint8_t *data;
    size_t len;
} slice_t;

typedef struct {
    char filename[MAX_FILENAME_LEN + 1];
    const uint8_t *file_data;
    size_t file_len;
} upload_part_t;

/* Utility safe strnlen */
static size_t s_strnlen(const char *s, size_t maxlen) {
    size_t i = 0;
    for (; i < maxlen && s[i] != '\0'; ++i) { }
    return i;
}

/* Case-insensitive compare for ASCII header names with explicit lengths */
static int s_strncaseeq(const char *a, size_t alen, const char *b) {
    size_t blen = s_strnlen(b, 1024);
    if (alen != blen) return 0;
    for (size_t i = 0; i < alen; ++i) {
        unsigned char ca = (unsigned char)a[i];
        unsigned char cb = (unsigned char)b[i];
        if (tolower(ca) != tolower(cb)) return 0;
    }
    return 1;
}

/* Memory search; returns pointer or NULL */
static const uint8_t *s_memmem(const uint8_t *hay, size_t haylen, const uint8_t *needle, size_t nlen) {
    if (nlen == 0 || haylen < nlen) return NULL;
    for (size_t i = 0; i + nlen <= haylen; ++i) {
        if (hay[i] == needle[0] && memcmp(hay + i, needle, nlen) == 0) {
            return hay + i;
        }
    }
    return NULL;
}

/* Trim ASCII spaces around a slice to produce a string into out (bounded) */
static void s_trim_to_buf(const char *in, size_t inlen, char *out, size_t outcap) {
    size_t start = 0, end = inlen;
    while (start < end && (in[start] == ' ' || in[start] == '\t')) start++;
    while (end > start && (in[end - 1] == ' ' || in[end - 1] == '\t')) end--;
    size_t n = end > start ? end - start : 0;
    if (n >= outcap) n = outcap - 1;
    if (outcap > 0) {
        memcpy(out, in + start, n);
        out[n] = '\0';
    }
}

/* Safely parse unsigned long from buffer with bounds and no trailing junk */
static int s_parse_size(const char *s, size_t slen, size_t *out) {
    if (slen == 0 || slen > 32) return -1;
    char tmp[33];
    memcpy(tmp, s, slen);
    tmp[slen] = '\0';
    errno = 0;
    char *endptr = NULL;
    unsigned long v = strtoul(tmp, &endptr, 10);
    if (errno != 0 || endptr == tmp || *endptr != '\0') return -1;
    if (v > SIZE_MAX) return -1;
    *out = (size_t)v;
    return 0;
}

/* Secure random bytes */
static int s_random_bytes(uint8_t *out, size_t n) {
#ifdef __linux__
    ssize_t r = getrandom(out, n, 0);
    if (r == (ssize_t)n) return 0;
    /* fallthrough to /dev/urandom if partial or not supported */
#endif
    int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
    if (fd < 0) return -1;
    size_t got = 0;
    while (got < n) {
        ssize_t m = read(fd, out + got, n - got);
        if (m < 0) { if (errno == EINTR) continue; close(fd); return -1; }
        if (m == 0) { close(fd); return -1; }
        got += (size_t)m;
    }
    close(fd);
    return 0;
}

/* Generate hex string from random bytes */
static int s_random_hex(char *out, size_t outcap, size_t bytes) {
    if (outcap < bytes * 2 + 1) return -1;
    uint8_t buf[32];
    if (bytes > sizeof(buf)) return -1;
    if (s_random_bytes(buf, bytes) != 0) return -1;
    static const char *hex = "0123456789abcdef";
    for (size_t i = 0; i < bytes; ++i) {
        out[2*i]   = hex[(buf[i] >> 4) & 0xF];
        out[2*i+1] = hex[buf[i] & 0xF];
    }
    out[bytes*2] = '\0';
    return 0;
}

/* Validate boundary: 1..MAX_BOUNDARY_LEN and only [A-Za-z0-9'()+_,./:=?-] minus risky; we restrict to [A-Za-z0-9._-] */
static int s_is_allowed_boundary(const char *b, size_t blen) {
    if (blen == 0 || blen > MAX_BOUNDARY_LEN) return 0;
    for (size_t i = 0; i < blen; ++i) {
        char c = b[i];
        if (!(isalnum((unsigned char)c) || c == '.' || c == '_' || c == '-')) {
            return 0;
        }
    }
    return 1;
}

/* Validate filename: enforce length, charset, no leading dot, no ".." */
static int s_is_allowed_filename(const char *name) {
    size_t n = s_strnlen(name, MAX_FILENAME_LEN + 1);
    if (n == 0 || n > MAX_FILENAME_LEN) return 0;
    if (name[0] == '.') return 0; /* avoid dotfiles */
    /* reject if contains path separators or ".." */
    for (size_t i = 0; i < n; ++i) {
        char c = name[i];
        if (!(isalnum((unsigned char)c) || c == '.' || c == '_' || c == '-')) return 0;
    }
    if (strstr(name, "..") != NULL) return 0;
    return 1;
}

/* Ensure uploads directory exists and open safely */
static int ensure_upload_dir(const char *path) {
    if (path == NULL) return -1;
    /* Try to create; ignore if exists */
    if (mkdir(path, 0700) != 0) {
        if (errno != EEXIST) {
            return -1;
        }
    }
    int dfd = open(path, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (dfd < 0) {
        return -1;
    }
    struct stat st;
    if (fstat(dfd, &st) != 0) {
        close(dfd);
        return -1;
    }
    if (!S_ISDIR(st.st_mode)) {
        close(dfd);
        errno = ENOTDIR;
        return -1;
    }
    return dfd;
}

/* Create a unique temporary file in dirfd */
static int create_temp_file(int dirfd, char *tmpname_out, size_t tmpname_cap) {
    char rnd[17];
    if (s_random_hex(rnd, sizeof(rnd), 8) != 0) return -1;
    int attempts = 10;
    while (attempts-- > 0) {
        int n = snprintf(tmpname_out, tmpname_cap, ".tmp-%s", rnd);
        if (n <= 0 || (size_t)n >= tmpname_cap) return -1;
        int fd = openat(dirfd, tmpname_out, O_CREAT | O_EXCL | O_WRONLY | O_CLOEXEC | O_NOFOLLOW, 0600);
        if (fd >= 0) {
            return fd;
        }
        if (errno != EEXIST) return -1;
        /* regenerate suffix */
        if (s_random_hex(rnd, sizeof(rnd), 8) != 0) return -1;
    }
    errno = EEXIST;
    return -1;
}

/* Atomically move temp file to final target without overwrite using linkat, then unlink temp */
static int finalize_temp_to_target(int dirfd, const char *tmpname, const char *finalname) {
    /* finalname must not contain '/' */
    if (strchr(finalname, '/') != NULL) { errno = EINVAL; return -1; }
    /* Create link; fails if final exists */
    if (linkat(dirfd, tmpname, dirfd, finalname, 0) != 0) {
        return -1;
    }
    /* Unlink temp */
    if (unlinkat(dirfd, tmpname, 0) != 0) {
        /* best-effort cleanup; final already created */
        return -1;
    }
    return 0;
}

/* Save upload data to base_dir/filename using atomic safe pattern */
static int save_upload(const char *base_dir, const char *filename, const uint8_t *data, size_t data_len) {
    if (!base_dir || !filename || !data) return -1;
    if (data_len > MAX_UPLOAD_SIZE) { errno = EFBIG; return -1; }
    if (!s_is_allowed_filename(filename)) { errno = EINVAL; return -1; }

    int dirfd = ensure_upload_dir(base_dir);
    if (dirfd < 0) return -1;

    char tmpname[64];
    int tfd = create_temp_file(dirfd, tmpname, sizeof(tmpname));
    if (tfd < 0) {
        close(dirfd);
        return -1;
    }

    int rc = -1;
    do {
        /* Validate file descriptor is a regular file */
        struct stat st;
        if (fstat(tfd, &st) != 0) break;
        if (!S_ISREG(st.st_mode)) { errno = EINVAL; break; }

        /* Write data */
        size_t written = 0;
        while (written < data_len) {
            ssize_t w = write(tfd, data + written, data_len - written);
            if (w < 0) { if (errno == EINTR) continue; goto out; }
            written += (size_t)w;
        }
        if (fsync(tfd) != 0) goto out;
        if (close(tfd) != 0) { tfd = -1; goto out; }
        tfd = -1;

        /* fsync directory before link to be safe (optional, after link is also fine) */
        /* Create final link atomically */
        if (finalize_temp_to_target(dirfd, tmpname, filename) != 0) goto out;

        /* Sync directory to persist new name */
        if (fsync(dirfd) != 0) goto out;

        rc = 0;
    } while (0);

out:
    if (tfd >= 0) {
        /* ensure temp is removed */
        close(tfd);
        unlinkat(dirfd, tmpname, 0);
    }
    close(dirfd);
    return rc;
}

/* Extract filename from Content-Disposition value; expects quoted filename; returns 0 on success */
static int parse_cd_filename(const char *val, size_t vlen, char *out, size_t outcap) {
    /* We accept: form-data; name="file"; filename="name.ext" (order may vary) */
    /* Search for filename="..." */
    const char *p = val;
    const char *end = val + vlen;
    int found = 0;
    while (p < end) {
        while (p < end && (*p == ';' || *p == ' ' || *p == '\t')) p++;
        const char *k = p;
        while (p < end && *p != '=' && *p != ';') p++;
        size_t klen = (size_t)(p - k);
        while (p < end && *p != '=') {
            if (*p == ';') break;
            p++;
        }
        if (p >= end || *p != '=') {
            while (p < end && *p != ';') p++;
            continue;
        }
        p++; /* skip '=' */
        while (p < end && (*p == ' ' || *p == '\t')) p++;
        if (p >= end) break;
        char valbuf[256];
        size_t vout = 0;
        if (*p == '"') {
            p++;
            const char *q = p;
            while (q < end && *q != '"') q++;
            if (q >= end) break;
            size_t len = (size_t)(q - p);
            if (len >= sizeof(valbuf)) len = sizeof(valbuf) - 1;
            memcpy(valbuf, p, len); valbuf[len] = '\0';
            p = q + 1;
        } else {
            const char *q = p;
            while (q < end && *q != ';') q++;
            size_t len = (size_t)(q - p);
            if (len >= sizeof(valbuf)) len = sizeof(valbuf) - 1;
            memcpy(valbuf, p, len); valbuf[len] = '\0';
            p = q;
        }
        /* Trim key */
        char key[64];
        s_trim_to_buf(k, klen, key, sizeof(key));
        if (s_strncaseeq(key, s_strnlen(key, sizeof(key)), "filename")) {
            /* Copy to out */
            size_t n = s_strnlen(valbuf, sizeof(valbuf));
            if (n >= outcap) n = outcap - 1;
            memcpy(out, valbuf, n);
            out[n] = '\0';
            found = 1;
        }
        while (p < end && *p != ';') p++;
        if (p < end && *p == ';') p++;
    }
    if (!found) return -1;
    return 0;
}

/* Multipart parser: extracts first file part named via Content-Disposition filename; validates size and filename */
static int parse_multipart(const uint8_t *body, size_t blen, const char *boundary, upload_part_t *out_part) {
    if (!body || !boundary || !out_part) return -1;
    size_t bl = s_strnlen(boundary, MAX_BOUNDARY_LEN + 1);
    if (bl == 0 || bl > MAX_BOUNDARY_LEN) return -1;
    if (!s_is_allowed_boundary(boundary, bl)) return -1;

    /* Initial boundary must be at the start */
    char initb[2 + MAX_BOUNDARY_LEN + 4];
    int ninit = snprintf(initb, sizeof(initb), "--%s\r\n", boundary);
    if (ninit <= 0) return -1;
    if (blen < (size_t)ninit || memcmp(body, initb, (size_t)ninit) != 0) {
        return -1;
    }

    const uint8_t *p = body + ninit;
    const uint8_t *end = body + blen;

    /* Parse part headers until blank line */
    const uint8_t *headers_end = NULL;
    const uint8_t *scan = p;
    size_t header_bytes = 0;
    while (scan + 2 <= end) {
        if (*scan == '\r' && *(scan + 1) == '\n') {
            size_t line_len = (size_t)(scan - p);
            header_bytes += (line_len + 2);
            if (header_bytes > MAX_PART_HEADERS) return -1;
            if (scan + 4 <= end && memcmp(scan, "\r\n\r\n", 4) == 0) {
                headers_end = scan + 4;
                break;
            }
            scan += 2;
            continue;
        }
        scan++;
    }
    if (headers_end == NULL) return -1;

    /* Process headers line by line */
    const uint8_t *line = p;
    char filename[MAX_FILENAME_LEN + 1] = {0};
    while (line < headers_end - 2) {
        const uint8_t *eol = s_memmem(line, (size_t)(headers_end - 2 - line), (const uint8_t *)"\r\n", 2);
        if (!eol) break;
        const uint8_t *colon = memchr(line, ':', (size_t)(eol - line));
        if (colon) {
            size_t klen = (size_t)(colon - line);
            char key[64];
            s_trim_to_buf((const char *)line, klen, key, sizeof(key));
            const uint8_t *valstart = colon + 1;
            while (valstart < eol && (*valstart == ' ' || *valstart == '\t')) valstart++;
            size_t vlen = (size_t)(eol - valstart);
            if (s_strncaseeq(key, s_strnlen(key, sizeof(key)), "Content-Disposition")) {
                /* parse filename */
                char valbuf[512];
                size_t copylen = vlen < sizeof(valbuf) - 1 ? vlen : sizeof(valbuf) - 1;
                memcpy(valbuf, valstart, copylen);
                valbuf[copylen] = '\0';
                if (parse_cd_filename(valbuf, copylen, filename, sizeof(filename)) != 0) {
                    return -1;
                }
            }
        }
        line = eol + 2;
    }
    if (!s_is_allowed_filename(filename)) return -1;

    /* Find end of part: CRLF--boundary or --boundary (if no CRLF at end of data) */
    char midb[4 + MAX_BOUNDARY_LEN];
    int nmid = snprintf(midb, sizeof(midb), "\r\n--%s", boundary);
    if (nmid <= 0) return -1;

    const uint8_t *content_start = headers_end;
    const uint8_t *marker = s_memmem(content_start, (size_t)(end - content_start), (const uint8_t *)midb, (size_t)nmid);
    if (!marker) {
        /* Some generators may not include CRLF before boundary if content empty; try with "--boundary" directly at end */
        char endb[2 + MAX_BOUNDARY_LEN];
        int nendb = snprintf(endb, sizeof(endb), "--%s", boundary);
        if (nendb <= 0) return -1;
        marker = s_memmem(content_start, (size_t)(end - content_start), (const uint8_t *)endb, (size_t)nendb);
        if (!marker) return -1;
    }

    /* Exclude trailing CRLF before boundary if present */
    const uint8_t *content_end = marker;
    if (content_end >= content_start + 2 && memcmp(content_end - 2, "\r\n", 2) == 0) {
        content_end -= 2;
    }

    size_t file_len = (size_t)(content_end - content_start);
    if (file_len > MAX_UPLOAD_SIZE) return -1;

    /* Populate out_part */
    memset(out_part, 0, sizeof(*out_part));
    memcpy(out_part->filename, filename, s_strnlen(filename, sizeof(out_part->filename)));
    out_part->file_data = content_start;
    out_part->file_len = file_len;
    return 0;
}

/* Parse HTTP request, extract upload, save, and compose response */
static int handle_http_upload_request(const uint8_t *req, size_t req_len, const char *base_dir, char *resp_buf, size_t resp_cap) {
    if (!req || !resp_buf) return -1;
    if (req_len == 0 || req_len > MAX_REQUEST_SIZE) {
        int n = snprintf(resp_buf, resp_cap, "HTTP/1.1 413 Payload Too Large\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
        return (n > 0 && (size_t)n < resp_cap) ? n : -1;
    }
    /* Find header end */
    const uint8_t *hdr_end = s_memmem(req, req_len, (const uint8_t *)"\r\n\r\n", 4);
    if (!hdr_end) {
        int n = snprintf(resp_buf, resp_cap, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
        return (n > 0 && (size_t)n < resp_cap) ? n : -1;
    }
    size_t headers_len = (size_t)(hdr_end - req) + 4;
    if (headers_len > MAX_HEADER_SECTION) {
        int n = snprintf(resp_buf, resp_cap, "HTTP/1.1 431 Request Header Fields Too Large\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
        return (n > 0 && (size_t)n < resp_cap) ? n : -1;
    }

    /* Parse request line */
    const uint8_t *line_end = s_memmem(req, headers_len, (const uint8_t *)"\r\n", 2);
    if (!line_end) {
        int n = snprintf(resp_buf, resp_cap, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
        return (n > 0 && (size_t)n < resp_cap) ? n : -1;
    }
    const uint8_t *sp1 = memchr(req, ' ', (size_t)(line_end - req));
    if (!sp1) { int n = snprintf(resp_buf, resp_cap, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"); return (n > 0 && (size_t)n < resp_cap) ? n : -1; }
    const uint8_t *sp2 = memchr(sp1 + 1, ' ', (size_t)(line_end - sp1 - 1));
    if (!sp2) { int n = snprintf(resp_buf, resp_cap, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"); return (n > 0 && (size_t)n < resp_cap) ? n : -1; }
    slice_t method = { req, (size_t)(sp1 - req) };
    slice_t target = { sp1 + 1, (size_t)(sp2 - (sp1 + 1)) };
    /* Only support POST /upload */
    if (!(method.len == 4 && memcmp(method.data, "POST", 4) == 0) ||
        !(target.len == 7 && memcmp(target.data, "/upload", 7) == 0)) {
        int n = snprintf(resp_buf, resp_cap, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
        return (n > 0 && (size_t)n < resp_cap) ? n : -1;
    }

    /* Parse headers for Content-Type and Content-Length */
    const uint8_t *hcur = line_end + 2;
    const uint8_t *hend = req + headers_len - 2;
    char content_type[256] = {0};
    char boundary[MAX_BOUNDARY_LEN + 1] = {0};
    size_t content_length = 0;
    int got_ct = 0, got_cl = 0;

    while (hcur < hend) {
        const uint8_t *eol = s_memmem(hcur, (size_t)(hend - hcur), (const uint8_t *)"\r\n", 2);
        if (!eol) break;
        const uint8_t *colon = memchr(hcur, ':', (size_t)(eol - hcur));
        if (colon) {
            size_t klen = (size_t)(colon - hcur);
            char key[64];
            s_trim_to_buf((const char *)hcur, klen, key, sizeof(key));
            const uint8_t *valstart = colon + 1;
            while (valstart < eol && (*valstart == ' ' || *valstart == '\t')) valstart++;
            size_t vlen = (size_t)(eol - valstart);
            if (s_strncaseeq(key, s_strnlen(key, sizeof(key)), "Content-Type")) {
                size_t ncpy = vlen < sizeof(content_type) - 1 ? vlen : sizeof(content_type) - 1;
                memcpy(content_type, valstart, ncpy);
                content_type[ncpy] = '\0';
                got_ct = 1;
                /* Extract boundary parameter */
                const char *bptr = strstr(content_type, "boundary=");
                if (bptr) {
                    bptr += 9;
                    char bval[MAX_BOUNDARY_LEN + 4];
                    size_t bl = 0;
                    if (*bptr == '"') {
                        bptr++;
                        const char *q = strchr(bptr, '"');
                        if (q) bl = (size_t)(q - bptr);
                    } else {
                        const char *q = strpbrk(bptr, " ;\t\r\n");
                        bl = (size_t)((q ? q : content_type + s_strnlen(content_type, sizeof(content_type))) - bptr);
                    }
                    if (bl > MAX_BOUNDARY_LEN) bl = MAX_BOUNDARY_LEN;
                    memcpy(boundary, bptr, bl);
                    boundary[bl] = '\0';
                }
            } else if (s_strncaseeq(key, s_strnlen(key, sizeof(key)), "Content-Length")) {
                size_t v = 0;
                if (s_parse_size((const char *)valstart, vlen, &v) == 0) {
                    content_length = v;
                    got_cl = 1;
                }
            }
        }
        hcur = eol + 2;
    }

    if (!got_ct || !got_cl) {
        int n = snprintf(resp_buf, resp_cap, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
        return (n > 0 && (size_t)n < resp_cap) ? n : -1;
    }
    if (content_length > MAX_REQUEST_SIZE) {
        int n = snprintf(resp_buf, resp_cap, "HTTP/1.1 413 Payload Too Large\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
        return (n > 0 && (size_t)n < resp_cap) ? n : -1;
    }
    /* Enforce multipart/form-data */
    if (strncasecmp(content_type, "multipart/form-data", 19) != 0 || boundary[0] == '\0' || !s_is_allowed_boundary(boundary, s_strnlen(boundary, sizeof(boundary)))) {
        int n = snprintf(resp_buf, resp_cap, "HTTP/1.1 415 Unsupported Media Type\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
        return (n > 0 && (size_t)n < resp_cap) ? n : -1;
    }

    const uint8_t *body = req + headers_len;
    size_t body_len = req_len - headers_len;
    if (body_len != content_length) {
        int n = snprintf(resp_buf, resp_cap, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
        return (n > 0 && (size_t)n < resp_cap) ? n : -1;
    }

    upload_part_t part;
    if (parse_multipart(body, body_len, boundary, &part) != 0) {
        int n = snprintf(resp_buf, resp_cap, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
        return (n > 0 && (size_t)n < resp_cap) ? n : -1;
    }

    /* Save file */
    if (save_upload(base_dir, part.filename, part.file_data, part.file_len) != 0) {
        /* Do not leak internal errors */
        int n = snprintf(resp_buf, resp_cap, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
        return (n > 0 && (size_t)n < resp_cap) ? n : -1;
    }

    const char body_ok[] = "{\"status\":\"ok\"}";
    int n = snprintf(resp_buf, resp_cap,
                     "HTTP/1.1 201 Created\r\nContent-Type: application/json\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n%s",
                     (size_t)strlen(body_ok), body_ok);
    return (n > 0 && (size_t)n < resp_cap) ? n : -1;
}

/* Build a minimal multipart HTTP request in memory for tests */
static int build_multipart_request(const char *method, const char *target, const char *boundary, const char *filename, const uint8_t *file_data, size_t file_len, uint8_t **out_req, size_t *out_len) {
    if (!method || !target || !boundary || !filename || !file_data || !out_req || !out_len) return -1;
    if (!s_is_allowed_boundary(boundary, s_strnlen(boundary, MAX_BOUNDARY_LEN+1))) return -1;

    char header[1024];
    char disp[256];
    int nd = snprintf(disp, sizeof(disp), "form-data; name=\"file\"; filename=\"%s\"", filename);
    if (nd <= 0 || (size_t)nd >= sizeof(disp)) return -1;

    /* Construct body into a dynamic buffer */
    size_t est = 512 + file_len + s_strnlen(boundary, MAX_BOUNDARY_LEN+1) * 3;
    uint8_t *buf = (uint8_t *)calloc(1, est);
    if (!buf) return -1;
    size_t off = 0;

    int m = snprintf((char *)buf + off, est - off, "--%s\r\n", boundary);
    if (m <= 0) { free(buf); return -1; } off += (size_t)m;
    m = snprintf((char *)buf + off, est - off, "Content-Disposition: %s\r\n", disp);
    if (m <= 0) { free(buf); return -1; } off += (size_t)m;
    m = snprintf((char *)buf + off, est - off, "Content-Type: application/octet-stream\r\n\r\n");
    if (m <= 0) { free(buf); return -1; } off += (size_t)m;

    if (off + file_len + 64 > est) {
        size_t newcap = off + file_len + 128;
        uint8_t *nb = (uint8_t *)realloc(buf, newcap);
        if (!nb) { free(buf); return -1; }
        buf = nb; est = newcap;
    }
    memcpy(buf + off, file_data, file_len);
    off += file_len;

    m = snprintf((char *)buf + off, est - off, "\r\n--%s--\r\n", boundary);
    if (m <= 0) { free(buf); return -1; } off += (size_t)m;

    int h = snprintf(header, sizeof(header),
                     "%s %s HTTP/1.1\r\nHost: localhost\r\nContent-Type: multipart/form-data; boundary=%s\r\nContent-Length: %zu\r\n\r\n",
                     method, target, boundary, off);
    if (h <= 0 || (size_t)h >= sizeof(header)) { free(buf); return -1; }

    size_t total = (size_t)h + off;
    uint8_t *req = (uint8_t *)malloc(total);
    if (!req) { free(buf); return -1; }
    memcpy(req, header, (size_t)h);
    memcpy(req + h, buf, off);
    free(buf);

    *out_req = req;
    *out_len = total;
    return 0;
}

/* Build a non-multipart request for negative tests */
static int build_plain_request(const char *method, const char *target, const char *content_type, const uint8_t *data, size_t data_len, uint8_t **out_req, size_t *out_len) {
    if (!method || !target || !content_type || !data || !out_req || !out_len) return -1;
    char header[1024];
    int h = snprintf(header, sizeof(header),
                     "%s %s HTTP/1.1\r\nHost: localhost\r\nContent-Type: %s\r\nContent-Length: %zu\r\n\r\n",
                     method, target, content_type, data_len);
    if (h <= 0 || (size_t)h >= sizeof(header)) return -1;
    size_t total = (size_t)h + data_len;
    uint8_t *req = (uint8_t *)malloc(total);
    if (!req) return -1;
    memcpy(req, header, (size_t)h);
    memcpy(req + h, data, data_len);
    *out_req = req;
    *out_len = total;
    return 0;
}

/* Extract status code from response for tests */
static int parse_status_code(const char *resp) {
    if (!resp || strncmp(resp, "HTTP/1.1 ", 9) != 0) return -1;
    const char *p = resp + 9;
    if (!isdigit((unsigned char)p[0]) || !isdigit((unsigned char)p[1]) || !isdigit((unsigned char)p[2])) return -1;
    return (p[0]-'0')*100 + (p[1]-'0')*10 + (p[2]-'0');
}

/* Main with 5 test cases */
int main(void) {
    /* Test 1: Valid upload small file */
    {
        const char *boundary = "testboundary123";
        const char *filename = "hello.txt";
        const uint8_t file_data[] = "Hello, world!\n";
        uint8_t *req = NULL; size_t rlen = 0;
        if (build_multipart_request("POST", "/upload", boundary, filename, file_data, sizeof(file_data)-1, &req, &rlen) != 0) {
            printf("Test1: build failed\n");
            return 1;
        }
        char resp[RESP_BUF_CAP];
        int n = handle_http_upload_request(req, rlen, DEFAULT_UPLOAD_DIR, resp, sizeof(resp));
        int code = n > 0 ? parse_status_code(resp) : -1;
        printf("Test1 status: %d\n", code);
        free(req);
    }

    /* Test 2: Invalid content type (not multipart) */
    {
        const uint8_t body[] = "plain body";
        uint8_t *req = NULL; size_t rlen = 0;
        if (build_plain_request("POST", "/upload", "text/plain", body, sizeof(body)-1, &req, &rlen) != 0) {
            printf("Test2: build failed\n");
            return 1;
        }
        char resp[RESP_BUF_CAP];
        int n = handle_http_upload_request(req, rlen, DEFAULT_UPLOAD_DIR, resp, sizeof(resp));
        int code = n > 0 ? parse_status_code(resp) : -1;
        printf("Test2 status: %d\n", code);
        free(req);
    }

    /* Test 3: Disallowed filename (path traversal attempt) */
    {
        const char *boundary = "bnd123";
        const char *filename = "..bad.txt";
        const uint8_t file_data[] = "DATA";
        uint8_t *req = NULL; size_t rlen = 0;
        if (build_multipart_request("POST", "/upload", boundary, filename, file_data, sizeof(file_data)-1, &req, &rlen) != 0) {
            printf("Test3: build failed\n");
            return 1;
        }
        char resp[RESP_BUF_CAP];
        int n = handle_http_upload_request(req, rlen, DEFAULT_UPLOAD_DIR, resp, sizeof(resp));
        int code = n > 0 ? parse_status_code(resp) : -1;
        printf("Test3 status: %d\n", code);
        free(req);
    }

    /* Test 4: Oversized payload (exceed MAX_UPLOAD_SIZE) */
    {
        const char *boundary = "bigbnd";
        const char *filename = "big.bin";
        size_t big = MAX_UPLOAD_SIZE + 1;
        uint8_t *data = (uint8_t *)malloc(big);
        if (!data) { printf("Test4: alloc failed\n"); return 1; }
        memset(data, 'A', big);
        uint8_t *req = NULL; size_t rlen = 0;
        if (build_multipart_request("POST", "/upload", boundary, filename, data, big, &req, &rlen) != 0) {
            printf("Test4: build failed\n");
            free(data);
            return 1;
        }
        free(data);
        char resp[RESP_BUF_CAP];
        int n = handle_http_upload_request(req, rlen, DEFAULT_UPLOAD_DIR, resp, sizeof(resp));
        int code = n > 0 ? parse_status_code(resp) : -1;
        printf("Test4 status: %d\n", code);
        free(req);
    }

    /* Test 5: Wrong target path */
    {
        const char *boundary = "okbnd";
        const char *filename = "ok.txt";
        const uint8_t file_data[] = "OK";
        uint8_t *req = NULL; size_t rlen = 0;
        if (build_multipart_request("POST", "/wrong", boundary, filename, file_data, sizeof(file_data)-1, &req, &rlen) != 0) {
            printf("Test5: build failed\n");
            return 1;
        }
        char resp[RESP_BUF_CAP];
        int n = handle_http_upload_request(req, rlen, DEFAULT_UPLOAD_DIR, resp, sizeof(resp));
        int code = n > 0 ? parse_status_code(resp) : -1;
        printf("Test5 status: %d\n", code);
        free(req);
    }

    printf("Done.\n");
    return 0;
}