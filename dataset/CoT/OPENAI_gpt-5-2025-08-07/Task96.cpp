#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <atomic>
#include <cstring>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

/*
Chain-of-Through process:
1. Understand: TCP echo server listening on specified port, echo back messages.
2. Security: Bind to localhost, timeouts, limit input length, handle errors, close descriptors.
3. Implementation: Accept loop in thread; per-connection handler with bounded reads and timeouts.
4. Review: Ensure RAII-like cleanup and robust error checks.
5. Output: Final secure code.
*/

static const int MAX_LINE = 8192;
static const int BACKLOG = 50;

class EchoServerCPP {
public:
    EchoServerCPP() : running(false), listen_fd(-1) {}
    ~EchoServerCPP() { stop(); }

    int start(int port) {
        if (running.load()) {
            throw std::runtime_error("Server already running");
        }
        listen_fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (listen_fd < 0) throw std::runtime_error("socket failed");

        int opt = 1;
        ::setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in addr {};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = ::inet_addr("127.0.0.1");
        addr.sin_port = htons(port);

        if (::bind(listen_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
            ::close(listen_fd);
            listen_fd = -1;
            throw std::runtime_error("bind failed");
        }
        if (::listen(listen_fd, BACKLOG) < 0) {
            ::close(listen_fd);
            listen_fd = -1;
            throw std::runtime_error("listen failed");
        }

        // Get actual port
        socklen_t len = sizeof(addr);
        if (::getsockname(listen_fd, (sockaddr*)&addr, &len) == 0) {
            actual_port = ntohs(addr.sin_port);
        } else {
            ::close(listen_fd);
            listen_fd = -1;
            throw std::runtime_error("getsockname failed");
        }

        running.store(true);
        accept_thread = std::thread(&EchoServerCPP::acceptLoop, this);
        return actual_port;
    }

    void stop() {
        if (!running.exchange(false)) return;
        if (listen_fd >= 0) {
            ::shutdown(listen_fd, SHUT_RDWR);
            ::close(listen_fd);
            listen_fd = -1;
        }
        if (accept_thread.joinable()) {
            accept_thread.join();
        }
        // Detached client threads may still run briefly; process exit cleans them.
    }

    int port() const { return actual_port; }

private:
    static bool readLineLimited(int fd, std::string& out) {
        out.clear();
        char ch;
        while (true) {
            ssize_t n = ::recv(fd, &ch, 1, 0);
            if (n == 0) {
                return !out.empty();
            } else if (n < 0) {
                if (errno == EINTR) continue;
                return false;
            }
            if (ch == '\n') break;
            if (ch != '\r') {
                if ((int)out.size() >= MAX_LINE) {
                    errno = EMSGSIZE;
                    return false;
                }
                out.push_back(ch);
            }
        }
        return true;
    }

    static void handleClient(int fd) {
        // Set timeouts
        timeval tv{};
        tv.tv_sec = 30;
        tv.tv_usec = 0;
        ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        ::setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

        std::string line;
        while (true) {
            bool ok = readLineLimited(fd, line);
            if (!ok) break;
            std::string resp = line + "\n";
            size_t sent = 0;
            while (sent < resp.size()) {
                ssize_t n = ::send(fd, resp.data() + sent, resp.size() - sent, 0);
                if (n < 0) {
                    if (errno == EINTR) continue;
                    ::close(fd);
                    return;
                }
                sent += static_cast<size_t>(n);
            }
        }
        ::close(fd);
    }

    void acceptLoop() {
        while (running.load()) {
            sockaddr_in cli{};
            socklen_t clilen = sizeof(cli);
            int cfd = ::accept(listen_fd, (sockaddr*)&cli, &clilen);
            if (cfd < 0) {
                if (!running.load()) break;
                if (errno == EINTR) continue;
                // brief sleep on error to avoid busy loop
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            std::thread(&EchoServerCPP::handleClient, cfd).detach();
        }
    }

    std::atomic<bool> running;
    int listen_fd;
    int actual_port{0};
    std::thread accept_thread;
};

static std::string echo_client(const std::string& host, int port, const std::string& message) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) throw std::runtime_error("socket failed");

    timeval tv{};
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    ::setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (::inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1) {
        ::close(fd);
        throw std::runtime_error("inet_pton failed");
    }

    if (::connect(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        ::close(fd);
        throw std::runtime_error("connect failed");
    }

    std::string data = message + "\n";
    size_t sent = 0;
    while (sent < data.size()) {
        ssize_t n = ::send(fd, data.data() + sent, data.size() - sent, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            ::close(fd);
            throw std::runtime_error("send failed");
        }
        sent += static_cast<size_t>(n);
    }

    // read until newline
    std::string out;
    char ch;
    while (true) {
        ssize_t n = ::recv(fd, &ch, 1, 0);
        if (n == 0) break;
        if (n < 0) {
            if (errno == EINTR) continue;
            ::close(fd);
            throw std::runtime_error("recv failed");
        }
        if (ch == '\n') break;
        if (ch != '\r') {
            if ((int)out.size() >= MAX_LINE) {
                ::close(fd);
                throw std::runtime_error("response too long");
            }
            out.push_back(ch);
        }
    }

    ::close(fd);
    return out;
}

int main() {
    EchoServerCPP server;
    int port = server.start(0);
    std::string host = "127.0.0.1";

    std::vector<std::string> tests = {
        "Hello",
        "",
        "The quick brown fox jumps over the lazy dog",
        "1234567890!@#$%^&*()_+",
        "Unicode likely echoed as bytes in C++ client"
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        try {
            std::string resp = echo_client(host, port, tests[i]);
            std::cout << "Test " << (i + 1) << " -> " << resp << std::endl;
        } catch (const std::exception& ex) {
            std::cerr << "Test " << (i + 1) << " error: " << ex.what() << std::endl;
        }
    }

    server.stop();
    return 0;
}