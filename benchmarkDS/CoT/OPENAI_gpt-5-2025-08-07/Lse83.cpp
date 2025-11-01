#include <iostream>
#include <string>
#include <cctype>
#include <thread>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cstring>

// POSIX sockets (Linux/macOS). For Windows, additional setup is required.
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

static const size_t MAX_BODY = 8192;

static void json_escape(const std::string &in, std::string &out) {
    out.clear();
    out.reserve(in.size() + 16);
    out.push_back('"');
    for (unsigned char ch : in) {
        switch (ch) {
            case '\"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (ch < 0x20) {
                    char buf[7];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", ch);
                    out += buf;
                } else {
                    out.push_back((char)ch);
                }
        }
    }
    out.push_back('"');
}

static bool is_ws(char c) {
    return c==' '||c=='\n'||c=='\r'||c=='\t';
}

// Parse JSON string at index i; returns decoded string and advances i.
// On failure, returns false.
static bool parse_json_string(const std::string &s, size_t &i, std::string &out) {
    if (i >= s.size() || s[i] != '"') return false;
    i++;
    out.clear();
    while (i < s.size()) {
        char c = s[i++];
        if (c == '"') return true;
        if (c == '\\') {
            if (i >= s.size()) return false;
            char e = s[i++];
            switch (e) {
                case '"': out.push_back('"'); break;
                case '\\': out.push_back('\\'); break;
                case '/': out.push_back('/'); break;
                case 'b': out.push_back('\b'); break;
                case 'f': out.push_back('\f'); break;
                case 'n': out.push_back('\n'); break;
                case 'r': out.push_back('\r'); break;
                case 't': out.push_back('\t'); break;
                case 'u': {
                    if (i + 4 > s.size()) return false;
                    unsigned int code = 0;
                    for (int k = 0; k < 4; ++k) {
                        char h = s[i++];
                        code <<= 4;
                        if (h >= '0' && h <= '9') code |= (h - '0');
                        else if (h >= 'a' && h <= 'f') code |= (h - 'a' + 10);
                        else if (h >= 'A' && h <= 'F') code |= (h - 'A' + 10);
                        else return false;
                    }
                    out.push_back((char)code);
                    break;
                }
                default: return false;
            }
        } else {
            out.push_back(c);
        }
    }
    return false;
}

static void skip_ws(const std::string &s, size_t &i) {
    while (i < s.size() && is_ws(s[i])) i++;
}

// Parse a non-string literal (number/boolean/null)
static std::string parse_literal(const std::string &s, size_t &i) {
    size_t start = i;
    while (i < s.size()) {
        char c = s[i];
        if (c == ',' || c == '}' || c == ']' || is_ws(c)) break;
        i++;
    }
    if (start == i) return "";
    return s.substr(start, i - start);
}

// Returns name value if present; else empty string
std::string extract_name(const std::string &payload) {
    size_t i = 0, n = payload.size();
    std::string token;
    while (i < n) {
        char c = payload[i];
        if (c == '"') {
            if (!parse_json_string(payload, i, token)) return "";
            skip_ws(payload, i);
            if (i < n && payload[i] == ':') {
                i++;
                skip_ws(payload, i);
                if (token == "name") {
                    if (i < n && payload[i] == '"') {
                        std::string val;
                        if (!parse_json_string(payload, i, val)) return "";
                        return val;
                    } else {
                        std::string lit = parse_literal(payload, i);
                        return lit;
                    }
                } else {
                    if (i < n) {
                        if (payload[i] == '"') {
                            std::string ignore;
                            if (!parse_json_string(payload, i, ignore)) return "";
                        } else {
                            parse_literal(payload, i);
                        }
                    }
                }
            }
        } else {
            i++;
        }
    }
    return "";
}

