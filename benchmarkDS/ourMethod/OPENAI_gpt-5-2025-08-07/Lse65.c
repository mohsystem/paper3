#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

static int is_regular_mode(mode_t m) {
    return S_ISREG(m);
}

static void safe_filename(const char* in, char* out, size_t outsz) {
    size_t j = 0;
    for (size_t i = 0; in[i] != '\0' && j + 1 < outsz; ++i) {
        char c = in[i];
        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '.' || c == '_' || c == '-' || c == ' ') {
            out[j++] = c;
        } else {
            out[j++] = '_';
        }
    }
    out[j] = '\0';
}

static void gen_boundary(char* out, size_t outsz) {
    // simple deterministic-ish boundary
    unsigned int seed = (unsigned int)getpid() ^ (unsigned int)time(NULL);
    snprintf(out, outsz, "----cboundary%08x%08x", seed, seed ^ 0x9e3779b9u);
}

static int read_file_secure(const char* path, unsigned char** buf, size_t* buflen, size_t max_size, char* err, size_t errsz) {
    size_t len = strlen(path);
    if (len < 4 || strcmp(path + len - 4, ".pdf") != 0) {
        snprintf(err, errsz, "Invalid extension");
        return -1;
    }
    int fd = open(path, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) {
        if (errno == ELOOP) snprintf(err, errsz, "Symlink not allowed");
        else snprintf(err, errsz, "Open failed");
        return -1;
    }
    struct stat st;
    if (fstat(fd, &st) != 0) {
        snprintf(err, errsz, "fstat failed");
        close(fd);
        return -1;
    }
    if (!is_regular_mode(st.st_mode)) {
        snprintf(err, errsz, "Not a regular file");
        close(fd);
        return -1;
    }
    if (st.st_size <= 0 || (size_t)st.st_size > max_size) {
        snprintf(err, errsz, "Invalid size");
        close(fd);
        return -1;
    }
    size_t sz = (size_t)st.st_size;
    unsigned char* data = (unsigned char*)malloc(sz);
    if (!data) {
        snprintf(err, errsz, "OOM");
        close(fd);
        return -1;
    }
    size_t off = 0;
    while (off < sz) {
        ssize_t r = read(fd, data + off, (sz - off) > 65536 ? 65536 : (sz - off));
        if (r < 0) {
            snprintf(err, errsz, "Read error");
            free(data);
            close(fd);
            return -1;
        }
        if (r == 0) break;
        off += (size_t)r;
    }
    close(fd);
    if (off != sz) {
        free(data);
        snprintf(err, errsz, "Short read");
        return -1;
    }
    if (sz < 5 || !(data[0] == '%' && data[1] == 'P' && data[2] == 'D' && data[3] == 'F' && data[4] == '-')) {
        free(data);
        snprintf(err, errsz, "Invalid PDF signature");
        return -1;
    }
    *buf = data;
    *buflen = sz;
    return 0;
}

static int send_all(int s, const void* buf, size_t len) {
    const unsigned char* p = (const unsigned char*)buf;
    size_t off = 0;
    while (off < len) {
        ssize_t w = send(s, p + off, len - off, 0);
        if (w <= 0) return -1;
        off += (size_t)w;
    }
    return 0;
}

static int upload_pdf_to_localhost(const char* path, const char* endpoint, size_t max_size, int port, char** out_resp, size_t* out_len) {
    unsigned char* content = NULL;
    size_t content_len = 0;
    char err[128];
    err[0] = '\0';
    if (!endpoint || endpoint[0] != '/') endpoint = "/upload";

    if (read_file_secure(path, &content, &content_len, max_size, err, sizeof(err)) != 0) {
        size_t l = strlen(err);
        *out_resp = (char*)malloc(l + 1);
        if (*out_resp) {
            memcpy(*out_resp, err, l + 1);
            *out_len = l;
        } else {
            *out_resp = NULL; *out_len = 0;
        }
        return -1;
    }

    char boundary[64];
    gen_boundary(boundary, sizeof(boundary));

    char filename[256];
    const char* base = strrchr(path, '/');
    base = base ? base + 1 : path;
    safe_filename(base, filename, sizeof(filename));

    char header[512];
    int hl = snprintf(header, sizeof(header),
                      "--%s\r\n"
                      "Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n"
                      "Content-Type: application/pdf\r\n"
                      "\r\n", boundary, filename);
    if (hl < 0 || (size_t)hl >= sizeof(header)) {
        free(content);
        return -1;
    }
    char footer[128];
    int fl = snprintf(footer, sizeof(footer), "\r\n--%s--\r\n", boundary);
    if (fl < 0 || (size_t)fl >= sizeof(footer)) {
        free(content);
        return -1;
    }

    size_t total_len = (size_t)hl + content_len + (size_t)fl;

    char reqhead[512];
    int rhl = snprintf(reqhead, sizeof(reqhead),
                       "POST %s HTTP/1.1\r\n"
                       "Host: 127.0.0.1:%d\r\n"
                       "Connection: close\r\n"
                       "Content-Type: multipart/form-data; boundary=%s\r\n"
                       "Content-Length: %zu\r\n"
                       "\r\n", endpoint, port, boundary, total_len);
    if (rhl < 0 || (size_t)rhl >= sizeof(reqhead)) {
        free(content);
        return -1;
    }

    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        free(content);
        return -1;
    }
    struct timeval tv; tv.tv_sec = 5; tv.tv_usec = 0;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);
    if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) != 1) {
        close(s);
        free(content);
        return -1;
    }
    if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        close(s);
        free(content);
        return -1;
    }

    if (send_all(s, reqhead, (size_t)rhl) != 0 ||
        send_all(s, header, (size_t)hl) != 0 ||
        send_all(s, content, content_len) != 0 ||
        send_all(s, footer, (size_t)fl) != 0) {
        close(s);
        free(content);
        return -1;
    }
    free(content);

    // Read response
    size_t cap = 4096, len = 0;
    char* resp = (char*)malloc(cap);
    if (!resp) { close(s); return -1; }
    for (;;) {
        if (cap - len < 2048) {
            size_t ncap = cap * 2;
            if (ncap > 16 * 1024 * 1024) break; // limit
            char* tmp = (char*)realloc(resp, ncap);
            if (!tmp) break;
            resp = tmp; cap = ncap;
        }
        ssize_t r = recv(s, resp + len, cap - len, 0);
        if (r < 0) { free(resp); close(s); return -1; }
        if (r == 0) break;
        len += (size_t)r;
    }
    close(s);
    *out_resp = resp;
    *out_len = len;

    // parse status
    int status = -1;
    if (len >= 12) {
        char* sp = memchr(resp, ' ', len);
        if (sp && (sp + 4 - resp) <= (ptrdiff_t)len) {
            status = atoi(sp + 1);
        }
    }
    return status;
}

