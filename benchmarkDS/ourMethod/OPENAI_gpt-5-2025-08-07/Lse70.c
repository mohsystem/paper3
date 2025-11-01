#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

#define MAX_UPLOAD_SIZE (5 * 1024 * 1024)
#define MAX_FILENAME_LEN 100

static int sanitize_filename(const char* in, char* out, size_t outsz) {
    if (!in || !out || outsz == 0) return 0;
    size_t len = strlen(in);
    if (len == 0 || len > MAX_FILENAME_LEN) return 0;
    for (size_t i = 0; i < len; i++) {
        char c = in[i];
        int ok = (c >= 'a' && c <= 'z') ||
                 (c >= 'A' && c <= 'Z') ||
                 (c >= '0' && c <= '9') ||
                 c == '.' || c == '_' || c == '-';
        if (!ok) return 0;
    }
    if ((len == 1 && in[0] == '.') || (len == 2 && in[0] == '.' && in[1] == '.')) return 0;
    if (len + 1 > outsz) return 0;
    memcpy(out, in, len + 1);
    return 1;
}

static int hex_random(char* out, size_t outlen) {
    // outlen must be even (2 chars per byte)
    if (outlen % 2 != 0) return 0;
    size_t nbytes = outlen / 2;
    unsigned char* buf = (unsigned char*)malloc(nbytes);
    if (!buf) return 0;
    int ok = 0;
    int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
    if (fd >= 0) {
        size_t off = 0;
        while (off < nbytes) {
            ssize_t r = read(fd, buf + off, nbytes - off);
            if (r <= 0) break;
            off += (size_t)r;
        }
        close(fd);
        if (off == nbytes) ok = 1;
    }
    if (!ok) {
        for (size_t i = 0; i < nbytes; i++) buf[i] = (unsigned char)(rand() & 0xFF);
    }
    static const char* hex = "0123456789abcdef";
    for (size_t i = 0; i < nbytes; i++) {
        out[2*i] = hex[(buf[i] >> 4) & 0xF];
        out[2*i + 1] = hex[buf[i] & 0xF];
    }
    free(buf);
    return 1;
}

// Returns 0 on success; outSavedName contains final name on success.
int upload_file(const char* baseDir, const char* filename, const unsigned char* data, size_t data_len, char* outSavedName, size_t outSavedNameSz) {
    if (!baseDir || !filename || !data || !outSavedName) return -1;
    if (data_len > MAX_UPLOAD_SIZE) return -1;

    // ensure base dir exists
    if (mkdir(baseDir, 0700) != 0) {
        if (errno != EEXIST) return -1;
    }
    int dfd = open(baseDir, O_DIRECTORY | O_RDONLY | O_CLOEXEC);
    if (dfd < 0) return -1;

    char safe[128];
    if (!sanitize_filename(filename, safe, sizeof(safe))) {
        close(dfd);
        return -1;
    }

    // final unique name: safe-<hex>-<ms>
    char hexbuf[16+1];
    if (!hex_random(hexbuf, 16)) {
        close(dfd);
        return -1;
    }
    hexbuf[16] = '\0';

    long long ms = (long long)time(NULL) * 1000LL;
    char finalName[256];
    if (snprintf(finalName, sizeof(finalName), "%s-%s-%lld", safe, hexbuf, ms) >= (int)sizeof(finalName)) {
        close(dfd);
        return -1;
    }

    // temp file
    char tmpHex[16+1];
    hex_random(tmpHex, 16);
    tmpHex[16] = '\0';
    char tmpName[256];
    if (snprintf(tmpName, sizeof(tmpName), "tmp-%s.upload", tmpHex) >= (int)sizeof(tmpName)) {
        close(dfd);
        return -1;
    }

    int tfd = openat(dfd, tmpName, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);
    if (tfd < 0) {
        close(dfd);
        return -1;
    }

    // write data
    size_t off = 0;
    while (off < data_len) {
        size_t chunk = data_len - off;
        if (chunk > 65536) chunk = 65536;
        ssize_t w = write(tfd, data + off, chunk);
        if (w <= 0) {
            close(tfd);
            unlinkat(dfd, tmpName, 0);
            close(dfd);
            return -1;
        }
        off += (size_t)w;
    }
    if (fsync(tfd) != 0) {
        close(tfd);
        unlinkat(dfd, tmpName, 0);
        close(dfd);
        return -1;
    }
    if (close(tfd) != 0) {
        unlinkat(dfd, tmpName, 0);
        close(dfd);
        return -1;
    }

    // rename to final
#if defined(RENAME_NOREPLACE)
    if (renameat2(dfd, tmpName, dfd, finalName, RENAME_NOREPLACE) != 0)
#else
    if (renameat(dfd, tmpName, dfd, finalName) != 0)
#endif
    {
        unlinkat(dfd, tmpName, 0);
        close(dfd);
        return -1;
    }

    if (snprintf(outSavedName, outSavedNameSz, "%s", finalName) >= (int)outSavedNameSz) {
        close(dfd);
        return -1;
    }
    close(dfd);
    return 0;
}

