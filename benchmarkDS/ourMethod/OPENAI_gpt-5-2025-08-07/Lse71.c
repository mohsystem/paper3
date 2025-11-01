#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define SERVER_PORT 9091
#define MAX_SIZE (10 * 1024 * 1024)

static bool is_dir(mode_t m) { return S_ISDIR(m); }

static bool ensure_upload_dir(int *dirfd) {
    if (mkdir("uploads", 0700) != 0 && errno != EEXIST) {
        return false;
    }
    int d = open("uploads", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (d < 0) return false;
    struct stat st;
    if (fstat(d, &st) != 0 || !is_dir(st.st_mode)) {
        close(d);
        return false;
    }
    *dirfd = d;
    return true;
}

static bool sanitize_name(const char *name) {
    if (name == NULL) return false;
    size_t n = strlen(name);
    if (n < 1 || n > 100) return false;
    if (name[0] == '.') return false;
    if (strstr(name, "..") != NULL) return false;
    for (size_t i = 0; i < n; i++) {
        char c = name[i];
        if (!((c >= 'a' && c <= 'z') ||
              (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') ||
              c == '.' || c == '_' || c == '-')) {
            return false;
        }
    }
    return true;
}

static void hex32(uint32_t v, char out[9]) {
    static const char *hexd = "0123456789abcdef";
    for (int i = 7; i >= 0; --i) {
        out[i] = hexd[v & 0xF];
        v >>= 4;
    }
    out[8] = '\0';
}

static char *gen_stored_name(const char *baseName) {
    const char *dot = strrchr(baseName, '.');
    char base[128]; base[0] = '\0';
    char ext[16]; ext[0] = '\0';
    if (dot && dot != baseName && dot[1] != '\0') {
        const char *e = dot + 1;
        size_t el = strlen(e);
        bool ok = (el >= 1 && el <= 10);
        for (size_t i = 0; i < el; i++) {
            if (!((e[i] >= 'a' && e[i] <= 'z') ||
                  (e[i] >= 'A' && e[i] <= 'Z') ||
                  (e[i] >= '0' && e[i] <= '9'))) { ok = false; break; }
        }
        if (ok) {
            size_t bl = (size_t)(dot - baseName);
            if (bl > sizeof(base)-1) bl = sizeof(base)-1;
            memcpy(base, baseName, bl);
            base[bl] = '\0';
            snprintf(ext, sizeof(ext), ".%s", e);
        } else {
            strncpy(base, baseName, sizeof(base)-1);
            base[sizeof(base)-1] = '\0';
        }
    } else {
        strncpy(base, baseName, sizeof(base)-1);
        base[sizeof(base)-1] = '\0';
    }
    struct timespec ts; clock_gettime(CLOCK_REALTIME, &ts);
    long long ms = (long long)ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;
    uint32_t r = (uint32_t)rand();
    char hx[9]; hex32(r, hx);
    char *out = (char*)malloc(256);
    if (!out) return NULL;
    snprintf(out, 256, "%s-%lld-%s%s", base, ms, hx, ext);
    return out;
}

static bool send_all(int fd, const char *buf, size_t len) {
    while (len > 0) {
        ssize_t w = write(fd, buf, len);
        if (w <= 0) return false;
        buf += w; len -= (size_t)w;
    }
    return true;
}

static int read_line(int fd, char *buf, size_t max) {
    size_t i = 0;
    char c;
    while (i + 1 < max) {
        ssize_t r = read(fd, &c, 1);
        if (r <= 0) return -1;
        buf[i++] = c;
        if (i >= 2 && buf[i-2] == '\r' && buf[i-1] == '\n') {
            buf[i] = '\0';
            return (int)i;
        }
    }
    return -1;
}

static char *url_decode(const char *s) {
    size_t n = strlen(s);
    char *o = (char*)malloc(n + 1);
    if (!o) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < n; i++) {
        if (s[i] == '%' && i + 2 < n) {
            char a = s[i+1], b = s[i+2];
            int hi = -1, lo = -1;
            if (a >= '0' && a <= '9') hi = a - '0';
            else if (a >= 'a' && a <= 'f') hi = a - 'a' + 10;
            else if (a >= 'A' && a <= 'F') hi = a - 'A' + 10;
            if (b >= '0' && b <= '9') lo = b - '0';
            else if (b >= 'a' && b <= 'f') lo = b - 'a' + 10;
            else if (b >= 'A' && b <= 'F') lo = b - 'A' + 10;
            if (hi >= 0 && lo >= 0) {
                o[j++] = (char)((hi << 4) | lo);
                i += 2;
                continue;
            }
        } else if (s[i] == '+') {
            o[j++] = ' ';
            continue;
        }
        o[j++] = s[i];
    }
    o[j] = '\0';
    return o;
}

static char *get_query_param(const char *path, const char *key) {
    const char *q = strchr(path, '?');
    if (!q) return NULL;
    q++;
    size_t keylen = strlen(key);
    const char *p = q;
    while (*p) {
        const char *amp = strchr(p, '&');
        size_t seglen = amp ? (size_t)(amp - p) : strlen(p);
        const char *eq = memchr(p, '=', seglen);
        if (eq) {
            size_t klen = (size_t)(eq - p);
            if (klen == keylen && strncmp(p, key, keylen) == 0) {
                size_t vlen = seglen - klen - 1;
                char *val = (char*)malloc(vlen + 1);
                if (!val) return NULL;
                memcpy(val, eq + 1, vlen);
                val[vlen] = '\0';
                char *dec = url_decode(val);
                free(val);
                return dec;
            }
        }
        if (!amp) break;
        p = amp + 1;
    }
    return NULL;
}

static void handle_client(int cfd) {
    // Read request headers
    char line[4096];
    int n = read_line(cfd, line, sizeof(line));
    if (n <= 0) { close(cfd); return; }
    // Request line
    char method[16], path[2048], ver[16];
    if (sscanf(line, "%15s %2047s %15s", method, path, ver) != 3) {
        send_all(cfd, "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n", 47);
        close(cfd); return;
    }
    if (strcmp(method, "POST") != 0 || strncmp(path, "/upload", 7) != 0) {
        send_all(cfd, "HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n", 45);
        close(cfd); return;
    }
    // Read headers
    size_t content_length = 0;
    for (;;) {
        int m = read_line(cfd, line, sizeof(line));
        if (m <= 0) { close(cfd); return; }
        if (strcmp(line, "\r\n") == 0) break;
        // Lowercase check for content-length
        char lower[4096];
        size_t L = strlen(line);
        for (size_t i = 0; i < L; i++) lower[i] = (char)tolower((unsigned char)line[i]);
        lower[L] = '\0';
        if (strncmp(lower, "content-length:", 15) == 0) {
            const char *p = line + 15;
            while (*p == ' ' || *p == '\t') p++;
            content_length = (size_t)strtoull(p, NULL, 10);
        }
    }
    if (content_length == 0) {
        send_all(cfd, "HTTP/1.1 411 Length Required\r\nConnection: close\r\n\r\n", 53);
        close(cfd); return;
    }
    if (content_length > MAX_SIZE) {
        send_all(cfd, "HTTP/1.1 413 Payload Too Large\r\nConnection: close\r\n\r\n", 55);
        close(cfd); return;
    }
    char *name = get_query_param(path, "name");
    if (!name || !sanitize_name(name)) {
        if (name) free(name);
        send_all(cfd, "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n", 47);
        close(cfd); return;
    }
    int dirfd;
    if (!ensure_upload_dir(&dirfd)) {
        free(name);
        send_all(cfd, "HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\n\r\n", 59);
        close(cfd); return;
    }
    char *stored = gen_stored_name(name);
    free(name);
    if (!stored) {
        close(dirfd);
        send_all(cfd, "HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\n\r\n", 59);
        close(cfd); return;
    }
    int flags = O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC;
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
    int fd = openat(dirfd, stored, flags, 0600);
    if (fd < 0) {
        if (errno == EEXIST) {
            send_all(cfd, "HTTP/1.1 409 Conflict\r\nConnection: close\r\n\r\n", 45);
        } else if (errno == ELOOP) {
            send_all(cfd, "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n", 47);
        } else {
            send_all(cfd, "HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\n\r\n", 59);
        }
        close(dirfd);
        free(stored);
        close(cfd);
        return;
    }

    size_t remaining = content_length;
    char buf[8192];
    while (remaining > 0) {
        ssize_t r = read(cfd, buf, (remaining > sizeof(buf)) ? sizeof(buf) : remaining);
        if (r <= 0) break;
        size_t off = 0;
        while (off < (size_t)r) {
            ssize_t w = write(fd, buf + off, (size_t)r - off);
            if (w <= 0) { remaining = 1; break; }
            off += (size_t)w;
        }
        if (off < (size_t)r) { remaining = 1; break; }
        remaining -= (size_t)r;
    }
    if (remaining != 0) {
        close(fd);
        unlinkat(dirfd, stored, 0);
        close(dirfd);
        free(stored);
        send_all(cfd, "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n", 47);
        close(cfd);
        return;
    }

    fsync(fd);
    close(fd);
#ifdef __linux__
    fsync(dirfd);
#endif
    close(dirfd);

    char body[512];
    int blen = snprintf(body, sizeof(body), "{\"stored\":\"%s\"}\n", stored);
    free(stored);
    char header[256];
    int hlen = snprintf(header, sizeof(header),
                        "HTTP/1.1 201 Created\r\n"
                        "Content-Type: application/json; charset=utf-8\r\n"
                        "Content-Length: %d\r\n"
                        "Connection: close\r\n\r\n",
                        blen);
    send_all(cfd, header, (size_t)hlen);
    send_all(cfd, body, (size_t)blen);
    close(cfd);
}

static void server_loop(bool *stopFlag) {
    int sfd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (sfd < 0) return;
    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(sfd, (struct sockaddr*)&addr, sizeof(addr)) != 0) { close(sfd); return; }
    if (listen(sfd, 16) != 0) { close(sfd); return; }
    while (!*stopFlag) {
        struct sockaddr_in cli;
        socklen_t cl = sizeof(cli);
        int cfd = accept4(sfd, (struct sockaddr*)&cli, &cl, SOCK_CLOEXEC);
        if (cfd < 0) {
            if (errno == EINTR) continue;
            break;
        }
        handle_client(cfd);
    }
    close(sfd);
}

