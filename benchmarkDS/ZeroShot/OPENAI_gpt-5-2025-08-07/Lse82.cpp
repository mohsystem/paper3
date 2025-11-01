#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <cctype>
#include <cstring>
#include <sstream>

#ifdef _WIN32
// For simplicity and portability, networking code is omitted on Windows.
// A minimal POSIX socket server is provided for Unix-like systems below.
#pragma message("Networking server not implemented for Windows in this single-file example.")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

static std::string url_decode(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        unsigned char c = s[i];
        if (c == '+') {
            out.push_back(' ');
        } else if (c == '%' && i + 2 < s.size() && std::isxdigit((unsigned char)s[i+1]) && std::isxdigit((unsigned char)s[i+2])) {
            auto hex = s.substr(i+1, 2);
            char* end = nullptr;
            int v = (int)strtol(hex.c_str(), &end, 16);
            out.push_back((char)v);
            i += 2;
        } else {
            out.push_back((char)c);
        }
    }
    return out;
}

static size_t skip_ws_forward(const std::string& s, size_t i) {
    size_t n = s.size();
    while (i < n && (s[i] == ' ' || s[i] == '\n' || s[i] == '\r' || s[i] == '\t')) i++;
    return i;
}

struct ParseResult {
    bool ok{false};
    std::string value;
    size_t nextIndex{0};
};

static bool is_hex(char c) {
    return (c >= '0' && c <= '9') || (c | 32) >= 'a' && (c | 32) <= 'f';
}

static ParseResult parse_json_string(const std::string& s, size_t startQuoteIdx) {
    ParseResult pr;
    size_t n = s.size();
    if (startQuoteIdx >= n || s[startQuoteIdx] != '"') return pr;
    std::string out;
    for (size_t i = startQuoteIdx + 1; i < n; ) {
        char c = s[i];
        if (c == '"') {
            pr.ok = true;
            pr.value = out;
            pr.nextIndex = i + 1;
            return pr;
        } else if (c == '\\') {
            if (i + 1 >= n) return pr;
            char esc = s[++i];
            switch (esc) {
                case '"': out.push_back('"'); break;
                case '\\': out.push_back('\\'); break;
                case '/': out.push_back('/'); break;
                case 'b': out.push_back('\b'); break;
                case 'f': out.push_back('\f'); break;
                case 'n': out.push_back('\n'); break;
                case 'r': out.push_back('\r'); break;
                case 't': out.push_back('\t'); break;
                case 'u': {
                    if (i + 4 >= n) return pr;
                    if (!(is_hex(s[i+1]) && is_hex(s[i+2]) && is_hex(s[i+3]) && is_hex(s[i+4]))) return pr;
                    int code = std::stoi(s.substr(i+1, 4), nullptr, 16);
                    out.push_back((char)code);
                    i += 4;
                    break;
                }
                default: return pr;
            }
            i++;
        } else {
            if ((unsigned char)c <= 0x1F) return pr;
            out.push_back(c);
            i++;
        }
    }
    return pr;
}

static long index_of_json_string_key(const std::string& s, const std::string& key) {
    size_t i = 0, n = s.size();
    while (i < n) {
        size_t quote = s.find('"', i);
        if (quote == std::string::npos) return -1;
        ParseResult pr = parse_json_string(s, quote);
        if (!pr.ok) return -1;
        if (pr.value == key) {
            size_t after = skip_ws_forward(s, pr.nextIndex);
            if (after < n && s[after] == ':') return (long)after;
        }
        i = pr.nextIndex;
    }
    return -1;
}

// Returns empty string if invalid or missing
std::string extract_name_from_payload(const std::string& payload) {
    if (payload.size() == 0 || payload.size() > 4096) return "";
    std::string s = payload;
    // Trim
    auto l = s.find_first_not_of(" \r\n\t");
    auto r = s.find_last_not_of(" \r\n\t");
    if (l == std::string::npos) return "";
    s = s.substr(l, r - l + 1);
    if (s.front() != '{' || s.back() != '}') return "";
    long keyPos = index_of_json_string_key(s, "name");
    if (keyPos < 0) return "";
    size_t colon = (size_t)keyPos;
    colon++; // move past ':'
    colon = skip_ws_forward(s, colon);
    if (colon >= s.size() || s[colon] != '"') return "";
    ParseResult pr = parse_json_string(s, colon);
    if (!pr.ok) return "";
    return pr.value;
}

