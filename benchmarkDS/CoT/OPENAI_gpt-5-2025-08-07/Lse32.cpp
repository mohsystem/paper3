// Chain-of-Through (concise):
// 1) Provide /redirect/<url>, redirect only to example.com.
// 2) Security: check scheme http/https, host equals example.com, no CR/LF, length limit.
// 3) Implement minimal HTTP server with strict parsing and safe headers.
// 4) Review: avoid buffer issues using std::string, cautious parsing.
// 5) Output: final code with tests.

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

static const size_t MAX_URL_LEN = 2048;

struct Result {
    int status;
    bool redirect;
    std::string location;
    std::string body;
};

static bool has_crlf(const std::string& s) {
    for (char c : s) { if (c == '\r' || c == '\n') return true; }
    return false;
}

static std::string tolower_str(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}

static std::string percent_decode(const std::string& in) {
    std::string out;
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        if (in[i] == '%' && i + 2 < in.size()) {
            int hi = std::isxdigit((unsigned char)in[i+1]) ? std::toupper(in[i+1]) : -1;
            int lo = std::isxdigit((unsigned char)in[i+2]) ? std::toupper(in[i+2]) : -1;
            if (hi != -1 && lo != -1) {
                int hv = (hi <= '9' ? hi - '0' : hi - 'A' + 10);
                int lv = (lo <= '9' ? lo - '0' : lo - 'A' + 10);
                out.push_back(static_cast<char>((hv << 4) + lv));
                i += 2;
            } else {
                out.push_back(in[i]);
            }
        } else {
            out.push_back(in[i]);
        }
    }
    return out;
}

static bool extract_scheme_host(const std::string& url, std::string& scheme, std::string& host, int& port) {
    scheme.clear(); host.clear(); port = -1;
    auto pos = url.find("://");
    if (pos == std::string::npos) return false;
    scheme = tolower_str(url.substr(0, pos));
    std::string rest = url.substr(pos + 3);
    // Remove userinfo if any
    auto at = rest.rfind('@');
    if (at != std::string::npos) rest = rest.substr(at + 1);
    // Host portion until / ? #
    size_t end = rest.find_first_of("/?#");
    std::string authority = (end == std::string::npos) ? rest : rest.substr(0, end);
    if (authority.empty()) return false;
    if (!authority.empty() && authority[0] == '[') {
        // IPv6 literal not allowed for example.com
        auto rb = authority.find(']');
        if (rb == std::string::npos) return false;
        host = authority.substr(1, rb - 1);
        // port
        if (rb + 1 < authority.size() && authority[rb + 1] == ':') {
            try {
                port = std::stoi(authority.substr(rb + 2));
            } catch (...) { port = -1; }
        }
    } else {
        auto colon = authority.find(':');
        if (colon == std::string::npos) {
            host = authority;
        } else {
            host = authority.substr(0, colon);
            try {
                port = std::stoi(authority.substr(colon + 1));
            } catch (...) { port = -1; }
        }
    }
    host = tolower_str(host);
    return true;
}

Result redirect_invalid_url(const std::string& input_url) {
    if (input_url.empty() || input_url.size() > MAX_URL_LEN || has_crlf(input_url)) {
        return {400, false, "", "Invalid or disallowed URL"};
    }
    std::string scheme, host;
    int port = -1;
    if (!extract_scheme_host(input_url, scheme, host, port)) {
        return {400, false, "", "Invalid or disallowed URL"};
    }
    if (!(scheme == "http" || scheme == "https")) {
        return {400, false, "", "Invalid or disallowed URL"};
    }
    if (host != "example.com") {
        return {400, false, "", "Invalid or disallowed URL"};
    }
    if (has_crlf(input_url)) {
        return {400, false, "", "Invalid or disallowed URL"};
    }
    return {302, true, input_url, "Redirecting..."};
}

static void send_response(int client_fd, const Result& res) {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << res.status << " "
        << (res.status == 302 ? "Found" : (res.status == 400 ? "Bad Request" : "OK")) << "\r\n";
    oss << "Content-Type: text/plain; charset=utf-8\r\n";
    if (res.redirect && !res.location.empty()) {
        oss << "Location: " << res.location << "\r\n";
    }
    std::string body = res.body.empty() ? "" : res.body;
    oss << "Content-Length: " << body.size() << "\r\n";
    oss << "Connection: close\r\n\r\n";
    oss << body;
    std::string resp = oss.str();
    send(client_fd, resp.c_str(), resp.size(), 0);
}

static void run_server() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::perror("socket");
        return;
    }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8082);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::perror("bind");
        close(server_fd);
        return;
    }
    if (listen(server_fd, 10) < 0) {
        std::perror("listen");
        close(server_fd);
        return;
    }
    std::cout << "C++ server listening on http://127.0.0.1:8082\n";
    char buf[4096];
    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) {
            std::perror("accept");
            break;
        }
        ssize_t n = recv(client_fd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) {
            close(client_fd);
            continue;
        }
        buf[n] = '\0';
        std::string req(buf, n);
        // Parse request line
        size_t sp1 = req.find(' ');
        size_t sp2 = req.find(' ', sp1 == std::string::npos ? 0 : sp1 + 1);
        if (sp1 == std::string::npos || sp2 == std::string::npos) {
            Result r = {400, false, "", "Invalid or disallowed URL"};
            send_response(client_fd, r);
            close(client_fd);
            continue;
        }
        std::string method = req.substr(0, sp1);
        std::string target = req.substr(sp1 + 1, sp2 - sp1 - 1);
        if (method != "GET") {
            Result r = {405, false, "", "Method Not Allowed"};
            send_response(client_fd, r);
            close(client_fd);
            continue;
        }
        const std::string prefix = "/redirect/";
        Result res;
        if (target == "/redirect") {
            res = {400, false, "", "Invalid or disallowed URL"};
        } else if (target.rfind(prefix, 0) == 0) {
            std::string encoded = target.substr(prefix.size());
            std::string decoded = percent_decode(encoded);
            res = redirect_invalid_url(decoded);
        } else {
            res = {404, false, "", "Not Found"};
        }
        send_response(client_fd, res);
        close(client_fd);
    }
    close(server_fd);
}

int main() {
    // 5 test cases
    std::string tests[] = {
        "https://example.com/path?q=1",
        "http://example.com",
        "https://notexample.com/bad",
        "javascript:alert(1)",
        "https://example.com:443/ok#frag"
    };
    for (const auto& t : tests) {
        Result r = redirect_invalid_url(t);
        std::cout << "Test: " << t << " => status=" << r.status << ", redirect=" << (r.redirect ? "true" : "false");
        if (!r.location.empty()) std::cout << ", location=" << r.location;
        std::cout << "\n";
    }
    run_server();
    return 0;
}