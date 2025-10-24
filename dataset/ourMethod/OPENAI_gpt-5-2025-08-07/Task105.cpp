#include <openssl/ssl.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>

static int connect_with_timeout(const std::string& host, int port, int timeout_ms) {
    struct addrinfo hints{};
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* res = nullptr;
    std::string port_str = std::to_string(port);
    if (getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res) != 0) {
        return -1;
    }
    int sockfd = -1;
    for (struct addrinfo* rp = res; rp != nullptr; rp = rp->ai_next) {
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd < 0) continue;

        int flags = fcntl(sockfd, F_GETFL, 0);
        if (flags < 0) { close(sockfd); sockfd = -1; continue; }
        if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) { close(sockfd); sockfd = -1; continue; }

        int ret = connect(sockfd, rp->ai_addr, rp->ai_addrlen);
        if (ret == 0) {
            // Connected immediately
        } else if (errno == EINPROGRESS) {
            fd_set wfds;
            FD_ZERO(&wfds);
            FD_SET(sockfd, &wfds);
            struct timeval tv{};
            tv.tv_sec = timeout_ms / 1000;
            tv.tv_usec = (timeout_ms % 1000) * 1000;
            ret = select(sockfd + 1, nullptr, &wfds, nullptr, &tv);
            if (ret <= 0) { close(sockfd); sockfd = -1; continue; }
            int so_error = 0;
            socklen_t len = sizeof(so_error);
            if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len) < 0 || so_error != 0) {
                close(sockfd); sockfd = -1; continue;
            }
        } else {
            close(sockfd);
            sockfd = -1;
            continue;
        }

        // set back to blocking
        fcntl(sockfd, F_SETFL, flags);

        // set timeouts for send/recv
        struct timeval tv{};
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

        break;
    }
    freeaddrinfo(res);
    return sockfd;
}

static std::string tls_http_get(const std::string& host, int port, const std::string& path, int timeout_ms, int max_bytes) {
    if (host.empty()) return "ERROR: invalid host";
    if (port < 1 || port > 65535) return "ERROR: invalid port";
    std::string reqPath = path;
    if (reqPath.empty() || reqPath[0] != '/') reqPath = "/";

    if (max_bytes <= 0) max_bytes = 2048;
    if (timeout_ms <= 0) timeout_ms = 5000;

    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) return "ERROR: SSL_CTX_new failed";

    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
    SSL_CTX_set_default_verify_paths(ctx);
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);

    int fd = connect_with_timeout(host, port, timeout_ms);
    if (fd < 0) {
        SSL_CTX_free(ctx);
        return "ERROR: TCP connect failed";
    }

    SSL* ssl = SSL_new(ctx);
    if (!ssl) {
        close(fd);
        SSL_CTX_free(ctx);
        return "ERROR: SSL_new failed";
    }
    if (!SSL_set_tlsext_host_name(ssl, host.c_str())) {
        SSL_free(ssl);
        close(fd);
        SSL_CTX_free(ctx);
        return "ERROR: SNI set failed";
    }
    SSL_set_fd(ssl, fd);

    if (SSL_connect(ssl) != 1) {
        SSL_free(ssl);
        close(fd);
        SSL_CTX_free(ctx);
        return "ERROR: TLS handshake failed";
    }

    // Verify certificate and hostname
    long verify_res = SSL_get_verify_result(ssl);
    if (verify_res != X509_V_OK) {
        SSL_free(ssl);
        close(fd);
        SSL_CTX_free(ctx);
        return "ERROR: certificate verify failed";
    }
    X509* cert = SSL_get_peer_certificate(ssl);
    if (!cert) {
        SSL_free(ssl);
        close(fd);
        SSL_CTX_free(ctx);
        return "ERROR: no server certificate";
    }
    int host_ok = X509_check_host(cert, host.c_str(), host.size(), 0, nullptr);
    X509_free(cert);
    if (host_ok != 1) {
        SSL_free(ssl);
        close(fd);
        SSL_CTX_free(ctx);
        return "ERROR: hostname verification failed";
    }

    std::string request = "GET " + reqPath + " HTTP/1.1\r\n"
                          "Host: " + host + "\r\n"
                          "User-Agent: SecureClient/1.0\r\n"
                          "Accept: */*\r\n"
                          "Connection: close\r\n\r\n";
    if (SSL_write(ssl, request.data(), (int)request.size()) <= 0) {
        SSL_free(ssl);
        close(fd);
        SSL_CTX_free(ctx);
        return "ERROR: write failed";
    }

    std::string out;
    out.reserve(std::min(max_bytes, 8192));
    std::vector<unsigned char> buf(4096);
    int remaining = max_bytes;
    while (remaining > 0) {
        int toRead = std::min<int>((int)buf.size(), remaining);
        int n = SSL_read(ssl, buf.data(), toRead);
        if (n <= 0) break;
        out.append(reinterpret_cast<const char*>(buf.data()), n);
        remaining -= n;
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(fd);
    SSL_CTX_free(ctx);
    EVP_cleanup();
    ERR_free_strings();

    return out;
}

int main() {
    std::vector<std::string> hosts = {
        "example.com",
        "www.google.com",
        "www.cloudflare.com",
        "www.github.com",
        "www.wikipedia.org"
    };
    for (const auto& h : hosts) {
        std::string resp = tls_http_get(h, 443, "/", 8000, 2048);
        std::string first = "ERROR: no response";
        if (!resp.empty() && resp.rfind("ERROR:", 0) != 0) {
            auto pos = resp.find("\r\n");
            first = (pos != std::string::npos) ? resp.substr(0, pos) : resp.substr(0, std::min<size_t>(120, resp.size()));
        } else {
            first = resp;
        }
        std::cout << h << " -> " << first << std::endl;
    }
    return 0;
}