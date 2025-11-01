#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_SIZE_BYTES_C (5*1024*1024)

int ensure_dir_c(const char* path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        return S_ISDIR(st.st_mode);
    }
    if (mkdir(path, 0700) == 0) return 1;
    return 0;
}

void random_hex_c(size_t nbytes, char* out, size_t outlen) {
    // out needs to be at least nbytes*2+1
    FILE* f = fopen("/dev/urandom", "rb");
    unsigned char* buf = (unsigned char*)malloc(nbytes);
    if (!buf) { snprintf(out, outlen, ""); return; }
    if (f) {
        fread(buf, 1, nbytes, f);
        fclose(f);
    } else {
        srand((unsigned)time(NULL) ^ getpid());
        for (size_t i=0;i<nbytes;i++) buf[i] = rand() & 0xFF;
    }
    const char* hexd = "0123456789abcdef";
    size_t p = 0;
    for (size_t i=0;i<nbytes && p+2 < outlen; i++) {
        out[p++] = hexd[buf[i]>>4];
        out[p++] = hexd[buf[i]&0xF];
    }
    out[p] = '\0';
    free(buf);
}

void sanitize_filename_c(const char* name, char* out, size_t outlen) {
    // basename
    const char* base = name;
    const char* slash = strrchr(name, '/');
    const char* bslash = strrchr(name, '\\');
    if (slash && bslash) base = (slash > bslash) ? slash + 1 : bslash + 1;
    else if (slash) base = slash + 1;
    else if (bslash) base = bslash + 1;

    size_t p = 0;
    for (const char* s = base; *s && p+1 < outlen; ++s) {
        char c = *s;
        if (isalnum((unsigned char)c) || c == '.' || c == '_' || c == '-') out[p++] = c;
        else out[p++] = '_';
        if (p >= 100) break;
    }
    out[p] = '\0';
    if (!strchr(out, '.')) {
        if (p+4 < outlen) { strcat(out, ".bin"); }
    }
}

int allowed_ext_c(const char* name) {
    const char* dot = strrchr(name, '.');
    if (!dot) return 0;
    char ext[16];
    size_t len = strlen(dot);
    if (len >= sizeof(ext)) return 0;
    for (size_t i=0;i<len;i++) ext[i] = (char)tolower((unsigned char)dot[i]);
    ext[len] = '\0';
    const char* allowed[] = {".txt",".png",".jpg",".jpeg",".pdf",".gif"};
    for (size_t i=0;i<sizeof(allowed)/sizeof(allowed[0]); i++) {
        if (strcmp(ext, allowed[i]) == 0) return 1;
    }
    return 0;
}

