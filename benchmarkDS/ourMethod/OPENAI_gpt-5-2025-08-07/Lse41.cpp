#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <cctype>
#include <cstring>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

// HTML Escape
static std::string html_escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 16);
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#x27;"; break;
            case '/': out += "&#x2F;"; break;
            default: out.push_back(c);
        }
    }
    return out;
}

static bool is_valid_username(const std::string& u) {
    if (u.empty() || u.size() > 64) return false;
    for (char c : u) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-' || c == '.')) {
            return false;
        }
    }
    return true;
}

static bool is_valid_email(const std::string& e) {
    if (e.size() < 5 || e.size() > 254) return false;
    size_t at = e.find('@');
    if (at == std::string::npos || at != e.rfind('@') || at == 0 || at + 1 >= e.size()) return false;
    std::string local = e.substr(0, at);
    std::string domain = e.substr(at + 1);
    if (domain.front() == '.' || domain.back() == '.') return false;
    if (domain.find('.') == std::string::npos) return false;
    auto ok_local = [](const std::string& s)->bool {
        for (char c : s) {
            if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '_' || c == '%' || c == '+' || c == '-')) return false;
        }
        return !s.empty();
    };
    auto ok_domain = [](const std::string& s)->bool {
        for (char c : s) {
            if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '-')) return false;
        }
        return !s.empty();
    };
    return ok_local(local) && ok_domain(domain);
}

static std::optional<std::string> extract_last4(const std::string& ssn) {
    std::string digits;
    digits.reserve(16);
    for (char c : ssn) {
        if (std::isdigit(static_cast<unsigned char>(c))) {
            digits.push_back(c);
            if (digits.size() > 16) break;
        }
    }
    if (digits.size() < 4) return std::nullopt;
    return digits.substr(digits.size() - 4);
}

static std::string generate_info_page(const std::string& username, const std::string& email, const std::string& ssn) {
    if (!is_valid_username(username)) throw std::invalid_argument("Invalid username");
    if (!is_valid_email(email)) throw std::invalid_argument("Invalid email");
    auto last4opt = extract_last4(ssn);
    if (!last4opt.has_value()) throw std::invalid_argument("Invalid SSN");
    std::string u = html_escape(username);
    std::string e = html_escape(email);
    std::string l4 = html_escape(last4opt.value());

    std::ostringstream oss;
    oss << "<!doctype html><html><head><meta charset=\"utf-8\">"
        << "<title>User Info</title></head><body>"
        << "<h1>User Info</h1><ul>"
        << "<li>Username: " << u << "</li>"
        << "<li>Email: " << e << "</li>"
        << "<li>SSN (last 4): " << l4 << "</li>"
        << "</ul></body></html>";
    return oss.str();
}

static std::string url_decode(const std::string& in) {
    std::string out;
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        char c = in[i];
        if (c == '%' && i + 2 < in.size()) {
            char h1 = in[i+1], h2 = in[i+2];
            auto hexval = [](char h)->int {
                if (h >= '0' && h <= '9') return h - '0';
                if (h >= 'a' && h <= 'f') return 10 + (h - 'a');
                if (h >= 'A' && h <= 'F') return 10 + (h - 'A');
                return -1;
            };
            int v1 = hexval(h1), v2 = hexval(h2);
            if (v1 >= 0 && v2 >= 0) {
                out.push_back(static_cast<char>((v1 << 4) | v2));
                i += 2;
            } else {
                out.push_back(c);
            }
        } else if (c == '+') {
            out.push_back(' ');
        } else {
            out.push_back(c);
        }
    }
    return out;
}

static std::map<std::string, std::string> parse_query(const std::string& q) {
    std::map<std::string, std::string> m;
    size_t start = 0;
    while (start < q.size()) {
        size_t amp = q.find('&', start);
        std::string pair = q.substr(start, (amp == std::string::npos ? q.size() : amp) - start);
        size_t eq = pair.find('=');
        if (eq != std::string::npos) {
            std::string k = url_decode(pair.substr(0, eq));
            std::string v = url_decode(pair.substr(eq + 1));
            m[k] = v;
        } else {
            m[url_decode(pair)] = "";
        }
        if (amp == std::string::npos) break;
        start = amp + 1;
    }
    return m;
}