static void write_all(int fd, const char* s) {
    size_t len = strlen(s);
    size_t off = 0;
    while (off < len) {
        ssize_t w = write(fd, s + off, len - off);
        if (w <= 0) break;
        off += (size_t)w;
    }
}

static int run_server(const char* baseDir, int port) {
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) return -1;
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(srv, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        close(srv);
        return -1;
    }
    if (listen(srv, 8) != 0) {
        close(srv);
        return -1;
    }
    printf("C server listening on http://127.0.0.1:%d/upload\n", port);

    for (;;) {
        int cfd = accept(srv, NULL, NULL);
        if (cfd < 0) continue;

        // read headers up to 16KB
        char header[16384+1];
        size_t have = 0;
        int header_done = 0;
        for (;;) {
            ssize_t r = read(cfd, header + have, sizeof(header) - 1 - have);
            if (r <= 0) break;
            have += (size_t)r;
            header[have] = '\0';
            if (strstr(header, "\r\n\r\n") != NULL) {
                header_done = 1;
                break;
            }
            if (have >= sizeof(header) - 1) break;
        }
        if (!header_done) {
            write_all(cfd, "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\nConnection: close\r\n\r\nBad Request\n");
            close(cfd);
            continue;
        }
        char* hdr_end = strstr(header, "\r\n\r\n");
        size_t head_len = (size_t)(hdr_end - header) + 2; // keep last \r

        size_t rest_len = have - (head_len + 2);
        char* rest = header + head_len + 2;

        // parse request line
        char method[8]={0}, path[64]={0}, proto[16]={0};
        sscanf(header, "%7s %63s %15s", method, path, proto);
        if (strcmp(method, "POST") != 0 || strcmp(path, "/upload") != 0) {
            write_all(cfd, "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 19\r\nConnection: close\r\n\r\nMethod Not Allowed\n");
            close(cfd);
            continue;
        }

        // parse headers
        long long content_length = -1;
        char filename[128]={0};
        char* line = header;
        while (line < hdr_end) {
            char* nl = strstr(line, "\r\n");
            if (!nl) break;
            size_t linelen = (size_t)(nl - line);
            if (linelen > 0) {
                char key[128]={0}, val[1024]={0};
                // simple split on ':'
                char* colon = memchr(line, ':', linelen);
                if (colon) {
                    size_t klen = (size_t)(colon - line);
                    if (klen >= sizeof(key)) klen = sizeof(key)-1;
                    memcpy(key, line, klen);
                    // lowercase key
                    for (size_t i = 0; i < klen; i++) {
                        if (key[i] >= 'A' && key[i] <= 'Z') key[i] = (char)(key[i] - 'A' + 'a');
                    }
                    // value
                    size_t vlen = (size_t)(linelen - (klen + 1));
                    while (vlen > 0 && (line[klen+1] == ' ' || line[klen+1] == '\t')) { line++; vlen--; }
                    if (vlen >= sizeof(val)) vlen = sizeof(val)-1;
                    memcpy(val, colon+1, vlen);
                    val[vlen] = '\0';
                    if (strcmp(key, "content-length") == 0) {
                        content_length = atoll(val);
                    } else if (strcmp(key, "x-filename") == 0) {
                        // trim trailing spaces
                        size_t fl = strlen(val);
                        while (fl > 0 && (val[fl-1] == ' ' || val[fl-1] == '\t')) { val[--fl] = '\0'; }
                        snprintf(filename, sizeof(filename), "%s", val);
                    }
                }
            }
            line = nl + 2;
        }
        if (content_length < 0 || content_length > MAX_UPLOAD_SIZE || filename[0] == '\0') {
            write_all(cfd, "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\nConnection: close\r\n\r\nBad Request\n");
            close(cfd);
            continue;
        }

        // read body
        unsigned char* body = (unsigned char*)malloc((size_t)content_length);
        if (!body) {
            write_all(cfd, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 22\r\nConnection: close\r\n\r\nInternal Server Error\n");
            close(cfd);
            continue;
        }
        size_t copied = 0;
        if (rest_len > 0) {
            size_t to_copy = rest_len;
            if (to_copy > (size_t)content_length) to_copy = (size_t)content_length;
            memcpy(body, rest, to_copy);
            copied += to_copy;
        }
        while ((long long)copied < content_length) {
            ssize_t r = read(cfd, body + copied, (size_t)(content_length - copied));
            if (r <= 0) break;
            copied += (size_t)r;
        }
        if ((long long)copied != content_length) {
            free(body);
            write_all(cfd, "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\nConnection: close\r\n\r\nBad Request\n");
            close(cfd);
            continue;
        }

        char saved[256];
        if (upload_file("uploads-c", filename, body, (size_t)content_length, saved, sizeof(saved)) == 0) {
            char msg[512];
            int mlen = snprintf(msg, sizeof(msg),
                "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %zu\r\nConnection: close\r\n\r\nSaved: %s\n",
                strlen("Saved: ") + strlen(saved) + 1, saved);
            if (mlen > 0) write_all(cfd, msg);
        } else {
            write_all(cfd, "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\nConnection: close\r\n\r\nBad Request\n");
        }
        free(body);
        close(cfd);
    }
    close(srv);
    return 0;
}

