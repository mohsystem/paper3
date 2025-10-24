#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <atomic>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

static const size_t MAX_MESSAGE_BYTES = 4096;
static const int BACKLOG = 50;
static const int SOCKET_TIMEOUT_SEC = 5;

struct ServerHandle {
    int listen_fd;
    std::atomic<bool> running;
    std::thread accept_thread;
    uint16_t port;

    ServerHandle(int fd, uint16_t p)
        : listen_fd(fd), running(true), accept_thread(), port(p) {}
    ServerHandle(const ServerHandle&) = delete;
    ServerHandle& operator=(const ServerHandle&) = delete;
    ServerHandle(ServerHandle&& other) noexcept
        : listen_fd(other.listen_fd),
          running(other.running.load()),
          accept_thread(std::move(other.accept_thread)),
          port(other.port) {
        other.listen_fd = -1;
        other.running = false;
    }
    ~ServerHandle() = default;
};

static bool set_socket_timeouts(int fd) {
    timeval tv;
    tv.tv_sec = SOCKET_TIMEOUT_SEC;
    tv.tv_usec = 0;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) != 0) return false;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) != 0) return false;
    return true;
}

static void close_fd_safe(int fd) {
    if (fd >= 0) close(fd);
}

static bool read_line_limited(int fd, std::string& out) {
    out.clear();
    char ch;
    size_t count = 0;
    while (count < MAX_MESSAGE_BYTES) {
        ssize_t n = recv(fd, &ch, 1, 0);
        if (n == 0) {
            return !out.empty(); // EOF
        } else if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        if (ch == '\n') break;
        if (ch == '\r') continue;
        out.push_back(ch);
        count++;
    }
    return true;
}

static void client_handler(int client_fd) {
    if (!set_socket_timeouts(client_fd)) {
        close_fd_safe(client_fd);
        return;
    }
    std::string line;
    while (true) {
        bool ok = read_line_limited(client_fd, line);
        if (!ok) break;
        std::string reply = line;
        reply.push_back('\n');
        size_t sent = 0;
        while (sent < reply.size()) {
            ssize_t n = send(client_fd, reply.data() + sent, reply.size() - sent, 0);
            if (n < 0) {
                if (errno == EINTR) continue;
                sent = reply.size();
                break;
            }
            sent += static_cast<size_t>(n);
        }
        if (!ok) break;
    }
    close_fd_safe(client_fd);
}

std::unique_ptr<ServerHandle> start_echo_server(uint16_t port) {
    if (port > 65535) {
        throw std::runtime_error("Port out of range");
    }
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) throw std::runtime_error("socket() failed");
    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1
    addr.sin_port = htons(port);

    if (bind(fd, (sockaddr*)&addr, sizeof(addr)) != 0) {
        close_fd_safe(fd);
        throw std::runtime_error("bind() failed");
    }
    if (listen(fd, BACKLOG) != 0) {
        close_fd_safe(fd);
        throw std::runtime_error("listen() failed");
    }
    // Get assigned port (for port 0)
    socklen_t len = sizeof(addr);
    if (getsockname(fd, (sockaddr*)&addr, &len) != 0) {
        close_fd_safe(fd);
        throw std::runtime_error("getsockname() failed");
    }
    uint16_t assigned_port = ntohs(addr.sin_port);

    std::unique_ptr<ServerHandle> handle(new ServerHandle(fd, assigned_port));
    handle->accept_thread = std::thread([h = handle.get()]() {
        while (h->running.load()) {
            sockaddr_in caddr{};
            socklen_t clen = sizeof(caddr);
            int cfd = accept(h->listen_fd, (sockaddr*)&caddr, &clen);
            if (cfd < 0) {
                if (errno == EINTR) continue;
                if (!h->running.load()) break;
                continue;
            }
            std::thread(client_handler, cfd).detach();
        }
    });
    return handle;
}

void stop_echo_server(ServerHandle& handle) {
    handle.running.store(false);
    shutdown(handle.listen_fd, SHUT_RDWR);
    close_fd_safe(handle.listen_fd);
    if (handle.accept_thread.joinable()) {
        handle.accept_thread.join();
    }
}

std::string send_echo(const std::string& host, uint16_t port, const std::string& message) {
    if (host.empty()) throw std::runtime_error("Host required");
    if (port == 0 || port > 65535) throw std::runtime_error("Port out of range");
    if (message.size() > MAX_MESSAGE_BYTES) throw std::runtime_error("Message too long");

    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) throw std::runtime_error("socket() failed");
    set_socket_timeouts(fd);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1) {
        close_fd_safe(fd);
        throw std::runtime_error("Invalid host");
    }
    if (connect(fd, (sockaddr*)&addr, sizeof(addr)) != 0) {
        close_fd_safe(fd);
        throw std::runtime_error("connect() failed");
    }

    std::string tosend = message;
    tosend.push_back('\n');
    size_t sent = 0;
    while (sent < tosend.size()) {
        ssize_t n = send(fd, tosend.data() + sent, tosend.size() - sent, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            close_fd_safe(fd);
            throw std::runtime_error("send() failed");
        }
        sent += static_cast<size_t>(n);
    }

    std::string reply;
    if (!read_line_limited(fd, reply)) {
        close_fd_safe(fd);
        throw std::runtime_error("read failed");
    }
    close_fd_safe(fd);
    return reply;
}

int main() {
    try {
        auto handle = start_echo_server(0);
        uint16_t port = handle->port;
        std::vector<std::string> tests = {
            "Hello, world!",
            "",
            "The quick brown fox jumps over 13 lazy dogs.",
            "Symbols: !@#$%^&*()_+-=[]{}|;:',.<>/?",
            "One more test line"
        };
        for (const auto& t : tests) {
            std::string r = send_echo("127.0.0.1", port, t);
            std::cout << "Sent: [" << t << "] | Echoed: [" << r << "]\n";
        }
        stop_echo_server(*handle);
    } catch (const std::exception& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
    }
    return 0;
}