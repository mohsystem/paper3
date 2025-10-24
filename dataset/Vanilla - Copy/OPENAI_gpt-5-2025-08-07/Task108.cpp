#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

static const int PORT = 5050;

class RemoteServer {
   public:
    RemoteServer(int port) : port_(port), running_(true), listen_fd_(-1) {}

    void start() { server_thread_ = std::thread(&RemoteServer::run, this); }
    void join() {
        if (server_thread_.joinable()) server_thread_.join();
    }

   private:
    int port_;
    bool running_;
    int listen_fd_;
    std::thread server_thread_;
    std::mutex mtx_;
    std::map<std::string, std::map<std::string, std::string>> store_;

    static bool readLine(int fd, std::string& out) {
        out.clear();
        char c;
        while (true) {
            ssize_t n = ::recv(fd, &c, 1, 0);
            if (n == 0) return false;
            if (n < 0) {
                if (errno == EINTR) continue;
                return false;
            }
            if (c == '\n') break;
            out.push_back(c);
        }
        return true;
    }

    static bool writeLine(int fd, const std::string& s) {
        std::string data = s + "\n";
        const char* p = data.c_str();
        size_t left = data.size();
        while (left > 0) {
            ssize_t n = ::send(fd, p, left, 0);
            if (n < 0) {
                if (errno == EINTR) continue;
                return false;
            }
            left -= n;
            p += n;
        }
        return true;
    }

    std::string process(const std::string& line) {
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;
        std::string ucmd;
        ucmd.resize(cmd.size());
        std::transform(cmd.begin(), cmd.end(), ucmd.begin(), ::toupper);

        if (ucmd == "CREATE") {
            std::string id;
            if (!(iss >> id)) return "ERROR Usage: CREATE <id>";
            std::lock_guard<std::mutex> lock(mtx_);
            if (store_.find(id) == store_.end()) store_[id] = {};
            return "OK";
        } else if (ucmd == "SET") {
            std::string id, key;
            if (!(iss >> id >> key)) return "ERROR Usage: SET <id> <key> <value>";
            std::string value;
            std::getline(iss, value);
            if (!value.empty() && value[0] == ' ') value.erase(0, 1);
            std::lock_guard<std::mutex> lock(mtx_);
            auto it = store_.find(id);
            if (it == store_.end()) return "ERROR NotFound";
            it->second[key] = value;
            return "OK";
        } else if (ucmd == "GET") {
            std::string id, key;
            if (!(iss >> id >> key)) return "ERROR Usage: GET <id> <key>";
            std::lock_guard<std::mutex> lock(mtx_);
            auto it = store_.find(id);
            if (it == store_.end()) return "ERROR NotFound";
            auto it2 = it->second.find(key);
            if (it2 == it->second.end()) return "ERROR NotFound";
            return "VALUE " + it2->second;
        } else if (ucmd == "DELETE") {
            std::string id;
            if (!(iss >> id)) return "ERROR Usage: DELETE <id>";
            std::lock_guard<std::mutex> lock(mtx_);
            store_.erase(id);
            return "OK";
        } else if (ucmd == "KEYS") {
            std::string id;
            if (!(iss >> id)) return "ERROR Usage: KEYS <id>";
            std::lock_guard<std::mutex> lock(mtx_);
            auto it = store_.find(id);
            if (it == store_.end()) return "ERROR NotFound";
            std::vector<std::string> keys;
            for (auto& kv : it->second) keys.push_back(kv.first);
            std::sort(keys.begin(), keys.end());
            std::ostringstream os;
            os << "KEYS ";
            for (size_t i = 0; i < keys.size(); ++i) {
                if (i) os << ",";
                os << keys[i];
            }
            return os.str();
        } else if (ucmd == "LIST") {
            std::lock_guard<std::mutex> lock(mtx_);
            std::vector<std::string> ids;
            for (auto& kv : store_) ids.push_back(kv.first);
            std::sort(ids.begin(), ids.end());
            std::ostringstream os;
            os << "LIST ";
            for (size_t i = 0; i < ids.size(); ++i) {
                if (i) os << ",";
                os << ids[i];
            }
            return os.str();
        } else if (ucmd == "EXIT") {
            return "OK BYE";
        } else if (ucmd == "SHUTDOWN") {
            running_ = false;
            if (listen_fd_ >= 0) ::close(listen_fd_);
            return "OK SHUTDOWN";
        }
        return "ERROR UnknownCommand";
    }

    void run() {
        listen_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
        if (listen_fd_ < 0) return;
        int opt = 1;
        setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        addr.sin_port = htons(port_);
        if (bind(listen_fd_, (sockaddr*)&addr, sizeof(addr)) < 0) {
            ::close(listen_fd_);
            listen_fd_ = -1;
            return;
        }
        if (listen(listen_fd_, 16) < 0) {
            ::close(listen_fd_);
            listen_fd_ = -1;
            return;
        }
        while (running_) {
            int cfd = ::accept(listen_fd_, nullptr, nullptr);
            if (cfd < 0) {
                if (!running_) break;
                continue;
            }
            std::thread([this, cfd]() {
                std::string line;
                while (readLine(cfd, line)) {
                    std::string resp = process(line);
                    if (!writeLine(cfd, resp)) break;
                    if (line == "EXIT" || line == "SHUTDOWN") break;
                }
                ::close(cfd);
            }).detach();
        }
        if (listen_fd_ >= 0) ::close(listen_fd_);
        listen_fd_ = -1;
    }
};

// Client helpers (functions accept params and return output)
std::string send_command(const std::string& cmd) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return std::string("ERROR socket");
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    if (::connect(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        ::close(fd);
        return std::string("ERROR connect");
    }
    std::string data = cmd + "\n";
    if (::send(fd, data.c_str(), data.size(), 0) < 0) {
        ::close(fd);
        return std::string("ERROR send");
    }
    std::string out;
    char c;
    while (true) {
        ssize_t n = ::recv(fd, &c, 1, 0);
        if (n <= 0) break;
        if (c == '\n') break;
        out.push_back(c);
    }
    ::close(fd);
    return out;
}
std::string create_obj(const std::string& id) { return send_command("CREATE " + id); }
std::string set_prop(const std::string& id, const std::string& key, const std::string& value) { return send_command("SET " + id + " " + key + " " + value); }
std::string get_prop(const std::string& id, const std::string& key) { return send_command("GET " + id + " " + key); }
std::string delete_obj(const std::string& id) { return send_command("DELETE " + id); }
std::string keys_obj(const std::string& id) { return send_command("KEYS " + id); }
std::string list_objs() { return send_command("LIST"); }
std::string shutdown_server() { return send_command("SHUTDOWN"); }

int main() {
    RemoteServer server(PORT);
    server.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Test case 1
    std::cout << create_obj("foo") << std::endl;
    std::cout << set_prop("foo", "a", "1") << std::endl;
    std::cout << get_prop("foo", "a") << std::endl;

    // Test case 2
    std::cout << create_obj("bar") << std::endl;
    std::cout << set_prop("bar", "x", "hello world") << std::endl;
    std::cout << keys_obj("bar") << std::endl;

    // Test case 3
    std::cout << list_objs() << std::endl;

    // Test case 4
    std::cout << delete_obj("foo") << std::endl;
    std::cout << list_objs() << std::endl;

    // Test case 5
    std::cout << get_prop("foo", "a") << std::endl;
    std::cout << get_prop("bar", "missing") << std::endl;

    std::cout << shutdown_server() << std::endl;
    server.join();
    return 0;
}