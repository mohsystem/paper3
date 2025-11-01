#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <ctime>
#include <iostream>
#include <random>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

static const int SERVER_PORT = 9090;
static const size_t MAX_SIZE = 10 * 1024 * 1024; // 10MB

static bool is_dir(mode_t m) { return S_ISDIR(m); }

static bool ensure_upload_dir(int &dirfd) {
    if (mkdir("uploads", 0700) != 0 && errno != EEXIST) {
        return false;
    }
    dirfd = open("uploads", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dirfd < 0) return false;
    struct stat st;
    if (fstat(dirfd, &st) != 0 || !is_dir(st.st_mode)) {
        close(dirfd);
        dirfd = -1;
        return false;
    }
    return true;
}

static bool sanitize_name(const std::string &name) {
    if (name.empty() || name.size() > 100) return false;
    if (name.rfind("..", 0) == 0) return false;
    if (name.find("..") != std::string::npos) return false;
    if (name[0] == '.') return false;
    static const std::regex re("^[A-Za-z0-9._-]+$");
    return std::regex_match(name, re);
}

static std::string gen_stored_name(const std::string &baseName) {
    std::string base = baseName;
    std::string ext;
    auto pos = baseName.rfind('.');
    if (pos != std::string::npos && pos > 0 && pos < baseName.size() - 1) {
        std::string e = baseName.substr(pos + 1);
        bool ok = (!e.empty() && e.size() <= 10);
        for (char c : e) {
            if (!std::isalnum(static_cast<unsigned char>(c))) { ok = false; break; }
        }
        if (ok) {
            base = baseName.substr(0, pos);
            ext = "." + e;
        }
    }
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  std::chrono::system_clock::now().time_since_epoch())
                  .count();
    std::random_device rd;
    std::mt19937 rng(rd());
    uint32_t r = rng();
    char hex[9];
    snprintf(hex, sizeof(hex), "%08x", r);
    std::ostringstream oss;
    oss << base << "-" << ms << "-" << hex << ext;
    return oss.str();
}

static bool send_all(int fd, const std::string &s) {
    const char *p = s.c_str();
    size_t n = s.size();
    while (n > 0) {
        ssize_t w = write(fd, p, n);
        if (w <= 0) return false;
        p += w; n -= (size_t)w;
    }
    return true;
}

static bool recv_until(int fd, std::string &out, const std::string &delim, size_t max_bytes) {
    char buf[1024];
    while (out.find(delim) == std::string::npos) {
        ssize_t r = read(fd, buf, sizeof(buf));
        if (r <= 0) return false;
        out.append(buf, buf + r);
        if (out.size() > max_bytes) return false;
    }
    return true;
}

static std::string get_header_value(const std::string &headers, const std::string &key) {
    std::string low = headers;
    // Simple case-insensitive search:
    std::string k = key;
    for (auto &c : low) c = std::tolower((unsigned char)c);
    for (auto &c : k) c = std::tolower((unsigned char)c);
    size_t pos = low.find("\r\n" + k + ":");
    if (pos == std::string::npos) {
        if (low.rfind(k + ":", 0) == 0) pos = 0;
        else return "";
    }
    size_t start = pos;
    if (pos == 0 && low.rfind(k + ":", 0) == 0) start = 0; else start = pos + 2;
    size_t colon = low.find(':', start);
    if (colon == std::string::npos) return "";
    size_t val_start = colon + 1;
    while (val_start < low.size() && (low[val_start] == ' ' || low[val_start] == '\t')) val_start++;
    size_t line_end = low.find("\r\n", val_start);
    if (line_end == std::string::npos) return "";
    return headers.substr(val_start, line_end - val_start);
}

static std::string url_decode(const std::string &s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '%' && i + 2 < s.size()) {
            char a = s[i+1], b = s[i+2];
            auto hex = [](char c)->int{
                if (c >= '0' && c <= '9') return c - '0';
                if (c >= 'a' && c <= 'f') return c - 'a' + 10;
                if (c >= 'A' && c <= 'F') return c - 'A' + 10;
                return -1;
            };
            int hi = hex(a), lo = hex(b);
            if (hi >= 0 && lo >= 0) {
                out.push_back((char)((hi<<4) | lo));
                i += 2; continue;
            }
        } else if (s[i] == '+') {
            out.push_back(' ');
            continue;
        }
        out.push_back(s[i]);
    }
    return out;
}