static void run_tests() {
    struct Case { std::string u,e,s; bool shouldPass; };
    Case cases[5] = {
        {"alice_01","alice@example.com","123-45-6789",true},
        {"bob","bob@example.org","0000",true},
        {"charlie.z","charlie.z@example.co.uk","111223333",true},
        {"bad user","x@example.com","111-22-3333",false},
        {"dana","invalid","9999",false}
    };
    int passed = 0;
    for (int i=0;i<5;i++) {
        try {
            std::string page = generate_info_page(cases[i].u, cases[i].e, cases[i].s);
            bool ok = page.find("<script") == std::string::npos;
            auto l4 = extract_last4(cases[i].s);
            if (l4.has_value()) ok = ok && (page.find(l4.value()) != std::string::npos);
            if (cases[i].shouldPass && ok) passed++;
        } catch (...) {
            if (!cases[i].shouldPass) passed++;
        }
    }
    // std::cout << "Tests passed: " << passed << "/5\n";
}

static void send_response(int client_fd, int code, const std::string& content_type, const std::string& body) {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << code << " "
        << (code==200?"OK":(code==400?"Bad Request":(code==404?"Not Found":"Error"))) << "\r\n";
    oss << "Content-Type: " << content_type << "\r\n";
    oss << "X-Content-Type-Options: nosniff\r\n";
    oss << "X-Frame-Options: DENY\r\n";
    oss << "Content-Security-Policy: default-src 'none'; style-src 'unsafe-inline'\r\n";
    oss << "Content-Length: " << body.size() << "\r\n";
    oss << "Connection: close\r\n\r\n";
    std::string hdr = oss.str();
    std::string resp = hdr + body;
    ssize_t total = 0;
    const char* buf = resp.c_str();
    size_t left = resp.size();
    while (left > 0) {
        ssize_t n = ::send(client_fd, buf + total, left, 0);
        if (n <= 0) break;
        total += n;
        left -= n;
    }
}

static void serve(uint16_t port) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return;
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(port);
    if (bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0) { close(fd); return; }
    if (listen(fd, 16) < 0) { close(fd); return; }

    char buf[4096];
    for (;;) {
        int cfd = accept(fd, nullptr, nullptr);
        if (cfd < 0) continue;
        ssize_t n = recv(cfd, buf, sizeof(buf)-1, 0);
        if (n <= 0) { close(cfd); continue; }
        buf[n] = '\0';
        // Parse request line
        std::string req(buf, buf + n);
        size_t line_end = req.find("\r\n");
        std::string line = (line_end == std::string::npos) ? req : req.substr(0, line_end);
        std::istringstream iss(line);
        std::string method, target, version;
        iss >> method >> target >> version;
        if (method != "GET") {
            send_response(cfd, 405, "text/plain; charset=UTF-8", "Method Not Allowed");
            close(cfd);
            continue;
        }
        // Extract path and query
        std::string path = target;
        std::string query;
        size_t qpos = target.find('?');
        if (qpos != std::string::npos) {
            path = target.substr(0, qpos);
            query = target.substr(qpos + 1);
        }
        if (path != "/info") {
            send_response(cfd, 404, "text/plain; charset=UTF-8", "Not Found");
            close(cfd);
            continue;
        }
        auto params = parse_query(query);
        auto itU = params.find("username");
        auto itE = params.find("email");
        auto itS = params.find("ssn");
        if (itU == params.end() || itE == params.end() || itS == params.end()) {
            send_response(cfd, 400, "text/plain; charset=UTF-8", "Missing parameters");
            close(cfd);
            continue;
        }
        try {
            std::string body = generate_info_page(itU->second, itE->second, itS->second);
            send_response(cfd, 200, "text/html; charset=UTF-8", body);
        } catch (...) {
            send_response(cfd, 400, "text/plain; charset=UTF-8", "Invalid input");
        }
        close(cfd);
    }
    close(fd);
}

int main() {
    run_tests();
    serve(8080);
    return 0;
}