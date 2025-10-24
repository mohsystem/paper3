#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <cstring>
#include <functional>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

struct ServerState {
    int listen_fd{-1};
    std::string secret;
    std::atomic<bool> running{false};
    std::atomic<int> counter{0};
    std::unordered_map<std::string, std::string> kv;
    std::mutex mtx;
};

static bool const_time_eq(const std::string &a, const std::string &b) {
    size_t la = a.size(), lb = b.size();
    if (la != lb) {
        volatile unsigned int diff = la ^ lb;
        size_t m = la < lb ? la : lb;
        for (size_t i = 0; i < m; ++i) diff |= (unsigned char)a[i] ^ (unsigned char)b[i];
        (void)diff;
        return false;
    }
    volatile unsigned int diff = 0;
    for (size_t i = 0; i < la; ++i) diff |= (unsigned char)a[i] ^ (unsigned char)b[i];
    return diff == 0;
}

static bool is_key_valid(const std::string &k) {
    if (k.empty() || k.size() > 32) return false;
    for (char c : k) {
        if (!(std::isalnum((unsigned char)c) || c == '_' || c == '-')) return false;
    }
    return true;
}

static bool set_socket_timeout(int fd, int sec) {
    timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = 0;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) != 0) return false;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) != 0) return false;
    return true;
}

static bool read_line_limited(int fd, std::string &out, size_t max_len, int total_timeout_sec) {
    out.clear();
    fd_set rfds;
    timeval tv;
    time_t start = time(nullptr);
    while (true) {
        time_t now = time(nullptr);
        if (now - start >= total_timeout_sec) return false;
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        int r = select(fd + 1, &rfds, nullptr, nullptr, &tv);
        if (r < 0) return false;
        if (r == 0) continue;
        char ch;
        ssize_t n = recv(fd, &ch, 1, 0);
        if (n <= 0) return false;
        if (ch == '\n') break;
        if (ch != '\r') {
            if (out.size() >= max_len) return false;
            out.push_back(ch);
        }
    }
    // trim
    while (!out.empty() && (out.back() == ' ' || out.back() == '\t')) out.pop_back();
    return true;
}

