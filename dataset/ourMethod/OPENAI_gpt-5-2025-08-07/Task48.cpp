#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cstring>
#include <functional>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

static bool set_tcp_nodelay(int fd) {
    int flag = 1;
    return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) == 0;
}

static bool set_reuseaddr(int fd) {
    int flag = 1;
    return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) == 0;
}

static int create_listen_socket(uint16_t port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;
    if (!set_reuseaddr(fd)) {
        close(fd);
        return -1;
    }
    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) != 1) {
        close(fd);
        return -1;
    }
    if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        close(fd);
        return -1;
    }
    if (listen(fd, 16) != 0) {
        close(fd);
        return -1;
    }
    return fd;
}

static int connect_socket(const std::string& host, uint16_t port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;
    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1) {
        close(fd);
        return -1;
    }
    if (connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        close(fd);
        return -1;
    }
    set_tcp_nodelay(fd);
    return fd;
}

static bool send_all(int fd, const char* data, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = send(fd, data + sent, len - sent, MSG_NOSIGNAL);
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        if (n == 0) return false;
        sent += static_cast<size_t>(n);
    }
    return true;
}

static bool send_line(int fd, const std::string& line) {
    std::string out = line;
    if (out.empty() || out.back() != '\n') out.push_back('\n');
    return send_all(fd, out.c_str(), out.size());
}

