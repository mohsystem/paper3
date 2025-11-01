#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

static const int PORT_CPP = 5000;
static const size_t MAX_BODY_SIZE_CPP = 1024 * 1024; // 1 MB

static std::string unquote(const std::string &s) {
    if (s.size() >= 2) {
        if ((s.front() == '"' && s.back() == '"') || (s.front() == '\'' && s.back() == '\'')) {
            return s.substr(1, s.size() - 2);
        }
    }
    return s;
}

static std::string strip_yaml_comments(const std::string &s) {
    bool inSingle = false, inDouble = false;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '\'' && !inDouble) inSingle = !inSingle;
        else if (c == '\"' && !inSingle) inDouble = !inDouble;
        else if (c == '#' && !inSingle && !inDouble) return s.substr(0, i);
    }
    return s;
}

static std::string trim(const std::string &s) {
    size_t b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) return "";
    size_t e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

static std::string parse_yaml_type(const std::string &yaml) {
    std::istringstream iss(yaml);
    std::string raw;
    while (std::getline(iss, raw)) {
        std::string line = trim(strip_yaml_comments(raw));
        if (line.empty()) continue;
        if (line.rfind("---", 0) == 0) continue;
        size_t pos = line.find(':');
        if (pos == std::string::npos || pos == 0) continue;
        std::string key = trim(line.substr(0, pos));
        std::string val = trim(line.substr(pos + 1));
        std::string keyLower = key;
        for (auto &ch : keyLower) ch = std::tolower(static_cast<unsigned char>(ch));
        if (keyLower == "type") {
            return unquote(val);
        }
    }
    return "";
}

static std::string url_decode(const std::string &s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '%' && i + 2 < s.size()) {
            char hex[3] = {s[i+1], s[i+2], 0};
            char *end = nullptr;
            long v = strtol(hex, &end, 16);
            if (*end == 0) {
                out.push_back(static_cast<char>(v));
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

static std::map<std::string, std::string> parse_query(const std::string &q) {
    std::map<std::string, std::string> m;
    std::istringstream iss(q);
    std::string pair;
    while (std::getline(iss, pair, '&')) {
        if (pair.empty()) continue;
        size_t pos = pair.find('=');
        std::string key = url_decode(pos == std::string::npos ? pair : pair.substr(0, pos));
        std::string val = pos == std::string::npos ? "" : url_decode(pair.substr(pos + 1));
        m[key] = val;
    }
    return m;
}

static std::string processPayloadCPP(const std::string &payload) {
    if (payload.empty()) return "Error: missing 'payload' parameter.";
    if (payload.size() > MAX_BODY_SIZE_CPP) return "Error: payload too large.";
    std::string t;
    try {
        t = parse_yaml_type(payload);
    } catch (...) {
        return "Error: invalid YAML payload.";
    }
    std::string tl = t;
    for (auto &c : tl) c = std::tolower(static_cast<unsigned char>(c));
    if (!tl.empty() && tl == "create") {
        return "Error: 'Create' type is not allowed.";
    }
    return "OK";
}

static void send_response(int client_fd, int status, const std::string &body) {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << status << " " << (status == 200 ? "OK" : "Bad Request") << "\r\n";
    oss << "Content-Type: text/plain; charset=utf-8\r\n";
    oss << "X-Content-Type-Options: nosniff\r\n";
    oss << "Cache-Control: no-store\r\n";
    oss << "Content-Length: " << body.size() << "\r\n";
    oss << "Connection: close\r\n";
    oss << "\r\n";
    oss << body;
    std::string resp = oss.str();
    (void)write(client_fd, resp.data(), resp.size());
}

static void run_server_cpp() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::perror("socket");
        return;
    }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT_CPP);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::perror("bind");
        close(server_fd);
        return;
    }
    if (listen(server_fd, 16) < 0) {
        std::perror("listen");
        close(server_fd);
        return;
    }
    std::cout << "C++ server listening on port " << PORT_CPP << std::endl;

    for (;;) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) {
            if (errno == EINTR) continue;
            std::perror("accept");
            break;
        }
        // Set a small recv timeout
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        std::string request;
        char buf[4096];
        ssize_t r;
        do {
            r = recv(client_fd, buf, sizeof(buf), 0);
            if (r > 0) request.append(buf, buf + r);
        } while (r > 0 && request.size() < MAX_BODY_SIZE_CPP + 8192);

        // Parse request line
        std::istringstream iss(request);
        std::string method, path, version;
        iss >> method >> path >> version;

        std::string payload;
        bool ok = true;

        // Headers
        std::string headersPart = request.substr(request.find("\r\n") + 2);
        std::string headerLine;
        std::string contentType;
        size_t contentLength = 0;

        // Simple header parse
        {
            size_t headerEnd = request.find("\r\n\r\n");
            size_t pos = request.find("\r\n");
            while (pos != std::string::npos && pos < headerEnd) {
                size_t next = request.find("\r\n", pos + 2);
                std::string line = request.substr(pos + 2, (next == std::string::npos ? headerEnd : next) - (pos + 2));
                if (!line.empty()) {
                    size_t colon = line.find(':');
                    if (colon != std::string::npos) {
                        std::string name = trim(line.substr(0, colon));
                        std::string value = trim(line.substr(colon + 1));
                        std::string lname = name;
                        for (auto &c : lname) c = std::tolower(static_cast<unsigned char>(c));
                        if (lname == "content-type") {
                            size_t sc = value.find(';');
                            contentType = sc == std::string::npos ? value : value.substr(0, sc);
                            for (auto &c : contentType) c = std::tolower(static_cast<unsigned char>(c));
                        } else if (lname == "content-length") {
                            contentLength = static_cast<size_t>(strtoul(value.c_str(), nullptr, 10));
                            if (contentLength > MAX_BODY_SIZE_CPP) ok = false;
                        }
                    }
                }
                if (next == std::string::npos || next >= headerEnd) break;
                pos = next;
            }
        }

        if (!ok) {
            send_response(client_fd, 400, "Error: payload too large.");
            close(client_fd);
            continue;
        }

        // Extract payload
        if (method == "GET") {
            std::string query;
            size_t qm = path.find('?');
            if (qm != std::string::npos) {
                query = path.substr(qm + 1);
                auto m = parse_query(query);
                auto it = m.find("payload");
                if (it != m.end()) payload = it->second;
            }
        } else if (method == "POST" || method == "PUT") {
            size_t bodyPos = request.find("\r\n\r\n");
            std::string body = (bodyPos != std::string::npos) ? request.substr(bodyPos + 4) : "";
            if (contentType == "application/x-www-form-urlencoded") {
                auto m = parse_query(body);
                auto it = m.find("payload");
                if (it != m.end()) payload = it->second;
            } else {
                payload = body;
            }
        }

        std::string result = processPayloadCPP(payload);
        int status = result.rfind("Error:", 0) == 0 ? 400 : 200;
        send_response(client_fd, status, result);
        close(client_fd);
    }

    close(server_fd);
}

int main() {
    // 5 test cases
    std::cout << "Test1 (no payload): " << processPayloadCPP("") << std::endl;
    std::cout << "Test2 (Create): " << processPayloadCPP("type: Create") << std::endl;
    std::cout << "Test3 (Update): " << processPayloadCPP("type: Update") << std::endl;
    std::cout << "Test4 (no type): " << processPayloadCPP("name: test\nvalue: 123") << std::endl;
    std::cout << "Test5 (Type with quotes): " << processPayloadCPP("type: 'Create'") << std::endl;

    run_server_cpp();
    return 0;
}