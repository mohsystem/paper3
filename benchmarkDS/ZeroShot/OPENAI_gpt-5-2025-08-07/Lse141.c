#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
typedef int socklen_t;
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

typedef struct {
    char url[256];
    char user[128];
    int connected;
} DBConnection;

static void safe_copy(char* dst, size_t dstsz, const char* src) {
    if (!dst || dstsz == 0) return;
    if (!src) { dst[0] = '\0'; return; }
    strncpy(dst, src, dstsz - 1);
    dst[dstsz - 1] = '\0';
}

const char* getServerName(const char* defaultValue) {
    const char* v = getenv("SERVER_NAME");
    if (v && *v) return v;
    return defaultValue ? defaultValue : "";
}

const char* getDbName(const char* defaultValue) {
    const char* v = getenv("DB_NAME");
    if (v && *v) return v;
    return defaultValue ? defaultValue : "";
}

const char* getDbUser(const char* defaultValue) {
    const char* v = getenv("DB_USER");
    if (v && *v) return v;
    return defaultValue ? defaultValue : "";
}

const char* getDbPassword(const char* defaultValue) {
    const char* v = getenv("DB_PASSWORD");
    if (v && *v) return v;
    return defaultValue ? defaultValue : "";
}

DBConnection getDbConnection(const char* url, const char* user, const char* password) {
    DBConnection conn;
    safe_copy(conn.url, sizeof(conn.url), url ? url : "");
    safe_copy(conn.user, sizeof(conn.user), user ? user : "");
    (void)password; // never log or expose
    if (url && (strncmp(url, "sqlite:", 7) == 0 || strncmp(url, "memory:", 7) == 0)) {
        conn.connected = 1;
    } else {
        conn.connected = 0;
    }
    return conn;
}

static void mask(const char* s, char* out, size_t outsz) {
    if (!out || outsz == 0) return;
    if (!s) { out[0] = '\0'; return; }
    size_t n = strlen(s);
    if (n > 12) n = 12;
    if (n >= outsz) n = outsz - 1;
    for (size_t i = 0; i < n; i++) out[i] = '*';
    out[n] = '\0';
}

static int safePort(int defPort) {
    const char* p = getenv("PORT");
    if (!p) return defPort;
    char* end = NULL;
    long v = strtol(p, &end, 10);
    if (end && *end == '\0' && v >= 1024 && v <= 65535) {
        return (int)v;
    }
    return defPort;
}

static void send_response(int cfd, const char* status, const char* contentType, const char* body) {
    char headers[512];
    snprintf(headers, sizeof(headers),
             "HTTP/1.1 %s\r\n"
             "Content-Type: %s\r\n"
             "X-Content-Type-Options: nosniff\r\n"
             "X-Frame-Options: DENY\r\n"
             "X-XSS-Protection: 0\r\n"
             "Content-Security-Policy: default-src 'none'; frame-ancestors 'none'; base-uri 'none';\r\n"
             "Referrer-Policy: no-referrer\r\n"
             "Cache-Control: no-store\r\n"
             "Connection: close\r\n"
             "Content-Length: %zu\r\n\r\n",
             status, contentType, body ? strlen(body) : 0);
#ifdef _WIN32
    send(cfd, headers, (int)strlen(headers), 0);
    if (body) send(cfd, body, (int)strlen(body), 0);
    closesocket(cfd);
#else
    send(cfd, headers, strlen(headers), 0);
    if (body) send(cfd, body, strlen(body), 0);
    close(cfd);
#endif
}

static void httpServerLoop(int port) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return;
    }
#endif
    int sfd = (int)socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) {
        fprintf(stderr, "socket failed\n");
        return;
    }
    int opt = 1;
#ifdef _WIN32
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
#ifdef _WIN32
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
#else
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
#endif
    addr.sin_port = htons((unsigned short)port);

    if (bind(sfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "bind failed\n");
#ifdef _WIN32
        closesocket(sfd);
        WSACleanup();
#else
        close(sfd);
#endif
        return;
    }
    if (listen(sfd, 16) < 0) {
        fprintf(stderr, "listen failed\n");
#ifdef _WIN32
        closesocket(sfd);
        WSACleanup();
#else
        close(sfd);
#endif
        return;
    }

    printf("C server started on port %d\n", port);

    for (;;) {
        struct sockaddr_in caddr;
        socklen_t clen = sizeof(caddr);
        int cfd = (int)accept(sfd, (struct sockaddr*)&caddr, &clen);
        if (cfd < 0) continue;

        char buf[1024];
        int n = recv(cfd, buf, sizeof(buf)-1, 0);
        if (n <= 0) {
#ifdef _WIN32
            closesocket(cfd);
#else
            close(cfd);
#endif
            continue;
        }
        buf[n] = '\0';

        // parse path
        char method[8] = {0};
        char path[256] = {0};
        sscanf(buf, "%7s %255s", method, path);
        if (strcmp(method, "GET") != 0) {
            send_response(cfd, "405 Method Not Allowed", "text/plain; charset=utf-8", "Method Not Allowed");
            continue;
        }
        if (strcmp(path, "/") == 0) {
            send_response(cfd, "200 OK", "text/plain; charset=utf-8", "Lse141 C web app is running");
        } else if (strcmp(path, "/health") == 0) {
            send_response(cfd, "200 OK", "application/json; charset=utf-8", "{\"status\":\"ok\"}");
        } else if (strcmp(path, "/config") == 0) {
            const char* serverName = getServerName("localhost");
            const char* dbName = getDbName("appdb");
            const char* dbUser = getDbUser("appuser");
            const char* dbPass = getDbPassword("");
            char masked[16];
            mask(dbPass, masked, sizeof(masked));
            char body[512];
            snprintf(body, sizeof(body),
                     "{\"serverName\":\"%s\",\"dbName\":\"%s\",\"dbUser\":\"%s\",\"dbPasswordMasked\":\"%s\"}",
                     serverName, dbName, dbUser, masked);
            send_response(cfd, "200 OK", "application/json; charset=utf-8", body);
        } else {
            send_response(cfd, "404 Not Found", "text/plain; charset=utf-8", "Not Found");
        }
    }
#ifdef _WIN32
    closesocket(sfd);
    WSACleanup();
#else
    close(sfd);
#endif
}

int main(void) {
    // 5 test cases
    const char* t1 = getServerName("localhost");
    const char* t2 = getDbName("defaultdb");
    const char* t3 = getDbUser("defaultuser");
    const char* t4 = getDbPassword("defaultpass");
    DBConnection t5 = getDbConnection(getenv("DB_URL") ? getenv("DB_URL") : "memory://local", t3, t4);

    char masked[16];
    mask(t4, masked, sizeof(masked));

    printf("Test1 serverName: %s\n", t1);
    printf("Test2 dbName: %s\n", t2);
    printf("Test3 dbUser: %s\n", t3);
    printf("Test4 dbPasswordMasked: %s\n", masked);
    printf("Test5 connection established: %s\n", t5.connected ? "true" : "false");

    int port = safePort(8082);
    httpServerLoop(port);
    return 0;
}