int main(void) {
    const char* base = "uploads-c";
    mkdir(base, 0700);

    // 5 test cases
    {
        unsigned char d[] = {'h','e','l','l','o'};
        char out[256];
        if (upload_file(base, "file.txt", d, sizeof(d), out, sizeof(out)) == 0) {
            printf("Test1 OK: %s\n", out);
        } else {
            printf("Test1 FAIL\n");
        }
    }
    {
        size_t sz = (size_t)MAX_UPLOAD_SIZE + 1;
        unsigned char* d = (unsigned char*)malloc(sz);
        if (d) {
            memset(d, 0, sz);
            char out[256];
            if (upload_file(base, "big.bin", d, sz, out, sizeof(out)) == 0) {
                printf("Test2 FAIL: %s\n", out);
            } else {
                printf("Test2 OK (rejected large)\n");
            }
            free(d);
        } else {
            printf("Test2 SKIPPED (alloc failed)\n");
        }
    }
    {
        unsigned char d[] = {1,2,3};
        char out[256];
        if (upload_file(base, "safe-01.log", d, sizeof(d), out, sizeof(out)) == 0) {
            printf("Test3 OK: %s\n", out);
        } else {
            printf("Test3 FAIL\n");
        }
    }
    {
        unsigned char d[] = {'x'};
        char out[256];
        if (upload_file(base, "../bad", d, sizeof(d), out, sizeof(out)) == 0) {
            printf("Test4 FAIL: %s\n", out);
        } else {
            printf("Test4 OK (rejected bad name)\n");
        }
    }
    {
        unsigned char d[] = {'d','a','t','a'};
        char out[256];
        if (upload_file(base, "bad$name.txt", d, sizeof(d), out, sizeof(out)) == 0) {
            printf("Test5 FAIL: %s\n", out);
        } else {
            printf("Test5 OK (rejected bad chars)\n");
        }
    }

    run_server(base, 8082);
    return 0;
}