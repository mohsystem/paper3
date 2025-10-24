/*
Secure file upload HTTP server in C with built-in test client.
- POSIX-only. Uses safe openat, O_NOFOLLOW, O_EXCL, fsync, and atomic rename.
- Anchors writes to a dedicated "uploads" directory with path traversal defenses.
- Accepts POST /upload with header "X-File-Name" (optional) and body as raw bytes.
- Limits Content-Length and header sizes to prevent abuse.
- Includes 5 test cases demonstrating normal and adversarial uploads.

Build: cc -pthread -Wall -Wextra -Werror -O2 upload_server.c -o upload_server
Run: ./upload_server
*/

#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <strings.h>
#include <sys/random.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#define MAX_HEADER_BYTES 8192
#define MAX_FILENAME_LEN 64
#define MAX_BODY_BYTES (1024 * 1024) /* 1 MiB */
#define LISTEN_BACKLOG 10
#define TEST_UPLOADS 5

typedef struct {
    int listen_fd;
    uint16_t port;
    int dirfd;
    pthread_mutex_t mtx;
    pthread_cond_t cv;
    bool ready;
} server_state_t;

static void ignore_sigpipe(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, NULL);
}

static void portable_explicit_bzero(void *p, size_t n) {
#if defined(__GLIBC__) && defined(__GLIBC_PREREQ)
#if __GLIBC_PREREQ(2, 25)
    explicit_bzero(p, n);
    return;
#endif
#endif
    volatile unsigned char *vp = (volatile unsigned char *)p;
    while (n--) *vp++ = 0;
}

/* Generate cryptographically-strong random bytes */
static int gen_random(unsigned char *out, size_t len) {
    size_t off = 0;
    while (off < len) {
        ssize_t r = getrandom(out + off, len - off, 0);
        if (r < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        off += (size_t)r;
    }
    return 0;
}

static void hex_encode(const unsigned char *in, size_t inlen, char *out, size_t outlen) {
    static const char *hex = "0123456789abcdef";
    size_t j = 0;
    for (size_t i = 0; i < inlen && j + 2 < outlen; i++) {
        out[j++] = hex[(in[i] >> 4) & 0xF];
        out[j++] = hex[in[i] & 0xF];
    }
    if (j < outlen) out[j] = '\0';
}

/* Sanitize user-supplied filename into a safe suffix. */
static void sanitize_filename(const char *in, char *out, size_t outsz) {
    if (!out || outsz == 0) return;
    size_t j = 0;
    if (!in) in = "";
    for (size_t i = 0; in[i] != '\0' && j + 1 < outsz; i++) {
        unsigned char c = (unsigned char)in[i];
        if (c == '/' || c == '\\') {
            out[j++] = '_';
            continue;
        }
        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '.' || c == '-' || c == '_' ) {
            out[j++] = (char)c;
        } else {
            out[j++] = '_';
        }
    }
    if (j == 0) {
        if (outsz >= 5) {
            strncpy(out, "file", outsz - 1);
            out[outsz - 1] = '\0';
        } else {
            out[0] = 'f';
            out[1] = '\0';
        }
    } else {
        out[j] = '\0';
    }
}

/* Ensure base directory exists and open it securely */
static int open_base_dir(const char *path) {
    if (!path) return -1;
    if (mkdir(path, 0700) != 0 && errno != EEXIST) {
        return -1;
    }
    int dfd = open(path, O_RDONLY | O_DIRECTORY | O_CLOEXEC
#ifdef O_NOFOLLOW
        | O_NOFOLLOW
#endif
    );
    if (dfd < 0) return -1;
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

static int fsync_dir(int dirfd) {
    if (dirfd < 0) return -1;
    return fsync(dirfd);
}

/* Create a secure temporary file within dirfd, returning its fd and name */
static int create_temp_file(int dirfd, char *tmp_name, size_t tmp_name_sz) {
    unsigned char rnd[16];
    if (gen_random(rnd, sizeof(rnd)) != 0) return -1;
    char hex[33] = {0};
    hex_encode(rnd, sizeof(rnd), hex, sizeof(hex));
    int n = snprintf(tmp_name, tmp_name_sz, ".tmp.%s", hex);
    if (n <= 0 || (size_t)n >= tmp_name_sz) return -1;

    int fd = openat(dirfd, tmp_name, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC
#ifdef O_NOFOLLOW
        | O_NOFOLLOW
#endif
        , 0600);
    if (fd < 0) return -1;

    struct stat st;
    if (fstat(fd, &st) != 0) {
        close(fd);
        unlinkat(dirfd, tmp_name, 0);
        return -1;
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        unlinkat(dirfd, tmp_name, 0);
        errno = EPERM;
        return -1;
    }
    return fd;
}

/* Atomically move tmp_name to final_name within dirfd after syncing file and dir. */
static int finalize_file(int dirfd, const char *tmp_name, const char *final_name) {
    if (renameat(dirfd, tmp_name, dirfd, final_name) != 0) {
        return -1;
    }
    if (fsync_dir(dirfd) != 0) {
        return -1;
    }
    return 0;
}

static int set_sock_timeouts(int fd, int ms) {
    struct timeval tv;
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) != 0) return -1;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) != 0) return -1;
#ifdef SO_NOSIGPIPE
    int on = 1;
    setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &on, sizeof(on));
