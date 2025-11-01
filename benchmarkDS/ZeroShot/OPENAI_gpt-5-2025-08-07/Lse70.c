#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#ifdef _WIN32
  #include <direct.h>
  #include <io.h>
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "Ws2_32.lib")
  #define mkdir_p(path) _mkdir(path)
  typedef SOCKET socket_t;
#else
  #include <sys/stat.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #define mkdir_p(path) mkdir(path, 0700)
  typedef int socket_t;
#endif

#define MAX_UPLOAD_BYTES (10 * 1024 * 1024)

static void sanitize_filename(const char* input, char* output, size_t outsz) {
    if (!input || !*input) {
        snprintf(output, outsz, "file");
        return;
    }
    size_t j = 0;
    for (size_t i = 0; input[i] && j + 1 < outsz; ++i) {
        char c = input[i];
        if (isalnum((unsigned char)c) || c == '.' || c == '_' || c == '-') {
            output[j++] = c;
        } else {
            output[j++] = '_';
        }
        if (j >= 100) break;
    }
    output[j] = '\0';
    while (output[0] == '.') {
        memmove(output, output + 1, strlen(output));
        if (!output[0]) break;
    }
    if (!output[0]) snprintf(output, outsz, "file");
}

static int ensure_uploads_dir(void) {
    if (mkdir_p("uploads") == 0) return 1;
#ifdef _WIN32
    if (errno == EEXIST) return 1;
#else
    if (errno == EEXIST) return 1;
#endif
    return 0;
}

// upload_file: saves data buffer to uploads directory; returns 0 on success
int upload_file(const char* originalName, const unsigned char* data, size_t data_len, size_t maxBytes, char* outPath, size_t outPathSize) {
    if (!data || data_len == 0) return -1;
    if (data_len > maxBytes) return -2;
    if (!ensure_uploads_dir()) return -3;

    char safe[128];
    sanitize_filename(originalName, safe, sizeof(safe));

    // split base/ext
    char base[128], ext[128];
    const char* dot = strrchr(safe, '.');
    if (dot && dot != safe && *(dot + 1)) {
        size_t blen = (size_t)(dot - safe);
        if (blen >= sizeof(base)) blen = sizeof(base) - 1;
        memcpy(base, safe, blen); base[blen] = '\0';
        snprintf(ext, sizeof(ext), "%s", dot);
    } else {
        snprintf(base, sizeof(base), "%s", safe);
        ext[0] = '\0';
    }

    // unique name
    unsigned int rnd = (unsigned int)rand();
    char filename[300];
    snprintf(filename, sizeof(filename), "uploads/%s-%08X%s", base, rnd, ext);

#ifdef _WIN32
    int fd = _open(filename, _O_CREAT | _O_EXCL | _O_WRONLY | _O_BINARY, _S_IREAD | _S_IWRITE);
    if (fd < 0) return -4;
    int written = _write(fd, data, (unsigned int)data_len);
    _close(fd);
#else
    FILE* f = fopen(filename, "wx");
    if (!f) return -4;
    size_t written = fwrite(data, 1, data_len, f);
    fclose(f);
#endif

    if ((size_t)written != data_len) {
        remove(filename);
        return -5;
    }
    snprintf(outPath, outPathSize, "%s", filename);
    return 0;
}

// Minimal HTTP server: POST /upload with X-Filename and Content-Length. Body is raw bytes.
static void http_server(unsigned short port) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif
    socket_t srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) return;

    int opt = 1;
