#include <arpa/inet.h>
#include <cerrno>
#include <cctype>
#include <csignal>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

// Function that accepts input as parameter and returns output
std::string build_message(const std::string& input) {
    std::string name = input;
    // trim
    auto l = name.find_first_not_of(" \t\r\n");
    auto r = name.find_last_not_of(" \t\r\n");
    if (l == std::string::npos) name.clear();
    else name = name.substr(l, r - l + 1);
    if (name.empty()) name = "World";
    if (name.size() < 1 || name.size() > 50) throw std::invalid_argument("Invalid length");
    for (unsigned char ch : name) {
        if (!(std::isalnum(ch) || ch == ' ' || ch == '_' || ch == '-')) {
            throw std::invalid_argument("Invalid characters");
        }
    }
    return "Hello, " + name + "!";
}

static std::string url_decode(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '+') {
            out.push_back(' ');
        } else if (s[i] == '%' && i + 2 < s.size()) {
            auto hex = s.substr(i + 1, 2);
            char* endp = nullptr;
            long v = strtol(hex.c_str(), &endp, 16);
            if (endp != nullptr && *endp == '\0' && v >= 0 && v <= 255) {
                out.push_back(static_cast<char>(v));
                i += 2;
            } else {
                // invalid percent-encoding, keep literal
                out.push_back(s[i]);
            }
        } else {
            out.push_back(s[i]);
        }
    }
    return out;
}

static std::string get_name_from_request_line(const std::string& req) {
    // Expect: GET /?name=value HTTP/1.1
    size_t methodEnd = req.find(' ');
    if (methodEnd == std::string::npos) return "";
    size_t pathEnd = req.find(' ', methodEnd + 1);
    if (pathEnd == std::string::npos) return "";
    std::string path = req.substr(methodEnd + 1, pathEnd - (methodEnd + 1));
    size_t qpos = path.find('?');
    if (qpos == std::string::npos) return "";
    std::string query = path.substr(qpos + 1);
    // parse query params
    size_t start = 0;
    while (start <= query.size()) {
        size_t amp = query.find('&', start);
        std::string pair = query.substr(start, (amp == std::string::npos) ? std::string::npos : amp - start);
        size_t eq = pair.find('=');
        std::string key = (eq == std::string::npos) ? pair : pair.substr(0, eq);
        std::string val = (eq == std::string::npos) ? "" : pair.substr(eq + 1);
        if (key == "name") {
            return url_decode(val);
        }
        if (amp == std::string::npos) break;
        start = amp + 1;
    }
    return "";
}

static volatile sig_atomic_t stop_flag = 0;
static void handle_sigint(int) { stop_flag = 1; }

int main() {
    // 5 test cases
    const char* tests[5] = {"World", "Alice", "Bob_12", "Eve-Ok", " "};
    for (const char* t : tests) {
        try {
            std::cout << build_message(std::string(t)) << std::endl;
        } catch (const std::exception& e) {
            std::cout << std::string("Error: ") + e.what() << std::endl;
        }
    }

    // Minimal HTTP server on localhost:8082
    std::signal(SIGINT, handle_sigint);
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        std::perror("socket");
        return 1;
    }

    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::perror("setsockopt");
        close(fd);
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8082);
    if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) != 1) {
        std::perror("inet_pton");
        close(fd);
        return 1;
    }

    if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::perror("bind");
        close(fd);
        return 1;
    }
    if (listen(fd, 16) < 0) {
        std::perror("listen");
        close(fd);
        return 1;
    }

    while (!stop_flag) {
        int cfd = accept(fd, nullptr, nullptr);
        if (cfd < 0) {
            if (errno == EINTR && stop_flag) break;
            std::perror("accept");
            continue;
        }
        char buf[1024];
        ssize_t n = recv(cfd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) {
            close(cfd);
            continue;
        }
        buf[n] = '\0';
        std::string req(buf);
        std::string name = get_name_from_request_line(req);
        std::string body;
        int status = 200;
        try {
            body = build_message(name);
        } catch (...) {
            status = 400;
            body = "Invalid name";
        }
        std::string statusText = (status == 200) ? "OK" : "Bad Request";
        std::string resp = "HTTP/1.1 " + std::to_string(status) + " " + statusText + "\r\n"
                           "Content-Type: text/plain; charset=utf-8\r\n"
                           "Content-Length: " + std::to_string(body.size()) + "\r\n"
                           "Connection: close\r\n"
                           "\r\n" + body;
        send(cfd, resp.data(), resp.size(), 0);
        close(cfd);
    }
    close(fd);
    return 0;
}