static bool write_line(int fd, const std::string &s) {
    std::string line = s + "\n";
    const char *buf = line.c_str();
    size_t left = line.size();
    while (left > 0) {
        ssize_t n = send(fd, buf, left, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        buf += n;
        left -= (size_t)n;
    }
    return true;
}

static void handle_client(int cfd, ServerState *state) {
    set_socket_timeout(cfd, 10);
    write_line(cfd, "WELCOME");
    std::string line;
    if (!read_line_limited(cfd, line, 1024, 10)) {
        write_line(cfd, "ERR AUTH");
        close(cfd);
        return;
    }
    if (line.rfind("TOKEN ", 0) != 0) {
        write_line(cfd, "ERR AUTH");
        close(cfd);
        return;
    }
    std::string token = line.substr(6);
    if (!const_time_eq(token, state->secret)) {
        write_line(cfd, "ERR AUTH");
        close(cfd);
        return;
    }
    write_line(cfd, "OK AUTH");
    while (true) {
        if (!read_line_limited(cfd, line, 1024, 15)) break;
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;
        for (auto &ch : cmd) ch = (char)std::toupper((unsigned char)ch);
        if (cmd == "QUIT") {
            write_line(cfd, "BYE");
            break;
        } else if (cmd == "PING") {
            write_line(cfd, "PONG");
        } else if (cmd == "GETCNT") {
            int v = state->counter.load();
            write_line(cfd, "COUNTER " + std::to_string(v));
        } else if (cmd == "INCR") {
            int n;
            if (!(iss >> n)) { write_line(cfd, "ERR ARG"); continue; }
            if (n < 0 || n > 1000000) { write_line(cfd, "ERR RANGE"); continue; }
            int v = state->counter.fetch_add(n) + n;
            write_line(cfd, "OK " + std::to_string(v));
        } else if (cmd == "DECR") {
            int n;
            if (!(iss >> n)) { write_line(cfd, "ERR ARG"); continue; }
            if (n < 0 || n > 1000000) { write_line(cfd, "ERR RANGE"); continue; }
            while (true) {
                int cur = state->counter.load();
                int next = cur - n;
                if (next < 0) { write_line(cfd, "ERR RANGE"); break; }
                if (state->counter.compare_exchange_weak(cur, next)) {
                    write_line(cfd, "OK " + std::to_string(next));
                    break;
                }
            }
        } else if (cmd == "RESET") {
            state->counter.store(0);
            write_line(cfd, "OK 0");
        } else if (cmd == "PUT") {
            std::string key, val;
            if (!(iss >> key >> val)) { write_line(cfd, "ERR ARG"); continue; }
            if (!is_key_valid(key) || val.size() > 256) { write_line(cfd, "ERR ARG"); continue; }
            {
                std::lock_guard<std::mutex> lk(state->mtx);
                state->kv[key] = val;
            }
            write_line(cfd, "OK");
        } else if (cmd == "GET") {
            std::string key;
            if (!(iss >> key)) { write_line(cfd, "ERR ARG"); continue; }
            if (!is_key_valid(key)) { write_line(cfd, "ERR ARG"); continue; }
            std::string val;
            {
                std::lock_guard<std::mutex> lk(state->mtx);
                auto it = state->kv.find(key);
                if (it != state->kv.end()) val = it->second;
            }
            if (val.empty()) write_line(cfd, "ERR NOT_FOUND");
            else write_line(cfd, "VALUE " + val);
        } else if (cmd == "DEL") {
            std::string key;
            if (!(iss >> key)) { write_line(cfd, "ERR ARG"); continue; }
            if (!is_key_valid(key)) { write_line(cfd, "ERR ARG"); continue; }
            {
                std::lock_guard<std::mutex> lk(state->mtx);
                state->kv.erase(key);
            }
            write_line(cfd, "OK");
        } else if (cmd == "LIST") {
            std::vector<std::string> keys;
            {
                std::lock_guard<std::mutex> lk(state->mtx);
                keys.reserve(state->kv.size());
                for (auto &p : state->kv) keys.push_back(p.first);
            }
            std::sort(keys.begin(), keys.end());
            std::ostringstream os;
            for (size_t i = 0; i < keys.size(); ++i) {
                if (i) os << ",";
                os << keys[i];
            }
            write_line(cfd, "KEYS " + os.str());
        } else {
            write_line(cfd, "ERR CMD");
        }
    }
    close(cfd);
}

static bool start_server(ServerState &state, uint16_t port, const std::string &secret) {
    state.secret = secret;
    state.listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (state.listen_fd < 0) return false;
    int yes = 1;
    setsockopt(state.listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    if (bind(state.listen_fd, (sockaddr *)&addr, sizeof(addr)) != 0) {
        close(state.listen_fd);
        return false;
    }
    if (listen(state.listen_fd, 50) != 0) {
        close(state.listen_fd);
        return false;
    }
    state.running.store(true);
    std::thread([&state]() {
        while (state.running.load()) {
            sockaddr_in caddr{};
            socklen_t clen = sizeof(caddr);
            int cfd = accept(state.listen_fd, (sockaddr *)&caddr, &clen);
            if (cfd < 0) {
                if (!state.running.load()) break;
                continue;
            }
            std::thread(handle_client, cfd, &state).detach();
        }
    }).detach();
    return true;
}

static void stop_server(ServerState &state) {
    state.running.store(false);
    if (state.listen_fd >= 0) close(state.listen_fd);
}

static std::vector<std::string> client_session(const std::string &host, uint16_t port, const std::string &token, const std::vector<std::string> &commands) {
    std::vector<std::string> out;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        out.push_back("ERR IO");
        return out;
    }
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &addr.sin_addr);
    if (connect(fd, (sockaddr *)&addr, sizeof(addr)) != 0) {
        close(fd);
        out.push_back("ERR IO");
        return out;
    }
    set_socket_timeout(fd, 5);
    auto readLine = [&](std::string &line) -> bool { return read_line_limited(fd, line, 2048, 5); };
    auto writeLine = [&](const std::string &s) -> bool { return write_line(fd, s); };
    std::string line;
    if (!readLine(line)) { out.push_back("ERR IO"); close(fd); return out; }
    out.push_back(line);
    writeLine("TOKEN " + token);
    if (!readLine(line)) { out.push_back("ERR IO"); close(fd); return out; }
    out.push_back(line);
    if (line.rfind("OK", 0) != 0) {
        close(fd);
        return out;
    }
    for (const auto &cmd : commands) {
        writeLine(cmd);
        if (!readLine(line)) { out.push_back("ERR IO"); break; }
        out.push_back(line);
    }
    close(fd);
    return out;
}

int main() {
    uint16_t port = 50508;
    std::string secret = "s3cure-Secret-Token";
    const char *env = std::getenv("REMOTE_SECRET");
    if (env && std::strlen(env) > 0) secret = env;
    ServerState state;
    if (!start_server(state, port, secret)) {
        std::cout << "Failed to start server\n";
        return 1;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    auto t1 = client_session("127.0.0.1", port, secret, {"PING", "QUIT"});
    std::cout << "Test1:";
    for (auto &s : t1) std::cout << " " << s;
    std::cout << "\n";

    auto t2 = client_session("127.0.0.1", port, secret, {"GETCNT", "INCR 5", "GETCNT", "QUIT"});
    std::cout << "Test2:";
    for (auto &s : t2) std::cout << " " << s;
    std::cout << "\n";

    auto t3 = client_session("127.0.0.1", port, secret, {"PUT foo bar", "GET foo", "QUIT"});
    std::cout << "Test3:";
    for (auto &s : t3) std::cout << " " << s;
    std::cout << "\n";

    auto t4 = client_session("127.0.0.1", port, secret, {"DEL foo", "GET foo", "QUIT"});
    std::cout << "Test4:";
    for (auto &s : t4) std::cout << " " << s;
    std::cout << "\n";

    auto t5 = client_session("127.0.0.1", port, "wrong-token", {"PING", "QUIT"});
    std::cout << "Test5:";
    for (auto &s : t5) std::cout << " " << s;
    std::cout << "\n";

    stop_server(state);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return 0;
}