static int http_post(const char *name, const uint8_t *data, size_t len) {
    int fd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (fd < 0) return -1;
    struct sockaddr_in addr; memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET; addr.sin_port = htons(SERVER_PORT); addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) { close(fd); return -1; }
    // URL-encode name
    char enc[512]; size_t j = 0;
    for (size_t i = 0; name[i] && j + 4 < sizeof(enc); i++) {
        unsigned char c = (unsigned char)name[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c=='.' || c=='_' || c=='-' ) {
            enc[j++] = c;
        } else {
            j += snprintf(enc + j, sizeof(enc) - j, "%%%02X", c);
        }
    }
    enc[j] = '\0';
    char head[1024];
    int hlen = snprintf(head, sizeof(head),
                        "POST /upload?name=%s HTTP/1.1\r\nHost: 127.0.0.1:%d\r\nContent-Type: application/octet-stream\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
                        enc, SERVER_PORT, len);
    if (!send_all(fd, head, (size_t)hlen)) { close(fd); return -1; }
    size_t off = 0;
    while (off < len) {
        ssize_t w = write(fd, data + off, len - off);
        if (w <= 0) break;
        off += (size_t)w;
    }
    // Read status line
    char ch; char line[256]; size_t L = 0;
    while (L + 1 < sizeof(line)) {
        ssize_t r = read(fd, &ch, 1);
        if (r <= 0) break;
        line[L++] = ch;
        if (L >= 2 && line[L-2] == '\r' && line[L-1] == '\n') break;
    }
    line[L] = '\0';
    int code = -1;
    if (strncmp(line, "HTTP/", 5) == 0) {
        char http[16]; int c;
        if (sscanf(line, "%15s %d", http, &c) == 2) code = c;
    }
    // drain
    char buf[512];
    while (read(fd, buf, sizeof(buf)) > 0) {}
    close(fd);
    return code;
}

