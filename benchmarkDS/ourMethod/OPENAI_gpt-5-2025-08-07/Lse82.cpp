#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cctype>
#include <cstring>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

static const size_t MAX_REQ_SIZE = 8192;
static const size_t MAX_QUERY_LEN = 4096;
static const size_t MAX_JSON_LEN = 2048;
static const size_t MAX_NAME_LEN = 256;

static std::string url_decode(const std::string &in) {
    std::string out;
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        char c = in[i];
        if (c == '+') {
            out.push_back(' ');
        } else if (c == '%' && i + 2 < in.size() &&
                   std::isxdigit(static_cast<unsigned char>(in[i + 1])) &&
                   std::isxdigit(static_cast<unsigned char>(in[i + 2]))) {
            int hi = std::isdigit(in[i + 1]) ? in[i + 1] - '0' : std::tolower(in[i + 1]) - 'a' + 10;
            int lo = std::isdigit(in[i + 2]) ? in[i + 2] - '0' : std::tolower(in[i + 2]) - 'a' + 10;
            out.push_back(static_cast<char>((hi << 4) | lo));
            i += 2;
        } else {
            out.push_back(c);
        }
    }
    return out;
}

static std::map<std::string, std::string> parse_simple_json(const std::string &json) {
    if (json.size() == 0 || json.size() > MAX_JSON_LEN) throw std::runtime_error("bad json size");
    size_t i = 0, n = json.size();
    auto skipws = [&](void) {
        while (i < n && std::isspace(static_cast<unsigned char>(json[i]))) i++;
    };
    auto parseString = [&](void) -> std::string {
        if (i >= n || json[i] != '"') throw std::runtime_error("expected quote");
        i++;
        std::string out;
        while (i < n) {
            char c = json[i++];
            if (c == '"') return out;
            if (c == '\\') {
                if (i >= n) throw std::runtime_error("bad escape");
                char e = json[i++];
                switch (e) {
                    case '"':
                    case '\\':
                    case '/': out.push_back(e); break;
                    case 'b': out.push_back('\b'); break;
                    case 'f': out.push_back('\f'); break;
                    case 'n': out.push_back('\n'); break;
                    case 'r': out.push_back('\r'); break;
                    case 't': out.push_back('\t'); break;
                    case 'u': {
                        if (i + 4 > n) throw std::runtime_error("bad unicode");
                        int code = 0;
                        for (int k = 0; k < 4; ++k) {
                            char h = json[i++];
                            code <<= 4;
                            if (h >= '0' && h <= '9') code += h - '0';
                            else if (h >= 'a' && h <= 'f') code += h - 'a' + 10;
                            else if (h >= 'A' && h <= 'F') code += h - 'A' + 10;
                            else throw std::runtime_error("bad unicode hex");
                        }
                        if (code >= 0 && code <= 0x7F) out.push_back(static_cast<char>(code));
                        else {
                            // minimal UTF-8 encoding for BMP subset
                            if (code <= 0x7FF) {
                                out.push_back(static_cast<char>(0xC0 | ((code >> 6) & 0x1F)));
                                out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
                            } else {
                                out.push_back(static_cast<char>(0xE0 | ((code >> 12) & 0x0F)));
                                out.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
                                out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
                            }
                        }
                        break;
                    }
                    default: throw std::runtime_error("bad escape");
                }
            } else {
                out.push_back(c);
            }
        }
        throw std::runtime_error("unterminated string");
    };

    std::map<std::string, std::string> obj;
    skipws();
    if (i >= n || json[i] != '{') throw std::runtime_error("expected {");
    i++;
    while (true) {
        skipws();
        if (i < n && json[i] == '}') { i++; break; }
        std::string key = parseString();
        skipws();
        if (i >= n || json[i] != ':') throw std::runtime_error("expected :");
        i++;
        skipws();
        std::string val = parseString();
        obj[key] = val;
        skipws();
        if (i < n && json[i] == ',') { i++; continue; }
        else if (i < n && json[i] == '}') { i++; break; }
        else if (i >= n) throw std::runtime_error("unterminated object");
        else throw std::runtime_error("bad separator");
    }
    skipws();
    if (i != n) throw std::runtime_error("trailing");
    return obj;
}

static std::string get_query_param(const std::string& raw_query, const std::string& key) {
    size_t start = 0;
    while (start < raw_query.size()) {
        size_t amp = raw_query.find('&', start);
        if (amp == std::string::npos) amp = raw_query.size();
        size_t eq = raw_query.find('=', start);
        if (eq != std::string::npos && eq < amp) {
            std::string k = raw_query.substr(start, eq - start);
            if (k == key) {
                return raw_query.substr(eq + 1, amp - (eq + 1));
            }
        }
        start = amp + 1;
    }
    return "";
}

static void send_response(int client_fd, int code, const std::string& body) {
    std::string status;
    switch (code) {
        case 200: status = "200 OK"; break;
        case 400: status = "400 Bad Request"; break;
        case 404: status = "404 Not Found"; break;
        case 405: status = "405 Method Not Allowed"; break;
        default:  status = "500 Internal Server Error"; break;
    }
    std::string headers = "HTTP/1.1 " + status + "\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: " + std::to_string(body.size()) + "\r\nConnection: close\r\n\r\n";
    std::string resp = headers + body;
    ssize_t sent = 0;
    const char* buf = resp.c_str();
    size_t len = resp.size();
    while (sent < (ssize_t)len) {
        ssize_t n = ::send(client_fd, buf + sent, len - sent, 0);
        if (n <= 0) break;
        sent += n;
    }
}

