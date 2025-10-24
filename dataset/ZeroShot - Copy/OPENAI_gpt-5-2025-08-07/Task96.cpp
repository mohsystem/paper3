#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

class EchoServer {
public:
    explicit EchoServer(uint16_t port, int maxClients = 16)
        : server_fd_(-1), running_(false), maxClients_(maxClients) {
        if (maxClients_ <= 0 || maxClients_ > 1024) {
            throw std::runtime_error("Invalid maxClients");
        }
        server_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd_ < 0) throw std::runtime_error("socket failed");

        int yes = 1;
        ::setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = ::inet_addr("127.0.0.1");
        addr.sin_port = htons(port);

        if (::bind(server_fd_, (sockaddr*)&addr, sizeof(addr)) < 0) {
            ::close(server_fd_);
            throw std::runtime_error("bind failed");
        }
        if (::listen(server_fd_, 50) < 0) {
            ::close(server_fd_);
            throw std::runtime_error("listen failed");
        }

        // get assigned port
        socklen_t len = sizeof(addr);
        if (::getsockname(server_fd_, (sockaddr*)&addr, &len) == 0) {
            port_ = ntohs(addr.sin_port);
        } else {
            port_ = port;
        }
    }

    ~EchoServer() {
        stop();
    }

    uint16_t port() const { return port_; }

    void start() {
        running_.store(true);
        accept_thread_ = std::thread(&EchoServer::acceptLoop, this);
    }

    void stop() {
        bool expected = true;
        if (!running_.compare_exchange_strong(expected, false)) {
            return; // already stopped
        }
        ::shutdown(server_fd_, SHUT_RDWR);
        ::close(server_fd_);
        if (accept_thread_.joinable()) accept_thread_.join();
    }

private:
    static constexpr size_t MAX_LEN = 8192;

    int server_fd_;
    uint16_t port_{0};
    std::atomic<bool> running_;
    std::thread accept_thread_;
    int maxClients_;
    std::atomic<int> activeClients_{0};

    static void setSocketTimeouts(int fd, int seconds) {
        timeval tv{};
        tv.tv_sec = seconds;
        tv.tv_usec = 0;
        ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        ::setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    }

    static void handleClient(int client_fd) {
        setSocketTimeouts(client_fd, 5);
        std::vector<char> buffer;
        buffer.reserve(256);
        char temp[512];
        bool done = false;
        size_t total = 0;
        while (!done && total < MAX_LEN) {
            ssize_t n = ::recv(client_fd, temp, sizeof(temp), 0);
            if (n <= 0) break;
            size_t copyN = 0;
            for (ssize_t i = 0; i < n; ++i) {
                if (temp[i] == '\n') {
                    done = true;
                    break;
                }
                ++copyN;
            }
            size_t toCopy = copyN;
            if (total + toCopy > MAX_LEN) toCopy = MAX_LEN - total;
            buffer.insert(buffer.end(), temp, temp + toCopy);
            total += toCopy;
            if (done) break;
        }
        // Echo back
        size_t sent = 0;
        while (sent < buffer.size()) {
            ssize_t m = ::send(client_fd, buffer.data() + sent, buffer.size() - sent, 0);
            if (m <= 0) break;
            sent += static_cast<size_t>(m);
        }
        ::shutdown(client_fd, SHUT_RDWR);
        ::close(client_fd);
    }

    void acceptLoop() {
        while (running_.load()) {
            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(server_fd_, &rfds);
            timeval tv{};
            tv.tv_sec = 1;
            tv.tv_usec = 0;
            int rv = ::select(server_fd_ + 1, &rfds, nullptr, nullptr, &tv);
            if (rv <= 0) continue;
            if (!FD_ISSET(server_fd_, &rfds)) continue;

            sockaddr_in cli{};
            socklen_t clilen = sizeof(cli);
            int cfd = ::accept(server_fd_, (sockaddr*)&cli, &clilen);
            if (cfd < 0) continue;

            int current = activeClients_.load();
            if (current >= maxClients_) {
                ::close(cfd);
                continue;
            }
            activeClients_.fetch_add(1);
            std::thread([cfd, this]() {
                handleClient(cfd);
                activeClients_.fetch_sub(1);
            }).detach();
        }
    }
};

static std::string echo_client(const std::string& host, uint16_t port, const std::string& message) {
    if (host.empty() || port == 0) throw std::runtime_error("Invalid host/port");

    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) throw std::runtime_error("socket failed");

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (::inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1) {
        ::close(fd);
        throw std::runtime_error("inet_pton failed");
    }

    timeval tv{};
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    ::setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    if (::connect(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        ::close(fd);
        throw std::runtime_error("connect failed");
    }

    std::string data = message;
    if (data.size() > 8192) data.resize(8192);
    std::string sendbuf = data + "\n";
    size_t sent = 0;
    while (sent < sendbuf.size()) {
        ssize_t n = ::send(fd, sendbuf.data() + sent, sendbuf.size() - sent, 0);
        if (n <= 0) break;
        sent += static_cast<size_t>(n);
    }
    ::shutdown(fd, SHUT_WR);

    std::string out;
    out.reserve(data.size());
    char buf[512];
    ssize_t n;
    while ((n = ::recv(fd, buf, sizeof(buf), 0)) > 0) {
        out.append(buf, buf + n);
        if (out.size() > 8192) break;
    }
    ::close(fd);
    return out;
}

int main() {
    try {
        EchoServer server(0, 16);
        server.start();
        uint16_t p = server.port();

        std::string tests[5] = {
            "hello",
            "test message",
            "",
            std::string(100, 'A'),
            "Line1\\nLine2 with symbols !@#$%^&*()_+"
        };
        for (int i = 0; i < 5; ++i) {
            std::string resp = echo_client("127.0.0.1", p, tests[i]);
            std::cout << "Case " << (i + 1) << ": " << resp << std::endl;
        }
        server.stop();
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}