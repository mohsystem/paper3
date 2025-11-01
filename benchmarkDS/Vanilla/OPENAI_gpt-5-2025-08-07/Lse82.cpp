// C++ single-file minimal HTTP server for /api that extracts "name" from JSON payload query param.
// Note: POSIX sockets used for simplicity.
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <thread>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

std::string trim(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace((unsigned char)s[a])) a++;
    while (b > a && std::isspace((unsigned char)s[b-1])) b--;
    return s.substr(a, b - a);
}

std::string url_decode(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '+') {
            out.push_back(' ');
        } else if (c == '%' && i + 2 < s.size()) {
            int hi = std::toupper(s[i+1]);
            int lo = std::toupper(s[i+2]);
            auto hex = [](int x)->int {
                if (x >= '0' && x <= '9') return x - '0';
                if (x >= 'A' && x <= 'F') return x - 'A' + 10;
                return 0;
            };
            out.push_back((char)((hex(hi) << 4) | hex(lo)));
            i += 2;
        } else {
            out.push_back(c);
        }
    }
    return out;
}

std::unordered_map<std::string, std::string> parse_query(const std::string& qraw) {
    std::unordered_map<std::string, std::string> m;
    std::string q = qraw;
    size_t start = 0;
    while (start <= q.size()) {
        size_t amp = q.find('&', start);
        std::string pair = q.substr(start, amp == std::string::npos ? std::string::npos : amp - start);
        if (!pair.empty()) {
            size_t eq = pair.find('=');
            std::string k = url_decode(pair.substr(0, eq));
            std::string v = eq == std::string::npos ? "" : url_decode(pair.substr(eq + 1));
            m[k] = v;
        }
        if (amp == std::string::npos) break;
        start = amp + 1;
    }
    return m;
}

std::string unescape_json_string(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    bool esc = false;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (esc) {
            switch (c) {
                case '"': out.push_back('"'); break;
                case '\\': out.push_back('\\'); break;
                case '/': out.push_back('/'); break;
                case 'b': out.push_back('\b'); break;
                case 'f': out.push_back('\f'); break;
                case 'n': out.push_back('\n'); break;
                case 'r': out.push_back('\r'); break;
                case 't': out.push_back('\t'); break;
                case 'u':
                    if (i + 4 < s.size()) {
                        std::string hex = s.substr(i+1, 4);
                        int cp = std::strtol(hex.c_str(), nullptr, 16);
                        out.push_back((char)cp);
                        i += 4;
                    } else {
                        out += "\\u";
                    }
                    break;
                default: out.push_back(c);
            }
            esc = false;
        } else if (c == '\\') {
            esc = true;
        } else {
            out.push_back(c);
        }
    }
    return out;
}

// Minimal flat JSON object parser into map<string,string>
std::unordered_map<std::string, std::string> parse_flat_json_object(const std::string& json) {
    std::unordered_map<std::string, std::string> m;
    std::string s = trim(json);
    if (!s.empty() && s.front() == '{') s.erase(s.begin());
    if (!s.empty() && s.back() == '}') s.pop_back();

    std::vector<std::string> pairs;
    std::string cur;
    bool inQuotes = false, esc = false;
    for (char c : s) {
        if (inQuotes) {
            cur.push_back(c);
            if (esc) esc = false;
            else if (c == '\\') esc = true;
            else if (c == '"') inQuotes = false;
        } else {
            if (c == '"') { inQuotes = true; cur.push_back(c); }
            else if (c == ',') { pairs.push_back(cur); cur.clear(); }
            else cur.push_back(c);
        }
    }
    if (!cur.empty()) pairs.push_back(cur);

    for (auto& p : pairs) {
        std::string t = trim(p);
        if (t.empty()) continue;
        int idx = -1;
        inQuotes = false; esc = false;
        for (int i = 0; i < (int)t.size(); ++i) {
            char c = t[i];
            if (inQuotes) {
                if (esc) esc = false;
                else if (c == '\\') esc = true;
                else if (c == '"') inQuotes = false;
            } else {
                if (c == '"') inQuotes = true;
                else if (c == ':') { idx = i; break; }
            }
        }
        if (idx < 0) continue;
        std::string k = trim(t.substr(0, idx));
        std::string v = trim(t.substr(idx+1));
        if (!k.empty() && k.front() == '"' && k.back() == '"' && k.size() >= 2) {
            k = k.substr(1, k.size()-2);
        }
        std::string val;
        if (!v.empty() && v.front() == '"' && v.back() == '"' && v.size() >= 2) {
            val = unescape_json_string(v.substr(1, v.size()-2));
        } else {
            val = v;
        }
        m[k] = val;
    }
    return m;
}

std::string get_name_from_payload(const std::string& payload) {
    auto m = parse_flat_json_object(payload);
    auto it = m.find("name");
    if (it == m.end()) return "";
    return it->second;
}

std::string http_response(int status, const std::string& body, const std::string& contentType="text/plain; charset=utf-8") {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << status << " OK\r\n";
    oss << "Content-Type: " << contentType << "\r\n";
    oss << "Content-Length: " << body.size() << "\r\n";
    oss << "Connection: close\r\n";
    oss << "\r\n";
    oss << body;
    return oss.str();
}

void serve() {
    int server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::perror("socket");
        return;
    }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080);
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
    // std::cout << "C++ server running on http://localhost:8080/api?payload=%7B%22name%22%3A%22World%22%7D\n";
    while (true) {
        int client = accept(server_fd, nullptr, nullptr);
        if (client < 0) continue;
        std::string req;
        char buf[4096];
        ssize_t n = recv(client, buf, sizeof(buf), 0);
        if (n > 0) req.assign(buf, buf + n);
        // parse request line
        std::istringstream iss(req);
        std::string method, path, ver;
        iss >> method >> path >> ver;
        std::string resp;
        if (method == "GET") {
            // split path and query
            std::string target = path;
            std::string query;
            size_t qpos = target.find('?');
            if (qpos != std::string::npos) {
                query = target.substr(qpos + 1);
                target = target.substr(0, qpos);
            }
            if (target == "/api") {
                auto params = parse_query(query);
                auto it = params.find("payload");
                if (it == params.end()) {
                    resp = http_response(400, "Missing payload parameter");
                } else {
                    std::string name = get_name_from_payload(it->second);
                    resp = http_response(200, name);
                }
            } else {
                resp = http_response(404, "Not Found");
            }
        } else {
            resp = http_response(405, "Method Not Allowed");
        }
        send(client, resp.data(), resp.size(), 0);
        close(client);
    }
}

int main() {
    // 5 test cases
    std::vector<std::string> tests = {
        "{\"name\":\"Alice\"}",
        "{\"name\":\"Bob\",\"age\":30}",
        "{\"age\":22,\"name\":\"Carol\"}",
        "{\"name\":\"D\\\"E\"}",
        "{\"name\":\"Eve Adams\",\"city\":\"NY\"}"
    };
    for (auto& t : tests) {
        std::cout << get_name_from_payload(t) << std::endl;
    }
    // Start server
    serve();
    return 0;
}