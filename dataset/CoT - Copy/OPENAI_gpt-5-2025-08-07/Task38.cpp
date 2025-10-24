#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

static const int PORT = 8082;
static const size_t MAX_BODY = 4096;
static const size_t MAX_MSG = 200;

std::unordered_map<std::string, std::string> tokenBySid;

std::string html_escape(const std::string &s) {
    std::string out;
    out.reserve(s.size() + 16);
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#x27;"; break;
            case '/': out += "&#x2F;"; break;
            default: out += c;
        }
    }
    return out;
}

bool validate_message(const std::string &raw) {
    if (raw.empty() || raw.size() > MAX_MSG) return false;
    const std::string allowed = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 .,;:!?@#()_-'\"";
    for (unsigned char c : raw) {
        if (c == '\r' || c == '\n' || c == '\t') return false;
        if (allowed.find(static_cast<char>(c)) == std::string::npos) return false;
    }
    return true;
}

std::string url_decode(const std::string &s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '+') {
            out += ' ';
        } else if (c == '%' && i + 2 < s.size()) {
            char h1 = s[i+1], h2 = s[i+2];
            auto hex = [](char h)->int{
                if (h >= '0' && h <= '9') return h - '0';
                if (h >= 'a' && h <= 'f') return 10 + (h - 'a');
                if (h >= 'A' && h <= 'F') return 10 + (h - 'A');
                return -1;
            };
            int v1 = hex(h1), v2 = hex(h2);
            if (v1 >= 0 && v2 >= 0) {
                out += static_cast<char>((v1<<4)|v2);
                i += 2;
            } else {
                out += c;
            }
        } else {
            out += c;
        }
    }
    return out;
}