#endif
    return 0;
}

static ssize_t send_all(int fd, const void *buf, size_t len) {
    const unsigned char *p = (const unsigned char *)buf;
    size_t off = 0;
    while (off < len) {
        ssize_t w = send(fd, p + off, len - off, MSG_NOSIGNAL);
        if (w < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (w == 0) return -1;
        off += (size_t)w;
    }
    return (ssize_t)off;
}

static int parse_request_line(const char *line, char *method, size_t msz, char *path, size_t psz) {
    // Expect "POST /upload HTTP/1.1"
    const char *sp1 = strchr(line, ' ');
    if (!sp1) return -1;
    size_t mlen = (size_t)(sp1 - line);
    if (mlen + 1 > msz) return -1;
    memcpy(method, line, mlen);
    method[mlen] = '\0';

    const char *sp2 = strchr(sp1 + 1, ' ');
    if (!sp2) return -1;
    size_t plen = (size_t)(sp2 - (sp1 + 1));
    if (plen + 1 > psz) return -1;
    memcpy(path, sp1 + 1, plen);
    path[plen] = '\0';

    return 0;
}

static long parse_content_length(const char *val) {
    if (!val) return -1;
    long n = 0;
    for (const char *p = val; *p; ++p) {
        if (*p < '0' || *p > '9') return -1;
        long prev = n;
        n = n * 10 + (*p - '0');
        if (n < prev) return -1; // overflow
        if (n > (long)MAX_BODY_BYTES) return -1;
    }
    return n;
}

static void http_send_error(int cfd, int code, const char *msg) {
    char body[256];
    int bl = snprintf(body, sizeof(body), "{\"error\":\"%s\"}\n", msg ? msg : "error");
    if (bl < 0) bl = 0;
    char hdr[512];
    int hl = snprintf(hdr, sizeof(hdr),
                      "HTTP/1.1 %d Error\r\n"
                      "Content-Type: application/json; charset=utf-8\r\n"
                      "Content-Length: %d\r\n"
                      "Connection: close\r\n"
                      "\r\n",
                      code, bl);
    if (hl > 0) {
        send_all(cfd, hdr, (size_t)hl);
        send_all(cfd, body, (size_t)bl);
    }
}

static void http_send_ok(int cfd, const char *stored_as, size_t size) {
    char body[512];
    int bl = snprintf(body, sizeof(body),
                      "{\"status\":\"ok\",\"stored_as\":\"%s\",\"size\":%zu}\n",
                      stored_as ? stored_as : "", size);
    if (bl < 0) bl = 0;
    char hdr[512];
    int hl = snprintf(hdr, sizeof(hdr),
                      "HTTP/1.1 201 Created\r\n"
                      "Content-Type: application/json; charset=utf-8\r\n"
                      "Content-Length: %d\r\n"
                      "Connection: close\r\n"
                      "\r\n",
                      bl);
    if (hl > 0) {
        send_all(cfd, hdr, (size_t)hl);
        send_all(cfd, body, (size_t)bl);
    }
}

static int read_headers(int cfd, char *buf, size_t bufsz, size_t *out_header_len, size_t *out_body_avail) {
    size_t off = 0;
    while (off + 4 < bufsz) {
        ssize_t r = recv(cfd, buf + off, bufsz - off, 0);
        if (r < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (r == 0) break;
        off += (size_t)r;
        // look for \r
\r

        for (size_t i = 0; i + 3 < off; i++) {
            if (buf[i] == '\r' && buf[i+1] == '\n' && buf[i+2] == '\r' && buf[i+3] == '\n') {
                *out_header_len = i + 4;
                *out_body_avail = off - *out_header_len;
                return 0;
            }
        }
        if (off >= MAX_HEADER_BYTES) break;
    }
    return -1;
}

static int handle_connection(int cfd, int dirfd) {
    char headerbuf[MAX_HEADER_BYTES + 4];
    size_t header_len = 0, body_avail = 0;
    if (read_headers(cfd, headerbuf, sizeof(headerbuf), &header_len, &body_avail) != 0) {
        http_send_error(cfd, 400, "bad_request");
        return -1;
    }
    headerbuf[header_len] = '\0';

    // Parse start line
    char *line = headerbuf;
    char *line_end = strstr(line, "\r\n");
    if (!line_end) { http_send_error(cfd, 400, "bad_request"); return -1; }
    *line_end = '\0';
    char method[16], path[128];
    if (parse_request_line(line, method, sizeof(method), path, sizeof(path)) != 0) {
        http_send_error(cfd, 400, "bad_request");
        return -1;
    }
    if (strcasecmp(method, "POST") != 0 || strcmp(path, "/upload") != 0) {
        http_send_error(cfd, 404, "not_found");
        return -1;
    }

    // Headers
    char *p = line_end + 2;
    long content_len = -1;
    char filename_hdr[MAX_FILENAME_LEN + 1];
    filename_hdr[0] = '\0';

    while (p < headerbuf + header_len) {
        char *eol = strstr(p, "\r\n");
        if (!eol) break;
        if (eol == p) {
            break; // end of headers
        }
        *eol = '\0';
        char *colon = strchr(p, ':');
        if (colon) {
            *colon = '\0';
            char *name = p;
            char *val = colon + 1;
            while (*val == ' ' || *val == '\t') val++;
            if (strcasecmp(name, "Content-Length") == 0) {
                long cl = parse_content_length(val);
                if (cl < 0 || cl > (long)MAX_BODY_BYTES) {
                    http_send_error(cfd, 413, "payload_too_large");
                    return -1;
                }
                content_len = cl;
            } else if (strcasecmp(name, "X-File-Name") == 0) {
                sanitize_filename(val, filename_hdr, sizeof(filename_hdr));
            }
        }
        p = eol + 2;
    }

    if (content_len < 0 || content_len > (long)MAX_BODY_BYTES) {
        http_send_error(cfd, 411, "length_required");
        return -1;
    }

    // Create temp file
    char tmpname[64];
    int tmpfd = create_temp_file(dirfd, tmpname, sizeof(tmpname));
    if (tmpfd < 0) {
        http_send_error(cfd, 500, "store_failed");
        return -1;
    }

    // Generate final name
    unsigned char rnd[16];
    if (gen_random(rnd, sizeof(rnd)) != 0) {
        close(tmpfd);
        unlinkat(dirfd, tmpname, 0);
        http_send_error(cfd, 500, "rand_failed");
        return -1;
    }
    char randhex[33] = {0};
    hex_encode(rnd, sizeof(rnd), randhex, sizeof(randhex));

    char suffix[MAX_FILENAME_LEN + 1];
    sanitize_filename(filename_hdr[0] ? filename_hdr : "upload.bin", suffix, sizeof(suffix));

    char finalname[128];
    int fnl = snprintf(finalname, sizeof(finalname), "%s_%s", randhex, suffix);
    if (fnl <= 0 || (size_t)fnl >= sizeof(finalname)) {
        close(tmpfd);
        unlinkat(dirfd, tmpname, 0);
        http_send_error(cfd, 500, "name_failed");
        return -1;
    }

    // Write body
    size_t to_read = (size_t)content_len;
    size_t written_total = 0;

    // First write any bytes already read beyond headers
    if (body_avail > 0) {
        size_t chunk = body_avail;
        if (chunk > to_read) chunk = to_read;
        ssize_t w = write(tmpfd, headerbuf + header_len, chunk);
        if (w < 0 || (size_t)w != chunk) {
            close(tmpfd);
            unlinkat(dirfd, tmpname, 0);
            http_send_error(cfd, 500, "write_failed");
            return -1;
        }
        to_read -= chunk;
        written_total += (size_t)w;
    }

    // Then read remaining from socket
    char buf[8192];
    while (to_read > 0) {
        size_t chunk = to_read;
        if (chunk > sizeof(buf)) chunk = sizeof(buf);
        ssize_t r = recv(cfd, buf, chunk, 0);
        if (r < 0) {
            if (errno == EINTR) continue;
            close(tmpfd);
            unlinkat(dirfd, tmpname, 0);
            http_send_error(cfd, 400, "recv_failed");
            return -1;
        }
        if (r == 0) {
            close(tmpfd);
            unlinkat(dirfd, tmpname, 0);
            http_send_error(cfd, 400, "incomplete_body");
            return -1;
        }
        ssize_t w = write(tmpfd, buf, (size_t)r);
        if (w < 0 || w != r) {
            close(tmpfd);
            unlinkat(dirfd, tmpname, 0);
            http_send_error(cfd, 500, "write_failed");
            return -1;
        }
        to_read -= (size_t)r;
        written_total += (size_t)r;
    }

    if (fsync(tmpfd) != 0) {
        close(tmpfd);
        unlinkat(dirfd, tmpname, 0);
        http_send_error(cfd, 500, "sync_failed");
        return -1;
    }
    close(tmpfd);

    if (finalize_file(dirfd, tmpname, finalname) != 0) {
        unlinkat(dirfd, tmpname, 0);
        http_send_error(cfd, 500, "finalize_failed");
        return -1;
    }

    http_send_ok(cfd, finalname, written_total);
    return 0;
}

static void *server_thread(void *arg) {
    server_state_t *st = (server_state_t *)arg;

    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd < 0) {
        perror("socket");
        return NULL;
    }
    int yes = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(0); // ephemeral

    if (bind(lfd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        perror("bind");
        close(lfd);
        return NULL;
    }
    if (listen(lfd, LISTEN_BACKLOG) != 0) {
        perror("listen");
        close(lfd);
        return NULL;
    }

    struct sockaddr_in got;
    socklen_t gl = sizeof(got);
    if (getsockname(lfd, (struct sockaddr *)&got, &gl) != 0) {
        perror("getsockname");
        close(lfd);
        return NULL;
    }

    pthread_mutex_lock(&st->mtx);
    st->listen_fd = lfd;
    st->port = ntohs(got.sin_port);
    st->ready = true;
    pthread_cond_broadcast(&st->cv);
    pthread_mutex_unlock(&st->mtx);

    for (int i = 0; i < TEST_UPLOADS; i++) {
        struct sockaddr_in cli;
        socklen_t cl = sizeof(cli);
        int cfd = accept(lfd, (struct sockaddr *)&cli, &cl);
        if (cfd < 0) {
            perror("accept");
            continue;
        }
        set_sock_timeouts(cfd, 10000);
        (void)handle_connection(cfd, st->dirfd);
        shutdown(cfd, SHUT_RDWR);
        close(cfd);
    }

    close(lfd);
    return NULL;
}

/* Simple client to upload bytes with optional filename header. */
static int client_upload(const char *host, uint16_t port, const char *fname_hdr, const unsigned char *data, size_t len, char *out_resp, size_t out_resp_sz) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;
#ifdef SO_NOSIGPIPE
    int on = 1;
    setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &on, sizeof(on));
#endif
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, host, &addr.sin_addr) != 1) {
        close(fd);
        return -1;
    }
    addr.sin_port = htons(port);
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        close(fd);
        return -1;
    }

    char header[1024];
    char fname_line[256] = "";
    if (fname_hdr && fname_hdr[0]) {
        char safe[MAX_FILENAME_LEN + 1];
        sanitize_filename(fname_hdr, safe, sizeof(safe));
        (void)snprintf(fname_line, sizeof(fname_line), "X-File-Name: %s\r\n", safe);
    }
    int hl = snprintf(header, sizeof(header),
                      "POST /upload HTTP/1.1\r\n"
                      "Host: %s:%u\r\n"
                      "Content-Length: %zu\r\n"
                      "Content-Type: application/octet-stream\r\n"
                      "%s"
                      "Connection: close\r\n"
                      "\r\n",
                      host, (unsigned)port, len, fname_line);
    if (hl <= 0 || (size_t)hl >= sizeof(header)) {
        close(fd);
        return -1;
    }

    if (send_all(fd, header, (size_t)hl) < 0) { close(fd); return -1; }
    if (len > 0 && send_all(fd, data, len) < 0) { close(fd); return -1; }

    // Read response
    size_t off = 0;
    char buf[4096];
    ssize_t r;
    while ((r = recv(fd, buf + off, sizeof(buf) - off - 1, 0)) > 0) {
        off += (size_t)r;
        if (off >= sizeof(buf) - 1) break;
    }
    if (off >= sizeof(buf)) off = sizeof(buf) - 1;
    buf[off] = '\0';
    if (out_resp && out_resp_sz > 0) {
        size_t copy = off < (out_resp_sz - 1) ? off : (out_resp_sz - 1);
        memcpy(out_resp, buf, copy);
        out_resp[copy] = '\0';
    }
    close(fd);
    return 0;
}