static void handle_client(int client_fd) {
    std::vector<char> buf(MAX_REQ_SIZE);
    ssize_t n = recv(client_fd, buf.data(), buf.size(), 0);
    if (n <= 0) {
        send_response(client_fd, 400, "Bad Request");
        close(client_fd);
        return;
    }
    std::string req(buf.data(), (size_t)n);
    // simple parse first line
    size_t line_end = req.find("\r\n");
    if (line_end == std::string::npos) {
        send_response(client_fd, 400, "Bad Request");
        close(client_fd);
        return;
    }
    std::string line = req.substr(0, line_end);
    // Expect: GET /api?payload=... HTTP/1.1
    if (line.rfind("GET ", 0) != 0) {
        send_response(client_fd, 405, "Method Not Allowed");
        close(client_fd);
        return;
    }
    size_t sp1 = line.find(' ');
    size_t sp2 = line.rfind(' ');
    if (sp1 == std::string::npos || sp2 == std::string::npos || sp2 <= sp1 + 1) {
        send_response(client_fd, 400, "Bad Request");
        close(client_fd);
        return;
    }
    std::string target = line.substr(sp1 + 1, sp2 - sp1 - 1);
    std::string path, query;
    size_t qpos = target.find('?');
    if (qpos == std::string::npos) { path = target; query = ""; }
    else { path = target.substr(0, qpos); query = target.substr(qpos + 1); }

    if (path != "/api") {
        send_response(client_fd, 404, "Not Found");
        close(client_fd);
        return;
    }
    if (query.empty() || query.size() > MAX_QUERY_LEN) {
        send_response(client_fd, 400, "Bad Request");
        close(client_fd);
        return;
    }

    std::string payload_param = get_query_param(query, "payload");
    if (payload_param.empty()) {
        send_response(client_fd, 400, "Bad Request");
        close(client_fd);
        return;
    }
    std::string json = url_decode(payload_param);
    if (json.empty() || json.size() > MAX_JSON_LEN) {
        send_response(client_fd, 400, "Bad Request");
        close(client_fd);
        return;
    }
    try {
        std::map<std::string, std::string> dict = parse_simple_json(json);
        auto it = dict.find("name");
        if (it == dict.end() || it->second.empty() || it->second.size() > MAX_NAME_LEN) {
            send_response(client_fd, 400, "Bad Request");
        } else {
            send_response(client_fd, 200, it->second);
        }
    } catch (...) {
        send_response(client_fd, 400, "Bad Request");
    }
    close(client_fd);
}

static void run_server(uint16_t port, int max_requests) {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) return;
    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(port);
    if (bind(s, (sockaddr*)&addr, sizeof(addr)) < 0) {
        close(s); return;
    }
    if (listen(s, 8) < 0) {
        close(s); return;
    }
    for (int i = 0; i < max_requests; ++i) {
        int c = accept(s, nullptr, nullptr);
        if (c < 0) break;
        handle_client(c);
    }
    close(s);
}

static std::string http_get(const std::string& host, uint16_t port, const std::string& path) {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) return "0:";
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &addr.sin_addr);
    if (connect(s, (sockaddr*)&addr, sizeof(addr)) < 0) {
        close(s);
        return "0:";
    }
    std::string req = "GET " + path + " HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\n\r\n";
    send(s, req.c_str(), req.size(), 0);
    std::string resp;
    char buf[2048];
    ssize_t n;
    while ((n = recv(s, buf, sizeof(buf), 0)) > 0) {
        resp.append(buf, buf + n);
    }
    close(s);
    // parse status code and body
    size_t sp = resp.find(' ');
    int code = 0;
    if (sp != std::string::npos && sp + 4 <= resp.size()) {
        code = std::atoi(resp.substr(sp + 1, 3).c_str());
    }
    size_t hdr_end = resp.find("\r\n\r\n");
    std::string body = (hdr_end != std::string::npos) ? resp.substr(hdr_end + 4) : "";
    return std::to_string(code) + ":" + body;
}

int main() {
    uint16_t port = 8082;
    std::thread thr(run_server, port, 5);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // 5 test cases
    std::cout << http_get("127.0.0.1", port, "/api?payload=" + std::string("%7B%22name%22%3A%22Alice%22%7D")) << std::endl;
    std::cout << http_get("127.0.0.1", port, "/api?payload=" + std::string("%7B%22name%22%3A%22Bob%22%2C%22age%22%3A%2230%22%7D")) << std::endl;
    std::cout << http_get("127.0.0.1", port, "/api") << std::endl;
    std::cout << http_get("127.0.0.1", port, "/api?payload=" + std::string("%7B%22name%22%3ABob%7D")) << std::endl;
    std::cout << http_get("127.0.0.1", port, "/api?payload=" + std::string("%7B%22title%22%3A%22Engineer%22%7D")) << std::endl;

    thr.join();
    return 0;
}