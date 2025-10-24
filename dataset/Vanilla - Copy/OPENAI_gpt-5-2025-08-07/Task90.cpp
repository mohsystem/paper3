#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>

#include <atomic>
#include <cstring>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class Task90 {
public:
    class ChatServer {
    public:
        explicit ChatServer(int port) : port_(port), server_fd_(-1), running_(false) {}

        bool start() {
            server_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
            if (server_fd_ < 0) {
                perror("socket");
                return false;
            }
            int opt = 1;
            setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = INADDR_ANY;
            addr.sin_port = htons(port_);
            if (bind(server_fd_, (sockaddr*)&addr, sizeof(addr)) < 0) {
                perror("bind");
                ::close(server_fd_);
                return false;
            }
            if (listen(server_fd_, 128) < 0) {
                perror("listen");
                ::close(server_fd_);
                return false;
            }
            running_ = true;
            accept_thread_ = std::thread(&ChatServer::acceptLoop, this);
            return true;
        }

        void stop() {
            running_ = false;
            if (server_fd_ >= 0) {
                ::shutdown(server_fd_, SHUT_RDWR);
                ::close(server_fd_);
                server_fd_ = -1;
            }
            if (accept_thread_.joinable()) accept_thread_.join();
            {
                std::lock_guard<std::mutex> lk(clients_mtx_);
                for (int c : clients_) {
                    ::shutdown(c, SHUT_RDWR);
                    ::close(c);
                }
                clients_.clear();
            }
            for (auto &t : client_threads_) {
                if (t.joinable()) t.join();
            }
            client_threads_.clear();
        }

        void broadcast(const std::string& msg) {
            std::lock_guard<std::mutex> lk(clients_mtx_);
            for (auto it = clients_.begin(); it != clients_.end();) {
                int c = *it;
                ssize_t n = ::send(c, msg.data(), msg.size(), MSG_NOSIGNAL);
                if (n < 0) {
                    ::close(c);
                    it = clients_.erase(it);
                } else {
                    ++it;
                }
            }
        }

    private:
        int port_;
        int server_fd_;
        std::atomic<bool> running_;
        std::thread accept_thread_;
        std::mutex clients_mtx_;
        std::vector<int> clients_;
        std::vector<std::thread> client_threads_;
        std::atomic<int> next_id_{1};

        void acceptLoop() {
            while (running_) {
                fd_set rfds;
                FD_ZERO(&rfds);
                FD_SET(server_fd_, &rfds);
                timeval tv{0, 200000}; // 200ms
                int r = select(server_fd_ + 1, &rfds, nullptr, nullptr, &tv);
                if (r <= 0) continue;
                sockaddr_in cli{};
                socklen_t cl = sizeof(cli);
                int cfd = ::accept(server_fd_, (sockaddr*)&cli, &cl);
                if (cfd < 0) continue;
                {
                    std::lock_guard<std::mutex> lk(clients_mtx_);
                    clients_.push_back(cfd);
                }
                int id = next_id_++;
                std::string welcome = "Welcome Client-" + std::to_string(id) + "\n";
                ::send(cfd, welcome.data(), welcome.size(), MSG_NOSIGNAL);
                client_threads_.emplace_back(&ChatServer::clientHandler, this, cfd, id);
            }
        }

        void clientHandler(int cfd, int id) {
            char buf[4096];
            std::string name = "Client-" + std::to_string(id);
            while (running_) {
                ssize_t n = ::recv(cfd, buf, sizeof(buf), 0);
                if (n <= 0) break;
                std::string msg = name + ": " + std::string(buf, buf + n);
                broadcast(msg);
            }
            {
                std::lock_guard<std::mutex> lk(clients_mtx_);
                auto it = std::find(clients_.begin(), clients_.end(), cfd);
                if (it != clients_.end()) clients_.erase(it);
            }
            ::shutdown(cfd, SHUT_RDWR);
            ::close(cfd);
        }
    };

    static ChatServer* start_server(int port) {
        ChatServer* srv = new ChatServer(port);
        if (!srv->start()) {
            delete srv;
            return nullptr;
        }
        return srv;
    }
};

int main() {
    int port = 50592;
    Task90::ChatServer* server = Task90::start_server(port);
    if (!server) {
        std::cout << "Failed to start server\n";
        return 1;
    }

    auto client_sim = [port](const std::string& name, const std::string& msg) {
        int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) { perror("socket"); return; }
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
        if (connect(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
            perror("connect"); ::close(fd); return;
        }
        char buf[4096];
        // read welcome (non-blocking with small timeout)
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        timeval tv{1, 0};
        if (select(fd + 1, &rfds, nullptr, nullptr, &tv) > 0) {
            ssize_t n = recv(fd, buf, sizeof(buf)-1, 0);
            if (n > 0) {
                buf[n] = 0;
                std::cout << name << " recv: " << buf;
            }
        }
        std::string line = msg + "\n";
        send(fd, line.data(), line.size(), 0);

        // read broadcasts briefly
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        tv = {0, 500000}; // 0.5s
        while (select(fd + 1, &rfds, nullptr, nullptr, &tv) > 0) {
            ssize_t n = recv(fd, buf, sizeof(buf)-1, 0);
            if (n <= 0) break;
            buf[n] = 0;
            std::cout << name << " recv: " << buf;
            break;
        }
        ::shutdown(fd, SHUT_RDWR);
        ::close(fd);
    };

    std::vector<std::thread> clients;
    for (int i = 1; i <= 5; ++i) {
        clients.emplace_back(client_sim, "TClient-" + std::to_string(i), "Hello from TClient-" + std::to_string(i));
    }
    for (auto &t : clients) t.join();

    server->stop();
    delete server;
    std::cout << "Server stopped.\n";
    return 0;
}