static bool recv_line(int fd, std::string& out) {
    out.clear();
    constexpr size_t MAX_LINE = 512;
    char ch = 0;
    size_t cnt = 0;
    while (true) {
        ssize_t n = recv(fd, &ch, 1, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        if (n == 0) {
            // connection closed
            return false;
        }
        if (ch == '\n') break;
        if (ch != '\r') {
            if (cnt + 1 >= MAX_LINE) {
                // line too long, drop until newline
                continue;
            }
            // allow printable ASCII only for safety
            if (static_cast<unsigned char>(ch) >= 32 && static_cast<unsigned char>(ch) <= 126) {
                out.push_back(ch);
                cnt++;
            }
        }
    }
    return true;
}

class ChatServer {
public:
    ChatServer() : listen_fd(-1), running(false), next_id(1) {}

    bool start(uint16_t port) {
        if (running.load()) return false;
        listen_fd = create_listen_socket(port);
        if (listen_fd < 0) {
            std::cerr << "Server: Failed to create listen socket\n";
            return false;
        }
        running.store(true);
        accept_thread = std::thread(&ChatServer::accept_loop, this);
        return true;
    }

    void stop() {
        if (!running.exchange(false)) return;
        if (listen_fd >= 0) {
            shutdown(listen_fd, SHUT_RDWR);
            close(listen_fd);
            listen_fd = -1;
        }
        if (accept_thread.joinable()) accept_thread.join();

        // Close all clients
        std::vector<int> fds_to_close;
        {
            std::lock_guard<std::mutex> lg(clients_mtx);
            for (auto& c : clients) {
                fds_to_close.push_back(c.fd);
            }
        }
        for (int fd : fds_to_close) {
            shutdown(fd, SHUT_RDWR);
            close(fd);
        }
        // give detached threads time to exit
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::lock_guard<std::mutex> lg(clients_mtx);
        clients.clear();
    }

    ~ChatServer() {
        stop();
    }

private:
    struct ClientInfo {
        int fd;
        int id;
        std::thread thr;
        ClientInfo(int f, int i) : fd(f), id(i) {}
    };

    int listen_fd;
    std::atomic<bool> running;
    std::thread accept_thread;

    std::mutex clients_mtx;
    std::vector<ClientInfo> clients;
    std::atomic<int> next_id;

    void accept_loop() {
        while (running.load()) {
            sockaddr_in cliaddr {};
            socklen_t len = sizeof(cliaddr);
            int cfd = accept(listen_fd, reinterpret_cast<sockaddr*>(&cliaddr), &len);
            if (cfd < 0) {
                if (errno == EINTR) continue;
                if (!running.load()) break;
                // brief pause to avoid busy loop on error
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            set_tcp_nodelay(cfd);
            int cid = next_id.fetch_add(1);
            {
                std::lock_guard<std::mutex> lg(clients_mtx);
                clients.emplace_back(cfd, cid);
                std::thread t(&ChatServer::client_thread, this, cfd, cid);
                t.detach();
            }
            std::ostringstream oss;
            oss << "Server: Client " << cid << " joined";
            broadcast(oss.str(), 0);
        }
    }

    void remove_client_locked(int fd) {
        for (auto it = clients.begin(); it != clients.end(); ++it) {
            if (it->fd == fd) {
                clients.erase(it);
                break;
            }
        }
    }

    void broadcast(const std::string& message, int sender_id) {
        std::string line = message;
        if (line.size() > 480) line = line.substr(0, 480);
        std::vector<int> fds;
        {
            std::lock_guard<std::mutex> lg(clients_mtx);
            fds.reserve(clients.size());
            for (auto& c : clients) {
                fds.push_back(c.fd);
            }
        }
        for (int fd : fds) {
            if (!send_line(fd, line)) {
                // On failure, remove client and close
                std::lock_guard<std::mutex> lg(clients_mtx);
                shutdown(fd, SHUT_RDWR);
                close(fd);
                remove_client_locked(fd);
            }
        }
    }

    void client_thread(int fd, int cid) {
        std::string line;
        while (running.load()) {
            if (!recv_line(fd, line)) {
                break;
            }
            if (line.empty()) continue;
            std::ostringstream oss;
            oss << "Client " << cid << ": " << line;
            broadcast(oss.str(), cid);
        }
        {
            std::lock_guard<std::mutex> lg(clients_mtx);
            remove_client_locked(fd);
        }
        shutdown(fd, SHUT_RDWR);
        close(fd);
        std::ostringstream oss;
        oss << "Server: Client " << cid << " left";
        broadcast(oss.str(), 0);
    }
};

class ChatClient {
public:
    ChatClient() : fd(-1), running(false) {}

    bool connectTo(const std::string& host, uint16_t port) {
        fd = connect_socket(host, port);
        if (fd < 0) return false;
        running.store(true);
        reader = std::thread(&ChatClient::reader_loop, this);
        return true;
    }

    bool sendMessage(const std::string& msg) {
        if (fd < 0) return false;
        std::string safe = msg;
        // sanitize to printable ASCII
        std::string filtered;
        filtered.reserve(safe.size());
        for (unsigned char c : safe) {
            if (c >= 32 && c <= 126) filtered.push_back(static_cast<char>(c));
        }
        if (filtered.empty()) filtered = "(empty)";
        return send_line(fd, filtered);
    }

    void closeClient() {
        if (!running.exchange(false)) return;
        if (fd >= 0) {
            shutdown(fd, SHUT_RDWR);
            close(fd);
            fd = -1;
        }
        if (reader.joinable()) reader.join();
    }

    ~ChatClient() {
        closeClient();
    }

private:
    int fd;
    std::atomic<bool> running;
    std::thread reader;

    void reader_loop() {
        std::string line;
        while (running.load()) {
            if (!recv_line(fd, line)) break;
            // Print received broadcast
            std::cout << "[Client] Recv: " << line << std::endl;
        }
    }
};

int main() {
    // Avoid SIGPIPE terminating the process
    signal(SIGPIPE, SIG_IGN);

    const uint16_t PORT = 19048;
    ChatServer server;
    if (!server.start(PORT)) {
        std::cerr << "Failed to start server\n";
        return 1;
    }

    // Create 5 clients (5 test cases)
    const int N = 5;
    std::vector<ChatClient*> clients;
    clients.reserve(N);
    for (int i = 0; i < N; ++i) {
        auto* c = new ChatClient();
        if (!c->connectTo("127.0.0.1", PORT)) {
            std::cerr << "Client " << i << " failed to connect\n";
            delete c;
            continue;
        }
        clients.push_back(c);
    }

    // Give time for connections
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Each client sends one message
    for (size_t i = 0; i < clients.size(); ++i) {
        std::ostringstream oss;
        oss << "Hello from test client #" << (i + 1);
        clients[i]->sendMessage(oss.str());
    }

    // Wait to receive broadcasts
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Close clients
    for (auto* c : clients) {
        c->closeClient();
        delete c;
    }

    // Stop server
    server.stop();

    std::cout << "Chat demo complete." << std::endl;
    return 0;
}