#ifdef _WIN32
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
#ifdef _WIN32
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
#else
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
#endif

    if (bind(srv, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
#ifdef _WIN32
        closesocket(srv);
        WSACleanup();
#else
        close(srv);
#endif
        return;
    }
    listen(srv, 5);

    for (;;) {
        struct sockaddr_in cli;
#ifdef _WIN32
        int clen = sizeof(cli);
        socket_t c = accept(srv, (struct sockaddr*)&cli, &clen);
#else
        socklen_t clen = sizeof(cli);
        socket_t c = accept(srv, (struct sockaddr*)&cli, &clen);
#endif
        if (c < 0) continue;

        // Read headers
        char ch;
        char headers[65536];
        size_t hlen = 0;
        int crlf = 0;
        while (hlen < sizeof(headers) - 1) {
#ifdef _WIN32
            int n = recv(c, &ch, 1, 0);
#else
            ssize_t n = recv(c, &ch, 1, 0);
#endif
            if (n <= 0) break;
            headers[hlen++] = ch;
            if (hlen >= 4 && headers[hlen-4] == '\r' && headers[hlen-3] == '\n' && headers[hlen-2] == '\r' && headers[hlen-1] == '\n') break;
        }
        headers[hlen] = '\0';

        // Parse request line
        char method[16], path[256], version[16];
        method[0]=path[0]=version[0]='\0';
        sscanf(headers, "%15s %255s %15s", method, path, version);
        if (strcmp(method, "POST") != 0 || strcmp(path, "/upload") != 0) {
            const char* resp = "HTTP/1.1 404 Not Found\r\nX-Content-Type-Options: nosniff\r\nX-Frame-Options: DENY\r\nContent-Security-Policy: default-src 'none'\r\nCache-Control: no-store\r\nContent-Length: 9\r\n\r\nNot Found";
#ifdef _WIN32
            send(c, resp, (int)strlen(resp), 0);
            closesocket(c);
#else
            send(c, resp, strlen(resp), 0);
            close(c);
#endif
            continue;
        }

        // Find headers
        char *hptr = headers;
        char *endh = strstr(headers, "\r\n\r\n");
        if (!endh) {
            const char* resp = "HTTP/1.1 400 Bad Request\r\nContent-Length: 16\r\n\r\nMalformed header";
#ifdef _WIN32
            send(c, resp, (int)strlen(resp), 0);
            closesocket(c);
#else
            send(c, resp, strlen(resp), 0);
            close(c);
#endif
            continue;
        }

        char xfilename[256] = {0};
        long content_length = -1;

        char *line = strtok(headers, "\r\n");
        while (line) {
            // Skip request line
            if (strncmp(line, "POST ", 5) == 0) {
                line = strtok(NULL, "\r\n");
                continue;
            }
            char key[256], val[1024];
            key[0]=val[0]='\0';
            if (sscanf(line, "%255[^:]: %1023[^\r\n]", key, val) == 2) {
                // to lower key
                for (char* p = key; *p; ++p) *p = (char)tolower(*p);
                if (strcmp(key, "x-filename") == 0) {
                    strncpy(xfilename, val, sizeof(xfilename)-1);
                } else if (strcmp(key, "content-length") == 0) {
                    content_length = strtol(val, NULL, 10);
                }
            }
            line = strtok(NULL, "\r\n");
        }

        if (xfilename[0] == '\0') {
            const char* resp = "HTTP/1.1 400 Bad Request\r\nContent-Length: 26\r\n\r\nMissing X-Filename header";
#ifdef _WIN32
            send(c, resp, (int)strlen(resp), 0);
            closesocket(c);
#else
            send(c, resp, strlen(resp), 0);
            close(c);
#endif
            continue;
        }
        if (content_length <= 0) {
            const char* resp = "HTTP/1.1 411 Length Required\r\nContent-Length: 22\r\n\r\nContent-Length required";
#ifdef _WIN32
            send(c, resp, (int)strlen(resp), 0);
            closesocket(c);
#else
            send(c, resp, strlen(resp), 0);
            close(c);
#endif
            continue;
        }
        if (content_length > MAX_UPLOAD_BYTES) {
            const char* resp = "HTTP/1.1 413 Payload Too Large\r\nContent-Length: 18\r\n\r\nPayload too large";
#ifdef _WIN32
            send(c, resp, (int)strlen(resp), 0);
            closesocket(c);
#else
            send(c, resp, strlen(resp), 0);
            close(c);
#endif
            continue;
        }

        // Read body
        size_t to_read = (size_t)content_length;
        unsigned char* body = (unsigned char*)malloc(to_read);
        if (!body) {
            const char* resp = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 15\r\n\r\nServer error 1";
#ifdef _WIN32
            send(c, resp, (int)strlen(resp), 0);
            closesocket(c);
#else
            send(c, resp, strlen(resp), 0);
            close(c);
#endif
            continue;
        }
        size_t got = 0;
        while (got < to_read) {
#ifdef _WIN32
            int n = recv(c, (char*)body + got, (int)(to_read - got), 0);
#else
            ssize_t n = recv(c, body + got, to_read - got, 0);
#endif
            if (n <= 0) break;
            got += (size_t)n;
        }
        if (got != to_read) {
            free(body);
            const char* resp = "HTTP/1.1 400 Bad Request\r\nContent-Length: 16\r\n\r\nIncomplete body";
#ifdef _WIN32
            send(c, resp, (int)strlen(resp), 0);
            closesocket(c);
#else
            send(c, resp, strlen(resp), 0);
            close(c);
#endif
            continue;
        }

        char saved[512];
        int rc = upload_file(xfilename, body, to_read, MAX_UPLOAD_BYTES, saved, sizeof(saved));
        free(body);

        if (rc != 0) {
            const char* resp = "HTTP/1.1 400 Bad Request\r\nContent-Length: 13\r\n\r\nUpload failed";
#ifdef _WIN32
            send(c, resp, (int)strlen(resp), 0);
            closesocket(c);
#else
            send(c, resp, strlen(resp), 0);
            close(c);
#endif
            continue;
        } else {
            char msg[1024];
            snprintf(msg, sizeof(msg), "Saved: %s", saved);
            char hdr[1024];
            snprintf(hdr, sizeof(hdr),
                     "HTTP/1.1 200 OK\r\n"
                     "X-Content-Type-Options: nosniff\r\n"
                     "X-Frame-Options: DENY\r\n"
                     "Content-Security-Policy: default-src 'none'\r\n"
                     "Cache-Control: no-store\r\n"
                     "Content-Type: text/plain; charset=utf-8\r\n"
                     "Content-Length: %zu\r\n\r\n", strlen(msg));
#ifdef _WIN32
            send(c, hdr, (int)strlen(hdr), 0);
            send(c, msg, (int)strlen(msg), 0);
            closesocket(c);
#else
            send(c, hdr, strlen(hdr), 0);
            send(c, msg, strlen(msg), 0);
            close(c);
#endif
        }
    }
#ifdef _WIN32
    closesocket(srv);
    WSACleanup();
#else
    close(srv);
#endif
}