#ifndef _WIN32
static void send_http_response(int fd, int code, const std::string& body) {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << code << " "
        << (code == 200 ? "OK" : (code == 400 ? "Bad Request" : (code == 404 ? "Not Found" : "Error"))) << "\r\n";
    oss << "Content-Type: text/plain; charset=utf-8\r\n";
    oss << "Content-Length: " << body.size() << "\r\n";
    oss << "Connection: close\r\n\r\n";
    std::string head = oss.str();
    send(fd, head.c_str(), head.size(), 0);
    if (!body.empty()) send(fd, body.c_str(), body.size(), 0);
}

static std::string get_query_param(const std::string& query, const std::string& key) {
    // naive parse: split by '&', then k=v
    size_t pos = 0;
    while (pos <= query.size()) {
        size_t amp = query.find('&', pos);
        std::string part = query.substr(pos, amp == std::string::npos ? std::string::npos : amp - pos);
        size_t eq = part.find('=');
        std::string k = eq == std::string::npos ? part : part.substr(0, eq);
        std::string v = eq == std::string::npos ? "" : part.substr(eq + 1);
        if (k == key) return v;
        if (amp == std::string::npos) break;
        pos = amp + 1;
    }
    return "";
}

static void handle_client(int cfd) {
    char buf[4096];
    ssize_t n = recv(cfd, buf, sizeof(buf)-1, 0);
    if (n <= 0) { close(cfd); return; }
    buf[n] = 0;
    std::string req(buf, (size_t)n);
    // Parse first line
    size_t lineEnd = req.find("\r\n");
    if (lineEnd == std::string::npos) { send_http_response(cfd, 400, "Bad Request"); close(cfd); return; }
    std::string line = req.substr(0, lineEnd);
    std::istringstream iss(line);
    std::string method, path, version;
    iss >> method >> path >> version;
    if (method != "GET") { send_http_response(cfd, 405, "Method Not Allowed"); close(cfd); return; }

    // Parse path and query
    std::string route = path;
    std::string query;
    size_t qpos = path.find('?');
    if (qpos != std::string::npos) {
        route = path.substr(0, qpos);
        query = path.substr(qpos + 1);
    }
    if (route != "/api") { send_http_response(cfd, 404, "Not Found"); close(cfd); return; }

    std::string rawPayload = get_query_param(query, "payload");
    if (rawPayload.empty()) { send_http_response(cfd, 400, "Missing payload parameter"); close(cfd); return; }
    if (rawPayload.size() > 16384) { send_http_response(cfd, 413, "Payload too large"); close(cfd); return; }
    std::string decoded = url_decode(rawPayload);
    std::string name = extract_name_from_payload(decoded);
    if (name.empty()) { send_http_response(cfd, 400, "Invalid payload or missing name"); close(cfd); return; }
    send_http_response(cfd, 200, name);
    close(cfd);
}

static void start_server() {
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) return;
    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(0x7F000001); // 127.0.0.1
    addr.sin_port = htons(8082);
    if (bind(sfd, (sockaddr*)&addr, sizeof(addr)) != 0) { close(sfd); return; }
    if (listen(sfd, 8) != 0) { close(sfd); return; }
    // Accept a few requests then stop (to avoid blocking indefinitely)
    for (int i = 0; i < 5; ++i) {
        int cfd = accept(sfd, nullptr, nullptr);
        if (cfd < 0) break;
        handle_client(cfd);
    }
    close(sfd);
}
#endif

int main() {
    // 5 test cases
    std::vector<std::string> tests = {
        "{\"name\":\"Alice\"}",
        "{\"age\":30}",
        "not json",
        "{\"name\":\"A\\u004c\\u0069\\u0063\\u0065\"}",
        "{\"name\":\"Bob \\\"The Builder\\\"\"}"
    };
    for (auto& t : tests) {
        auto r = extract_name_from_payload(t);
        std::cout << (r.empty() ? "(null)" : r) << "\n";
    }
#ifndef _WIN32
    // Start server in background to keep sample non-blocking
    std::thread srv([](){ start_server(); });
    srv.detach();
    // Sleep to keep process alive for a while (optional)
    std::this_thread::sleep_for(std::chrono::hours(24));
#else
    // Server not implemented on Windows in this minimal example
#endif
    return 0;
}