#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <regex>
#include <string>
#include <thread>
#include <vector>

static const int SERVER_PORT = 8081;
static std::atomic<bool> g_stop{false};
static std::map<std::string, std::string> g_db;
static std::mutex g_db_mutex;

static bool is_valid_username(const std::string &u) {
    if (u.size() < 1 || u.size() > 32) return false;
    for (char c : u) {
        if (!((c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') ||
              c == '_' || c == '-')) {
            return false;
        }
    }
    return true;
}

static std::string url_decode(const std::string &s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '%' && i + 2 < s.size()) {
            char hex[3] = {s[i + 1], s[i + 2], 0};
            char *endptr = nullptr;
            long v = strtol(hex, &endptr, 16);
            if (endptr != hex + 2 || v < 0 || v > 255) {
                return std::string();
            }
            out.push_back(static_cast<char>(v));
            i += 2;
        } else if (s[i] == '+') {
            out.push_back(' ');
        } else {
            out.push_back(s[i]);
        }
    }
    return out;
}

static bool execute_is_moderator_query(const std::string &username) {
    std::lock_guard<std::mutex> lock(g_db_mutex);
    auto it = g_db.find(username);
    if (it == g_db.end()) return false;
    return it->second == "moderator";
}

static void write_full(int fd, const std::string &data) {
    size_t off = 0;
    while (off < data.size()) {
        ssize_t n = ::send(fd, data.data() + off, data.size() - off, 0);
        if (n <= 0) break;
        off += static_cast<size_t>(n);
    }
}

static void handle_client(int cfd) {
    char buf[2048];
    memset(buf, 0, sizeof(buf));
    ssize_t n = recv(cfd, buf, sizeof(buf) - 1, 0);
    if (n <= 0) {
        close(cfd);
        return;
    }
    // Parse very simple HTTP GET line
    std::string req(buf, static_cast<size_t>(n));
    // Expect: GET /is_moderator?username=... HTTP/1.1
    size_t sp1 = req.find(' ');
    if (sp1 == std::string::npos) {
        close(cfd);
        return;
    }
    size_t sp2 = req.find(' ', sp1 + 1);
    if (sp2 == std::string::npos) {
        close(cfd);
        return;
    }
    std::string method = req.substr(0, sp1);
    std::string target = req.substr(sp1 + 1, sp2 - sp1 - 1);
    if (method != "GET") {
        std::string body = "{\"error\":\"method not allowed\"}";
        std::string resp = "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " +
                           std::to_string(body.size()) + "\r\nConnection: close\r\n\r\n" + body;
        write_full(cfd, resp);
        close(cfd);
        return;
    }

    std::string path = target;
    std::string query;
    size_t qpos = target.find('?');
    if (qpos != std::string::npos) {
        path = target.substr(0, qpos);
        query = target.substr(qpos + 1);
    }

    if (path != "/is_moderator") {
        std::string body = "{\"error\":\"not found\"}";
        std::string resp = "HTTP/1.1 404 Not Found\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " +
                           std::to_string(body.size()) + "\r\nConnection: close\r\n\r\n" + body;
        write_full(cfd, resp);
        close(cfd);
        return;
    }

    std::string username;
    if (!query.empty() && query.size() <= 2048) {
        // parse username=...
        size_t p = query.find("username=");
        if (p == 0 || (p != std::string::npos && (query[p - 1] == '&'))) {
            size_t start = p + 9;
            size_t end = query.find('&', start);
            std::string raw = query.substr(start, (end == std::string::npos) ? std::string::npos : end - start);
            std::string decoded = url_decode(raw);
            if (!decoded.empty()) username = decoded;
        }
    }

    if (!is_valid_username(username)) {
        std::string body = "{\"error\":\"invalid username\"}";
        std::string resp = "HTTP/1.1 400 Bad Request\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " +
                           std::to_string(body.size()) + "\r\nConnection: close\r\n\r\n" + body;
        write_full(cfd, resp);
        close(cfd);
        return;
    }

    bool is_mod = execute_is_moderator_query(username);
    // JSON escape minimal (usernames validated to safe charset)
    std::string body = std::string("{\"username\":\"") + username + "\",\"is_moderator\":" + (is_mod ? "true" : "false") + "}";
    std::string resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: " +
                       std::to_string(body.size()) + "\r\nConnection: close\r\n\r\n" + body;
    write_full(cfd, resp);
    close(cfd);
}

static void server_thread() {
    int sfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) {
        std::cerr << "socket failed\n";
        return;
    }
    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(SERVER_PORT);

    if (bind(sfd, (sockaddr *)&addr, sizeof(addr)) < 0) {
        std::cerr << "bind failed: " << strerror(errno) << "\n";
        close(sfd);
        return;
    }
    if (listen(sfd, 16) < 0) {
        std::cerr << "listen failed\n";
        close(sfd);
        return;
    }

    while (!g_stop.load()) {
        sockaddr_in cli{};
        socklen_t clilen = sizeof(cli);
        int cfd = accept(sfd, (sockaddr *)&cli, &clilen);
        if (cfd < 0) {
            if (errno == EINTR) continue;
            if (g_stop.load()) break;
            continue;
        }
        handle_client(cfd);
    }

    close(sfd);
}

static std::string http_get_local(const std::string &username) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return "";

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(SERVER_PORT);
    if (connect(fd, (sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd);
        return "";
    }
    std::string path = "/is_moderator?username=" + username;
    std::string req = "GET " + path + " HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n";
    write_full(fd, req);

    std::string resp;
    char buf[1024];
    ssize_t n;
    while ((n = recv(fd, buf, sizeof(buf), 0)) > 0) {
        resp.append(buf, buf + n);
    }
    close(fd);

    // Extract body after double CRLF
    std::string::size_type pos = resp.find("\r\n\r\n");
    if (pos == std::string::npos) return resp;
    return resp.substr(pos + 4);
}

static bool load_db_from_file(const std::string &path) {
    std::ifstream in(path);
    if (!in) return false;
    std::string line;
    std::lock_guard<std::mutex> lock(g_db_mutex);
    g_db.clear();
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        // Expect "username,role"
        size_t comma = line.find(',');
        if (comma == std::string::npos) continue;
        std::string u = line.substr(0, comma);
        std::string r = line.substr(comma + 1);
        if (is_valid_username(u) && (r == "moderator" || r == "user")) {
            g_db[u] = r;
        }
    }
    return true;
}

int main() {
    // "Connect to database" by loading from a file (as a simple DB simulation)
    const std::string dbfile = "users_db_cpp.txt";
    {
        std::ofstream out(dbfile, std::ios::trunc);
        out << "alice,moderator\n";
        out << "bob,user\n";
        out << "carol,moderator\n";
        out << "dave,user\n";
        out << "eve,moderator\n";
    }
    if (!load_db_from_file(dbfile)) {
        std::cerr << "Failed to load DB\n";
        return 1;
    }

    std::thread t(server_thread);
    // 5 test cases
    std::vector<std::string> tests = {"alice", "bob", "carol", "dave", "unknown"};
    for (const auto &u : tests) {
        std::string body = http_get_local(u);
        std::cout << body << std::endl;
    }
    g_stop.store(true);
    // Nudge server to exit accept
    (void)http_get_local("alice");
    t.join();
    return 0;
}