std::unordered_map<std::string, std::string> parse_form(const std::string &body) {
    std::unordered_map<std::string, std::string> m;
    size_t start = 0;
    while (start <= body.size()) {
        size_t amp = body.find('&', start);
        std::string pair = body.substr(start, amp == std::string::npos ? std::string::npos : amp - start);
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

std::string gen_token(size_t bytes) {
    std::random_device rd;
    std::uniform_int_distribution<int> dist(0, 255);
    std::ostringstream oss;
    for (size_t i = 0; i < bytes; ++i) {
        oss << std::hex;
        int b = dist(rd);
        if (b < 16) oss << '0';
        oss << std::nouppercase << b;
    }
    return oss.str();
}

bool constant_time_eq(const std::string &a, const std::string &b) {
    if (a.size() != b.size()) return false;
    unsigned char r = 0;
    for (size_t i = 0; i < a.size(); ++i) r |= (unsigned char)a[i] ^ (unsigned char)b[i];
    return r == 0;
}

std::string get_header(const std::string &headers, const std::string &name) {
    std::istringstream iss(headers);
    std::string line;
    std::string lname = name;
    std::transform(lname.begin(), lname.end(), lname.begin(), ::tolower);
    while (std::getline(iss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        size_t colon = line.find(':');
        if (colon == std::string::npos) continue;
        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);
        auto trim = [](std::string &s){
            size_t a = s.find_first_not_of(" \t");
            size_t b = s.find_last_not_of(" \t");
            if (a == std::string::npos) { s.clear(); return; }
            s = s.substr(a, b - a + 1);
        };
        trim(key); trim(value);
        std::string lkey = key;
        std::transform(lkey.begin(), lkey.end(), lkey.begin(), ::tolower);
        if (lkey == lname) return value;
    }
    return "";
}

std::string get_cookie_sid(const std::string &cookie) {
    std::istringstream iss(cookie);
    std::string part;
    while (std::getline(iss, part, ';')) {
        size_t eq = part.find('=');
        std::string k = part.substr(0, eq);
        std::string v = eq == std::string::npos ? "" : part.substr(eq + 1);
        auto trim = [](std::string &s){
            size_t a = s.find_first_not_of(" \t");
            size_t b = s.find_last_not_of(" \t");
            if (a == std::string::npos) { s.clear(); return; }
            s = s.substr(a, b - a + 1);
        };
        trim(k); trim(v);
        if (k == "sid") return v;
    }
    return "";
}

std::string csp_header() {
    return "default-src 'none'; style-src 'self' 'unsafe-inline'; script-src 'none'; img-src 'self'; base-uri 'none'; form-action 'self'";
}

std::string page_form(const std::string &csrf) {
    std::ostringstream oss;
    oss << "<!doctype html><html><head><meta charset=\"utf-8\">"
        << "<meta http-equiv=\"Content-Security-Policy\" content=\"" << csp_header() << "\">"
        << "<title>Echo</title></head><body>"
        << "<h1>Secure Echo</h1>"
        << "<form method=\"POST\" action=\"/echo\">"
        << "<label>Message: <input type=\"text\" name=\"message\" maxlength=\"" << MAX_MSG << "\"></label>"
        << "<input type=\"hidden\" name=\"csrf\" value=\"" << csrf << "\">"
        << "<button type=\"submit\">Send</button></form>"
        << "</body></html>";
    return oss.str();
}

std::string page_msg(const std::string &msg) {
    std::ostringstream oss;
    oss << "<!doctype html><html><head><meta charset=\"utf-8\">"
        << "<meta http-equiv=\"Content-Security-Policy\" content=\"" << csp_header() << "\">"
        << "<title>Echo</title></head><body><p>" << html_escape(msg) << "</p>"
        << "<p><a href=\"/\">Back</a></p></body></html>";
    return oss.str();
}

void add_common_headers(std::ostringstream &resp) {
    resp << "X-Content-Type-Options: nosniff\r\n";
    resp << "Referrer-Policy: no-referrer\r\n";
    resp << "X-Frame-Options: DENY\r\n";
    resp << "Content-Security-Policy: " << csp_header() << "\r\n";
    resp << "Content-Type: text/html; charset=utf-8\r\n";
}

void handle_client(int client_fd) {
    char buf[8192];
    ssize_t n = recv(client_fd, buf, sizeof(buf)-1, 0);
    if (n <= 0) return;
    buf[n] = '\0';
    std::string req(buf);
    size_t header_end = req.find("\r\n\r\n");
    if (header_end == std::string::npos) return;
    std::string start_line, headers;
    {
        size_t first_crlf = req.find("\r\n");
        start_line = req.substr(0, first_crlf);
        headers = req.substr(first_crlf + 2, header_end - (first_crlf + 2));
    }
    std::istringstream sl(start_line);
    std::string method, path, version;
    sl >> method >> path >> version;

    std::string cookie = get_header(headers, "cookie");
    std::string sid = get_cookie_sid(cookie);
    if (sid.empty() || sid.size() < 16) sid = gen_token(16);

    std::ostringstream resp;
    if (method == "GET" && path == "/") {
        std::string csrf = gen_token(32);
        tokenBySid[sid] = csrf;
        std::string body = page_form(csrf);
        resp << "HTTP/1.1 200 OK\r\n";
        add_common_headers(resp);
        resp << "Set-Cookie: sid=" << sid << "; HttpOnly; Path=/; SameSite=Strict\r\n";
        resp << "Content-Length: " << body.size() << "\r\n\r\n";
        resp << body;
    } else if (method == "POST" && path == "/echo") {
        std::string ctype = get_header(headers, "content-type");
        std::string cl = get_header(headers, "content-length");
        size_t content_length = 0;
        if (!cl.empty()) content_length = std::stoul(cl);
        if (content_length > MAX_BODY || ctype.find("application/x-www-form-urlencoded") != 0) {
            std::string body = page_msg("Invalid request.");
            resp << "HTTP/1.1 400 Bad Request\r\n";
            add_common_headers(resp);
            resp << "Set-Cookie: sid=" << sid << "; HttpOnly; Path=/; SameSite=Strict\r\n";
            resp << "Content-Length: " << body.size() << "\r\n\r\n" << body;
        } else {
            std::string bodyData = req.substr(header_end + 4);
            // If partial, read remainder
            while (bodyData.size() < content_length) {
                ssize_t m = recv(client_fd, buf, sizeof(buf), 0);
                if (m <= 0) break;
                bodyData.append(buf, buf + m);
                if (bodyData.size() > MAX_BODY) break;
            }
            if (bodyData.size() > MAX_BODY) {
                std::string body = page_msg("Payload too large.");
                resp << "HTTP/1.1 413 Payload Too Large\r\n";
                add_common_headers(resp);
                resp << "Content-Length: " << body.size() << "\r\n\r\n" << body;
            } else {
                auto form = parse_form(bodyData);
                std::string csrf = form.count("csrf") ? form["csrf"] : "";
                std::string expected = tokenBySid.count(sid) ? tokenBySid[sid] : "";
                if (expected.empty() || !constant_time_eq(expected, csrf)) {
                    std::string body = page_msg("Invalid CSRF token.");
                    resp << "HTTP/1.1 400 Bad Request\r\n";
                    add_common_headers(resp);
                    resp << "Set-Cookie: sid=" << sid << "; HttpOnly; Path=/; SameSite=Strict\r\n";
                    resp << "Content-Length: " << body.size() << "\r\n\r\n" << body;
                } else {
                    std::string raw = form.count("message") ? form["message"] : "";
                    if (!validate_message(raw)) {
                        std::string body = page_msg("Invalid input. Only basic punctuation and letters/numbers allowed.");
                        resp << "HTTP/1.1 400 Bad Request\r\n";
                        add_common_headers(resp);
                        resp << "Set-Cookie: sid=" << sid << "; HttpOnly; Path=/; SameSite=Strict\r\n";
                        resp << "Content-Length: " << body.size() << "\r\n\r\n" << body;
                    } else {
                        std::ostringstream body;
                        body << "<!doctype html><html><head><meta charset=\"utf-8\">"
                             << "<meta http-equiv=\"Content-Security-Policy\" content=\"" << csp_header() << "\">"
                             << "<title>Echo</title></head><body><h1>Echo result</h1><p>You said: <strong>"
                             << html_escape(raw) << "</strong></p><p><a href=\"/\">Back</a></p></body></html>";
                        std::string b = body.str();
                        resp << "HTTP/1.1 200 OK\r\n";
                        add_common_headers(resp);
                        resp << "Set-Cookie: sid=" << sid << "; HttpOnly; Path=/; SameSite=Strict\r\n";
                        resp << "Content-Length: " << b.size() << "\r\n\r\n" << b;
                    }
                }
            }
        }
    } else {
        std::string body = page_msg("Not found.");
        resp << "HTTP/1.1 404 Not Found\r\n";
        add_common_headers(resp);
        resp << "Content-Length: " << body.size() << "\r\n\r\n" << body;
    }

    std::string out = resp.str();
    send(client_fd, out.data(), out.size(), 0);
}

void run_server() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); return; }
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);
    if (bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); close(fd); return; }
    if (listen(fd, 16) < 0) { perror("listen"); close(fd); return; }
    std::cout << "C++ server running on http://localhost:" << PORT << "/\n";
    while (true) {
        int c = accept(fd, nullptr, nullptr);
        if (c < 0) {
            if (errno == EINTR) continue;
            perror("accept"); break;
        }
        handle_client(c);
        close(c);
    }
    close(fd);
}

void run_tests() {
    std::vector<std::string> tests = {
        "<script>alert(1)</script>",
        "Hello, World!",
        "This_is-OK() #1!?",
        std::string(500, 'A'),
        "Emoji 🙂"
    };
    for (auto &t : tests) {
        bool ok = validate_message(t);
        std::cout << "Test: " << (t.size() > 30 ? t.substr(0,30) + "..." : t) << " => " << (ok ? ("OK:" + html_escape(t)) : "INVALID") << "\n";
    }
}

int main() {
    signal(SIGPIPE, SIG_IGN);
    run_tests();
    run_server();
    return 0;
}