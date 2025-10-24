#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

static bool isValidIdOrKey(const std::string &s) {
    if (s.size() < 1 || s.size() > 64) return false;
    for (char c : s) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_')) return false;
    }
    return true;
}

static bool isValidValue(const std::string &s) {
    if (s.size() > 256) return false;
    for (char c : s) {
        if (c == ' ') continue;
        if (c < 33 || c > 126) return false;
    }
    return true;
}

class ObjectStore {
   public:
    std::string newObject(const std::string &id) {
        if (!isValidIdOrKey(id)) return "ERR invalid_id";
        std::lock_guard<std::mutex> lg(mu_);
        if (store_.count(id)) return "ERR exists";
        store_[id] = {};
        return "OK";
    }
    std::string putField(const std::string &id, const std::string &key, const std::string &value) {
        if (!isValidIdOrKey(id)) return "ERR invalid_id";
        if (!isValidIdOrKey(key)) return "ERR invalid_key";
        if (!isValidValue(value)) return "ERR invalid_value";
        std::lock_guard<std::mutex> lg(mu_);
        auto it = store_.find(id);
        if (it == store_.end()) return "ERR not_found";
        it->second[key] = value;
        return "OK";
    }
    std::string getField(const std::string &id, const std::string &key) {
        if (!isValidIdOrKey(id)) return "ERR invalid_id";
        if (!isValidIdOrKey(key)) return "ERR invalid_key";
        std::lock_guard<std::mutex> lg(mu_);
        auto it = store_.find(id);
        if (it == store_.end()) return "ERR not_found";
        auto it2 = it->second.find(key);
        if (it2 == it->second.end()) return "ERR not_found";
        return "OK " + it2->second;
    }
    std::string delField(const std::string &id, const std::string &key) {
        if (!isValidIdOrKey(id)) return "ERR invalid_id";
        if (!isValidIdOrKey(key)) return "ERR invalid_key";
        std::lock_guard<std::mutex> lg(mu_);
        auto it = store_.find(id);
        if (it == store_.end()) return "ERR not_found";
        auto it2 = it->second.find(key);
        if (it2 == it->second.end()) return "ERR not_found";
        it->second.erase(it2);
        return "OK";
    }
    std::string keys(const std::string &id) {
        if (!isValidIdOrKey(id)) return "ERR invalid_id";
        std::lock_guard<std::mutex> lg(mu_);
        auto it = store_.find(id);
        if (it == store_.end()) return "ERR not_found";
        std::vector<std::string> ks;
        ks.reserve(it->second.size());
        for (auto &p : it->second) ks.push_back(p.first);
        std::sort(ks.begin(), ks.end());
        std::string out = "OK ";
        for (size_t i = 0; i < ks.size(); ++i) {
            out += ks[i];
            if (i + 1 < ks.size()) out += ",";
        }
        return out;
    }
    std::string drop(const std::string &id) {
        if (!isValidIdOrKey(id)) return "ERR invalid_id";
        std::lock_guard<std::mutex> lg(mu_);
        auto it = store_.find(id);
        if (it == store_.end()) return "ERR not_found";
        store_.erase(it);
        return "OK";
    }

   private:
    std::mutex mu_;
    std::map<std::string, std::map<std::string, std::string>> store_;
};

static std::string processCommand(const std::string &line, ObjectStore &store) {
    if (line.empty()) return "ERR empty";
    std::vector<std::string> parts;
    std::string cur;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == ' ') {
            if (!cur.empty()) {
                parts.push_back(cur);
                cur.clear();
            }
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) parts.push_back(cur);
    if (parts.empty()) return "ERR empty";
    std::string cmd = parts[0];
    for (auto &ch : cmd) ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    try {
        if (cmd == "PING") return "OK PONG";
        if (cmd == "NEW" && parts.size() == 2) return store.newObject(parts[1]);
        if (cmd == "PUT" && parts.size() == 4) return store.putField(parts[1], parts[2], parts[3]);
        if (cmd == "GET" && parts.size() == 3) return store.getField(parts[1], parts[2]);
        if (cmd == "DEL" && parts.size() == 3) return store.delField(parts[1], parts[2]);
        if (cmd == "KEYS" && parts.size() == 2) return store.keys(parts[1]);
        if (cmd == "DROP" && parts.size() == 2) return store.drop(parts[1]);
        if (cmd == "QUIT") return "OK BYE";
        return "ERR unknown";
    } catch (...) {
        return "ERR error";
    }
}