int save_uploaded_file_c(const unsigned char* data, size_t len, const char* original_filename, const char* content_type, size_t max_size_bytes, const char* upload_dir, char* outpath, size_t outpath_len, char* err, size_t err_len) {
    if (!data) { snprintf(err, err_len, "No data"); return 0; }
    if (!original_filename || !*original_filename) { snprintf(err, err_len, "Missing filename"); return 0; }
    if (len == 0) { snprintf(err, err_len, "Empty upload"); return 0; }
    if (len > max_size_bytes) { snprintf(err, err_len, "File too large"); return 0; }
    if (!ensure_dir_c(upload_dir)) { snprintf(err, err_len, "Cannot create upload dir"); return 0; }
    chmod(upload_dir, 0700);

    char sanitized[128];
    sanitize_filename_c(original_filename, sanitized, sizeof(sanitized));
    if (!allowed_ext_c(sanitized)) { snprintf(err, err_len, "Disallowed file extension"); return 0; }

    char rnd[64];
    random_hex_c(12, rnd, sizeof(rnd));
    const char* dot = strrchr(sanitized, '.');
    char unique[192];
    snprintf(unique, sizeof(unique), "%s%s", rnd, dot ? dot : ".bin");

    char target[PATH_MAX];
    snprintf(target, sizeof(target), "%s/%s", upload_dir, unique);

    int fd = open(target, O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (fd < 0) { snprintf(err, err_len, "Open failed: %s", strerror(errno)); return 0; }
    ssize_t w = write(fd, data, len);
    if (w < 0 || (size_t)w != len) {
        int e = errno;
        close(fd);
        unlink(target);
        snprintf(err, err_len, "Write failed: %s", strerror(e));
        return 0;
    }
    if (fsync(fd) != 0) {
        int e = errno;
        close(fd);
        unlink(target);
        snprintf(err, err_len, "Sync failed: %s", strerror(e));
        return 0;
    }
    close(fd);

    // Resolve absolute and ensure inside upload_dir
    char realdir[PATH_MAX], realtgt[PATH_MAX];
    if (!realpath(upload_dir, realdir) || !realpath(target, realtgt)) {
        unlink(target);
        snprintf(err, err_len, "Path resolve failed");
        return 0;
    }
    size_t dlen = strlen(realdir);
    if (realdir[dlen-1] != '/') {
        realdir[dlen] = '/'; realdir[dlen+1] = '\0'; dlen++;
    }
    if (strncmp(realtgt, realdir, dlen) != 0) {
        unlink(target);
        snprintf(err, err_len, "Invalid path");
        return 0;
    }
    snprintf(outpath, outpath_len, "%s", realtgt);
    return 1;
}

/* Minimal HTTP server for GET / and POST /upload (X-Filename header, raw body) */
void respond_text_c(int cfd, int code, const char* msg) {
    char head[512];
    int hl = snprintf(head, sizeof(head),
                      "HTTP/1.1 %d \r\n"
                      "Content-Type: text/plain; charset=utf-8\r\n"
                      "X-Content-Type-Options: nosniff\r\n"
                      "Content-Length: %zu\r\n"
                      "Connection: close\r\n\r\n",
                      code, strlen(msg));
    send(cfd, head, hl, 0);
    send(cfd, msg, (int)strlen(msg), 0);
}
void respond_html_c(int cfd, const char* html) {
    char head[512];
    int hl = snprintf(head, sizeof(head),
                      "HTTP/1.1 200 OK\r\n"
                      "Content-Type: text/html; charset=utf-8\r\n"
                      "X-Content-Type-Options: nosniff\r\n"
                      "Content-Length: %zu\r\n"
                      "Connection: close\r\n\r\n",
                      strlen(html));
    send(cfd, head, hl, 0);
    send(cfd, html, (int)strlen(html), 0);
}

void handle_client_c(int cfd, const char* upload_dir) {
    const size_t HDR_LIMIT = 8192;
    char* req = (char*)malloc(HDR_LIMIT + 1);
    size_t used = 0;
    int found = 0;
    while (used < HDR_LIMIT) {
        ssize_t n = recv(cfd, req + used, HDR_LIMIT - used, 0);
        if (n <= 0) break;
        used += (size_t)n;
        req[used] = '\0';
        if (strstr(req, "\r\n\r\n")) { found = 1; break; }
        if ((size_t)n < 1024) break;
    }
    if (!found) { respond_text_c(cfd, 400, "Bad Request"); free(req); return; }

    char *hdr_end = strstr(req, "\r\n\r\n");
    size_t header_len = (size_t)(hdr_end - req) + 4;

    // Request line
    char* line_end = strstr(req, "\r\n");
    if (!line_end) { respond_text_c(cfd, 400, "Bad Request"); free(req); return; }
    char method[8]={0}, path[256]={0}, ver[16]={0};
    sscanf(req, "%7s %255s %15s", method, path, ver);

    if (strcmp(method, "GET") == 0 && strcmp(path, "/") == 0) {
        const char* html =
            "<!doctype html><html><head><meta charset='utf-8'><title>C Upload</title></head>"
            "<body><h1>Upload a file</h1>"
            "<input type='file' id='f'><button id='u'>Upload</button>"
            "<pre id='o'></pre>"
            "<script>"
            "document.getElementById('u').onclick=async()=>{"
            "let f=document.getElementById('f').files[0];"
            "if(!f){alert('Choose a file');return;}"
            "let buf=await f.arrayBuffer();"
            "let r=await fetch('/upload',{method:'POST',headers:{'X-Filename':f.name,'Content-Type':'application/octet-stream','Content-Length':buf.byteLength},body:buf});"
            "document.getElementById('o').textContent=await r.text();};"
            "</script></body></html>";
        respond_html_c(cfd, html);
        free(req);
        return;
    }
    if (!(strcmp(method, "POST") == 0 && strcmp(path, "/upload") == 0)) {
        respond_text_c(cfd, 404, "Not Found");
        free(req);
        return;
    }

    // Parse headers for Content-Length and X-Filename
    long long content_len = -1;
    char xfn[256] = {0};
    {
        char* p = req;
        char* end = hdr_end;
        while (p < end) {
            char* e = strstr(p, "\r\n");
            if (!e || e > end) break;
            if (p == req) { p = e + 2; continue; } // skip request line
            size_t len = (size_t)(e - p);
            char line[1024];
            if (len >= sizeof(line)) len = sizeof(line) - 1;
            memcpy(line, p, len); line[len] = '\0';
            // Split at :
            char* colon = strchr(line, ':');
            if (colon) {
                *colon = '\0';
                char* key = line;
                char* val = colon + 1;
                while (*val && isspace((unsigned char)*val)) val++;
                for (char* q = key; *q; ++q) *q = (char)tolower((unsigned char)*q);
                if (strcmp(key, "content-length") == 0) {
                    content_len = atoll(val);
                } else if (strcmp(key, "x-filename") == 0) {
                    strncpy(xfn, val, sizeof(xfn)-1);
                }
            }
            p = e + 2;
        }
    }
    if (content_len <= 0 || content_len > MAX_SIZE_BYTES_C) {
        respond_text_c(cfd, 413, "Payload Too Large");
        free(req);
        return;
    }
    if (xfn[0] == '\0') {
        respond_text_c(cfd, 400, "Missing X-Filename");
        free(req);
        return;
    }

    // Body
    size_t body_in_req = used - header_len;
    unsigned char* data = (unsigned char*)malloc((size_t)content_len);
    size_t have = 0;
    if (body_in_req > 0) {
        size_t take = (size_t)content_len < body_in_req ? (size_t)content_len : body_in_req;
        memcpy(data, req + header_len, take);
        have += take;
    }
    free(req);
    while (have < (size_t)content_len) {
        ssize_t n = recv(cfd, data + have, (size_t)content_len - have, 0);
        if (n <= 0) break;
        have += (size_t)n;
    }
    if (have != (size_t)content_len) {
        respond_text_c(cfd, 400, "Incomplete body");
        free(data);
        return;
    }

    char outp[PATH_MAX];
    char err[256];
    if (!save_uploaded_file_c(data, have, xfn, "application/octet-stream", MAX_SIZE_BYTES_C, "uploads_c", outp, sizeof(outp), err, sizeof(err))) {
        char msg[512];
        snprintf(msg, sizeof(msg), "Upload failed: %s", err);
        respond_text_c(cfd, 400, msg);
    } else {
        const char* base = strrchr(outp, '/');
        base = base ? base + 1 : outp;
        char msg[512];
        snprintf(msg, sizeof(msg), "Saved: %s", base);
        respond_text_c(cfd, 200, msg);
    }
    free(data);
}

void run_server_c(unsigned short port) {
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) { perror("socket"); return; }
    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(port);
    if (bind(sfd, (struct sockaddr*)&addr, sizeof(addr)) != 0) { perror("bind"); close(sfd); return; }
    if (listen(sfd, 16) != 0) { perror("listen"); close(sfd); return; }
    printf("C server running on http://127.0.0.1:%hu\n", port);
    while (1) {
        int cfd = accept(sfd, NULL, NULL);
        if (cfd < 0) continue;
        handle_client_c(cfd, "uploads_c");
        close(cfd);
    }
}

