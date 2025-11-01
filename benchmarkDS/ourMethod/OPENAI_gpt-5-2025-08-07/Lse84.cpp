#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

static const int PORT = 5000;
static const size_t MAX_INPUT_SIZE = 65536;

static std::string url_decode(const std::string &in) {
    std::string out;
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        char c = in[i];
        if (c == '+') {
            out.push_back(' ');
        } else if (c == '%' && i + 2 < in.size()) {
            char a = in[i + 1], b = in[i + 2];
            auto hex = [](char x) -> int {
                if (x >= '0' && x <= '9') return x - '0';
                if (x >= 'a' && x <= 'f') return x - 'a' + 10;
                if (x >= 'A' && x <= 'F') return x - 'A' + 10;
                return -1;
            };
            int hi = hex(a), lo = hex(b);
            if (hi >= 0 && lo >= 0) {
                out.push_back(static_cast<char>((hi << 4) | lo));
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

static std::map<std::string, std::string> parse_query(const std::string &q) {
    std::map<std::string, std::string> m;
    size_t start = 0;
    while (start <= q.size()) {
        size_t amp = q.find('&', start);
        std::string pair = q.substr(start, (amp == std::string::npos) ? std::string::npos : amp - start);
        size_t eq = pair.find('=');
        std::string k = url_decode(eq == std::string::npos ? pair : pair.substr(0, eq));
        std::string v = url_decode(eq == std::string::npos ? "" : pair.substr(eq + 1));
        if (!k.empty() && k.size() <= 100) m[k] = v;
        if (amp == std::string::npos) break;
        start = amp + 1;
    }
    return m;
}

static bool parse_simple_yaml(const std::string &src, std::map<std::string, std::string> &out) {
    if (src.size() > MAX_INPUT_SIZE) return false;
    out.clear();
    std::istringstream iss(src);
    std::string line;
    size_t count = 0;
    while (std::getline(iss, line)) {
        // strip CR if present
        if (!line.empty() && line.back() == '\r') line.pop_back();
        // trim
        size_t b = 0, e = line.size();
        while (b < e && (line[b] == ' ' || line[b] == '\t')) b++;
        while (e > b && (line[e - 1] == ' ' || line[e - 1] == '\t')) e--;
        if (b >= e) continue;
        if (line[b] == '#') continue;
        std::string t = line.substr(b, e - b);
        size_t colon = t.find(':');
        if (colon == std::string::npos || colon == 0) return false;
        std::string key = t.substr(0, colon);
        std::string val = t.substr(colon + 1);
        // trim key
        size_t kb = 0, ke = key.size();
        while (kb < ke && (key[kb] == ' ' || key[kb] == '\t')) kb++;
        while (ke > kb && (key[ke - 1] == ' ' || key[ke - 1] == '\t')) ke--;
        key = key.substr(kb, ke - kb);
        if (key.empty() || key.size() > 64) return false;
        for (char ch : key) {
            if (!(std::isalnum(static_cast<unsigned char>(ch)) || ch == '_')) return false;
        }
        // trim val
        size_t vb = 0, ve = val.size();
        while (vb < ve && (val[vb] == ' ' || val[vb] == '\t')) vb++;
        while (ve > vb && (val[ve - 1] == ' ' || val[ve - 1] == '\t')) ve--;
        val = val.substr(vb, ve - vb);
        if (val.size() >= 2 && ((val.front() == '"' && val.back() == '"') || (val.front() == '\'' && val.back() == '\''))) {
            val = val.substr(1, val.size() - 2);
        }
        if (val.size() > 4096) return false;
        out[key] = val;
        count++;
        if (count > 64) return false;
    }
    return !out.empty();
}

static void process_request(const std::map<std::string, std::string> &params, int &status, std::string &body) {
    auto it = params.find("payload");
    if (it == params.end()) {
        status = 400; body = "Error: missing payload parameter"; return;
    }
    const std::string &payload = it->second;
    if (payload.size() > MAX_INPUT_SIZE) {
        status = 413; body = "Error: payload too large"; return;
    }
    std::map<std::string, std::string> ymap;
    if (!parse_simple_yaml(payload, ymap)) {
        status = 400; body = "Error: invalid payload format"; return;
    }
    auto ty = ymap.find("type");
    if (ty != ymap.end() && ty->second == "Create") {
        status = 400; body = "Error: operation not allowed"; return;
    }
    status = 200; body = "OK";
}

static std::string http_response(int code, const std::string &msg) {
    std::string status = (code == 200) ? "OK" :
                         (code == 400) ? "Bad Request" :
                         (code == 413) ? "Payload Too Large" : "Error";
    std::ostringstream oss;
    std::string body = msg + "\n";
    oss << "HTTP/1.1 " << code << " " << status << "\r\n"
        << "Content-Type: text/plain; charset=utf-8\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Connection: close\r\n\r\n"
        << body;
    return oss.str();
}

static void handle_client(int fd) {
    char buf[8192];
    std::string data;
    // Read headers
    while (true) {
        ssize_t r = recv(fd, buf, sizeof(buf), 0);
        if (r <= 0) break;
        data.append(buf, buf + r);
        if (data.size() > MAX_INPUT_SIZE * 2) break;
        if (data.find("\r\n\r\n") != std::string::npos) break;
    }
    if (data.find("\r\n\r\n") == std::string::npos) {
        std::string resp = http_response(400, "Error: invalid request");
        send(fd, resp.c_str(), resp.size(), 0);
        close(fd);
        return;
    }
    size_t hdr_end = data.find("\r\n\r\n");
    std::string headers = data.substr(0, hdr_end);
    std::string method, path, proto;
    {
        std::istringstream iss(headers);
        std::string line;
        if (!std::getline(iss, line)) {
            std::string resp = http_response(400, "Error: invalid request");
            send(fd, resp.c_str(), resp.size(), 0);
            close(fd); return;
        }
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::istringstream rl(line);
        rl >> method >> path >> proto;
    }
    // Parse headers
    size_t content_length = 0;
    std::string content_type;
    {
        std::istringstream iss(headers);
        std::string line;
        std::getline(iss, line); // skip request line
        while (std::getline(iss, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            auto pos = line.find(':');
            if (pos == std::string::npos) continue;
            std::string k = line.substr(0, pos);
            std::string v = line.substr(pos + 1);
            // trim v
            size_t b = 0; while (b < v.size() && (v[b] == ' ' || v[b] == '\t')) b++;
            v = v.substr(b);
            for (auto &c : k) c = std::tolower(static_cast<unsigned char>(c));
            if (k == "content-length") {
                content_length = static_cast<size_t>(std::strtoul(v.c_str(), nullptr, 10));
            } else if (k == "content-type") {
                content_type = v;
            }
        }
    }
    std::map<std::string, std::string> params;
    // Query params
    std::string query;
    auto qm = path.find('?');
    if (qm != std::string::npos) {
        query = path.substr(qm + 1);
    }
    if (!query.empty()) {
        auto qmap = parse_query(query);
        params.insert(qmap.begin(), qmap.end());
    }
    // Body
    std::string body;
    size_t have = data.size() - (hdr_end + 4);
    body = data.substr(hdr_end + 4);
    while (have < content_length && have < MAX_INPUT_SIZE) {
        ssize_t r = recv(fd, buf, sizeof(buf), 0);
        if (r <= 0) break;
        body.append(buf, buf + r);
        have += static_cast<size_t>(r);
        if (body.size() > MAX_INPUT_SIZE) break;
    }
    if (content_length > MAX_INPUT_SIZE) {
        std::string resp = http_response(413, "Error: payload too large");
        send(fd, resp.c_str(), resp.size(), 0);
        close(fd); return;
    }
    // Content-Type simple handling
    std::string ctype_l = content_type;
    for (auto &c : ctype_l) c = std::tolower(static_cast<unsigned char>(c));
    if (!body.empty()) {
        if (ctype_l.find("application/x-www-form-urlencoded") == 0) {
            auto qmap = parse_query(body);
            for (auto &kv : qmap) params[kv.first] = kv.second;
        } else if (ctype_l.find("text/plain") == 0) {
            if (params.find("payload") == params.end()) params["payload"] = body;
        }
    }
    int status = 500;
    std::string msg;
    process_request(params, status, msg);
    std::string resp = http_response(status, msg);
    send(fd, resp.c_str(), resp.size(), 0);
    close(fd);
}

static void server_thread(std::atomic<bool> &stop_flag) {
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) return;
    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);
    if (bind(sfd, (sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sfd); return;
    }
    if (listen(sfd, 16) < 0) {
        close(sfd); return;
    }
    while (!stop_flag.load()) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(sfd, &rfds);
        timeval tv{};
        tv.tv_sec = 0;
        tv.tv_usec = 300000;
        int r = select(sfd + 1, &rfds, nullptr, nullptr, &tv);
        if (r > 0 && FD_ISSET(sfd, &rfds)) {
            sockaddr_in cli{};
            socklen_t cl = sizeof(cli);
            int cfd = accept(sfd, (sockaddr *)&cli, &cl);
            if (cfd >= 0) {
                handle_client(cfd);
            }
        }
    }
    close(sfd);
}

// Simple HTTP client for tests
static std::pair<int, std::string> http_request(const std::string &method, const std::string &path, const std::string &body = "", const std::string &ctype = "") {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return {-1, "socket error"};
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    if (connect(fd, (sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd);
        return {-1, "connect error"};
    }
    std::ostringstream req;
    req << method << " " << path << " HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n";
    if (!body.empty()) {
        req << "Content-Length: " << body.size() << "\r\n";
        if (!ctype.empty()) req << "Content-Type: " << ctype << "\r\n";
    }
    req << "\r\n";
    if (!body.empty()) req << body;
    std::string s = req.str();
    send(fd, s.c_str(), s.size(), 0);
    char buf[4096];
    std::string resp;
    ssize_t r;
    while ((r = recv(fd, buf, sizeof(buf), 0)) > 0) {
        resp.append(buf, buf + r);
    }
    close(fd);
    // Parse status and body
    int code = -1;
    size_t sp1 = resp.find(' ');
    if (sp1 != std::string::npos && sp1 + 4 < resp.size()) {
        code = std::atoi(resp.substr(sp1 + 1, 3).c_str());
    }
    size_t hdr_end = resp.find("\r\n\r\n");
    std::string b = (hdr_end != std::string::npos) ? resp.substr(hdr_end + 4) : "";
    return {code, b};
}

int main() {
    std::atomic<bool> stop{false};
    std::thread th(server_thread, std::ref(stop));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // 1) Missing payload
    auto r1 = http_request("GET", "/");
    std::cout << "[GET /] -> HTTP " << r1.first << " | Body: " << r1.second.substr(0, 120) << std::endl;

    // 2) payload with type: Create
    std::string y1 = "type: Create\nuser: alice";
    std::string q1 = "/?payload=" + url_decode(url_decode("")); // placeholder to ensure url_decode linked
    (void)q1; // suppress unused
    std::ostringstream p2;
    p2 << "/?payload=";
    {
        auto enc = [](const std::string &in) {
            static const char *hex = "0123456789ABCDEF";
            std::string o;
            for (unsigned char c : in) {
                if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~' || c == ' ') {
                    o.push_back(c == ' ' ? '+' : c);
                } else {
                    o.push_back('%');
                    o.push_back(hex[c >> 4]);
                    o.push_back(hex[c & 0xF]);
                }
            }
            return o;
        };
        p2 << enc(y1);
    }
    auto r2 = http_request("GET", p2.str());
    std::cout << "[GET payload=Create] -> HTTP " << r2.first << " | Body: " << r2.second.substr(0, 120) << std::endl;

    // 3) payload with type: Update
    std::string y2 = "type: Update\nid: 42";
    std::ostringstream p3;
    p3 << "/?payload=";
    {
        auto enc = [](const std::string &in) {
            static const char *hex = "0123456789ABCDEF";
            std::string o;
            for (unsigned char c : in) {
                if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~' || c == ' ') {
                    o.push_back(c == ' ' ? '+' : c);
                } else {
                    o.push_back('%');
                    o.push_back(hex[c >> 4]);
                    o.push_back(hex[c & 0xF]);
                }
            }
            return o;
        };
        p3 << enc(y2);
    }
    auto r3 = http_request("GET", p3.str());
    std::cout << "[GET payload=Update] -> HTTP " << r3.first << " | Body: " << r3.second.substr(0, 120) << std::endl;

    // 4) invalid YAML
    std::string y3 = "type Create\n";
    std::ostringstream p4;
    {
        auto enc = [](const std::string &in) {
            static const char *hex = "0123456789ABCDEF";
            std::string o;
            for (unsigned char c : in) {
                if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~' || c == ' ') {
                    o.push_back(c == ' ' ? '+' : c);
                } else {
                    o.push_back('%');
                    o.push_back(hex[c >> 4]);
                    o.push_back(hex[c & 0xF]);
                }
            }
            return o;
        };
        p4 << "/?payload=" << enc(y3);
    }
    auto r4 = http_request("GET", p4.str());
    std::cout << "[GET invalid YAML] -> HTTP " << r4.first << " | Body: " << r4.second.substr(0, 120) << std::endl;

    // 5) too large payload via POST
    std::string big(70000, 'A');
    std::string y4 = "type: Update\npad: " + big;
    auto enc_form = [](const std::string &in) {
        static const char *hex = "0123456789ABCDEF";
        std::string o;
        for (unsigned char c : in) {
            if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~' || c == ' ') {
                o.push_back(c == ' ' ? '+' : c);
            } else {
                o.push_back('%');
                o.push_back(hex[c >> 4]);
                o.push_back(hex[c & 0xF]);
            }
        }
        return o;
    };
    std::string form = "payload=" + enc_form(y4);
    auto r5 = http_request("POST", "/", form, "application/x-www-form-urlencoded");
    std::cout << "[POST big payload] -> HTTP " << r5.first << " | Body: " << r5.second.substr(0, 120) << std::endl;

    stop.store(true);
    th.join();
    return 0;
}