#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cctype>
#include <chrono>
#include <cstring>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

struct Session {
    std::string username;
    std::string email;
    std::string ssn;
    bool has_ssn;
};

static std::map<std::string, Session> SESSIONS;
static std::mutex SESS_MTX;

static std::string info(const Session& s) {
    if (!s.has_ssn || s.username.empty() || s.email.empty()) {
        return std::string();
    }
    std::ostringstream oss;
    oss << "username=" << s.username << "; email=" << s.email << "; ssn=" << s.ssn;
    return oss.str();
}

static bool is_valid_sid(const std::string& sid) {
    if (sid.size() < 8 || sid.size() > 128) return false;
    for (char c : sid) {
        if (!std::isalnum(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

static std::string get_sid_from_cookie(const std::string& headers) {
    std::string needle = "\r\nCookie:";
    size_t idx = headers.find(needle);
    if (idx == std::string::npos) {
        // also check if Cookie is first header (edge)
        if (headers.rfind("Cookie:", 0) == 0) idx = 0;
        else return std::string();
    }
    size_t line_start = (idx == 0) ? 0 : idx + 2; // skip CRLF if present
    size_t line_end = headers.find("\r\n", line_start);
    if (line_end == std::string::npos) line_end = headers.size();
    std::string line = headers.substr(line_start, line_end - line_start);
    // line like "Cookie: SID=xxx; other=..."
    size_t colon = line.find(':');
    if (colon == std::string::npos) return std::string();
    std::string cookie_str = line.substr(colon + 1);
    // split by ';'
    std::istringstream iss(cookie_str);
    std::string part;
    while (std::getline(iss, part, ';')) {
        // trim
        size_t b = 0;
        while (b < part.size() && std::isspace(static_cast<unsigned char>(part[b]))) b++;
        size_t e = part.size();
        while (e > b && std::isspace(static_cast<unsigned char>(part[e - 1]))) e--;
        std::string kv = part.substr(b, e - b);
        size_t eq = kv.find('=');
        if (eq == std::string::npos) continue;
        std::string k = kv.substr(0, eq);
        std::string v = kv.substr(eq + 1);
        // trim key
        size_t kb = 0;
        while (kb < k.size() && std::isspace(static_cast<unsigned char>(k[kb]))) kb++;
        size_t ke = k.size();
        while (ke > kb && std::isspace(static_cast<unsigned char>(k[ke - 1]))) ke--;
        k = k.substr(kb, ke - kb);
        if (k == "SID") {
            // trim v
            size_t vb = 0;
            while (vb < v.size() && std::isspace(static_cast<unsigned char>(v[vb]))) vb++;
            size_t ve = v.size();
            while (ve > vb && std::isspace(static_cast<unsigned char>(v[ve - 1]))) ve--;
            v = v.substr(vb, ve - vb);
            if (is_valid_sid(v)) return v;
            return std::string();
        }
    }
    return std::string();
}

static void send_response(int fd, int code, const std::string& body) {
    std::ostringstream oss;
    std::string status;
    switch (code) {
        case 200: status = "200 OK"; break;
        case 400: status = "400 Bad Request"; break;
        case 401: status = "401 Unauthorized"; break;
        case 404: status = "404 Not Found"; break;
        case 405: status = "405 Method Not Allowed"; break;
        default: status = "500 Internal Server Error"; break;
    }
    oss << "HTTP/1.1 " << status << "\r\n";
    oss << "Content-Type: text/plain; charset=UTF-8\r\n";
    oss << "Cache-Control: no-store\r\n";
    oss << "Content-Length: " << body.size() << "\r\n";
    oss << "Connection: close\r\n";
    oss << "\r\n";
    oss << body;
    std::string resp = oss.str();
    ssize_t total = 0;
    const char* p = resp.data();
    ssize_t left = static_cast<ssize_t>(resp.size());
    while (left > 0) {
        ssize_t n = ::send(fd, p + total, static_cast<size_t>(left), 0);
        if (n <= 0) break;
        total += n;
        left -= n;
    }
}

static void handle_client(int fd) {
    // Read request (headers only)
    std::string req;
    char buf[4096];
    while (true) {
        ssize_t n = ::recv(fd, buf, sizeof(buf), 0);
        if (n <= 0) break;
        req.append(buf, buf + n);
        if (req.find("\r\n\r\n") != std::string::npos) break;
        if (req.size() > 8192) break; // cap headers
    }
    if (req.empty()) {
        send_response(fd, 400, "Bad Request");
        ::close(fd);
        return;
    }
    // Parse request line
    size_t line_end = req.find("\r\n");
    if (line_end == std::string::npos) {
        send_response(fd, 400, "Bad Request");
        ::close(fd);
        return;
    }
    std::string line = req.substr(0, line_end);
    std::istringstream lss(line);
    std::string method, path, version;
    lss >> method >> path >> version;
    if (method != "GET") {
        send_response(fd, 405, "Method Not Allowed");
        ::close(fd);
        return;
    }
    if (path != "/info") {
        send_response(fd, 404, "Not Found");
        ::close(fd);
        return;
    }
    std::string sid = get_sid_from_cookie(req);
    if (sid.empty()) {
        send_response(fd, 401, "Unauthorized");
        ::close(fd);
        return;
    }
    Session s;
    {
        std::lock_guard<std::mutex> lk(SESS_MTX);
        auto it = SESSIONS.find(sid);
        if (it == SESSIONS.end()) {
            send_response(fd, 401, "Unauthorized");
            ::close(fd);
            return;
        }
        s = it->second;
    }
    std::string body = info(s);
    if (body.empty()) {
        send_response(fd, 400, "Bad Request");
        ::close(fd);
        return;
    }
    send_response(fd, 200, body);
    ::close(fd);
}

static void server_thread(uint16_t port, std::atomic<bool>& ready, std::atomic<bool>& stop_flag) {
    int srv = ::socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) return;
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(port);
    if (bind(srv, (sockaddr*)&addr, sizeof(addr)) < 0) {
        ::close(srv);
        return;
    }
    if (listen(srv, 16) < 0) {
        ::close(srv);
        return;
    }
    ready.store(true);
    while (!stop_flag.load()) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(srv, &fds);
        timeval tv{};
        tv.tv_sec = 0;
        tv.tv_usec = 200000;
        int r = select(srv + 1, &fds, nullptr, nullptr, &tv);
        if (r > 0 && FD_ISSET(srv, &fds)) {
            sockaddr_in cli{};
            socklen_t cl = sizeof(cli);
            int fd = accept(srv, (sockaddr*)&cli, &cl);
            if (fd >= 0) {
                handle_client(fd);
            }
        }
    }
    ::close(srv);
}

static bool http_test(const std::string& method, const std::string& host, uint16_t port, const std::string& path, const std::string& cookie, int expected_code, const std::string* expected_body_opt) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return false;
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1) {
        ::close(fd);
        return false;
    }
    if (connect(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        ::close(fd);
        return false;
    }
    std::ostringstream req;
    req << method << " " << path << " HTTP/1.1\r\n";
    req << "Host: " << host << ":" << port << "\r\n";
    req << "Connection: close\r\n";
    if (!cookie.empty()) {
        req << "Cookie: " << cookie << "\r\n";
    }
    req << "\r\n";
    std::string reqs = req.str();
    ssize_t left = static_cast<ssize_t>(reqs.size());
    const char* p = reqs.data();
    while (left > 0) {
        ssize_t n = ::send(fd, p, static_cast<size_t>(left), 0);
        if (n <= 0) break;
        p += n;
        left -= n;
    }
    // Read response
    std::string resp;
    char buf[4096];
    while (true) {
        ssize_t n = ::recv(fd, buf, sizeof(buf), 0);
        if (n <= 0) break;
        resp.append(buf, buf + n);
    }
    ::close(fd);
    // Parse status line
    size_t line_end = resp.find("\r\n");
    if (line_end == std::string::npos) return false;
    std::string status_line = resp.substr(0, line_end);
    int code = 0;
    {
        std::istringstream sl(status_line);
        std::string httpv;
        sl >> httpv >> code;
    }
    bool status_ok = (code == expected_code);
    bool body_ok = true;
    if (expected_body_opt && code == 200) {
        size_t header_end = resp.find("\r\n\r\n");
        if (header_end == std::string::npos) return false;
        std::string body = resp.substr(header_end + 4);
        body_ok = (body == *expected_body_opt);
    }
    return status_ok && body_ok;
}

int main() {
    // Populate sessions
    const std::string SID1 = "b7f1c2d3e4f5a6b7c8d9e0f1a2b3c4d5e6f7a8b9c0d1e2f3a4b5c6d7e8f9a0b1";
    const std::string SID2 = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    const std::string SID3 = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    {
        std::lock_guard<std::mutex> lk(SESS_MTX);
        SESSIONS[SID1] = Session{"alice", "alice@example.com", "123-45-6789", true};
        SESSIONS[SID2] = Session{"bob", "bob@example.com", "", false}; // missing ssn
        SESSIONS[SID3] = Session{"carol", "carol@example.com", "987-65-4321", true};
    }

    uint16_t port = 18105;
    std::atomic<bool> ready(false);
    std::atomic<bool> stop_flag(false);
    std::thread th(server_thread, port, std::ref(ready), std::ref(stop_flag));
    auto start = std::chrono::steady_clock::now();
    while (!ready.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (std::chrono::steady_clock::now() - start > std::chrono::seconds(2)) break;
    }

    int passed = 0, total = 5;
    const std::string host = "127.0.0.1";
    const std::string path = "/info";

    // Test 1: valid SID, do not print body
    std::string expected1 = "username=alice; email=alice@example.com; ssn=123-45-6789";
    if (http_test("GET", host, port, path, "SID=" + SID1, 200, &expected1)) passed++;

    // Test 2: missing cookie
    if (http_test("GET", host, port, path, "", 401, nullptr)) passed++;

    // Test 3: unknown SID
    if (http_test("GET", host, port, path, "SID=deadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeef", 401, nullptr)) passed++;

    // Test 4: missing ssn -> 400
    if (http_test("GET", host, port, path, "SID=" + SID2, 400, nullptr)) passed++;

    // Test 5: wrong method -> 405
    if (http_test("POST", host, port, path, "SID=" + SID1, 405, nullptr)) passed++;

    std::cout << "Tests passed: " << passed << "/" << total << std::endl;

    stop_flag.store(true);
    th.join();
    return 0;
}