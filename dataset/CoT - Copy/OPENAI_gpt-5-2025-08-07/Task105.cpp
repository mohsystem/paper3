#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>

static int connect_with_timeout(int sockfd, const struct sockaddr *addr, socklen_t addrlen, int timeout_ms) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags < 0) return -1;
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) return -1;

    int ret = ::connect(sockfd, addr, addrlen);
    if (ret == 0) {
        // Connected immediately
        fcntl(sockfd, F_SETFL, flags);
        return 0;
    }
    if (errno != EINPROGRESS) {
        fcntl(sockfd, F_SETFL, flags);
        return -1;
    }

    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(sockfd, &wfds);
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    ret = select(sockfd + 1, NULL, &wfds, NULL, &tv);
    if (ret <= 0) {
        // timeout or error
        fcntl(sockfd, F_SETFL, flags);
        errno = (ret == 0) ? ETIMEDOUT : errno;
        return -1;
    }

    int so_error = 0;
    socklen_t len = sizeof(so_error);
    if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len) < 0) {
        fcntl(sockfd, F_SETFL, flags);
        return -1;
    }
    if (so_error != 0) {
        fcntl(sockfd, F_SETFL, flags);
        errno = so_error;
        return -1;
    }

    // restore flags
    fcntl(sockfd, F_SETFL, flags);
    return 0;
}

static int dial_host(const std::string &host, int port, int timeout_ms) {
    struct addrinfo hints{};
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *res = nullptr;
    std::string portstr = std::to_string(port);
    int rc = getaddrinfo(host.c_str(), portstr.c_str(), &hints, &res);
    if (rc != 0) return -1;

    int sockfd = -1;
    for (struct addrinfo *p = res; p != nullptr; p = p->ai_next) {
        int s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (s < 0) continue;

        if (connect_with_timeout(s, p->ai_addr, p->ai_addrlen, timeout_ms) == 0) {
            // set I/O timeouts
            struct timeval tv;
            tv.tv_sec = timeout_ms / 1000;
            tv.tv_usec = (timeout_ms % 1000) * 1000;
            setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
            setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
            sockfd = s;
            break;
        }
        close(s);
    }
    freeaddrinfo(res);
    return sockfd;
}

std::string https_get(const std::string &host, int port, const std::string &path_in, int timeout_ms) {
    std::string path = path_in.empty() ? "/" : (path_in[0] == '/' ? path_in : "/" + path_in);
    if (port <= 0) port = 443;
    if (timeout_ms <= 0) timeout_ms = 10000;

    OPENSSL_init_ssl(0, nullptr);
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) return "ERROR: SSL_CTX_new failed";

    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);

    // Use system default trust store
    if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
        // Attempt common Unix path as a fallback (may still fail if not present)
        SSL_CTX_load_verify_locations(ctx, "/etc/ssl/certs/ca-certificates.crt", nullptr);
    }

    // Hostname verification
    X509_VERIFY_PARAM *param = SSL_CTX_get0_param(ctx);
    X509_VERIFY_PARAM_set_hostflags(param, 0);
    X509_VERIFY_PARAM_set1_host(param, host.c_str(), 0);

    int sockfd = dial_host(host, port, timeout_ms);
    if (sockfd < 0) {
        SSL_CTX_free(ctx);
        return std::string("ERROR: connect failed: ") + strerror(errno);
    }

    SSL *ssl = SSL_new(ctx);
    if (!ssl) {
        close(sockfd);
        SSL_CTX_free(ctx);
        return "ERROR: SSL_new failed";
    }

    // SNI
    if (SSL_set_tlsext_host_name(ssl, host.c_str()) != 1) {
        SSL_free(ssl);
        close(sockfd);
        SSL_CTX_free(ctx);
        return "ERROR: SNI setup failed";
    }

    SSL_set_fd(ssl, sockfd);

    if (SSL_connect(ssl) != 1) {
        unsigned long err = ERR_get_error();
        char buf[256];
        ERR_error_string_n(err, buf, sizeof(buf));
        SSL_free(ssl);
        close(sockfd);
        SSL_CTX_free(ctx);
        return std::string("ERROR: SSL_connect failed: ") + buf;
    }

    long verify = SSL_get_verify_result(ssl);
    if (verify != X509_V_OK) {
        SSL_free(ssl);
        close(sockfd);
        SSL_CTX_free(ctx);
        return "ERROR: certificate verify failed";
    }

    std::string req = "GET " + path + " HTTP/1.1\r\n"
                      "Host: " + host + "\r\n"
                      "User-Agent: Task105-CPP/1.0\r\n"
                      "Accept: */*\r\n"
                      "Connection: close\r\n\r\n";

    if (SSL_write(ssl, req.c_str(), (int)req.size()) <= 0) {
        SSL_free(ssl);
        close(sockfd);
        SSL_CTX_free(ctx);
        return "ERROR: SSL_write failed";
    }

    // Read status line
    std::string status;
    char ch;
    bool got_cr = false;
    while (true) {
        int n = SSL_read(ssl, &ch, 1);
        if (n <= 0) break;
        if (ch == '\r') {
            got_cr = true;
            continue;
        }
        if (ch == '\n') break;
        if (got_cr) {
            // unexpected sequence, treat as normal char
            got_cr = false;
        }
        status.push_back(ch);
        if (status.size() > 8192) break; // avoid excessive growth
    }
    if (status.empty()) status = "ERROR: No response received";

    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sockfd);
    SSL_CTX_free(ctx);
    return status;
}

int main() {
    std::vector<std::tuple<std::string,int,std::string>> tests = {
        {"www.google.com", 443, "/"},
        {"www.github.com", 443, "/"},
        {"www.cloudflare.com", 443, "/"},
        {"www.wikipedia.org", 443, "/"},
        {"example.com", 443, "/"}
    };
    for (const auto &t : tests) {
        std::string host, path;
        int port;
        std::tie(host, port, path) = t;
        try {
            std::string res = https_get(host, port, path, 10000);
            std::cout << host << ": " << res << std::endl;
        } catch (const std::exception &e) {
            std::cout << host << ": ERROR - " << e.what() << std::endl;
        }
    }
    return 0;
}