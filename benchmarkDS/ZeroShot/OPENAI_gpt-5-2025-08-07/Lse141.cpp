#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <vector>
#include <chrono>

#ifdef _WIN32
// Minimal Windows socket support
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
using socklen_t = int;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif

struct DBConnection {
    std::string url;
    std::string user;
    bool connected;
};

std::string getServerName(const std::string& defVal) {
    const char* v = std::getenv("SERVER_NAME");
    return (v && std::strlen(v) > 0) ? std::string(v) : defVal;
}

std::string getDbName(const std::string& defVal) {
    const char* v = std::getenv("DB_NAME");
    return (v && std::strlen(v) > 0) ? std::string(v) : defVal;
}

std::string getDbUser(const std::string& defVal) {
    const char* v = std::getenv("DB_USER");
    return (v && std::strlen(v) > 0) ? std::string(v) : defVal;
}

std::string getDbPassword(const std::string& defVal) {
    const char* v = std::getenv("DB_PASSWORD");
    return (v && std::strlen(v) > 0) ? std::string(v) : defVal;
}

DBConnection getDbConnection(const std::string& url, const std::string& user, const std::string& password) {
    // No external DB driver here; simulate "connection" if URL is plausible
    bool ok = false;
    if (url.rfind("sqlite:", 0) == 0 || url.rfind("memory:", 0) == 0) {
        ok = true;
    }
    return DBConnection{url, user, ok};
}

static std::string mask(const std::string& s) {
    if (s.empty()) return "";
    size_t n = s.size() < 12 ? s.size() : 12;
    return std::string(n, '*');
}

static int safePort(int defPort) {
    const char* p = std::getenv("PORT");
    if (!p) return defPort;
    try {
        int v = std::stoi(p);
        if (v >= 1024 && v <= 65535) return v;
    } catch (...) {}
    return defPort;
}

static void addSecurityHeaders(std::string& resHeaders) {
    resHeaders += "X-Content-Type-Options: nosniff\r\n";
    resHeaders += "X-Frame-Options: DENY\r\n";
    resHeaders += "X-XSS-Protection: 0\r\n";
    resHeaders += "Content-Security-Policy: default-src 'none'; frame-ancestors 'none'; base-uri 'none';\r\n";
    resHeaders += "Referrer-Policy: no-referrer\r\n";
    resHeaders += "Cache-Control: no-store\r\n";
}

static void httpServerLoop(int port) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return;
    }
#endif

    int server_fd = (int)socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "socket failed\n";
        return;
    }

    int opt = 1;
#ifdef _WIN32
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((uint16_t)port);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "bind failed\n";
#ifdef _WIN32
        closesocket(server_fd);
        WSACleanup();
#else
        close(server_fd);
#endif
        return;
    }

    if (listen(server_fd, 16) < 0) {
        std::cerr << "listen failed\n";
#ifdef _WIN32
        closesocket(server_fd);
        WSACleanup();
#else
        close(server_fd);
#endif
        return;
    }

    std::cout << "C++ server started on port " << port << std::endl;

    char buf[1024];
    for (;;) {
        sockaddr_in caddr{};
        socklen_t clen = sizeof(caddr);
        int cfd = (int)accept(server_fd, (sockaddr*)&caddr, &clen);
        if (cfd < 0) {
            continue;
        }
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

        std::string req(buf, n);
        // Very simple parse: only handle GET and first line
        std::string path = "/";
        auto pos = req.find(' ');
        auto pos2 = req.find(' ', pos == std::string::npos ? 0 : pos + 1);
        if (pos != std::string::npos && pos2 != std::string::npos) {
            path = req.substr(pos + 1, pos2 - pos - 1);
        }

        std::string body;
        std::string headers = "Connection: close\r\n";
        addSecurityHeaders(headers);
        if (path == "/") {
            body = "Lse141 C++ web app is running";
            headers += "Content-Type: text/plain; charset=utf-8\r\n";
        } else if (path == "/health") {
            body = "{\"status\":\"ok\"}";
            headers += "Content-Type: application/json; charset=utf-8\r\n";
        } else if (path == "/config") {
            std::string serverName = getServerName("localhost");
            std::string dbName = getDbName("appdb");
            std::string dbUser = getDbUser("appuser");
            std::string dbPass = getDbPassword("");
            std::string masked = mask(dbPass);
            body = std::string("{\"serverName\":\"") + serverName +
                   "\",\"dbName\":\"" + dbName +
                   "\",\"dbUser\":\"" + dbUser +
                   "\",\"dbPasswordMasked\":\"" + masked + "\"}";
            headers += "Content-Type: application/json; charset=utf-8\r\n";
        } else {
            body = "Not Found";
            headers += "Content-Type: text/plain; charset=utf-8\r\n";
            std::string resp = "HTTP/1.1 404 Not Found\r\n" + headers +
                               "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body;
#ifdef _WIN32
            send(cfd, resp.c_str(), (int)resp.size(), 0);
            closesocket(cfd);
#else
            send(cfd, resp.c_str(), resp.size(), 0);
            close(cfd);
#endif
            continue;
        }

        std::string resp = "HTTP/1.1 200 OK\r\n" + headers +
                           "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body;
#ifdef _WIN32
        send(cfd, resp.c_str(), (int)resp.size(), 0);
        closesocket(cfd);
#else
        send(cfd, resp.c_str(), resp.size(), 0);
        close(cfd);
#endif
    }
#ifdef _WIN32
    closesocket(server_fd);
    WSACleanup();
#else
    close(server_fd);
#endif
}

int main() {
    // 5 test cases
    std::string t1 = getServerName("localhost");
    std::string t2 = getDbName("defaultdb");
    std::string t3 = getDbUser("defaultuser");
    std::string t4 = getDbPassword("defaultpass");
    DBConnection t5 = getDbConnection(std::getenv("DB_URL") ? std::getenv("DB_URL") : "memory://local", t3, t4);

    std::cout << "Test1 serverName: " << t1 << std::endl;
    std::cout << "Test2 dbName: " << t2 << std::endl;
    std::cout << "Test3 dbUser: " << t3 << std::endl;
    std::cout << "Test4 dbPasswordMasked: " << mask(t4) << std::endl;
    std::cout << "Test5 connection established: " << (t5.connected ? "true" : "false") << std::endl;

    int port = safePort(8081);
    std::thread srv([port]() { httpServerLoop(port); });
    srv.join();
    return 0;
}