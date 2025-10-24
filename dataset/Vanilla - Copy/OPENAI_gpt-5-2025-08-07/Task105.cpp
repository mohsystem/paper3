#include <openssl/ssl.h>
#include <openssl/err.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <sstream>
#include <iostream>

static int connect_with_timeout(const std::string& host, int port, int timeout_sec) {
    struct addrinfo hints{}, *res=nullptr, *p=nullptr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    char portstr[16];
    snprintf(portstr, sizeof(portstr), "%d", port);
    if (getaddrinfo(host.c_str(), portstr, &hints, &res) != 0) {
        return -1;
    }

    int sockfd = -1;
    for (p = res; p != nullptr; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd < 0) continue;

        int flags = fcntl(sockfd, F_GETFL, 0);
        fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

        int rc = connect(sockfd, p->ai_addr, p->ai_addrlen);
        if (rc < 0 && errno == EINPROGRESS) {
            fd_set wfds;
            FD_ZERO(&wfds);
            FD_SET(sockfd, &wfds);
            struct timeval tv;
            tv.tv_sec = timeout_sec;
            tv.tv_usec = 0;
            rc = select(sockfd + 1, nullptr, &wfds, nullptr, &tv);
            if (rc > 0) {
                int so_error = 0;
                socklen_t len = sizeof(so_error);
                getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);
                if (so_error == 0) {
                    fcntl(sockfd, F_SETFL, flags);
                    break; // success
                } else {
                    close(sockfd);
                    sockfd = -1;
                    continue;
                }
            } else {
                close(sockfd);
                sockfd = -1;
                continue;
            }
        } else if (rc == 0) {
            fcntl(sockfd, F_SETFL, flags);
            break; // success
        } else {
            close(sockfd);
            sockfd = -1;
            continue;
        }
    }
    freeaddrinfo(res);
    return sockfd;
}

std::string connectTLS(const std::string& host, int port, int timeoutSeconds) {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        return "ERROR: SSL_CTX_new failed";
    }

    SSL_CTX_set_default_verify_paths(ctx);
    SSL* ssl = nullptr;
    int sock = -1;
    std::string result;

    do {
        sock = connect_with_timeout(host, port, timeoutSeconds);
        if (sock < 0) {
            result = "ERROR: TCP connect failed";
            break;
        }

        ssl = SSL_new(ctx);
        if (!ssl) {
            result = "ERROR: SSL_new failed";
            break;
        }

        SSL_set_tlsext_host_name(ssl, host.c_str());
        X509_VERIFY_PARAM* param = SSL_get0_param(ssl);
        X509_VERIFY_PARAM_set_hostflags(param, 0);
        X509_VERIFY_PARAM_set1_host(param, host.c_str(), 0);
        SSL_set_verify(ssl, SSL_VERIFY_PEER, nullptr);

        SSL_set_fd(ssl, sock);

        if (SSL_connect(ssl) != 1) {
            unsigned long e = ERR_get_error();
            result = std::string("ERROR: SSL_connect failed: ") + (e ? ERR_error_string(e, nullptr) : "unknown");
            break;
        }

        const char* cipher = SSL_get_cipher(ssl);
        const char* proto = SSL_get_version(ssl);
        X509* cert = SSL_get_peer_certificate(ssl);
        std::string subject, issuer;
        if (cert) {
            char subj[1024];
            char issr[1024];
            X509_NAME_oneline(X509_get_subject_name(cert), subj, sizeof(subj));
            X509_NAME_oneline(X509_get_issuer_name(cert), issr, sizeof(issr));
            subject = subj;
            issuer = issr;
            X509_free(cert);
        } else {
            subject = "unverified";
            issuer = "unverified";
        }

        std::ostringstream oss;
        oss << "OK " << host << ":" << port
            << " protocol=" << (proto ? proto : "")
            << " cipher=" << (cipher ? cipher : "")
            << " subject=" << subject
            << " issuer=" << issuer;
        result = oss.str();

    } while (false);

    if (ssl) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    if (sock >= 0) close(sock);
    SSL_CTX_free(ctx);
    EVP_cleanup();

    return result;
}

int main() {
    std::pair<std::string,int> tests[5] = {
        {"www.google.com", 443},
        {"www.cloudflare.com", 443},
        {"www.github.com", 443},
        {"www.stackoverflow.com", 443},
        {"expired.badssl.com", 443}
    };
    for (auto &t : tests) {
        std::cout << connectTLS(t.first, t.second, 8) << std::endl;
    }
    return 0;
}