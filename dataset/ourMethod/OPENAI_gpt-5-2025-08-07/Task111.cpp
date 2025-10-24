#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <chrono>
#include <cinttypes>
#include <cstring>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace safe {

static inline std::string trim(const std::string &s) {
    size_t a = 0, b = s.size();
    while (a < b && (s[a] == ' ' || s[a] == '\t' || s[a] == '\r' || s[a] == '\n')) a++;
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r' || s[b - 1] == '\n')) b--;
    return s.substr(a, b - a);
}

static inline bool ascii_printable(const std::string &s, size_t maxlen) {
    if (s.size() > maxlen) return false;
    for (unsigned char c : s) {
        if (c < 0x09) return false;
        if (c == 0x0B || c == 0x0C) return false;
        if (c < 0x20 && c != '\n' && c != '\r' && c != '\t') return false;
    }
    return true;
}

static inline bool parse_int64(const std::string &s, int64_t &out) {
    std::string t = trim(s);
    if (t.empty() || t.size() > 32) return false;
    int sign = 1;
    size_t i = 0;
    if (t[0] == '+' || t[0] == '-') {
        sign = (t[0] == '-') ? -1 : 1;
        i++;
    }
    if (i >= t.size()) return false;
    int64_t val = 0;
    for (; i < t.size(); ++i) {
        char c = t[i];
        if (c < '0' || c > '9') return false;
        int d = c - '0';
        if (val > (std::numeric_limits<int64_t>::max() - d) / 10) return false;
        val = val * 10 + d;
    }
    out = val * sign;
    return true;
}

static inline std::string xml_escape(const std::string &in, size_t maxlen) {
    std::string out;
    out.reserve(in.size());
    size_t count = 0;
    for (char ch : in) {
        if (count++ >= maxlen) break;
        switch (ch) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&apos;"; break;
            default: out += ch; break;
        }
    }
    return out;
}

} // namespace safe

struct XmlRpcValue {
    enum Type { INT, STRING, NONE } type{NONE};
    int64_t i{0};
    std::string s;
};

static std::string http_build_response(const std::string &xml, int status = 200, const char *status_text = "OK") {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << status << " " << status_text << "\r\n";
    oss << "Content-Type: text/xml; charset=utf-8\r\n";
    oss << "X-Content-Type-Options: nosniff\r\n";
    oss << "Connection: close\r\n";
    oss << "Content-Length: " << xml.size() << "\r\n\r\n";
    oss << xml;
    return oss.str();
}

static std::string xml_fault(int code, const std::string &msg) {
    std::ostringstream oss;
    std::string esc = safe::xml_escape(msg, 256);
    oss << "<?xml version=\"1.0\"?>"
        << "<methodResponse><fault><value><struct>"
        << "<member><name>faultCode</name><value><int>" << code << "</int></value></member>"
        << "<member><name>faultString</name><value><string>" << esc << "</string></value></member>"
        << "</struct></value></fault></methodResponse>";
    return oss.str();
}

static std::string xml_int_response(int64_t v) {
    std::ostringstream oss;
    oss << "<?xml version=\"1.0\"?>"
        << "<methodResponse><params><param><value><int>" << v
        << "</int></value></param></params></methodResponse>";
    return oss.str();
}

static std::string xml_string_response(const std::string &v) {
    std::ostringstream oss;
    oss << "<?xml version=\"1.0\"?>"
        << "<methodResponse><params><param><value><string>"
        << safe::xml_escape(v, 1024)
        << "</string></value></param></params></methodResponse>";
    return oss.str();
}

static bool between_tags(const std::string &body, const std::string &tag, std::string &out) {
    std::string open = "<" + tag + ">";
    std::string close = "</" + tag + ">";
    size_t a = body.find(open);
    if (a == std::string::npos) return false;
    size_t b = body.find(close, a + open.size());
    if (b == std::string::npos) return false;
    out = body.substr(a + open.size(), b - (a + open.size()));
    return true;
}

