#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// HTML escape to prevent XSS
static std::string htmlEscape(const std::string &s) {
    std::string out;
    out.reserve(s.size() + 16);
    for (unsigned char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#39;"; break;
            default:
                if (c < 32) out += ' ';
                else out += c;
        }
    }
    return out;
}

static std::string safeTruncate(const std::string &s, size_t maxlen) {
    if (s.size() <= maxlen) return s;
    return s.substr(0, maxlen);
}

static std::string extractLast4(const std::string &ssn) {
    std::string digits;
    digits.reserve(4);
    for (int i = (int)ssn.size() - 1; i >= 0; --i) {
        if (std::isdigit((unsigned char)ssn[i])) {
            digits.push_back(ssn[i]);
            if (digits.size() == 4) break;
        }
    }
    if (digits.size() < 4) return "";
    std::reverse(digits.begin(), digits.end());
    return digits;
}

static bool isValidEmail(const std::string &email) {
    if (email.empty() || email.size() > 256) return false;
    auto at = email.find('@');
    if (at == std::string::npos || at == 0 || at == email.size() - 1) return false;
    auto dot = email.find('.', at + 1);
    if (dot == std::string::npos || dot == at + 1 || dot == email.size() - 1) return false;
    for (char c : email) {
        unsigned char uc = (unsigned char)c;
        if (uc <= 31 || uc == ' ' || uc == '@') return false;
    }
    return true;
}

static std::string buildInfoPage(const std::string &username, const std::string &email, const std::string &ssn) {
    std::string u = htmlEscape(safeTruncate(username, 256));
    std::string e = htmlEscape(safeTruncate(email, 256));
    std::string last4 = extractLast4(ssn);
    if (u.empty() || !isValidEmail(email) || last4.empty()) {
        return std::string("<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Invalid input</title></head><body><p>Invalid or missing parameters.</p></body></html>");
    }
    std::ostringstream body;
    body << "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Info</title></head><body>"
         << "<h1>User Info</h1><ul>"
         << "<li>Username: " << u << "</li>"
         << "<li>Email: " << e << "</li>"
         << "<li>SSN (last 4): " << last4 << "</li>"
         << "</ul></body></html>";
    return body.str();
}

static std::string urlDecode(const std::string &s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '%' && i + 2 < s.size() &&
            std::isxdigit((unsigned char)s[i + 1]) &&
            std::isxdigit((unsigned char)s[i + 2])) {
            int hi = std::isdigit((unsigned char)s[i+1]) ? s[i+1]-'0' : std::toupper(s[i+1])-'A'+10;
            int lo = std::isdigit((unsigned char)s[i+2]) ? s[i+2]-'0' : std::toupper(s[i+2])-'A'+10;
            out.push_back((char)((hi<<4) | lo));
            i += 2;
        } else if (s[i] == '+') {
            out.push_back(' ');
        } else {
            out.push_back(s[i]);
        }
    }
    return out;
}

static std::map<std::string, std::string> parseQuery(const std::string &qs) {
    std::map<std::string, std::string> m;
    size_t start = 0;
    while (start < qs.size()) {
        size_t amp = qs.find('&', start);
        if (amp == std::string::npos) amp = qs.size();
        std::string pair = qs.substr(start, amp - start);
        size_t eq = pair.find('=');
        std::string key = urlDecode(eq == std::string::npos ? pair : pair.substr(0, eq));
        std::string val = urlDecode(eq == std::string::npos ? "" : pair.substr(eq + 1));
        if (key.size() <= 64 && val.size() <= 512)
            m[key] = val;
        start = amp + 1;
    }
    return m;
}

static void sendAll(int fd, const std::string &data) {
    size_t sent = 0;
    while (sent < data.size()) {
        ssize_t n = ::send(fd, data.data() + sent, data.size() - sent, 0);
        if (n <= 0) break;
        sent += (size_t)n;
    }
}

static void handleClient(int cfd) {
    char buf[8192];
    ssize_t n = recv(cfd, buf, sizeof(buf) - 1, 0);
    if (n <= 0) { close(cfd); return; }
    buf[n] = '\0';
    // Parse request line
    std::istringstream iss(std::string(buf));
    std::string method, target, version;
    iss >> method >> target >> version;
    if (method != "GET") {
        std::string resp = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 18\r\nContent-Type: text/plain; charset=UTF-8\r\nX-Content-Type-Options: nosniff\r\n\r\nMethod Not Allowed";
        sendAll(cfd, resp);
        close(cfd);
        return;
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
        std::string body = "Not Found";
        std::ostringstream resp;
        resp << "HTTP/1.1 404 Not Found\r\n"
             << "Content-Length: " << body.size() << "\r\n"
             << "Content-Type: text/plain; charset=UTF-8\r\n"
             << "X-Content-Type-Options: nosniff\r\n\r\n"
             << body;
        sendAll(cfd, resp.str());
        close(cfd);
        return;
    }
    auto params = parseQuery(query);
    std::string username = params.count("username") ? params["username"] : "";
    std::string email = params.count("email") ? params["email"] : "";
    std::string ssn = params.count("ssn") ? params["ssn"] : "";
    std::string body;
    int status = 200;
    if (username.empty() || email.empty() || ssn.empty()) {
        body = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Bad Request</title></head><body><p>Missing required parameters: username, email, ssn.</p></body></html>";
        status = 400;
    } else {
        body = buildInfoPage(username, email, ssn);
        if (body.find("Invalid input") != std::string::npos) status = 400;
    }
    std::ostringstream resp;
    resp << "HTTP/1.1 " << (status == 200 ? "200 OK" : "400 Bad Request") << "\n"
         << "Content-Type: text/html; charset=UTF-8\r\n"
         << "Content-Security-Policy: default-src 'none'; style-src 'unsafe-inline'\r\n"
         << "X-Content-Type-Options: nosniff\r\n"
         << "Content-Length: " << body.size() << "\r\n\r\n"
         << body;
    sendAll(cfd, resp.str());
    close(cfd);
}

static void startServer(int port) {
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) return;
    int yes = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(sfd, (sockaddr *)&addr, sizeof(addr)) < 0) { close(sfd); return; }
    if (listen(sfd, 16) < 0) { close(sfd); return; }
    for (;;) {
        int cfd = accept(sfd, nullptr, nullptr);
        if (cfd < 0) continue;
        std::thread(handleClient, cfd).detach();
    }
}

int main() {
    // 5 test cases for the core function
    std::cout << buildInfoPage("alice", "alice@example.com", "123-45-6789") << std::endl;
    std::cout << buildInfoPage("<bob>", "bob@example.org", "0000") << std::endl;
    std::cout << buildInfoPage("charlie & co", "charlie@sub.example.com", "xxx-yy-9876") << std::endl;
    std::cout << buildInfoPage("dora", "invalid-email", "111-22-3333") << std::endl;
    std::cout << buildInfoPage("", "eve@example.net", "4444") << std::endl;

    // Start server on port 8082
    std::thread srv([](){ startServer(8082); });
    srv.join();
    return 0;
}