#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

static void echo_client_handler(int client_fd) {
    char buf[4096];
    while (true) {
        ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
        if (n <= 0) break;
        ssize_t sent = 0;
        while (sent < n) {
            ssize_t m = send(client_fd, buf + sent, n - sent, 0);
            if (m <= 0) { n = -1; break; }
            sent += m;
        }
        if (n <= 0) break;
    }
    close(client_fd);
}

static void echo_server_loop(int listen_fd, std::atomic<bool>& stop_flag) {
    while (!stop_flag.load()) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(listen_fd, &rfds);
        timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 500000; // 0.5s
        int rv = select(listen_fd + 1, &rfds, nullptr, nullptr, &tv);
        if (rv < 0) {
            if (errno == EINTR) continue;
            break;
        }
        if (rv == 0) continue;
        if (FD_ISSET(listen_fd, &rfds)) {
            sockaddr_in cliaddr{};
            socklen_t clilen = sizeof(cliaddr);
            int cfd = accept(listen_fd, (sockaddr*)&cliaddr, &clilen);
            if (cfd < 0) {
                if (errno == EINTR) continue;
                if (errno == EBADF) break;
                continue;
            }
            echo_client_handler(cfd);
        }
    }
    close(listen_fd);
}

static uint16_t start_echo_server(uint16_t port, std::atomic<bool>& stop_flag, std::thread& out_thread) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return 0;
    }
    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(port);
    if (bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(fd);
        return 0;
    }
    if (listen(fd, 128) < 0) {
        perror("listen");
        close(fd);
        return 0;
    }
    sockaddr_in real{};
    socklen_t len = sizeof(real);
    if (getsockname(fd, (sockaddr*)&real, &len) < 0) {
        perror("getsockname");
        close(fd);
        return 0;
    }
    uint16_t actual_port = ntohs(real.sin_port);
    out_thread = std::thread(echo_server_loop, fd, std::ref(stop_flag));
    return actual_port;
}

static std::string echo_once(const std::string& host, uint16_t port, const std::string& msg) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return "";
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1) {
        close(fd);
        return "";
    }
    if (connect(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        return "";
    }
    // send all
    const char* data = msg.data();
    size_t left = msg.size();
    while (left > 0) {
        ssize_t n = send(fd, data, left, 0);
        if (n <= 0) { close(fd); return ""; }
        data += n;
        left -= n;
    }
    shutdown(fd, SHUT_WR);
    std::string out;
    char buf[4096];
    while (true) {
        ssize_t n = recv(fd, buf, sizeof(buf), 0);
        if (n <= 0) break;
        out.append(buf, buf + n);
    }
    close(fd);
    return out;
}

int main() {
    std::atomic<bool> stop_flag(false);
    std::thread server_thread;
    uint16_t port = start_echo_server(0, stop_flag, server_thread);
    if (port == 0) {
        std::cerr << "Failed to start server\n";
        return 1;
    }

    std::vector<std::string> tests = {
        "hello",
        "world",
        "こんにちは",
        "1234567890",
        "echo test with spaces"
    };

    for (const auto& msg : tests) {
        std::string echoed = echo_once("127.0.0.1", port, msg);
        std::cout << "Sent:   " << msg << "\n";
        std::cout << "Echoed: " << echoed << "\n";
        std::cout << "Match:  " << (msg == echoed ? "true" : "false") << "\n";
        std::cout << "---\n";
    }

    stop_flag.store(true);
    // Wake up select by connecting
    int wakefd = socket(AF_INET, SOCK_STREAM, 0);
    if (wakefd >= 0) {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
        connect(wakefd, (sockaddr*)&addr, sizeof(addr));
        close(wakefd);
    }
    server_thread.join();
    return 0;
}