static bool readLineLimited(int fd, std::string &out, size_t maxLen) {
    out.clear();
    char ch;
    while (out.size() < maxLen) {
        ssize_t n = recv(fd, &ch, 1, 0);
        if (n == 0) return false;  // closed
        if (n < 0) return false;
        if (ch == '\n') break;
        if (ch == '\r') continue;
        out.push_back(ch);
    }
    if (out.size() >= maxLen) return false;
    return true;
}

class Server {
   public:
    explicit Server(uint16_t port) : running_(true) {
        sock_ = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sock_ < 0) throw std::runtime_error("socket failed");
        int yes = 1;
        setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
        sockaddr_in addr {};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        addr.sin_port = htons(port);
        if (bind(sock_, (sockaddr *)&addr, sizeof(addr)) < 0) throw std::runtime_error("bind failed");
        if (listen(sock_, 16) < 0) throw std::runtime_error("listen failed");
        // get actual port
        socklen_t len = sizeof(addr);
        if (getsockname(sock_, (sockaddr *)&addr, &len) == 0) {
            port_ = ntohs(addr.sin_port);
        } else {
            port_ = port;
        }
        thr_ = std::thread(&Server::acceptLoop, this);
    }
    ~Server() {
        close();
    }
    uint16_t port() const { return port_; }
    void close() {
        if (running_) {
            running_ = false;
            ::shutdown(sock_, SHUT_RDWR);
            ::close(sock_);
            if (thr_.joinable()) thr_.join();
        }
    }

   private:
    int sock_;
    uint16_t port_;
    std::thread thr_;
    bool running_;
    ObjectStore store_;

    void acceptLoop() {
        while (running_) {
            int cfd = ::accept(sock_, nullptr, nullptr);
            if (cfd < 0) break;
            std::thread(&Server::handleClient, this, cfd).detach();
        }
    }
    void handleClient(int cfd) {
        std::string line;
        while (true) {
            if (!readLineLimited(cfd, line, 1024)) break;
            std::string resp = processCommand(line, store_);
            resp.push_back('\n');
            send(cfd, resp.data(), resp.size(), 0);
            if (resp.rfind("OK BYE", 0) == 0) break;
        }
        ::shutdown(cfd, SHUT_RDWR);
        ::close(cfd);
    }
};

static std::vector<std::string> sendCommands(const std::string &host, uint16_t port, const std::vector<std::string> &cmds) {
    std::vector<std::string> res;
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return {"ERR socket"};
    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1) {
        ::close(fd);
        return {"ERR addr"};
    }
    if (connect(fd, (sockaddr *)&addr, sizeof(addr)) < 0) {
        ::close(fd);
        return {"ERR connect"};
    }
    for (auto &cmd : cmds) {
        std::string line = cmd + "\n";
        send(fd, line.data(), line.size(), 0);
        std::string resp;
        if (!readLineLimited(fd, resp, 1024)) resp = "ERR no_response";
        res.push_back(resp);
    }
    std::string q = "QUIT\n";
    send(fd, q.data(), q.size(), 0);
    std::string tmp;
    readLineLimited(fd, tmp, 1024);
    ::shutdown(fd, SHUT_RDWR);
    ::close(fd);
    return res;
}

int main() {
    try {
        Server srv(0);
        uint16_t port = srv.port();

        auto t1 = sendCommands("127.0.0.1", port, {"NEW objA"});
        std::cout << "Test1: " << (t1.empty() ? "ERR" : t1[0]) << std::endl;

        auto t2 = sendCommands("127.0.0.1", port, {"PUT objA key1 v1"});
        std::cout << "Test2: " << (t2.empty() ? "ERR" : t2[0]) << std::endl;

        auto t3 = sendCommands("127.0.0.1", port, {"GET objA key1"});
        std::cout << "Test3: " << (t3.empty() ? "ERR" : t3[0]) << std::endl;

        auto t4 = sendCommands("127.0.0.1", port, {"KEYS objA"});
        std::cout << "Test4: " << (t4.empty() ? "ERR" : t4[0]) << std::endl;

        auto t5 = sendCommands("127.0.0.1", port, {"DEL objA key1", "GET objA key1"});
        std::cout << "Test5a: " << (t5.size() > 0 ? t5[0] : "ERR") << std::endl;
        std::cout << "Test5b: " << (t5.size() > 1 ? t5[1] : "ERR") << std::endl;

        srv.close();
    } catch (const std::exception &e) {
        std::cout << "Server error\n";
    }
    return 0;
}