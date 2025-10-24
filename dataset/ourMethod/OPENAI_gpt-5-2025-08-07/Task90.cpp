#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <functional>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <condition_variable>
#include <chrono>

class ChatServer {
public:
    ChatServer() : running(false), server_fd(-1) {}
    ~ChatServer() { stop(); }

    bool start(uint16_t port) {
        if (running.load()) return false;
        server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            return false;
        }
        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1
        addr.sin_port = htons(port);
        if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
            ::close(server_fd);
            server_fd = -1;
            return false;
        }
        if (listen(server_fd, 20) < 0) {
            ::close(server_fd);
            server_fd = -1;
            return false;
        }
        // Set accept timeout via select-like behavior: we will use nonblocking accept with timeout using timeval? Simpler: use blocking but accept loop checks running via shutdown of server_fd. We'll use recv timeouts on clients.
        running.store(true);
        accept_thread = std::thread(&ChatServer::acceptLoop, this);
        return true;
    }

    void stop() {
        if (!running.exchange(false)) return;
        // Closing server_fd will unblock accept
        if (server_fd >= 0) {
            ::shutdown(server_fd, SHUT_RDWR);
            ::close(server_fd);
            server_fd = -1;
        }
        if (accept_thread.joinable()) accept_thread.join();
        // Close clients
        std::vector<int> toClose;
        {
            std::lock_guard<std::mutex> lg(clients_mx);
            for (auto &c : clients) toClose.push_back(c.fd);
            clients.clear();
        }
        for (int fd : toClose) {
            ::shutdown(fd, SHUT_RDWR);
            ::close(fd);
        }
    }

    uint16_t getPort() const {
        if (server_fd < 0) return 0;
        sockaddr_in sa{};
        socklen_t sl = sizeof(sa);
        if (getsockname(server_fd, (sockaddr*)&sa, &sl) == 0) {
            return ntohs(sa.sin_port);
        }
        return 0;
    }