static std::string get_query_param(const std::string &path, const std::string &key) {
    auto qpos = path.find('?');
    if (qpos == std::string::npos) return "";
    std::string qs = path.substr(qpos + 1);
    std::istringstream iss(qs);
    std::string kv;
    while (std::getline(iss, kv, '&')) {
        auto eq = kv.find('=');
        if (eq == std::string::npos) continue;
        std::string k = url_decode(kv.substr(0, eq));
        if (k == key) {
            return url_decode(kv.substr(eq + 1));
        }
    }
    return "";
}

static void handle_client(int cfd) {
    std::string data;
    if (!recv_until(cfd, data, "\r\n\r\n", 65536)) {
        close(cfd); return;
    }
    // Parse request line
    size_t line_end = data.find("\r\n");
    if (line_end == std::string::npos) {
        send_all(cfd, "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n");
        close(cfd); return;
    }
    std::string reqline = data.substr(0, line_end);
    std::istringstream rl(reqline);
    std::string method, path, ver;
    rl >> method >> path >> ver;
    if (method != "POST" || path.rfind("/upload", 0) != 0) {
        send_all(cfd, "HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n");
        close(cfd); return;
    }
    std::string headers = data.substr(0, data.find("\r\n\r\n") + 2);
    std::string clstr = get_header_value(headers, "Content-Length");
    if (clstr.empty()) {
        send_all(cfd, "HTTP/1.1 411 Length Required\r\nConnection: close\r\n\r\n");
        close(cfd); return;
    }
    size_t content_length = 0;
    try {
        content_length = (size_t)std::stoull(clstr);
    } catch (...) {
        send_all(cfd, "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n");
        close(cfd); return;
    }
    if (content_length > MAX_SIZE) {
        send_all(cfd, "HTTP/1.1 413 Payload Too Large\r\nConnection: close\r\n\r\n");
        close(cfd); return;
    }

    std::string name = get_query_param(path, "name");
    if (!sanitize_name(name)) {
        send_all(cfd, "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n");
        close(cfd); return;
    }
    int dirfd = -1;
    if (!ensure_upload_dir(dirfd)) {
        send_all(cfd, "HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\n\r\n");
        close(cfd); return;
    }
    std::string stored = gen_stored_name(name);

    int flags = O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC;
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
    int fd = openat(dirfd, stored.c_str(), flags, 0600);
    if (fd < 0) {
        if (errno == EEXIST) {
            send_all(cfd, "HTTP/1.1 409 Conflict\r\nConnection: close\r\n\r\n");
        } else if (errno == ELOOP) {
            send_all(cfd, "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n");
        } else {
            send_all(cfd, "HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\n\r\n");
        }
        close(dirfd);
        close(cfd);
        return;
    }

    // body may already contain some bytes after headers
    size_t header_bytes = data.size() - (headers.size() + 2);
    size_t to_write = content_length;
    if (header_bytes > 0) {
        size_t w = header_bytes;
        if (w > to_write) w = to_write;
        ssize_t wr = write(fd, data.data() + headers.size() + 2, w);
        if (wr < 0) { close(fd); close(dirfd); close(cfd); return; }
        to_write -= (size_t)wr;
    }
    char buf[8192];
    while (to_write > 0) {
        ssize_t r = read(cfd, buf, (to_write > sizeof(buf)) ? sizeof(buf) : to_write);
        if (r <= 0) { break; }
        ssize_t off = 0;
        while (off < r) {
            ssize_t w = write(fd, buf + off, r - off);
            if (w <= 0) { to_write = 0; break; }
            off += w;
        }
        if (off < r) { to_write = 0; break; }
        to_write -= (size_t)r;
    }
    if (to_write != 0) {
        // short body, cleanup
        close(fd);
        unlinkat(dirfd, stored.c_str(), 0);
        close(dirfd);
        send_all(cfd, "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n");
        close(cfd);
        return;
    }

    fsync(fd);
    close(fd);
    // fsync directory for durability
#ifdef __linux__
    fsync(dirfd);
#endif
    close(dirfd);

    std::ostringstream body;
    body << "{\"stored\":\"" << stored << "\"}\n";
    std::ostringstream resp;
    resp << "HTTP/1.1 201 Created\r\n"
         << "Content-Type: application/json; charset=utf-8\r\n"
         << "Content-Length: " << body.str().size() << "\r\n"
         << "Connection: close\r\n\r\n"
         << body.str();
    send_all(cfd, resp.str());
    close(cfd);
}