static void print_trimmed(const char *label, const char *resp) {
    if (!resp) return;
    const char *body = strstr(resp, "\r\n\r\n");
    if (body) body += 4; else body = resp;
    printf("%s %.*s\n", label, (int)strcspn(body, "\r\n"), body);
}

/* MAIN with 5 test cases */
int main(void) {
    ignore_sigpipe();

    const char *base_dir = "uploads";
    int dirfd = open_base_dir(base_dir);
    if (dirfd < 0) {
        fprintf(stderr, "Failed to open base dir\n");
        return 1;
    }

    server_state_t st;
    memset(&st, 0, sizeof(st));
    st.dirfd = dirfd;
    pthread_mutex_init(&st.mtx, NULL);
    pthread_cond_init(&st.cv, NULL);

    pthread_t th;
    if (pthread_create(&th, NULL, server_thread, &st) != 0) {
        fprintf(stderr, "Failed to start server\n");
        close(dirfd);
        return 1;
    }

    // Wait until server reports ready and port number
    pthread_mutex_lock(&st.mtx);
    while (!st.ready) pthread_cond_wait(&st.cv, &st.mtx);
    uint16_t port = st.port;
    pthread_mutex_unlock(&st.mtx);

    // 5 test cases
    char resp[4096];

    const unsigned char data1[] = "Hello, secure world!";
    if (client_upload("127.0.0.1", port, "greeting.txt", data1, sizeof(data1) - 1, resp, sizeof(resp)) == 0) {
        print_trimmed("Test1:", resp);
    } else {
        printf("Test1: client error\n");
    }

    const unsigned char data2[] = "this should not traverse dirs";
    if (client_upload("127.0.0.1", port, "../../etc/passwd", data2, sizeof(data2) - 1, resp, sizeof(resp)) == 0) {
        print_trimmed("Test2:", resp);
    } else {
        printf("Test2: client error\n");
    }

    size_t bigsz = 100 * 1024; // 100 KiB
    unsigned char *big = (unsigned char *)malloc(bigsz);
    if (!big) bigsz = 0;
    for (size_t i = 0; i < bigsz; i++) big[i] = (unsigned char)('A' + (i % 26));
    if (client_upload("127.0.0.1", port, "big.bin", big, bigsz, resp, sizeof(resp)) == 0) {
        print_trimmed("Test3:", resp);
    } else {
        printf("Test3: client error\n");
    }
    if (big) { portable_explicit_bzero(big, bigsz); free(big); }

    const unsigned char data4[] = "non-ascii name";
    if (client_upload("127.0.0.1", port, "файл-名?.bin", data4, sizeof(data4) - 1, resp, sizeof(resp)) == 0) {
        print_trimmed("Test4:", resp);
    } else {
        printf("Test4: client error\n");
    }

    const unsigned char data5[] = "no filename header provided";
    if (client_upload("127.0.0.1", port, NULL, data5, sizeof(data5) - 1, resp, sizeof(resp)) == 0) {
        print_trimmed("Test5:", resp);
    } else {
        printf("Test5: client error\n");
    }

    pthread_join(th, NULL);
    close(dirfd);
    return 0;
}