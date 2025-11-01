#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

static std::string trim(const std::string &s) {
    size_t b = 0, e = s.size();
    while (b < e && (unsigned char)s[b] <= ' ') b++;
    while (e > b && (unsigned char)s[e - 1] <= ' ') e--;
    return s.substr(b, e - b);
}

static int hexval(char c) {
    if ('0' <= c && c <= '9') return c - '0';
    if ('a' <= c && c <= 'f') return 10 + (c - 'a');
    if ('A' <= c && c <= 'F') return 10 + (c - 'A');
    return -1;
}

static std::string url_decode(const std::string &s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '+') {
            out.push_back(' ');
        } else if (c == '%' && i + 2 < s.size()) {
            int h1 = hexval(s[i + 1]);
            int h2 = hexval(s[i + 2]);
            if (h1 >= 0 && h2 >= 0) {
                out.push_back(static_cast<char>((h1 << 4) | h2));
                i += 2;
            } else {
                out.push_back(c);
            }
        } else {
            out.push_back(c);
        }
    }
    return out;
}

static std::string yaml_get_type(const std::string &payload) {
    std::string data = payload;
    for (size_t i = 0; i < data.size(); ++i) {
        if (data[i] == '\r') data[i] = '\n';
    }
    std::istringstream iss(data);
    std::string line;
    while (std::getline(iss, line)) {
        std::string l = trim(line);
        if (l.empty() || (!l.empty() && l[0] == '#')) continue;
        size_t pos = l.find(':');
        if (pos == std::string::npos) continue;
        std::string key = trim(l.substr(0, pos));
        std::string val = trim(l.substr(pos + 1));
        if (val.size() >= 2) {
            if ((val.front() == '"' && val.back() == '"') || (val.front() == '\'' && val.back() == '\'')) {
                val = val.substr(1, val.size() - 2);
            }
        }
        if (key == "type") return val;
    }
    return std::string();
}

static std::string handle_payload(const std::string &payload) {
    if (payload.empty()) return "Error: missing payload parameter";
    std::string t = yaml_get_type(payload);
    if (!t.empty() && t == "Create") {
        return "Error: payload type Create is not allowed";
    }
    return "OK";
}

static std::string process_query(const std::string &query) {
    if (query.empty()) return "Error: missing payload parameter";
    std::map<std::string, std::string> params;
    std::istringstream iss(query);
    std::string pair;
    while (std::getline(iss, pair, '&')) {
        if (pair.empty()) continue;
        size_t pos = pair.find('=');
        std::string k = url_decode(pos == std::string::npos ? pair : pair.substr(0, pos));
        std::string v = url_decode(pos == std::string::npos ? "" : pair.substr(pos + 1));
        params[k] = v;
    }
    auto it = params.find("payload");
    if (it == params.end() || it->second.empty()) return "Error: missing payload parameter";
    return handle_payload(it->second);
}

static void handle_client(int client_fd) {
    char buf[4096];
    ssize_t n = read(client_fd, buf, sizeof(buf) - 1);
    if (n <= 0) {
        close(client_fd);
        return;
    }
    buf[n] = '\0';
    std::string req(buf);
    // Parse first line
    size_t line_end = req.find("\r\n");
    if (line_end == std::string::npos) line_end = req.find('\n');
    std::string request_line = req.substr(0, line_end);
    // Expecting: GET /path?query HTTP/1.1
    std::string query;
    {
        size_t sp1 = request_line.find(' ');
        size_t sp2 = request_line.rfind(' ');
        if (sp1 != std::string::npos && sp2 != std::string::npos && sp2 > sp1) {
            std::string path = request_line.substr(sp1 + 1, sp2 - sp1 - 1);
            size_t qpos = path.find('?');
            if (qpos != std::string::npos) query = path.substr(qpos + 1);
        }
    }
    std::string body = process_query(query);
    std::ostringstream resp;
    resp << "HTTP/1.1 200 OK\r\n"
         << "Content-Type: text/plain; charset=utf-8\r\n"
         << "Content-Length: " << body.size() << "\r\n"
         << "Connection: close\r\n\r\n"
         << body;
    std::string out = resp.str();
    send(client_fd, out.c_str(), out.size(), 0);
    close(client_fd);
}

static void server_loop() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return;
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(5000);
    if (bind(fd, (sockaddr*)&addr, sizeof(addr)) != 0) {
        close(fd);
        return;
    }
    if (listen(fd, 16) != 0) {
        close(fd);
        return;
    }
    while (true) {
        int cfd = accept(fd, nullptr, nullptr);
        if (cfd < 0) continue;
        std::thread(handle_client, cfd).detach();
    }
}

int main() {
    std::thread th(server_loop);
    th.detach();

    // 5 test cases
    std::cout << handle_payload("") << std::endl;                  // Missing -> error
    std::cout << handle_payload("type: Create") << std::endl;      // Create -> error
    std::cout << handle_payload("type: Update") << std::endl;      // OK
    std::cout << handle_payload("name: x") << std::endl;           // OK
    std::cout << handle_payload("TYPE: Create") << std::endl;      // OK

    // Prevent exit
    while (true) pause();
    return 0;
}