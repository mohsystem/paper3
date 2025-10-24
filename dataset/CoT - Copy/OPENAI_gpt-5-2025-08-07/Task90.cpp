#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <errno.h>
#include <thread>
#include <atomic>
#include <mutex>

// Step 1-5: Secure multi-client chat server with broadcast, limited clients, timeouts, sanitize, and safe resource handling.

namespace cpp_chat_secure {
    struct Server {
        int srv_fd = -1;
        std::atomic<bool> running{false};
        std::thread accept_thread;
        std::vector<int> clients;
        std::mutex clients_mtx;
        int max_clients = 16;
        int socket_timeout_sec = 15;
        size_t max_msg_len = 1024;

        static void set_nonblock(int fd, bool nb) {
            int flags = fcntl(fd, F_GETFL, 0);
            if (flags < 0) return;
            if (nb) fcntl(fd, F_SETFL, flags | O_NONBLOCK);
            else fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
        }

        static void set_rcv_timeout(int fd, int sec) {
            struct timeval tv;
            tv.tv_sec = sec;
            tv.tv_usec = 0;
            setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        }

        static void safe_close(int fd) {
            if (fd >= 0) close(fd);
        }

        std::string sanitize(const std::string &in) {
            std::string out;
            out.reserve(std::min(in.size(), max_msg_len));
            for (char c : in) {
                unsigned char uc = static_cast<unsigned char>(c);
                if (uc >= 32 && uc != 127) {
                    out.push_back(c);
                    if (out.size() >= max_msg_len) break;
                }
            }
            // trim
            while (!out.empty() && (out.back() == ' ')) out.pop_back();
            return out;
        }

        void broadcast(const std::string &msg, int src_fd) {
            std::string s = sanitize(msg);
            if (s.empty()) return;
            int src_idx = -1;
            {
                std::lock_guard<std::mutex> lk(clients_mtx);
                for (size_t i = 0; i < clients.size(); ++i) {
                    if (clients[i] == src_fd) { src_idx = static_cast<int>(i); break; }
                }
            }
            std::string final = "[Client-" + std::to_string(src_idx >= 0 ? src_idx : -1) + "]: " + s + "\n";
            std::lock_guard<std::mutex> lk(clients_mtx);
            for (int fd : clients) {
                if (fd == src_fd) continue;
                ssize_t _ = send(fd, final.data(), final.size(), MSG_NOSIGNAL);
                (void)_;
            }
        }

        void remove_client(int fd) {
            std::lock_guard<std::mutex> lk(clients_mtx);
            auto it = std::find(clients.begin(), clients.end(), fd);
            if (it != clients.end()) clients.erase(it);
        }

        void client_thread(int fd) {
            set_rcv_timeout(fd, socket_timeout_sec);
            // Send welcome
            std::string welcome = "[Server]: Welcome!\n";
            send(fd, welcome.data(), welcome.size(), MSG_NOSIGNAL);

            std::string line;
            line.reserve(max_msg_len);
            char ch;
            while (running.load()) {
                ssize_t r = recv(fd, &ch, 1, 0);
                if (r == 0) break;
                if (r < 0) {
                    if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR) continue;
                    break;
                }
                if (ch == '\n') {
                    std::string trimmed = line;
                    // check quit
                    {
                        std::string tmp = trimmed;
                        // lowercase
                        for (auto &c : tmp) c = std::tolower(static_cast<unsigned char>(c));
                        // trim
                        while (!tmp.empty() && (tmp.back() == ' ' || tmp.back() == '\r')) tmp.pop_back();
                        while (!tmp.empty() && (tmp.front() == ' ')) tmp.erase(tmp.begin());
                        if (tmp == "quit") break;
                    }
                    broadcast(trimmed, fd);
                    line.clear();
                } else if (ch != '\r') {
                    if (line.size() < max_msg_len) {
                        line.push_back(ch);
                    } else {
                        std::string warn = "[Server]: Message too long. Disconnecting.\n";
                        send(fd, warn.data(), warn.size(), MSG_NOSIGNAL);
                        break;
                    }
                }
            }
            safe_close(fd);
            remove_client(fd);
        }