static std::vector<std::string> all_between_tags(const std::string &body, const std::string &tag) {
    std::vector<std::string> v;
    std::string open = "<" + tag + ">";
    std::string close = "</" + tag + ">";
    size_t pos = 0;
    while (true) {
        size_t a = body.find(open, pos);
        if (a == std::string::npos) break;
        size_t b = body.find(close, a + open.size());
        if (b == std::string::npos) break;
        v.emplace_back(body.substr(a + open.size(), b - (a + open.size())));
        pos = b + close.size();
    }
    return v;
}

static bool parse_param_value(const std::string &param_xml, XmlRpcValue &out) {
    // Extract <value>...</value>
    std::string value_sec;
    if (!between_tags(param_xml, "value", value_sec)) return false;
    std::string v = safe::trim(value_sec);
    // Try int
    std::string int_content;
    if (between_tags(v, "int", int_content) || between_tags(v, "i4", int_content)) {
        int64_t iv = 0;
        if (!safe::parse_int64(int_content, iv)) return false;
        out.type = XmlRpcValue::INT;
        out.i = iv;
        return true;
    }
    // Try string
    std::string str_content;
    if (between_tags(v, "string", str_content)) {
        if (!safe::ascii_printable(str_content, 1024)) return false;
        out.type = XmlRpcValue::STRING;
        out.s = str_content;
        return true;
    }
    // Support bare value text inside <value>text</value>
    if (!v.empty()) {
        if (!safe::ascii_printable(v, 1024)) return false;
        out.type = XmlRpcValue::STRING;
        out.s = v;
        return true;
    }
    return false;
}