int main(void) {
    ensure_dir_c("uploads_c");
    chmod("uploads_c", 0700);

    // Start server in background
    pid_t pid = fork();
    if (pid == 0) {
        run_server_c(8083);
        exit(0);
    }

    // 5 tests
    printf("Running C tests:\n");
    {
        char outp[PATH_MAX], err[256];
        if (save_uploaded_file_c((const unsigned char*)"hello", 5, "hello.txt", "text/plain", MAX_SIZE_BYTES_C, "uploads_c", outp, sizeof(outp), err, sizeof(err)))
            printf("Test 1 OK: %s\n", outp);
        else
            printf("Test 1 FAILED: %s\n", err);
    }
    {
        char outp[PATH_MAX], err[256];
        if (save_uploaded_file_c((const unsigned char*)"x", 1, "../../evil.txt", "text/plain", MAX_SIZE_BYTES_C, "uploads_c", outp, sizeof(outp), err, sizeof(err)))
            printf("Test 2 OK (sanitized): %s\n", outp);
        else
            printf("Test 2 FAILED: %s\n", err);
    }
    {
        char outp[PATH_MAX], err[256];
        if (!save_uploaded_file_c((const unsigned char*)"bin", 3, "malware.exe", "application/octet-stream", MAX_SIZE_BYTES_C, "uploads_c", outp, sizeof(outp), err, sizeof(err)))
            printf("Test 3 OK (rejected): %s\n", err);
        else
            printf("Test 3 FAILED (accepted): %s\n", outp);
    }
    {
        char outp[PATH_MAX], err[256];
        size_t bigsz = MAX_SIZE_BYTES_C + 1;
        unsigned char* big = (unsigned char*)malloc(bigsz);
        memset(big, 'A', bigsz);
        if (!save_uploaded_file_c(big, bigsz, "big.txt", "text/plain", MAX_SIZE_BYTES_C, "uploads_c", outp, sizeof(outp), err, sizeof(err)))
            printf("Test 4 OK (oversize rejected): %s\n", err);
        else
            printf("Test 4 FAILED (oversize accepted): %s\n", outp);
        free(big);
    }
    {
        char outp[PATH_MAX], err[256];
        unsigned char img[4] = {0x89, 0x50, 0x4E, 0x47};
        if (save_uploaded_file_c(img, sizeof(img), "my image (1).jpg", "image/jpeg", MAX_SIZE_BYTES_C, "uploads_c", outp, sizeof(outp), err, sizeof(err)))
            printf("Test 5 OK: %s\n", outp);
        else
            printf("Test 5 FAILED: %s\n", err);
    }

    // Keep parent alive
    for (;; ) pause();
    return 0;
}