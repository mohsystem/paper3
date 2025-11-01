#include <arpa/inet.h>
#include <netinet/in.h>
#include <regex>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>

bool matches_allowed(const std::string& target) {
    static const std::regex re(".*example\\.com.*");
    return std::regex_match(target, re);
}

std::string resolve_redirect(const std::string& target) {
    return matches_allowed(target) ? target : "/";
}

std::string url_decode(const std::string& s) {
    std::string out;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '%' && i + 2 < s.size()) {
            std::string hex = s.substr(i + 1, 2);
            char ch = static_cast<char>(strtol(hex.c_str(), nullptr, 16));
            out.push_back(ch);
            i += 2;
        } else if (s[i] == '+') {
            out.push_back(' ');
        } else {
            out.push_back(s[i]);
        }
    }
    return out;
}

std::string get_query_param(const std::string& query, const std::string& key) {
    size_t start = 0;
    while (start < query.size()) {
        size_t amp = query.find('&', start);
        if (amp == std::string::npos) amp = query.size();
        size_t eq = query.find('=', start);
        std::string k = query.substr(start, (eq == std::string::npos || eq > amp) ? amp - start : eq - start);
        std::string v;
        if (eq != std::string::npos && eq < amp) {
            v = query.substr(eq + 1, amp - (eq + 1));
        }
        if (k == key) {
            return url_decode(v);
        }
        start = amp + 1;
    }
    return "";
}

void send_response(int client, const std::string& status, const std::string& headers, const std::string& body) {
    std::string resp = "HTTP/1.1 " + status + "\r\n" + headers + "\r\n" + body;
    send(client, resp.c_str(), resp.size(), 0);
}

void handle_client(int client) {
    char buf[4096];
    ssize_t n = recv(client, buf, sizeof(buf) - 1, 0);
    if (n <= 0) {
        close(client);
        return;
    }
    buf[n] = 0;
    std::string req(buf);
    size_t lineEnd = req.find("\r\n");
    if (lineEnd == std::string::npos) lineEnd = req.find('\n');
    std::string line = req.substr(0, lineEnd);
    // Parse "GET /path?query HTTP/1.1"
    std::string method, target, version;
    {
        size_t p1 = line.find(' ');
        size_t p2 = (p1 == std::string::npos) ? std::string::npos : line.find(' ', p1 + 1);
        method = (p1 == std::string::npos) ? "" : line.substr(0, p1);
        target = (p2 == std::string::npos) ? "" : line.substr(p1 + 1, p2 - (p1 + 1));
        version = (p2 == std::string::npos) ? "" : line.substr(p2 + 1);
    }
    if (method != "GET") {
        send_response(client, "405 Method Not Allowed", "Content-Length: 0\r\nConnection: close", "");
        close(client);
        return;
    }
    std::string path = target;
    std::string query;
    size_t qpos = target.find('?');
    if (qpos != std::string::npos) {
        path = target.substr(0, qpos);
        query = target.substr(qpos + 1);
    }

    if (path == "/go") {
        std::string t = get_query_param(query, "target");
        std::string loc = resolve_redirect(t);
        std::string headers = "Location: " + loc + "\r\nContent-Length: 0\r\nConnection: close";
        send_response(client, "302 Found", headers, "");
    } else if (path == "/") {
        std::string body = "Home";
        std::string headers = "Content-Type: text/plain; charset=utf-8\r\nContent-Length: " + std::to_string(body.size()) + "\r\nConnection: close";
        send_response(client, "200 OK", headers, body);
    } else {
        std::string body = "Not Found";
        std::string headers = "Content-Type: text/plain; charset=utf-8\r\nContent-Length: " + std::to_string(body.size()) + "\r\nConnection: close";
        send_response(client, "404 Not Found", headers, body);
    }

    close(client);
}

void server_thread_func(int port, int max_requests) {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(port);
    bind(s, (sockaddr*)&addr, sizeof(addr));
    listen(s, 16);
    for (int i = 0; i < max_requests; ++i) {
        int client = accept(s, nullptr, nullptr);
        if (client >= 0) {
            handle_client(client);
        }
    }
    close(s);
}

std::pair<int, std::string> http_get_no_follow(const std::string& host, int port, const std::string& path) {
    int c = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host.c_str());
    addr.sin_port = htons(port);
    connect(c, (sockaddr*)&addr, sizeof(addr));
    std::string req = "GET " + path + " HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\n\r\n";
    send(c, req.c_str(), req.size(), 0);
    std::string resp;
    char buf[4096];
    ssize_t n;
    while ((n = recv(c, buf, sizeof(buf), 0)) > 0) {
        resp.append(buf, buf + n);
    }
    close(c);
    int status = 0;
    size_t sp1 = resp.find(' ');
    size_t sp2 = resp.find(' ', sp1 + 1);
    if (sp1 != std::string::npos && sp2 != std::string::npos) {
        status = std::stoi(resp.substr(sp1 + 1, sp2 - sp1 - 1));
    }
    std::string location;
    std::string needle = "\r\nLocation: ";
    size_t pos = resp.find(needle);
    if (pos != std::string::npos) {
        size_t start = pos + needle.size();
        size_t end = resp.find("\r\n", start);
        location = resp.substr(start, end - start);
    }
    return {status, location};
}

int main() {
    const int port = 19081;
    const int requests = 5;
    std::thread srv(server_thread_func, port, requests);
    // Give server time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    std::vector<std::string> tests = {
        "http://example.com/page",
        "https://sub.example.com",
        "http://evil.com/?next=example.com",
        "https://another.com/path",
        "" // missing target (request /go without target)
    };

    for (int i = 0; i < (int)tests.size(); ++i) {
        std::string path;
        if (tests[i].empty()) {
            path = "/go";
        } else {
            // simple URL-encoding for space and reserved chars
            auto encode = [](const std::string& s) {
                static const char* hex = "0123456789ABCDEF";
                std::string o;
                for (unsigned char ch : s) {
                    if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch=='-'||ch=='_'||ch=='.'||ch=='~') {
                        o.push_back(ch);
                    } else {
                        o.push_back('%');
                        o.push_back(hex[(ch >> 4) & 0xF]);
                        o.push_back(hex[ch & 0xF]);
                    }
                }
                return o;
            };
            path = "/go?target=" + encode(tests[i]);
        }
        auto res = http_get_no_follow("127.0.0.1", port, path);
        std::cout << "Test " << (i + 1) << ": target=" << (tests[i].empty() ? "(missing)" : tests[i]) << " -> redirect to " << res.second << std::endl;
    }

    srv.join();
    return 0;
}