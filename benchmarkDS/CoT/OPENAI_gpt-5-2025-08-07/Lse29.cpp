#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>

static const std::regex ALLOW_RE("(?i)^https?://[^\\s]*example\\.com[^\\s]*$");
static const size_t MAX_LEN = 2048;

std::string urlDecode(const std::string &s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '%' && i + 2 < s.size()) {
            char hi = s[i + 1], lo = s[i + 2];
            auto hex = [](char c) -> int {
                if (c >= '0' && c <= '9') return c - '0';
                if (c >= 'a' && c <= 'f') return c - 'a' + 10;
                if (c >= 'A' && c <= 'F') return c - 'A' + 10;
                return -1;
            };
            int h = hex(hi), l = hex(lo);
            if (h >= 0 && l >= 0) {
                out.push_back(static_cast<char>(h * 16 + l));
                i += 2;
            } else {
                out.push_back(s[i]);
            }
        } else if (s[i] == '+') {
            out.push_back(' ');
        } else {
            out.push_back(s[i]);
        }
    }
    return out;
}

bool containsCtl(const std::string &s) {
    for (unsigned char c : s) {
        if (c == '\r' || c == '\n') return true;
        if (c < 0x20 && c != '\t') return true;
        if (c == 0x7F) return true;
    }
    return false;
}

std::string evaluate_redirect(const std::string &target_in) {
    if (target_in.empty()) return "/";
    std::string target = target_in;
    // trim
    size_t start = target.find_first_not_of(" \t\r\n");
    size_t end = target.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) return "/";
    target = target.substr(start, end - start + 1);
    if (target.size() == 0 || target.size() > MAX_LEN) return "/";
    if (containsCtl(target)) return "/";
    if (!std::regex_match(target, ALLOW_RE)) return "/";
    return target;
}

std::string get_query_param(const std::string &query, const std::string &key) {
    size_t pos = 0;
    while (pos < query.size()) {
        size_t amp = query.find('&', pos);
        std::string pair = query.substr(pos, amp == std::string::npos ? std::string::npos : amp - pos);
        size_t eq = pair.find('=');
        std::string k = eq == std::string::npos ? pair : pair.substr(0, eq);
        std::string v = eq == std::string::npos ? "" : pair.substr(eq + 1);
        if (urlDecode(k) == key) return urlDecode(v);
        if (amp == std::string::npos) break;
        pos = amp + 1;
    }
    return "";
}

void send_response(int client_fd, const std::string &location) {
    std::ostringstream oss;
    oss << "HTTP/1.1 302 Found\r\n";
    oss << "Location: " << location << "\r\n";
    oss << "Content-Type: text/plain; charset=utf-8\r\n";
    oss << "Cache-Control: no-store\r\n";
    oss << "Content-Security-Policy: default-src 'none'; frame-ancestors 'none'\r\n";
    std::string body = "Redirecting to: " + location + "\n";
    oss << "Content-Length: " << body.size() << "\r\n";
    oss << "Connection: close\r\n\r\n";
    oss << body;
    std::string resp = oss.str();
    send(client_fd, resp.c_str(), resp.size(), 0);
}

void run_server_cpp() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return;
    }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8082);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(server_fd, (sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return;
    }
    if (listen(server_fd, 8) < 0) {
        perror("listen");
        close(server_fd);
        return;
    }
    std::cout << "C++ server listening on http://127.0.0.1:8082/go?target=...\n";
    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) {
            perror("accept");
            break;
        }
        char buf[4096];
        ssize_t n = recv(client_fd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) {
            close(client_fd);
            continue;
        }
        buf[n] = '\0';
        std::string req(buf);
        // parse request line
        size_t line_end = req.find("\r\n");
        std::string request_line = line_end == std::string::npos ? req : req.substr(0, line_end);
        // Expect "GET /go?target=... HTTP/1.1"
        size_t sp1 = request_line.find(' ');
        size_t sp2 = request_line.find(' ', sp1 == std::string::npos ? 0 : sp1 + 1);
        std::string path_query = (sp1 != std::string::npos && sp2 != std::string::npos) ? request_line.substr(sp1 + 1, sp2 - sp1 - 1) : "/";
        std::string path = path_query;
        std::string query;
        size_t qpos = path_query.find('?');
        if (qpos != std::string::npos) {
            path = path_query.substr(0, qpos);
            query = path_query.substr(qpos + 1);
        }
        std::string target;
        if (path == "/go") {
            target = get_query_param(query, "target");
        }
        std::string location = evaluate_redirect(target);
        send_response(client_fd, location);
        close(client_fd);
    }
    close(server_fd);
}

int main() {
    // 5 test cases
    std::string tests[] = {
        "https://example.com",
        "http://sub.example.com/path?x=1",
        "https://notexample.com",
        "https://example.com.evil.com/attack",
        "/local/path?target=https://example.com"
    };
    for (int i = 0; i < 5; ++i) {
        std::cout << "C++ Test " << (i + 1) << ": input=" << tests[i] << " -> redirect=" << evaluate_redirect(tests[i]) << "\n";
    }
    run_server_cpp();
    return 0;
}