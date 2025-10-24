#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include <cerrno>
#include <memory>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <arpa/inet.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>

class Task105 {
public:
    static std::string establish_secure_connection(const std::string& host, int port, int timeout_ms) {
        if (host.empty()) return "ERROR: Invalid host";
        if (port <= 0 || port > 65535) return "ERROR: Invalid port";
        if (timeout_ms <= 0) return "ERROR: Invalid timeout";

        SSL_load_error_strings();
        OpenSSL_add_ssl_algorithms();

        SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
        if (!ctx) {
            return "ERROR: SSL_CTX_new failed";
        }

        std::string result;
        do {
            if (SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION) != 1) {
                result = "ERROR: Failed to set min TLS version";
                break;
            }

            SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
            if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
                result = "ERROR: Failed to load default CA paths";
                break;
            }

            // Resolve host
            struct addrinfo hints{};
            memset(&hints, 0, sizeof(hints));
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_family = AF_UNSPEC;

            struct addrinfo* res = nullptr;
            std::string portstr = std::to_string(port);
            int gai = getaddrinfo(host.c_str(), portstr.c_str(), &hints, &res);
            if (gai != 0) {
                result = std::string("ERROR: getaddrinfo: ") + gai_strerror(gai);
                break;
            }

            int fd = -1;
            for (struct addrinfo* p = res; p != nullptr; p = p->ai_next) {
                fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
                if (fd < 0) continue;

                // Non-blocking connect with timeout
                int flags = fcntl(fd, F_GETFL, 0);
                if (flags < 0) { close(fd); fd = -1; continue; }
                if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) { close(fd); fd = -1; continue; }

                int rc = ::connect(fd, p->ai_addr, p->ai_addrlen);
                if (rc < 0 && errno != EINPROGRESS) {
                    close(fd);
                    fd = -1;
                    continue;
                }

                if (rc == 0) {
                    // Connected immediately
                } else {
                    fd_set wfds;
                    FD_ZERO(&wfds);
                    FD_SET(fd, &wfds);
                    struct timeval tv;
                    tv.tv_sec = timeout_ms / 1000;
                    tv.tv_usec = (timeout_ms % 1000) * 1000;

                    rc = select(fd + 1, nullptr, &wfds, nullptr, &tv);
                    if (rc <= 0) {
                        close(fd);
                        fd = -1;
                        continue;
                    }
                    int so_error = 0;
                    socklen_t slen = sizeof(so_error);
                    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &so_error, &slen) < 0 || so_error != 0) {
                        close(fd);
                        fd = -1;
                        continue;
                    }
                }

                // Back to blocking
                fcntl(fd, F_SETFL, flags);

                // Set reasonable timeouts for IO
                struct timeval rcv;
                rcv.tv_sec = timeout_ms / 1000;
                rcv.tv_usec = (timeout_ms % 1000) * 1000;
                setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &rcv, sizeof(rcv));
                setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &rcv, sizeof(rcv));

                // Now perform TLS handshake
                SSL* ssl = SSL_new(ctx);
                if (!ssl) {
                    close(fd);
                    fd = -1;
                    continue;
                }

                // SNI
                if (SSL_set_tlsext_host_name(ssl, host.c_str()) != 1) {
                    SSL_free(ssl);
                    close(fd);
                    fd = -1;
                    continue;
                }

                // Hostname verification
                if (SSL_set1_host(ssl, host.c_str()) != 1) {
                    SSL_free(ssl);
                    close(fd);
                    fd = -1;
                    continue;
                }

                if (SSL_set_fd(ssl, fd) != 1) {
                    SSL_free(ssl);
                    close(fd);
                    fd = -1;
                    continue;
                }

                rc = SSL_connect(ssl);
                if (rc != 1) {
                    SSL_free(ssl);
                    close(fd);
                    fd = -1;
                    continue;
                }

                const char* proto = SSL_get_version(ssl);
                const char* cipher = SSL_get_cipher_name(ssl);

                std::string subject;
                X509* cert = SSL_get_peer_certificate(ssl);
                if (cert) {
                    char* subj = X509_NAME_oneline(X509_get_subject_name(cert), nullptr, 0);
                    if (subj) {
                        subject = subj;
                        OPENSSL_free(subj);
                    }
                    X509_free(cert);
                }

                std::ostringstream oss;
                oss << "OK host=" << host
                    << " port=" << port
                    << " protocol=" << (proto ? proto : "unknown")
                    << " cipher=" << (cipher ? cipher : "unknown")
                    << " subject=\"" << subject << "\"";
                result = oss.str();

                SSL_shutdown(ssl);
                SSL_free(ssl);
                close(fd);
                break; // done for the first successful connection
            }

            freeaddrinfo(res);
            if (result.empty()) {
                result = "ERROR: Connection/Handshake failed";
            }
        } while (false);

        SSL_COMP_free_compression_methods(); // no-op on newer OpenSSL
        EVP_cleanup();
        ERR_free_strings();

        return result;
    }
};

int main() {
    std::vector<std::string> hosts = {
        "www.google.com",
        "www.cloudflare.com",
        "www.github.com",
        "www.wikipedia.org",
        "api.openai.com"
    };
    int port = 443;
    int timeout_ms = 8000;

    for (const auto& h : hosts) {
        std::cout << Task105::establish_secure_connection(h, port, timeout_ms) << std::endl;
    }
    return 0;
}