static void handle_client(int client_fd) {
    const size_t MAX_REQ = 16384;
    std::vector<char> buf(MAX_REQ);
    ssize_t r = recv(client_fd, buf.data(), buf.size(), 0);
    if (r <= 0) { close(client_fd); return; }
    std::string req(buf.data(), buf.data() + r);

    auto bad = [&](int code, const std::string &msg){
        std::ostringstream oss;
        std::string body = std::string("{\"error\":") + "\"" + msg + "\"}";
        oss << "HTTP/1.1 " << code << " Error\r\n"
            << "Content-Type: application/json; charset=utf-8\r\n"
            << "Content-Length: " << body.size() << "\r\n"
            << "Connection: close\r\n\r\n"
            << body;
        std::string resp = oss.str();
        send(client_fd, resp.c_str(), resp.size(), 0);
        close(client_fd);
    };

    if (req.rfind("POST ", 0) != 0) {
        bad(405, "Method Not Allowed");
        return;
    }

    auto tolower_copy = [](std::string s){
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
        return s;
    };
    std::string lower = tolower_copy(req);

    size_t clpos = lower.find("\r\ncontent-length:");
    if (clpos == std::string::npos) clpos = lower.find("\ncontent-length:");
    if (clpos == std::string::npos) {
        bad(411, "Content-Length required");
        return;
    }
    size_t line_end = req.find("\r\n", clpos + 1);
    std::string clline = req.substr(clpos, (line_end == std::string::npos ? req.size() : line_end) - clpos);
    size_t colon = clline.find(':');
    if (colon == std::string::npos) { bad(400, "Bad Content-Length"); return; }
    int content_length = 0;
    try {
        content_length = std::stoi(clline.substr(colon + 1));
    } catch (...) {
        bad(400, "Bad Content-Length");
        return;
    }
    if (content_length < 0 || content_length > (int)MAX_BODY) {
        bad(413, "Payload Too Large");
        return;
    }

    size_t ctpos = lower.find("\r\ncontent-type:");
    if (ctpos == std::string::npos) ctpos = lower.find("\ncontent-type:");
    if (ctpos == std::string::npos) {
        bad(400, "Invalid Content-Type");
        return;
    }
    size_t ct_end = req.find("\r\n", ctpos + 1);
    std::string ctline = req.substr(ctpos, (ct_end == std::string::npos ? req.size() : ct_end) - ctpos);
    std::string ctlower = tolower_copy(ctline);
    if (ctlower.find("application/json") == std::string::npos) {
        bad(400, "Invalid Content-Type");
        return;
    }

    size_t body_pos = req.find("\r\n\r\n");
    if (body_pos == std::string::npos) body_pos = req.find("\n\n");
    if (body_pos == std::string::npos) { bad(400, "Bad Request"); return; }
    body_pos += (req.substr(body_pos - 3, 4) == "\r\n\r\n" ? 4 : 2);

    std::string body = req.substr(body_pos);
    // If body incomplete, try to read remaining
    while ((int)body.size() < content_length) {
        r = recv(client_fd, buf.data(), buf.size(), 0);
        if (r <= 0) break;
        body.append(buf.data(), r);
        if (body.size() > MAX_BODY) break;
    }
    if ((int)body.size() < content_length) { bad(400, "Incomplete Body"); return; }

    std::string name = extract_name(body);
    if (name.empty()) {
        bad(400, "'name' not found or invalid JSON");
        return;
    }
    std::string escaped;
    json_escape(name, escaped);
    std::string resp_body = std::string("{\"name\":") + escaped + "}";

    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\n"
        << "Content-Type: application/json; charset=utf-8\r\n"
        << "Content-Length: " << resp_body.size() << "\r\n"
        << "Connection: close\r\n\r\n"
        << resp_body;
    std::string resp = oss.str();
    send(client_fd, resp.c_str(), resp.size(), 0);
    close(client_fd);
}

static void run_server_cpp() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return;
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(8082);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { close(fd); return; }
    if (listen(fd, 10) < 0) { close(fd); return; }
    // Accept loop
    while (true) {
        int cfd = accept(fd, nullptr, nullptr);
        if (cfd < 0) break;
        handle_client(cfd);
        // For demo, handle one client at a time. You can break to stop server.
        // break; // uncomment to stop after first request
    }
    close(fd);
}

int main() {
    // Start server in background
    std::thread th(run_server_cpp);
    th.detach();

    // 5 test cases for extraction
    std::vector<std::string> tests = {
        "{\"name\":\"Alice\"}",
        "{\"foo\":\"bar\"}",
        "{\"name\":\"Bob \\\"Builder\\\"\"}",
        "{\"name\":\"\"}",
        "{invalid json"
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        std::string res = extract_name(tests[i]);
        std::cout << "Test " << (i + 1) << ": " << (res.empty() ? "null" : res) << "\n";
    }
    return 0;
}