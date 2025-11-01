#include <arpa/inet.h>
#include <csignal>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <regex>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

static const int PORT = 8082;
static const size_t MAX_PARAM_LEN = 2048;
static const size_t MAX_QUERY_LEN = 4096;
static const size_t RECV_BUF = 8192;

static std::regex allow_re("example\\.com", std::regex_constants::icase);

std::string strip_crlf(const std::string &s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c != '\r' && c != '\n') out.push_back(c);
    }
    return out;
}

std::string url_decode_limited(const std::string &in, size_t max_len) {
    std::string out;
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        if (out.size() >= max_len) break;
        unsigned char c = static_cast<unsigned char>(in[i]);
        if (c == '%' && i + 2 < in.size()) {
            auto hex = [](char ch)->int {
                if (ch >= '0' && ch <= '9') return ch - '0';
                if (ch >= 'a' && ch <= 'f') return 10 + (ch - 'a');
                if (ch >= 'A' && ch <= 'F') return 10 + (ch - 'A');
                return -1;
            };
            int hi = hex(in[i+1]);
            int lo = hex(in[i+2]);
            if (hi >= 0 && lo >= 0) {
                out.push_back(static_cast<char>((hi << 4) | lo));
                i += 2;
                continue;
            }
        }
        if (c == '+') out.push_back(' ');
        else out.push_back(static_cast<char>(c));
    }
    return out;
}

std::string decide_redirect(const std::string &target) {
    if (target.empty()) return "/";
    std::string cleaned = strip_crlf(target);
    if (cleaned.size() > MAX_PARAM_LEN) return "/";
    if (std::regex_search(cleaned, allow_re)) return cleaned;
    return "/";
}

std::string parse_target_param(const std::string &request_line) {
    // Extract path + query from "GET /path?query HTTP/1.1"
    size_t sp1 = request_line.find(' ');
    if (sp1 == std::string::npos) return "";
    size_t sp2 = request_line.find(' ', sp1 + 1);
    if (sp2 == std::string::npos) return "";
    std::string pathq = request_line.substr(sp1 + 1, sp2 - (sp1 + 1));
    size_t qpos = pathq.find('?');
    if (qpos == std::string::npos) return "";
    std::string query = pathq.substr(qpos + 1);
    if (query.size() > MAX_QUERY_LEN) return "";
    // find target=
    size_t start = 0;
    while (start < query.size()) {
        size_t amp = query.find('&', start);
        if (amp == std::string::npos) amp = query.size();
        size_t eq = query.find('=', start);
        if (eq != std::string::npos && eq < amp) {
            std::string name = query.substr(start, eq - start);
            if (name == "target") {
                std::string raw = query.substr(eq + 1, amp - (eq + 1));
                std::string dec = url_decode_limited(raw, MAX_PARAM_LEN);
                return dec;
            }
        }
        start = amp + 1;
    }
    return "";
}

void serve() {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        std::perror("socket");
        return;
    }
    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0) {
        std::perror("setsockopt");
        // continue anyway
    }
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(PORT);
    if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        std::perror("bind");
        close(fd);
        return;
    }
    if (listen(fd, 8) != 0) {
        std::perror("listen");
        close(fd);
        return;
    }
    std::cout << "C++ server listening on http://127.0.0.1:" << PORT << " - try /?target=http://example.com\n";

    char buf[RECV_BUF];
    while (true) {
        int cfd = accept(fd, nullptr, nullptr);
        if (cfd < 0) {
            if (errno == EINTR) continue;
            std::perror("accept");
            break;
        }
        // Read request up to headers end
        std::string req;
        while (req.size() < RECV_BUF) {
            ssize_t n = recv(cfd, buf, sizeof(buf), 0);
            if (n <= 0) break;
            req.append(buf, buf + n);
            if (req.find("\r\n\r\n") != std::string::npos) break;
        }
        // Extract request line
        std::string firstLine;
        size_t crlf = req.find("\r\n");
        if (crlf != std::string::npos) firstLine = req.substr(0, crlf);
        std::string target = parse_target_param(firstLine);
        std::string location = decide_redirect(target);
        std::string resp = "HTTP/1.1 302 Found\r\n"
                           "Location: " + location + "\r\n"
                           "Content-Length: 0\r\n"
                           "Connection: close\r\n"
                           "\r\n";
        send(cfd, resp.c_str(), resp.size(), 0);
        close(cfd);
    }
    close(fd);
}

int main() {
    // 5 test cases
    const char* tests[] = {
        "http://example.com/path",
        "https://sub.example.com/",
        "http://evil.com/?q=example.com",
        "/local/path",
        "javascript:example.com"
    };
    for (const char* t : tests) {
        std::cout << "Input: " << t << " -> Redirect: " << decide_redirect(std::string(t)) << "\n";
    }
    std::signal(SIGPIPE, SIG_IGN);
    serve();
    return 0;
}