static bool valid_method_name(const std::string &name) {
    if (name.empty() || name.size() > 32) return false;
    for (char c : name) {
        if (!(c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) return false;
    }
    return true;
}

static std::string handle_xmlrpc_request(const std::string &xml, std::atomic<bool> &stop_flag) {
    // Minimal validation
    if (xml.find("<!ENTITY") != std::string::npos || xml.find("<!DOCTYPE") != std::string::npos) {
        return xml_fault(400, "External entities and DTDs are not allowed.");
    }
    std::string methodNameSec;
    if (!between_tags(xml, "methodName", methodNameSec)) {
        return xml_fault(400, "Missing methodName");
    }
    std::string method = safe::trim(methodNameSec);
    if (!valid_method_name(method)) {
        return xml_fault(400, "Invalid methodName");
    }
    // Extract params
    std::vector<XmlRpcValue> params;
    std::string paramsBlock;
    if (between_tags(xml, "params", paramsBlock)) {
        auto paramBlocks = all_between_tags(paramsBlock, "param");
        for (const auto &p : paramBlocks) {
            XmlRpcValue v;
            if (!parse_param_value(p, v)) {
                return xml_fault(400, "Invalid param");
            }
            params.push_back(v);
        }
    }
    // Dispatch
    if (method == "add") {
        if (params.size() != 2 || params[0].type != XmlRpcValue::INT || params[1].type != XmlRpcValue::INT) {
            return xml_fault(400, "add expects two integers");
        }
        int64_t a = params[0].i, b = params[1].i;
        int64_t r = a + b;
        return xml_int_response(r);
    } else if (method == "multiply") {
        if (params.size() != 2 || params[0].type != XmlRpcValue::INT || params[1].type != XmlRpcValue::INT) {
            return xml_fault(400, "multiply expects two integers");
        }
        int64_t r = params[0].i * params[1].i;
        return xml_int_response(r);
    } else if (method == "echo") {
        if (params.size() != 1 || params[0].type != XmlRpcValue::STRING) {
            return xml_fault(400, "echo expects one string");
        }
        return xml_string_response(params[0].s);
    } else if (method == "strlen") {
        if (params.size() != 1 || params[0].type != XmlRpcValue::STRING) {
            return xml_fault(400, "strlen expects one string");
        }
        return xml_int_response(static_cast<int64_t>(params[0].s.size()));
    } else if (method == "shutdown") {
        if (!params.empty()) {
            return xml_fault(400, "shutdown expects no params");
        }
        stop_flag.store(true);
        return xml_string_response("ok");
    }
    return xml_fault(404, "Unknown method");
}

static bool read_until_delim(int fd, std::string &out, const std::string &delim, size_t max_bytes) {
    out.clear();
    std::string buf;
    buf.resize(1024);
    while (out.size() < max_bytes) {
        ssize_t n = recv(fd, buf.data(), buf.size(), 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        if (n == 0) break;
        out.append(buf.data(), static_cast<size_t>(n));
        if (out.find(delim) != std::string::npos) return true;
    }
    return out.find(delim) != std::string::npos;
}

static bool parse_content_length(const std::string &headers, size_t &cl_out) {
    cl_out = 0;
    size_t pos = 0;
    while (true) {
        size_t end = headers.find("\r\n", pos);
        if (end == std::string::npos) break;
        std::string line = headers.substr(pos, end - pos);
        pos = end + 2;
        if (line.empty()) break;
        // Case-insensitive match
        std::string key = "content-length:";
        if (line.size() >= key.size()) {
            std::string head = line.substr(0, key.size());
            for (char &c : head) c = static_cast<char>(::tolower(c));
            if (head == key) {
                std::string v = safe::trim(line.substr(key.size()));
                int64_t cl = 0;
                if (!safe::parse_int64(v, cl) || cl < 0 || cl > 8192) return false;
                cl_out = static_cast<size_t>(cl);
                return true;
            }
        }
    }
    return false;
}

static bool read_http_request(int fd, std::string &body) {
    body.clear();
    std::string headers;
    if (!read_until_delim(fd, headers, "\r\n\r\n", 4096)) {
        return false;
    }
    size_t header_end = headers.find("\r\n\r\n");
    if (header_end == std::string::npos) return false;
    std::string header_part = headers.substr(0, header_end + 2); // include last CRLF for parsing lines
    size_t cl = 0;
    if (!parse_content_length(header_part, cl)) return false;
    // Any extra bytes after header in headers string are part of body
    std::string remainder = headers.substr(header_end + 4);
    body = remainder;
    while (body.size() < cl && body.size() < 8192) {
        char buf[1024];
        ssize_t n = recv(fd, buf, sizeof(buf), 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        if (n == 0) break;
        size_t to_add = static_cast<size_t>(n);
        if (body.size() + to_add > 8192) return false;
        body.append(buf, to_add);
    }
    if (body.size() != cl) return false;
    if (!safe::ascii_printable(body, 8192)) return false;
    return true;
}

static bool send_all(int fd, const std::string &data) {
    size_t sent = 0;
    while (sent < data.size()) {
        ssize_t n = send(fd, data.data() + sent, data.size() - sent, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        if (n == 0) break;
        sent += static_cast<size_t>(n);
    }
    return sent == data.size();
}

struct ServerConfig {
    std::string bind_ip{"127.0.0.1"};
    uint16_t port{8088};
};

static int run_server(const ServerConfig &cfg, std::atomic<bool> &stop_flag) {
    int srv = ::socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) return -1;
    int yes = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    setsockopt(srv, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(cfg.port);
    if (inet_pton(AF_INET, cfg.bind_ip.c_str(), &addr.sin_addr) != 1) {
        close(srv);
        return -1;
    }
    if (bind(srv, (sockaddr *)&addr, sizeof(addr)) < 0) {
        close(srv);
        return -1;
    }
    if (listen(srv, 8) < 0) {
        close(srv);
        return -1;
    }
    while (!stop_flag.load()) {
        sockaddr_in cli{};
        socklen_t cl = sizeof(cli);
        int cfd = accept(srv, (sockaddr *)&cli, &cl);
        if (cfd < 0) {
            if (errno == EINTR) continue;
            // Accept timeout or transient errors
            continue;
        }
        // Per-connection receive timeout
        setsockopt(cfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        std::string body;
        bool ok = read_http_request(cfd, body);
        std::string xml_resp;
        if (!ok) {
            xml_resp = xml_fault(400, "Bad Request");
            std::string resp = http_build_response(xml_resp, 400, "Bad Request");
            send_all(cfd, resp);
            close(cfd);
            continue;
        }
        std::string rpc_resp = handle_xmlrpc_request(body, stop_flag);
        std::string resp = http_build_response(rpc_resp, 200, "OK");
        send_all(cfd, resp);
        close(cfd);
    }
    close(srv);
    return 0;
}

static bool client_post_xml(const std::string &host, uint16_t port, const std::string &xml, std::string &resp_body) {
    resp_body.clear();
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return false;
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1) {
        close(fd);
        return false;
    }
    if (connect(fd, (sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd);
        return false;
    }
    std::ostringstream req;
    req << "POST /RPC2 HTTP/1.1\r\n"
        << "Host: " << host << ":" << port << "\r\n"
        << "Content-Type: text/xml\r\n"
        << "Connection: close\r\n"
        << "Content-Length: " << xml.size() << "\r\n\r\n"
        << xml;
    std::string s = req.str();
    if (!send_all(fd, s)) {
        close(fd);
        return false;
    }
    // Read response headers
    std::string headers;
    if (!read_until_delim(fd, headers, "\r\n\r\n", 8192)) {
        close(fd);
        return false;
    }
    size_t header_end = headers.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        close(fd);
        return false;
    }
    std::string header_part = headers.substr(0, header_end + 2);
    size_t cl = 0;
    if (!parse_content_length(header_part, cl)) {
        close(fd);
        return false;
    }
    std::string remainder = headers.substr(header_end + 4);
    resp_body = remainder;
    while (resp_body.size() < cl && resp_body.size() < 65536) {
        char buf[1024];
        ssize_t n = recv(fd, buf, sizeof(buf), 0);
        if (n <= 0) break;
        resp_body.append(buf, static_cast<size_t>(n));
    }
    close(fd);
    return resp_body.size() == cl;
}

int main() {
    std::atomic<bool> stop_flag{false};
    ServerConfig cfg;
    std::thread server_thr([&]() { run_server(cfg, stop_flag); });
    // Wait for server to be ready
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    auto call = [&](const std::string &xml) -> std::string {
        std::string resp;
        for (int i = 0; i < 10; ++i) {
            if (client_post_xml("127.0.0.1", cfg.port, xml, resp)) return resp;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        return std::string();
    };

    // Test case 1: add
    std::string add_req =
        "<?xml version=\"1.0\"?>"
        "<methodCall><methodName>add</methodName><params>"
        "<param><value><int>7</int></value></param>"
        "<param><value><int>5</int></value></param>"
        "</params></methodCall>";
    std::string r1 = call(add_req);
    std::cout << "add response: " << safe::trim(r1) << std::endl;

    // Test case 2: multiply
    std::string mul_req =
        "<?xml version=\"1.0\"?>"
        "<methodCall><methodName>multiply</methodName><params>"
        "<param><value><int>3</int></value></param>"
        "<param><value><int>4</int></value></param>"
        "</params></methodCall>";
    std::string r2 = call(mul_req);
    std::cout << "multiply response: " << safe::trim(r2) << std::endl;

    // Test case 3: echo
    std::string echo_req =
        "<?xml version=\"1.0\"?>"
        "<methodCall><methodName>echo</methodName><params>"
        "<param><value><string>Hello, XML-RPC!</string></value></param>"
        "</params></methodCall>";
    std::string r3 = call(echo_req);
    std::cout << "echo response: " << safe::trim(r3) << std::endl;

    // Test case 4: strlen
    std::string strlen_req =
        "<?xml version=\"1.0\"?>"
        "<methodCall><methodName>strlen</methodName><params>"
        "<param><value><string>ABCDE</string></value></param>"
        "</params></methodCall>";
    std::string r4 = call(strlen_req);
    std::cout << "strlen response: " << safe::trim(r4) << std::endl;

    // Test case 5: shutdown
    std::string shutdown_req =
        "<?xml version=\"1.0\"?>"
        "<methodCall><methodName>shutdown</methodName></methodCall>";
    std::string r5 = call(shutdown_req);
    std::cout << "shutdown response: " << safe::trim(r5) << std::endl;

    // Wait for server to stop
    if (server_thr.joinable()) server_thr.join();
    return 0;
}