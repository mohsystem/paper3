// Chain-of-Through in code (comments only):
// 1) Problem: HTTP server on port 5000; check 'payload' parameter; parse YAML; if type == Create -> error.
// 2) Security: Input size caps, minimal YAML parser (flat key: value), safe URL decoding, simple GET handling.
// 3) Implementation: POSIX sockets, single-threaded accept loop in background thread; core function test cases.
// 4) Review: Defensive parsing, length checks, response headers, handle bad requests.
// 5) Final: Implementation below with 5 core logic tests.

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

static std::string trim(const std::string &s) {
    size_t b = 0, e = s.size();
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
    return s.substr(b, e - b);
}

static std::string unquote(const std::string &v) {
    if (v.size() >= 2 && ((v.front() == '"' && v.back() == '"') || (v.front() == '\'' && v.back() == '\''))) {
        return v.substr(1, v.size() - 2);
    }
    return v;
}

static std::string urlDecode(const std::string &s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        unsigned char c = s[i];
        if (c == '%' && i + 2 < s.size()) {
            char h1 = s[i + 1], h2 = s[i + 2];
            auto hex = [](char ch)->int {
                if (ch >= '0' && ch <= '9') return ch - '0';
                if (ch >= 'a' && ch <= 'f') return 10 + (ch - 'a');
                if (ch >= 'A' && ch <= 'F') return 10 + (ch - 'A');
                return -1;
            };
            int hi = hex(h1), lo = hex(h2);
            if (hi >= 0 && lo >= 0) {
                out.push_back(static_cast<char>((hi << 4) | lo));
                i += 2;
            } else {
                out.push_back(c);
            }
        } else if (c == '+') {
            out.push_back(' ');
        } else {
            out.push_back(static_cast<char>(c));
        }
    }
    return out;
}

static bool parseSimpleYamlMap(const std::string &yaml, std::map<std::string, std::string> &out) {
    out.clear();
    if (yaml.size() > 10000) return false;
    std::istringstream iss(yaml);
    std::string line;
    size_t lines = 0;
    size_t total = 0;
    while (std::getline(iss, line)) {
        if (++lines > 1000) return false;
        total += line.size();
        if (total > 10000) return false;
        std::string t = trim(line);
        if (t.empty() || (!t.empty() && t[0] == '#')) continue;
        size_t pos = t.find(':');
        if (pos == std::string::npos || pos == 0) return false;
        std::string key = trim(t.substr(0, pos));
        std::string val = trim(t.substr(pos + 1));
        if (key.empty()) return false;
        val = unquote(val);
        if (!val.empty()) {
            char c = val[0];
            if (c == '&' || c == '*' || c == '!') return false;
        }
        out[key] = val;
    }
    return true;
}

// Core logic
static std::string processPayload(const std::string &payload) {
    if (payload.empty()) {
        return std::string("Error: Missing payload parameter");
    }
    if (payload.size() > 10000) {
        return std::string("Error: Payload too large");
    }
    std::map<std::string, std::string> m;
    if (!parseSimpleYamlMap(payload, m)) {
        return std::string("Error: Invalid YAML");
    }
    auto it = m.find("type");
    if (it != m.end() && it->second == "Create") {
        return std::string("Error: Create type is not allowed");
    }
    return std::string("OK");
}

static std::map<std::string, std::string> parseQuery(const std::string &raw) {
    std::map<std::string, std::string> q;
    size_t start = 0;
    while (start < raw.size()) {
        size_t amp = raw.find('&', start);
        std::string pair = raw.substr(start, amp == std::string::npos ? std::string::npos : amp - start);
        size_t eq = pair.find('=');
        std::string name = (eq == std::string::npos) ? pair : pair.substr(0, eq);
        std::string val = (eq == std::string::npos) ? "" : pair.substr(eq + 1);
        name = urlDecode(name);
        val = urlDecode(val);
        if (val.size() > 10000) val = val.substr(0, 10000);
        if (!name.empty() && q.find(name) == q.end()) {
            q[name] = val;
        }
        if (amp == std::string::npos) break;
        start = amp + 1;
    }
    return q;
}

static void sendHttpResponse(int client_fd, int status, const std::string &body) {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << status << (status == 200 ? " OK" : " Error") << "\r\n";
    oss << "Content-Type: text/plain; charset=utf-8\r\n";
    oss << "Content-Length: " << body.size() << "\r\n";
    oss << "Connection: close\r\n\r\n";
    oss << body;
    std::string resp = oss.str();
    (void)send(client_fd, resp.data(), resp.size(), 0);
}

static void handleClient(int client_fd) {
    char buf[8192];
    ssize_t n = recv(client_fd, buf, sizeof(buf) - 1, 0);
    if (n <= 0) {
        close(client_fd);
        return;
    }
    buf[n] = '\0';
    std::string req(buf);
    // Parse request line
    size_t lineEnd = req.find("\r\n");
    if (lineEnd == std::string::npos) {
        sendHttpResponse(client_fd, 400, "Error: Bad Request");
        close(client_fd);
        return;
    }
    std::string reqLine = req.substr(0, lineEnd);
    std::istringstream rl(reqLine);
    std::string method, path, version;
    rl >> method >> path >> version;
    if (method != "GET") {
        sendHttpResponse(client_fd, 405, "Error: Method Not Allowed");
        close(client_fd);
        return;
    }
    // Extract query
    std::string query;
    size_t qpos = path.find('?');
    if (qpos != std::string::npos) {
        query = path.substr(qpos + 1);
    }
    std::string payload;
    if (!query.empty()) {
        auto params = parseQuery(query);
        auto it = params.find("payload");
        if (it != params.end()) {
            payload = it->second;
        }
    }
    if (payload.empty()) {
        sendHttpResponse(client_fd, 400, "Error: Missing payload parameter");
        close(client_fd);
        return;
    }
    std::string result = processPayload(payload);
    int status = (result.rfind("Error:", 0) == 0) ? 400 : 200;
    sendHttpResponse(client_fd, status, result);
    close(client_fd);
}

static void runServer() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Error: Could not create socket\n";
        return;
    }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(5000);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Error: Bind failed\n";
        close(server_fd);
        return;
    }
    if (listen(server_fd, 16) < 0) {
        std::cerr << "Error: Listen failed\n";
        close(server_fd);
        return;
    }
    std::cout << "C++ server listening on http://0.0.0.0:5000/\n";
    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) continue;
        handleClient(client_fd);
    }
}

int main() {
    std::thread srv(runServer);
    srv.detach();

    // 5 test cases for core logic
    std::vector<std::string> tests = {
        "type: Create",
        "type: Update",
        "foo: bar",
        "type: 'Create'",
        "type Create"
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        std::string out = processPayload(tests[i]);
        std::cout << "Test " << (i + 1) << " => " << out << "\n";
    }
    // Keep main alive
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }
    return 0;
}