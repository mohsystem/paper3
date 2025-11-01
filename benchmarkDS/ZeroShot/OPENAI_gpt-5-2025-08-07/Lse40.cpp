#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <functional>
#include <iostream>
#include <random>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>

struct SessionData {
    std::string username;
    std::string email;
    std::string ssn;
};

static std::unordered_map<std::string, SessionData> SESSIONS;

static std::string htmlEscape(const std::string &s) {
    std::string out;
    out.reserve(s.size() * 2);
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#x27;"; break;
            case '/': out += "&#x2F;"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20 && c != '\n' && c != '\r' && c != '\t') {
                    // skip control chars
                } else {
                    out += c;
                }
        }
    }
    return out;
}

static std::string last4Digits(const std::string &ssn) {
    std::string digits;
    digits.reserve(ssn.size());
    for (char c : ssn) {
        if (c >= '0' && c <= '9') digits.push_back(c);
    }
    if (digits.size() < 4) return "****";
    return digits.substr(digits.size() - 4);
}

// Core function: accepts inputs and returns the resulting HTML page.
static std::string render_info_page(const std::string &username, const std::string &email, const std::string &ssn) {
    std::string safeUser = htmlEscape(username);
    std::string safeEmail = htmlEscape(email);
    std::string l4 = htmlEscape(last4Digits(ssn));

    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buf[128];
    std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", std::gmtime(&now));

    std::ostringstream oss;
    oss << "<!doctype html><html lang=\"en\"><head>"
        << "<meta charset=\"utf-8\">"
        << "<meta http-equiv=\"Content-Security-Policy\" content=\"default-src 'self'\">"
        << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
        << "<title>Info</title>"
        << "</head><body>"
        << "<h1>Account Info</h1>"
        << "<ul>"
        << "<li>Username: " << safeUser << "</li>"
        << "<li>Email: " << safeEmail << "</li>"
        << "<li>SSN (last 4): " << l4 << "</li>"
        << "</ul>"
        << "<p>Generated at " << htmlEscape(std::string(buf)) << "</p>"
        << "</body></html>";
    return oss.str();
}

static std::string randomSID() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);
    std::ostringstream oss;
    for (int i = 0; i < 8; ++i) {
        uint32_t v = dist(gen);
        oss << std::hex;
        oss.width(8);
        oss.fill('0');
        oss << v;
    }
    std::string sid = oss.str();
    for (auto &c : sid) c = std::toupper(c);
    return sid; // 64 hex chars
}

static std::string getHeader(const std::string &req, const std::string &name) {
    std::string nl = "\r\n";
    std::string key = name + ":";
    size_t pos = 0;
    while (true) {
        pos = req.find(key, pos);
        if (pos == std::string::npos) return "";
        if (pos == 0 || req[pos - 1] == '\n') {
            size_t end = req.find("\r\n", pos);
            if (end == std::string::npos) return "";
            size_t startVal = pos + key.size();
            while (startVal < end && (req[startVal] == ' ' || req[startVal] == '\t')) startVal++;
            return req.substr(startVal, end - startVal);
        }
        pos += key.size();
    }
}

static std::string getCookieValue(const std::string &cookieHeader, const std::string &name) {
    if (cookieHeader.empty()) return "";
    std::string needle = name + "=";
    size_t start = 0;
    while (start < cookieHeader.size()) {
        size_t sep = cookieHeader.find(';', start);
        std::string part = cookieHeader.substr(start, sep == std::string::npos ? std::string::npos : sep - start);
        // trim
        size_t b = part.find_first_not_of(" \t");
        size_t e = part.find_last_not_of(" \t");
        if (b != std::string::npos) part = part.substr(b, e - b + 1);
        if (part.rfind(needle, 0) == 0) {
            return part.substr(needle.size());
        }
        if (sep == std::string::npos) break;
        start = sep + 1;
    }
    return "";
}

int main() {
    // 5 test cases for render_info_page
    std::cout << render_info_page("alice", "alice@example.com", "123-45-6789") << std::endl;
    std::cout << render_info_page("bob<script>", "bob@example.com", "000-00-0001") << std::endl;
    std::cout << render_info_page("carol", "carol+test@example.com", "9999") << std::endl;
    std::cout << render_info_page("dave", "dave.o'reilly@example.com", "12-3456") << std::endl;
    std::cout << render_info_page("eve", "eve@example.com", "abcd-ef-ghij") << std::endl;

    int server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::perror("socket");
        return 1;
    }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8082);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::perror("bind");
        close(server_fd);
        return 1;
    }
    if (listen(server_fd, 10) < 0) {
        std::perror("listen");
        close(server_fd);
        return 1;
    }
    std::cout << "C++ server started on http://127.0.0.1:8082/info" << std::endl;

    while (true) {
        int client = accept(server_fd, nullptr, nullptr);
        if (client < 0) {
            std::perror("accept");
            continue;
        }
        char buffer[4096];
        ssize_t n = recv(client, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            close(client);
            continue;
        }
        buffer[n] = '\0';
        std::string req(buffer);
        // Basic parse
        if (req.rfind("GET /info", 0) != 0 && req.find("GET /info ") == std::string::npos) {
            std::string resp = "HTTP/1.1 404 Not Found\r\nContent-Length: 9\r\nConnection: close\r\n\r\nNot Found";
            send(client, resp.c_str(), resp.size(), 0);
            close(client);
            continue;
        }
        std::string cookie = getHeader(req, "Cookie");
        std::string sid = getCookieValue(cookie, "SID");
        bool newSession = false;
        std::regex sidRegex("^[A-Fa-f0-9]{64}$");
        if (sid.empty() || !std::regex_match(sid, sidRegex) || SESSIONS.find(sid) == SESSIONS.end()) {
            sid = randomSID();
            SESSIONS[sid] = SessionData{"demo_user", "demo.user@example.com", "111-22-3333"};
            newSession = true;
        }
        SessionData sd = SESSIONS[sid];
        std::string body = render_info_page(sd.username, sd.email, sd.ssn);
        std::ostringstream resp;
        resp << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: text/html; charset=utf-8\r\n"
             << "X-Content-Type-Options: nosniff\r\n"
             << "X-Frame-Options: DENY\r\n"
             << "Referrer-Policy: no-referrer\r\n"
             << "Cache-Control: no-store\r\n";
        if (newSession) {
            resp << "Set-Cookie: SID=" << sid << "; Path=/; HttpOnly; SameSite=Strict; Secure\r\n";
        }
        resp << "Content-Length: " << body.size() << "\r\n"
             << "Connection: close\r\n\r\n"
             << body;
        std::string respStr = resp.str();
        send(client, respStr.c_str(), respStr.size(), 0);
        close(client);
    }
    close(server_fd);
    return 0;
}