        int start(int port, int maxClients) {
            if (running.load()) return -1;
            max_clients = std::max(1, maxClients);
            srv_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (srv_fd < 0) return -1;
            int yes = 1;
            setsockopt(srv_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = inet_addr("127.0.0.1");
            addr.sin_port = htons(static_cast<uint16_t>(port));

            if (bind(srv_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
                safe_close(srv_fd); srv_fd = -1; return -1;
            }
            if (listen(srv_fd, 50) < 0) {
                safe_close(srv_fd); srv_fd = -1; return -1;
            }
            // Get actual port
            socklen_t len = sizeof(addr);
            if (getsockname(srv_fd, (struct sockaddr*)&addr, &len) == 0) {
                // ok
            }
            running.store(true);
            accept_thread = std::thread([this]() {
                while (running.load()) {
                    sockaddr_in cli{};
                    socklen_t clen = sizeof(cli);
                    int cfd = accept(srv_fd, (struct sockaddr*)&cli, &clen);
                    if (cfd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(50));
                            continue;
                        }
                        // transient errors tolerated
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        continue;
                    }
                    {
                        std::lock_guard<std::mutex> lk(clients_mtx);
                        if ((int)clients.size() >= max_clients) {
                            safe_close(cfd);
                            continue;
                        }
                        clients.push_back(cfd);
                    }
                    std::thread(&Server::client_thread, this, cfd).detach();
                }
            });
            // set accept non-blocking for responsiveness
            set_nonblock(srv_fd, true);
            return ntohs(addr.sin_port);
        }

        void stop() {
            if (!running.exchange(false)) return;
            safe_close(srv_fd);
            srv_fd = -1;
            if (accept_thread.joinable()) accept_thread.join();
            std::lock_guard<std::mutex> lk(clients_mtx);
            for (int fd : clients) safe_close(fd);
            clients.clear();
        }
    };
}

static int start_server(int port, int max_clients, cpp_chat_secure::Server &server) {
    return server.start(port, max_clients);
}
static void stop_server(cpp_chat_secure::Server &server) { server.stop(); }

// Client for testing
static void client_send_and_receive(const std::string &host, int port, const std::string &msg, int expectReads, int timeoutSec) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return;
    struct timeval tv; tv.tv_sec = timeoutSec; tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    sockaddr_in addr{}; addr.sin_family = AF_INET; addr.sin_port = htons((uint16_t)port);
    inet_pton(AF_INET, host.c_str(), &addr.sin_addr);
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) { close(fd); return; }
    std::string line = msg + "\n";
    send(fd, line.data(), line.size(), MSG_NOSIGNAL);
    // attempt to read a few lines
    auto read_line = [&](std::string &out)->bool{
        out.clear();
        char ch;
        while (true) {
            ssize_t r = recv(fd, &ch, 1, 0);
            if (r == 0) return !out.empty();
            if (r < 0) {
                if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR) {
                    if (out.empty()) return false;
                    else return true;
                }
                return false;
            }
            if (ch == '\n') return true;
            if (ch != '\r') out.push_back(ch);
            if (out.size() > 2048) return true;
        }
    };
    std::string recvbuf;
    for (int i = 0; i < expectReads; ++i) {
        if (!read_line(recvbuf)) break;
        // std::cout << "Client got: " << recvbuf << std::endl;
    }
    std::string quit = "quit\n";
    send(fd, quit.data(), quit.size(), MSG_NOSIGNAL);
    close(fd);
}

int main() {
    cpp_chat_secure::Server server;
    int port = start_server(0, 16, server);
    if (port <= 0) return 1;

    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&, i]() {
            client_send_and_receive("127.0.0.1", port, "Hello from C++ client " + std::to_string(i), 3, 3);
        });
    }
    for (auto &t : threads) t.join();

    stop_server(server);
    // std::cout << "C++ server test completed.\n";
    return 0;
}