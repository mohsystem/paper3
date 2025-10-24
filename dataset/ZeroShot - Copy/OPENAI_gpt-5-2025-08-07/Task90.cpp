#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstring>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>

static const size_t MAX_MSG_LEN = 512;
static const int CLIENT_IDLE_TIMEOUT_SEC = 60;
static const int DEFAULT_MAX_CLIENTS = 100;

static bool is_valid_port(int port) {
    return port >= 1024 && port <= 65535;
}

static std::string sanitize_message(const std::string &input, size_t max_len) {
    std::string out;
    out.reserve(std::min(max_len, input.size()));
    size_t count = 0;
    for (size_t i = 0; i < input.size() && count < max_len; ++i) {
        unsigned char c = static_cast<unsigned char>(input[i]);
        if (c == '\r' || c == '\n') {
            if (out.empty() || out.back() != ' ') {
                out.push_back(' ');
                ++count;
            }
        } else if (c == '\t' || (c >= 0x20 && !(c >= 0x7F && c <= 0x9F))) {
            out.push_back(static_cast<char>(c));
            ++count;
        }
    }
    // Trim
    while (!out.empty() && (out.front() == ' ' || out.front() == '\t')) out.erase(out.begin());
    while (!out.empty() && (out.back() == ' ' || out.back() == '\t')) out.pop_back();
    if (out.size() > max_len) out.resize(max_len);
    return out;
}

class ChatServer {
public:
    ChatServer(int port, int maxClients)
        : port_(port), maxClients_(std::max(1, maxClients)), listenFd_(-1), running_(false) {}

    bool start() {
        if (!is_valid_port(port_)) return false;
        if (running_) return true;

        listenFd_ = ::socket(AF_INET, SOCK_STREAM, 0);
        if (listenFd_ < 0) {
            return false;
        }
        int yes = 1;
        setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

        // Non-blocking
        int flags = fcntl(listenFd_, F_GETFL, 0);
        fcntl(listenFd_, F_SETFL, flags | O_NONBLOCK);

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(static_cast<uint16_t>(port_));
        if (bind(listenFd_, (sockaddr*)&addr, sizeof(addr)) < 0) {
            stop();
            return false;
        }
        if (listen(listenFd_, std::min(maxClients_, 50)) < 0) {
            stop();
            return false;
        }

        running_ = true;
        // Main loop in current thread (call run() manually if desired)
        return true;
    }

