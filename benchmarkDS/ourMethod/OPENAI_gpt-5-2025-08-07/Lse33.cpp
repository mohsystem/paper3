#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <vector>
#include <cstring>

#ifdef __unix__
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

struct RedirectResult {
    int statusCode;       // 302 or 400
    std::string location; // when 302
    std::string body;     // when 400
};

static const size_t MAX_URL_LENGTH = 2048;
static const size_t MAX_QUERY_LENGTH = 4096;

static std::string trim(const std::string &s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(start, end - start);
}

static bool starts_with_icase(const std::string &s, const std::string &prefix) {
    if (s.size() < prefix.size()) return false;
    for (size_t i = 0; i < prefix.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(s[i])) != std::tolower(static_cast<unsigned char>(prefix[i]))) {
            return false;
        }
    }
    return true;
}

static std::string to_lower(std::string v) {
    std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return v;
}

static std::string url_decode(const std::string &s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '%' && i + 2 < s.size()) {
            char h1 = s[i+1], h2 = s[i+2];
            auto hex = [](char c)->int {
                if (c >= '0' && c <= '9') return c - '0';
                if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
                if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
                return -1;
            };
            int v1 = hex(h1), v2 = hex(h2);
            if (v1 >= 0 && v2 >= 0) {
                out.push_back(static_cast<char>(v1 * 16 + v2));
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

RedirectResult redirect_invalid_url(const std::string &url) {
    if (url.empty()) {
        return {400, "", "Invalid URL"};
    }
    std::string trimmed = trim(url);
    if (trimmed.empty() || trimmed.size() > MAX_URL_LENGTH) {
        return {400, "", "Invalid URL"};
    }
    if (trimmed.find('\r') != std::string::npos || trimmed.find('\n') != std::string::npos) {
        return {400, "", "Invalid URL"};
    }
    if (!(starts_with_icase(trimmed, "http://") || starts_with_icase(trimmed, "https://"))) {
        return {400, "", "Invalid URL"};
    }
    size_t scheme_end = trimmed.find("://");
    if (scheme_end == std::string::npos) {
        return {400, "", "Invalid URL"};
    }
    size_t host_start = scheme_end + 3;
    size_t auth_end = trimmed.find_first_of("/?#", host_start);
    if (auth_end == std::string::npos) auth_end = trimmed.size();
    std::string authority = trimmed.substr(host_start, auth_end - host_start);
    if (authority.empty() || authority.size() > 255) {
        return {400, "", "Invalid URL"};
    }
    if (authority.find('@') != std::string::npos) {
        return {400, "", "Invalid URL"};
    }
    std::string host = authority;
    size_t colon = authority.find(':');
    if (colon != std::string::npos) host = authority.substr(0, colon);
    if (to_lower(host) != "example.com") {
        return {400, "", "Invalid URL"};
    }
    std::string safeLocation;
    safeLocation.reserve(trimmed.size());
    for (char c : trimmed) {
        if (c != '\r' && c != '\n') safeLocation.push_back(c);
    }
    return {302, safeLocation, ""};
}

#ifdef __unix__
static void send_all(int fd, const std::string &data) {
    size_t sent = 0;
    while (sent < data.size()) {
        ssize_t n = ::send(fd, data.data() + sent, data.size() - sent, 0);
        if (n <= 0) break;
        sent += static_cast<size_t>(n);
    }
}

static void run_server(uint16_t port) {
    int srv = ::socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) {
        std::cerr << "Socket error\n";
        return;
    }
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(port);
    if (bind(srv, (sockaddr*)&addr, sizeof(addr)) != 0) {
        std::cerr << "Bind error\n";
        close(srv);
        return;
    }
    if (listen(srv, 5) != 0) {
        std::cerr << "Listen error\n";
        close(srv);
        return;
    }
    std::cout << "Server on http://127.0.0.1:" << port << "/go?url=http%3A%2F%2Fexample.com%2F\n";
    for (;;) {
        int cfd = accept(srv, nullptr, nullptr);
        if (cfd < 0) break;
        char buf[4096];
        ssize_t n = recv(cfd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) { close(cfd); continue; }
        buf[n] = '\0';
        // Parse request line
        std::string req(buf);
        size_t line_end = req.find("\r\n");
        if (line_end == std::string::npos) { close(cfd); continue; }
        std::string line = req.substr(0, line_end);
        // Expect: GET /go?url=... HTTP/1.1
        std::string method = "GET ";
        if (line.rfind(method, 0) != 0) {
            send_all(cfd, "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n");
            close(cfd);
            continue;
        }
        size_t sp1 = line.find(' ');
        size_t sp2 = line.find(' ', sp1 + 1);
        if (sp1 == std::string::npos || sp2 == std::string::npos) {
            send_all(cfd, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
            close(cfd);
            continue;
        }
        std::string target = line.substr(sp1 + 1, sp2 - sp1 - 1);
        size_t qpos = target.find('?');
        std::string path = (qpos == std::string::npos) ? target : target.substr(0, qpos);
        std::string query = (qpos == std::string::npos) ? "" : target.substr(qpos + 1);
        if (path != "/go" || query.empty() || query.size() > MAX_QUERY_LENGTH) {
            send_all(cfd, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
            close(cfd);
            continue;
        }
        // find url param
        std::string urlParam;
        size_t start = 0;
        while (start <= query.size()) {
            size_t amp = query.find('&', start);
            std::string pair = query.substr(start, amp == std::string::npos ? std::string::npos : (amp - start));
            size_t eq = pair.find('=');
            std::string key = (eq == std::string::npos) ? pair : pair.substr(0, eq);
            if (key == "url") {
                std::string val = (eq == std::string::npos) ? "" : pair.substr(eq + 1);
                urlParam = url_decode(val);
                break;
            }
            if (amp == std::string::npos) break;
            start = amp + 1;
        }
        if (urlParam.empty()) {
            send_all(cfd, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
            close(cfd);
            continue;
        }
        RedirectResult rr = redirect_invalid_url(urlParam);
        if (rr.statusCode == 302) {
            std::string resp = "HTTP/1.1 302 Found\r\nLocation: " + rr.location + "\r\nContent-Length: 0\r\n\r\n";
            send_all(cfd, resp);
        } else {
            std::string body = "Invalid URL";
            std::string resp = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: " +
                               std::to_string(body.size()) + "\r\n\r\n" + body;
            send_all(cfd, resp);
        }
        close(cfd);
    }
    close(srv);
}
#endif

int main(int argc, char** argv) {
    // 5 test cases
    std::vector<std::string> tests = {
        "http://example.com/",
        "https://example.com/path?x=1",
        "https://sub.example.com/",
        "ftp://example.com/",
        "https://example.com/\r\nX: a"
    };
    for (const auto &t : tests) {
        RedirectResult r = redirect_invalid_url(t);
        if (r.statusCode == 302) {
            std::cout << "OK 302 -> " << r.location << "\n";
        } else {
            std::cout << "ERR " << r.statusCode << "\n";
        }
    }

    // Optionally start server: ./a.out serve
#ifdef __unix__
    if (argc > 1 && std::string(argv[1]) == "serve") {
        run_server(8082);
    }
#endif
    return 0;
}