int main(void) {
    // Seed random
    srand((unsigned int)time(NULL));

    // Start server in background (best-effort, simple)
#ifdef _WIN32
    HANDLE th = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)http_server, (LPVOID)(uintptr_t)8083, 0, NULL);
    if (th) CloseHandle(th);
#else
    pid_t pid = fork();
    if (pid == 0) {
        http_server(8083);
        exit(0);
    }
#endif

    printf("Running tests for upload_file...\n");

    // Test 1
    {
        const unsigned char data[] = "Hello world";
        char out[512];
        int rc = upload_file("hello.txt", data, sizeof(data)-1, 1024, out, sizeof(out));
        if (rc == 0) printf("Test 1 saved: %s\n", out); else printf("Test 1 failed: %d\n", rc);
    }

    // Test 2
    {
        const unsigned char data[] = "evil content";
        char out[512];
        int rc = upload_file("../evil.sh", data, sizeof(data)-1, 1024, out, sizeof(out));
        if (rc == 0) printf("Test 2 saved: %s\n", out); else printf("Test 2 failed: %d\n", rc);
    }

    // Test 3
    {
        const unsigned char data[] = "data";
        char out[512];
        int rc = upload_file("inv@lid:/name?.bin", data, sizeof(data)-1, 1024, out, sizeof(out));
        if (rc == 0) printf("Test 3 saved: %s\n", out); else printf("Test 3 failed: %d\n", rc);
    }

    // Test 4
    {
        const unsigned char data[] = "bashrc-like";
        char out[512];
        int rc = upload_file(".bashrc", data, sizeof(data)-1, 1024, out, sizeof(out));
        if (rc == 0) printf("Test 4 saved: %s\n", out); else printf("Test 4 failed: %d\n", rc);
    }

    // Test 5
    {
        size_t bigsz = 2 * 1024 * 1024;
        unsigned char* big = (unsigned char*)malloc(bigsz);
        if (big) {
            memset(big, 0, bigsz);
            char out[512];
            int rc = upload_file("toolarge.dat", big, bigsz, 1024 * 1024, out, sizeof(out));
            if (rc == 0) printf("Test 5 unexpectedly succeeded: %s\n", out); else printf("Test 5 correctly failed: %d\n", rc);
            free(big);
        }
    }

    printf("HTTP server listening on http://127.0.0.1:8083/upload\n");
    // Keep running
#ifdef _WIN32
    Sleep(24 * 60 * 60 * 1000);
#else
    sleep(24 * 60 * 60);
#endif
    return 0;
}