static int write_file_secure(const char* path, const unsigned char* data, size_t len) {
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (fd < 0) return -1;
    size_t off = 0;
    while (off < len) {
        ssize_t w = write(fd, data + off, (len - off) > 65536 ? 65536 : (len - off));
        if (w <= 0) { close(fd); return -1; }
        off += (size_t)w;
    }
    fsync(fd);
    close(fd);
    return 0;
}

int main(void) {
    const size_t max_size = 512 * 1024;
    char dir[] = "/tmp/c_pdfup_XXXXXX";
    if (!mkdtemp(dir)) {
        fprintf(stderr, "Failed to create temp dir\n");
        return 1;
    }

    const char* f1 = "/t1.pdf";
    const char* f2 = "/t2.pdf";
    const char* f3 = "/notes.txt";
    const char* f4 = "/bad.pdf";
    const char* f5 = "/big.pdf";

    char p1[256], p2[256], p3[256], p4[256], p5[256];
    snprintf(p1, sizeof(p1), "%s%s", dir, f1);
    snprintf(p2, sizeof(p2), "%s%s", dir, f2);
    snprintf(p3, sizeof(p3), "%s%s", dir, f3);
    snprintf(p4, sizeof(p4), "%s%s", dir, f4);
    snprintf(p5, sizeof(p5), "%s%s", dir, f5);

    const char* minimal = "%PDF-1.4\n1 0 obj\n<<>>\nendobj\ntrailer\n<<>>\n%%EOF\n";
    write_file_secure(p1, (const unsigned char*)minimal, strlen(minimal));
    write_file_secure(p2, (const unsigned char*)minimal, strlen(minimal));
    write_file_secure(p3, (const unsigned char*)minimal, strlen(minimal));
    {
        const char* bad = "HELLO";
        write_file_secure(p4, (const unsigned char*)bad, strlen(bad));
    }
    {
        size_t cap = max_size + 128 * 1024;
        unsigned char* big = (unsigned char*)malloc(cap);
        size_t off = 0;
        const char* head = "%PDF-1.4\n";
        memcpy(big + off, head, strlen(head)); off += strlen(head);
        for (size_t i = 0; i < (max_size / (64 * 1024)) + 2; ++i) {
            memset(big + off, 'A', 64 * 1024); off += 64 * 1024;
            if (off >= cap - 16) break;
        }
        const char* eof = "%%EOF\n";
        memcpy(big + off, eof, strlen(eof)); off += strlen(eof);
        write_file_secure(p5, big, off);
        free(big);
    }

    char* resp = NULL; size_t rlen = 0;
    int s1 = upload_pdf_to_localhost(p1, "/upload", max_size, 8000, &resp, &rlen);
    printf("Test1: status=%d resp_len=%zu\n", s1, rlen);
    free(resp); resp = NULL; rlen = 0;

    int s2 = upload_pdf_to_localhost(p2, "/upload", max_size, 8000, &resp, &rlen);
    printf("Test2: status=%d resp_len=%zu\n", s2, rlen);
    free(resp); resp = NULL; rlen = 0;

    int s3 = upload_pdf_to_localhost(p3, "/upload", max_size, 8000, &resp, &rlen);
    printf("Test3: status=%d err='%.*s'\n", s3, (int)rlen, resp ? resp : "");
    free(resp); resp = NULL; rlen = 0;

    int s4 = upload_pdf_to_localhost(p4, "/upload", max_size, 8000, &resp, &rlen);
    printf("Test4: status=%d err='%.*s'\n", s4, (int)rlen, resp ? resp : "");
    free(resp); resp = NULL; rlen = 0;

    int s5 = upload_pdf_to_localhost(p5, "/upload", max_size, 8000, &resp, &rlen);
    printf("Test5: status=%d err='%.*s'\n", s5, (int)rlen, resp ? resp : "");
    free(resp); resp = NULL; rlen = 0;

    unlink(p1); unlink(p2); unlink(p3); unlink(p4); unlink(p5);
    rmdir(dir);
    return 0;
}