private:
    struct Client {
        int fd;
        std::string name;
        std::thread th;
    };

    std::atomic<bool> running;
    int server_fd;
    std::thread accept_thread;

    std::mutex clients_mx;
    std::vector<Client> clients;
    int clientIdGen = 1;

    static std::string sanitize(const std::string &in, size_t maxLen=1024) {
        std::string out;
        out.reserve(in.size());
        size_t n = in.size() < maxLen ? in.size() : maxLen;
        for (size_t i = 0; i < n; ++i) {
            unsigned char c = static_cast<unsigned char>(in[i]);
            if (c == '\n' || c == '\r') continue;
            if ((c >= 32 && c <= 126)) {
                out.push_back(static_cast<char>(c));
            } else {
                out.push_back('?');
            }
        }
        return out;
    }

    static bool sendAll(int fd, const std::string &data) {
        const char* buf = data.c_str();
        size_t total = 0;
        size_t len = data.size();
        while (total < len) {
            ssize_t n = ::send(fd, buf + total, len - total, 0);
            if (n <= 0) {
                if (errno == EINTR) continue;
                return false;
            }
            total += static_cast<size_t>(n);
        }
        return true;
    }

    void broadcast(const std::string &msg) {
        std::vector<int> fds;
        {
            std::lock_guard<std::mutex> lg(clients_mx);
            for (auto &c : clients) fds.push_back(c.fd);
        }
        std::vector<int> dead;
        std::string line = msg + "\n";
        for (int fd : fds) {
            if (!sendAll(fd, line)) {
                dead.push_back(fd);
            }
        }
        if (!dead.empty()) {
            std::lock_guard<std::mutex> lg(clients_mx);
            auto it = clients.begin();
            while (it != clients.end()) {
                bool remove = false;
                for (int d : dead) {
                    if (it->fd == d) { remove = true; break; }
                }
                if (remove) {
                    ::shutdown(it->fd, SHUT_RDWR);
                    ::close(it->fd);
                    if (it->th.joinable()) it->th.detach();
                    it = clients.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    static bool recvLine(int fd, std::string &out) {
        out.clear();
        char buf[512];
        while (true) {
            ssize_t n = ::recv(fd, buf, sizeof(buf), 0);
            if (n == 0) {
                return false; // closed
            }
            if (n < 0) {
                if (errno == EINTR) continue;
                return false;
            }
            for (ssize_t i = 0; i < n; ++i) {
                char c = buf[i];
                if (c == '\n') {
                    return true;
                } else if (c != '\r') {
                    if (out.size() < 2048) out.push_back(c);
                }
            }
            if (out.size() > 4096) return true; // cap
        }
    }

    void clientThread(int fd, std::string name) {
        // Set recv timeout
        timeval tv{};
        tv.tv_sec = 15;
        tv.tv_usec = 0;
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        std::string line;
        while (running.load()) {
            if (!recvLine(fd, line)) break;
            std::string s = sanitize(line);
            if (s == "/quit") break;
            if (!s.empty()) {
                broadcast(name + ": " + s);
            }
        }
        // Remove client
        {
            std::lock_guard<std::mutex> lg(clients_mx);
            auto it = clients.begin();
            while (it != clients.end()) {
                if (it->fd == fd) {
                    if (it->th.joinable() && std::this_thread::get_id() != it->th.get_id()) {
                        it->th.detach();
                    }
                    break;
                }
                ++it;
            }
        }
        ::shutdown(fd, SHUT_RDWR);
        ::close(fd);
        broadcast(std::string("[server] ") + name + " left");
    }

    void acceptLoop() {
        while (running.load()) {
            sockaddr_in cli{};
            socklen_t cl = sizeof(cli);
            int cfd = ::accept(server_fd, (sockaddr*)&cli, &cl);
            if (cfd < 0) {
                if (errno == EINTR) continue;
                if (!running.load()) break;
                // Sleep briefly to avoid spin on error
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }
            int one = 1;
            setsockopt(cfd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
            std::string name = "client-" + std::to_string(clientIdGen++);
            {
                std::lock_guard<std::mutex> lg(clients_mx);
                clients.push_back(Client{cfd, name, std::thread(&ChatServer::clientThread, this, cfd, name)});
            }
            broadcast(std::string("[server] ") + name + " joined");
        }
        // Join client threads (detach if still running)
        std::lock_guard<std::mutex> lg(clients_mx);
        for (auto &c : clients) {
            if (c.th.joinable()) c.th.detach();
        }
    }
};

// Simple barrier for tests
class Barrier {
public:
    explicit Barrier(int count) : threshold(count), count(count), generation(0) {}
    void wait() {
        std::unique_lock<std::mutex> lk(mx);
        int gen = generation;
        if (--count == 0) {
            generation++;
            count = threshold;
            cv.notify_all();
        } else {
            cv.wait_for(lk, std::chrono::seconds(5), [this, gen] { return gen != generation; });
        }
    }
private:
    std::mutex mx;
    std::condition_variable cv;
    int threshold;
    int count;
    int generation;
};

// Test client
struct TestClient {
    int id;
    std::string host;
    uint16_t port;
    Barrier &bar;
    int expected;
    int received{0};
    std::thread th;

    static bool sendAll(int fd, const std::string &s) {
        const char* p = s.c_str();
        size_t n = s.size();
        size_t off = 0;
        while (off < n) {
            ssize_t w = ::send(fd, p + off, n - off, 0);
            if (w <= 0) {
                if (errno == EINTR) continue;
                return false;
            }
            off += static_cast<size_t>(w);
        }
        return true;
    }

    static bool recvLineTO(int fd, std::string &out) {
        out.clear();
        char ch;
        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (std::chrono::steady_clock::now() < deadline) {
            ssize_t n = ::recv(fd, &ch, 1, MSG_DONTWAIT);
            if (n == 1) {
                if (ch == '\n') return true;
                if (ch != '\r') {
                    if (out.size() < 4096) out.push_back(ch);
                }
            } else if (n == 0) {
                return false;
            } else {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    continue;
                }
                if (errno == EINTR) continue;
                return false;
            }
        }
        return !out.empty();
    }

    void run() {
        int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) return;
        sockaddr_in sa{};
        sa.sin_family = AF_INET;
        sa.sin_port = htons(port);
        inet_pton(AF_INET, host.c_str(), &sa.sin_addr);
        if (connect(fd, (sockaddr*)&sa, sizeof(sa)) < 0) {
            ::close(fd);
            return;
        }
        // Ready barrier
        bar.wait();
        // Send one message
        std::string msg = "Hello from test client " + std::to_string(id) + "\n";
        sendAll(fd, msg);
        // Read expected messages
        int count = 0;
        std::string line;
        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (count < expected && std::chrono::steady_clock::now() < deadline) {
            if (!recvLineTO(fd, line)) break;
            if (!line.empty()) count++;
        }
        received = count;
        sendAll(fd, std::string("/quit\n"));
        ::shutdown(fd, SHUT_RDWR);
        ::close(fd);
    }
};

int main() {
    ChatServer server;
    if (!server.start(0)) {
        std::cerr << "Failed to start server\n";
        return 1;
    }
    uint16_t port = server.getPort();
    std::string host = "127.0.0.1";

    const int N = 5;
    Barrier bar(N);
    int expectedMin = N;

    std::vector<TestClient> clients;
    clients.reserve(N);
    for (int i = 0; i < N; ++i) {
        clients.push_back(TestClient{i + 1, host, port, bar, expectedMin, 0});
    }
    for (int i = 0; i < N; ++i) {
        clients[i].th = std::thread(&TestClient::run, &clients[i]);
    }
    for (int i = 0; i < N; ++i) {
        if (clients[i].th.joinable()) clients[i].th.join();
    }
    for (int i = 0; i < N; ++i) {
        std::cout << "Client " << (i + 1) << " received " << clients[i].received << " lines\n";
    }
    server.stop();
    std::cout << "Server stopped.\n";
    return 0;
}