static void server_thread_func(bool &stopFlag) {
    int sfd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (sfd < 0) return;
    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(sfd, (sockaddr*)&addr, sizeof(addr)) != 0) {
        close(sfd); return;
    }
    if (listen(sfd, 16) != 0) {
        close(sfd); return;
    }
    while (!stopFlag) {
        sockaddr_in cli{};
        socklen_t cl = sizeof(cli);
        int cfd = accept4(sfd, (sockaddr*)&cli, &cl, SOCK_CLOEXEC);
        if (cfd < 0) {
            if (errno == EINTR) continue;
            break;
        }
        handle_client(cfd);
    }
    close(sfd);
}

static int http_post(const std::string &name, const std::string &data) {
    int fd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (fd < 0) return -1;
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (connect(fd, (sockaddr*)&addr, sizeof(addr)) != 0) {
        close(fd); return -1;
    }
    std::ostringstream req;
    req << "POST /upload?name=";
    // URL-encode name
    std::ostringstream enc;
    for (unsigned char c : name) {
        if (std::isalnum(c) || c == '.' || c == '_' || c == '-' ) enc << c;
        else { char buf[4]; snprintf(buf, sizeof(buf), "%%%02X", c); enc << buf; }
    }
    req << enc.str() << " HTTP/1.1\r\n";
    req << "Host: 127.0.0.1:" << SERVER_PORT << "\r\n";
    req << "Content-Type: application/octet-stream\r\n";
    req << "Content-Length: " << data.size() << "\r\n";
    req << "Connection: close\r\n\r\n";
    std::string head = req.str();
    if (!send_all(fd, head)) { close(fd); return -1; }
    if (!data.empty()) {
        if (!send_all(fd, data)) { close(fd); return -1; }
    }
    // read status
    std::string line;
    char ch;
    while (true) {
        ssize_t r = read(fd, &ch, 1);
        if (r <= 0) break;
        line.push_back(ch);
        if (line.size() >= 2 && line[line.size()-2] == '\r' && line[line.size()-1] == '\n') break;
    }
    int code = -1;
    if (line.rfind("HTTP/", 0) == 0) {
        std::istringstream iss(line);
        std::string http; iss >> http; iss >> code;
    }
    // drain
    char buf[1024];
    while (read(fd, buf, sizeof(buf)) > 0) {}
    close(fd);
    return code;
}

int main() {
    bool stop = false;
    std::thread th(server_thread_func, std::ref(stop));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    int ok = 0;
    // 1) Small valid upload
    ok += (http_post("hello.txt", "Hello") == 201) ? 1 : 0;
    // 2) Invalid name (traversal)
    ok += (http_post("../evil", "data") == 400) ? 1 : 0;
    // 3) Empty file
    ok += (http_post("empty.bin", "") == 201) ? 1 : 0;
    // 4) Too large
    std::string big(MAX_SIZE + 1, 'x');
    ok += (http_post("big.bin", big) == 413) ? 1 : 0;
    // 5) Another valid
    std::string randdata(1024, '\0');
    std::mt19937 rng{std::random_device{}()};
    for (auto &c : randdata) c = (char)(rng() & 0xFF);
    ok += (http_post("rand.dat", randdata) == 201) ? 1 : 0;

    stop = true;
    // Connect once to unblock accept
    int fd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    sockaddr_in addr{}; addr.sin_family = AF_INET; addr.sin_port = htons(SERVER_PORT); addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    connect(fd, (sockaddr*)&addr, sizeof(addr)); close(fd);

    th.join();
    std::cout << "Tests passed: " << ok << "/5\n";
    return 0;
}