    void run() {
        if (!running_) return;
        std::vector<pollfd> fds;
        std::unordered_map<int, std::string> buffers;
        std::unordered_map<int, time_t> lastActive;

        auto add_client = [&](int fd) {
            pollfd p{};
            p.fd = fd;
            p.events = POLLIN;
            p.revents = 0;
            fds.push_back(p);
            buffers[fd] = std::string();
            lastActive[fd] = std::time(nullptr);
            send_line(fd, "Welcome to Secure Chat Server");
        };

        auto remove_fd = [&](int fd) {
            ::close(fd);
            buffers.erase(fd);
            lastActive.erase(fd);
            fds.erase(std::remove_if(fds.begin(), fds.end(), [&](const pollfd& p) { return p.fd == fd; }), fds.end());
        };

        // initialize with listening socket
        pollfd lp{};
        lp.fd = listenFd_;
        lp.events = POLLIN;
        lp.revents = 0;
        fds.push_back(lp);

        while (running_) {
            int ret = ::poll(fds.data(), fds.size(), 1000);
            if (ret < 0) {
                if (errno == EINTR) continue;
                break;
            }
            time_t now = std::time(nullptr);

            // check events
            for (size_t i = 0; i < fds.size();) {
                pollfd &p = fds[i];
                if (p.fd == listenFd_) {
                    if (p.revents & POLLIN) {
                        // accept all pending
                        while (true) {
                            sockaddr_in cli{};
                            socklen_t cl = sizeof(cli);
                            int cfd = ::accept(listenFd_, (sockaddr*)&cli, &cl);
                            if (cfd < 0) {
                                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                                break;
                            }
                            // non-blocking
                            int cflags = fcntl(cfd, F_GETFL, 0);
                            fcntl(cfd, F_SETFL, cflags | O_NONBLOCK);

                            if ((int)lastActive.size() >= maxClients_) {
                                ::close(cfd);
                                continue;
                            }
                            add_client(cfd);
                        }
                    }
                    ++i;
                } else {
                    bool advance = true;
                    if (p.revents & (POLLERR | POLLHUP | POLLNVAL)) {
                        int fd = p.fd;
                        remove_fd(fd);
                        advance = false;
                    } else if (p.revents & POLLIN) {
                        int fd = p.fd;
                        char buf[1024];
                        ssize_t n = ::recv(fd, buf, sizeof(buf), 0);
                        if (n <= 0) {
                            remove_fd(fd);
                            advance = false;
                        } else {
                            lastActive[fd] = now;
                            std::string &acc = buffers[fd];
                            acc.append(buf, buf + n);
                            // process lines
                            size_t pos;
                            while ((pos = acc.find('\n')) != std::string::npos) {
                                std::string line = acc.substr(0, pos);
                                acc.erase(0, pos + 1);
                                std::string msg = sanitize_message(line, MAX_MSG_LEN);
                                if (!msg.empty()) {
                                    broadcast(msg, fd, fds);
                                }
                            }
                            if (acc.size() > MAX_MSG_LEN) {
                                acc.erase(0, acc.size() - MAX_MSG_LEN);
                            }
                        }
                    }
                    if (advance) ++i;
                }
            }

            // idle timeouts
            for (size_t i = 0; i < fds.size();) {
                pollfd &p = fds[i];
                if (p.fd != listenFd_) {
                    auto it = lastActive.find(p.fd);
                    if (it != lastActive.end() && (now - it->second) > CLIENT_IDLE_TIMEOUT_SEC) {
                        int fd = p.fd;
                        remove_fd(fd);
                        continue; // don't i++
                    }
                }
                ++i;
            }
        }
        // cleanup
        for (size_t i = 0; i < fds.size(); ++i) {
            if (fds[i].fd >= 0) ::close(fds[i].fd);
        }
        fds.clear();
        buffers.clear();
    }

    void stop() {
        running_ = false;
        if (listenFd_ >= 0) {
            ::close(listenFd_);
            listenFd_ = -1;
        }
    }

private:
    void send_line(int fd, const std::string &line) {
        std::string out = line + "\n";
        (void)::send(fd, out.data(), out.size(), MSG_NOSIGNAL);
    }

    void broadcast(const std::string &msg, int senderFd, const std::vector<pollfd> &fds) {
        std::string out = msg + "\n";
        for (const auto &p : fds) {
            if (p.fd >= 0 && p.fd != listenFd_ && p.fd != senderFd) {
                (void)::send(p.fd, out.data(), out.size(), MSG_NOSIGNAL);
            }
        }
    }

    int port_;
    int maxClients_;
    int listenFd_;
    bool running_;
};

// Tests
static void run_tests() {
    int passed = 0;
    if (sanitize_message(std::string("Hello\0World", 11), MAX_MSG_LEN) == "HelloWorld") passed++;
    if (sanitize_message(std::string(600, 'a'), 128).size() == 128) passed++;
    if (is_valid_port(8080)) passed++;
    if (!is_valid_port(70000)) passed++;
    if (!is_valid_port(1023)) passed++;
    std::cout << "Tests passed: " << passed << "/5" << std::endl;
}

int main(int argc, char** argv) {
    run_tests();
    if (argc >= 2) {
        int port = std::atoi(argv[1]);
        ChatServer server(port, DEFAULT_MAX_CLIENTS);
        if (server.start()) {
            std::cout << "Secure Chat Server running on port " << port << ". Press Ctrl+C to stop." << std::endl;
            // Run in foreground
            server.run();
            server.stop();
        } else {
            std::cout << "Failed to start server. Invalid port or system error." << std::endl;
        }
    }
    return 0;
}