int main(void) {
    srand((unsigned)time(NULL));
    bool stop = false;
    pid_t pid = fork();
    if (pid < 0) return 1;
    if (pid == 0) {
        server_loop(&stop);
        _exit(0);
    }
    // Give server time
    usleep(200000);

    int ok = 0;
    // 1) Small valid upload
    uint8_t d1[] = {'H','e','l','l','o'};
    ok += (http_post("hello.txt", d1, sizeof(d1)) == 201) ? 1 : 0;
    // 2) Invalid name
    uint8_t d2[] = {'x'};
    ok += (http_post("../evil", d2, sizeof(d2)) == 400) ? 1 : 0;
    // 3) Empty file
    ok += (http_post("empty.bin", (const uint8_t*)"", 0) == 201) ? 1 : 0;
    // 4) Too large
    size_t biglen = (size_t)MAX_SIZE + 1;
    uint8_t *big = (uint8_t*)malloc(biglen);
    if (big) memset(big, 'x', biglen);
    ok += (big && http_post("big.bin", big, biglen) == 413) ? 1 : 0;
    free(big);
    // 5) Another valid
    uint8_t d5[1024]; for (size_t i=0;i<sizeof(d5);i++) d5[i] = (uint8_t)(rand() & 0xFF);
    ok += (http_post("rand.dat", d5, sizeof(d5)) == 201) ? 1 : 0;

    // Stop server
    stop = true;
    // connect to unblock accept
    int fd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    struct sockaddr_in addr; memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET; addr.sin_port = htons(SERVER_PORT); addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    connect(fd, (struct sockaddr*)&addr, sizeof(addr)); close(fd);

    // Kill child
    kill(pid, SIGTERM);
    printf("Tests passed: %d/5\n", ok);
    return 0;
}