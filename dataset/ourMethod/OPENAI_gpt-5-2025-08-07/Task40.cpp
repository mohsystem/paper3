#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

static std::string hexEncode(const std::vector<uint8_t>& data) {
    const char* hex = "0123456789abcdef";
    std::string out;
    out.reserve(data.size() * 2);
    for (uint8_t b : data) {
        out.push_back(hex[b >> 4]);
        out.push_back(hex[b & 0x0F]);
    }
    return out;
}

static std::string generateRandomToken(size_t bytes) {
    if (bytes == 0 || bytes > 1024) bytes = 32;
    std::random_device rd;
    std::vector<uint8_t> buf(bytes);
    for (size_t i = 0; i < bytes; ++i) {
        buf[i] = static_cast<uint8_t>(rd());
    }
    return hexEncode(buf);
}

static bool constantTimeEquals(const std::string& a, const std::string& b) {
    size_t len = a.size();
    if (b.size() != len) return false;
    uint8_t diff = 0;
    for (size_t i = 0; i < len; ++i) {
        diff |= static_cast<uint8_t>(a[i]) ^ static_cast<uint8_t>(b[i]);
    }
    return diff == 0;
}

static std::string htmlEscape(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#39;"; break;
            default: out.push_back(c); break;
        }
    }
    return out;
}

static int hexVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static bool urlDecode(const std::string& in, std::string& out) {
    out.clear();
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        char c = in[i];
        if (c == '+') {
            out.push_back(' ');
        } else if (c == '%' && i + 2 < in.size()) {
            int hi = hexVal(in[i + 1]);
            int lo = hexVal(in[i + 2]);
            if (hi < 0 || lo < 0) return false;
            out.push_back(static_cast<char>((hi << 4) | lo));
            i += 2;
        } else {
            out.push_back(c);
        }
    }
    return true;
}

static std::map<std::string, std::string> parseFormURLEncoded(const std::string& body) {
    std::map<std::string, std::string> kv;
    size_t start = 0;
    while (start <= body.size()) {
        size_t amp = body.find('&', start);
        std::string pair = body.substr(start, amp == std::string::npos ? std::string::npos : amp - start);
        if (!pair.empty()) {
            size_t eq = pair.find('=');
            std::string k_enc = pair.substr(0, eq);
            std::string v_enc = (eq == std::string::npos) ? std::string() : pair.substr(eq + 1);
            std::string k, v;
            if (urlDecode(k_enc, k) && urlDecode(v_enc, v)) {
                if (k.size() <= 64 && v.size() <= 1024) {
                    kv[k] = v;
                }
            }
        }
        if (amp == std::string::npos) break;
        start = amp + 1;
    }
    return kv;
}

struct Session {
    std::string id;
    std::string csrf;
    std::chrono::steady_clock::time_point lastAccess;
};

static std::unordered_map<std::string, Session> g_sessions;

static std::map<std::string, std::string> parseHeaders(const std::string& headers_only) {
    std::map<std::string, std::string> headers;
    std::istringstream iss(headers_only);
    std::string line;
    // Skip request line
    std::getline(iss, line);
    while (std::getline(iss, line)) {
        if (!line.empty() && (line.back() == '\r' || line.back() == '\n')) line.pop_back();
        if (line.empty()) break;
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string name = line.substr(0, colon);
            std::string value = line.substr(colon + 1);
            // Trim spaces
            auto ltrim = [](std::string& s) {
                size_t i = 0;
                while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) ++i;
                s.erase(0, i);
            };
            auto rtrim = [](std::string& s) {
                while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r' || s.back() == '\n')) s.pop_back();
            };
            ltrim(value);
            rtrim(value);
            std::string lname = name;
            std::transform(lname.begin(), lname.end(), lname.begin(), [](unsigned char ch){ return std::tolower(ch); });
            headers[lname] = value;
        }
    }
    return headers;
}

static std::map<std::string, std::string> parseCookies(const std::string& cookieHeader) {
    std::map<std::string, std::string> cookies;
    size_t pos = 0;
    while (pos < cookieHeader.size()) {
        size_t semi = cookieHeader.find(';', pos);
        std::string pair = cookieHeader.substr(pos, semi == std::string::npos ? std::string::npos : semi - pos);
        // trim
        size_t start = 0; while (start < pair.size() && (pair[start] == ' ' || pair[start] == '\t')) ++start;
        size_t end = pair.size(); while (end > start && (pair[end - 1] == ' ' || pair[end - 1] == '\t')) --end;
        if (end > start) {
            std::string trimmed = pair.substr(start, end - start);
            size_t eq = trimmed.find('=');
            if (eq != std::string::npos) {
                std::string k = trimmed.substr(0, eq);
                std::string v = trimmed.substr(eq + 1);
                cookies[k] = v;
            }
        }
        if (semi == std::string::npos) break;
        pos = semi + 1;
    }
    return cookies;
}

static Session& getOrCreateSession(const std::map<std::string, std::string>& headers, bool& isNew, std::string& setCookie) {
    isNew = false;
    setCookie.clear();
    std::string sid;
    auto it = headers.find("cookie");
    if (it != headers.end()) {
        auto cookies = parseCookies(it->second);
        auto c = cookies.find("SID");
        if (c != cookies.end()) sid = c->second;
    }
    if (!sid.empty()) {
        auto sit = g_sessions.find(sid);
        if (sit != g_sessions.end()) {
            sit->second.lastAccess = std::chrono::steady_clock::now();
            return sit->second;
        }
    }
    // Create new session
    Session s;
    s.id = generateRandomToken(16);
    s.csrf = generateRandomToken(32);
    s.lastAccess = std::chrono::steady_clock::now();
    auto res = g_sessions.emplace(s.id, s);
    isNew = true;
    setCookie = "Set-Cookie: SID=" + s.id + "; Path=/; HttpOnly; SameSite=Strict; Secure\r\n";
    return res.first->second;
}

static std::string buildFormPage(const Session& sess, const std::string& message) {
    std::ostringstream html;
    html << "<!doctype html><html><head><meta charset=\"utf-8\">"
         << "<meta http-equiv=\"Content-Security-Policy\" content=\"default-src 'none'; style-src 'self' 'unsafe-inline'; form-action 'self'; base-uri 'none'\">"
         << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
         << "<title>User Settings</title></head><body>"
         << "<h1>User Settings</h1>";
    if (!message.empty()) {
        html << "<p>" << htmlEscape(message) << "</p>";
    }
    html << "<form method=\"POST\" action=\"/update\">"
         << "<label>Display Name: <input type=\"text\" name=\"displayName\" maxlength=\"60\" required></label><br>"
         << "<label>Email: <input type=\"email\" name=\"email\" maxlength=\"120\" required></label><br>"
         << "<input type=\"hidden\" name=\"csrf\" value=\"" << sess.csrf << "\">"
         << "<button type=\"submit\">Update</button>"
         << "</form>"
         << "</body></html>";
    return html.str();
}

static bool isValidDisplayName(const std::string& s) {
    if (s.empty() || s.size() > 60) return false;
    // Basic ASCII check and printable
    for (unsigned char c : s) {
        if (c < 32 || c == 127) return false;
    }
    return true;
}
static bool isValidEmail(const std::string& s) {
    if (s.size() < 3 || s.size() > 120) return false;
    if (s.find('@') == std::string::npos) return false;
    if (s.find(' ') != std::string::npos) return false;
    return true;
}

static void sendAll(int fd, const std::string& data) {
    size_t sent = 0;
    while (sent < data.size()) {
        ssize_t n = ::send(fd, data.data() + sent, data.size() - sent, 0);
        if (n <= 0) {
            if (errno == EINTR) continue;
            break;
        }
        sent += static_cast<size_t>(n);
    }
}

static void handleClient(int client_fd) {
    const size_t MAX_REQ = 16384;
    std::string req;
    req.reserve(2048);
    char buf[2048];
    ssize_t n = 0;
    size_t header_end = std::string::npos;
    size_t content_length = 0;
    while (req.size() < MAX_REQ) {
        n = ::recv(client_fd, buf, sizeof(buf), 0);
        if (n <= 0) break;
        req.append(buf, buf + n);
        header_end = req.find("\r\n\r\n");
        if (header_end != std::string::npos) {
            // parse content-length
            std::string headers_only = req.substr(0, header_end + 2); // include last \r

            auto headers = parseHeaders(headers_only);
            auto it = headers.find("content-length");
            if (it != headers.end()) {
                try {
                    content_length = static_cast<size_t>(std::stoul(it->second));
                    if (content_length > MAX_REQ) {
                        break;
                    }
                } catch (...) {
                    content_length = 0;
                }
            }
            size_t total_needed = header_end + 4 + content_length;
            if (req.size() >= total_needed) break;
        }
    }
    if (req.empty()) {
        ::close(client_fd);
        return;
    }
    // Parse request line
    std::istringstream iss(req);
    std::string requestLine;
    std::getline(iss, requestLine);
    if (!requestLine.empty() && requestLine.back() == '\r') requestLine.pop_back();
    std::istringstream rl(requestLine);
    std::string method, target, version;
    rl >> method >> target >> version;

    size_t headers_end_pos = req.find("\r\n\r\n");
    std::string headers_raw = req.substr(0, headers_end_pos + 2);
    auto headers = parseHeaders(headers_raw);
    std::string body;
    if (headers_end_pos != std::string::npos) {
        body = req.substr(headers_end_pos + 4);
    }

    bool isNew = false;
    std::string setCookieHeader;
    Session& sess = getOrCreateSession(headers, isNew, setCookieHeader);

    std::ostringstream resp;
    std::string content;
    std::string ctype = "Content-Type: text/html; charset=utf-8\r\n";
    std::string secHeaders =
        "X-Content-Type-Options: nosniff\r\n"
        "X-Frame-Options: DENY\r\n"
        "Referrer-Policy: no-referrer\r\n";

    if (method == "GET" && target == "/") {
        content = buildFormPage(sess, "");
        resp << "HTTP/1.1 200 OK\r\n" << ctype << secHeaders;
        if (!setCookieHeader.empty()) resp << setCookieHeader;
        resp << "Content-Length: " << content.size() << "\r\n\r\n" << content;
    } else if (method == "POST" && target == "/update") {
        auto itCT = headers.find("content-type");
        bool formOk = false;
        if (itCT != headers.end()) {
            std::string v = itCT->second;
            std::transform(v.begin(), v.end(), v.begin(), [](unsigned char ch){ return std::tolower(ch); });
            if (v.find("application/x-www-form-urlencoded") != std::string::npos) formOk = true;
        }
        if (!formOk) {
            content = "<h1>415 Unsupported Media Type</h1>";
            resp << "HTTP/1.1 415 Unsupported Media Type\r\n" << ctype << secHeaders
                 << "Content-Length: " << content.size() << "\r\n\r\n" << content;
        } else {
            auto form = parseFormURLEncoded(body);
            std::string csrf = form.count("csrf") ? form["csrf"] : "";
            if (!constantTimeEquals(csrf, sess.csrf)) {
                content = "<h1>403 Forbidden</h1><p>Invalid CSRF token.</p>";
                resp << "HTTP/1.1 403 Forbidden\r\n" << ctype << secHeaders;
                if (!setCookieHeader.empty()) resp << setCookieHeader;
                resp << "Content-Length: " << content.size() << "\r\n\r\n" << content;
            } else {
                std::string displayName = form.count("displayName") ? form["displayName"] : "";
                std::string email = form.count("email") ? form["email"] : "";
                bool ok = isValidDisplayName(displayName) && isValidEmail(email);
                if (!ok) {
                    content = buildFormPage(sess, "Invalid input.");
                    resp << "HTTP/1.1 400 Bad Request\r\n" << ctype << secHeaders;
                    if (!setCookieHeader.empty()) resp << setCookieHeader;
                    resp << "Content-Length: " << content.size() << "\r\n\r\n" << content;
                } else {
                    // Rotate CSRF token after successful POST to implement single-use semantics
                    sess.csrf = generateRandomToken(32);
                    std::ostringstream msg;
                    msg << "Settings updated for " << displayName << " (" << email << ")";
                    content = buildFormPage(sess, msg.str());
                    resp << "HTTP/1.1 200 OK\r\n" << ctype << secHeaders;
                    if (!setCookieHeader.empty()) resp << setCookieHeader;
                    resp << "Content-Length: " << content.size() << "\r\n\r\n" << content;
                }
            }
        }
    } else {
        content = "<h1>404 Not Found</h1>";
        resp << "HTTP/1.1 404 Not Found\r\n" << ctype << secHeaders
             << "Content-Length: " << content.size() << "\r\n\r\n" << content;
    }
    sendAll(client_fd, resp.str());
    ::close(client_fd);
}

static int run_server(uint16_t port) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return 1;
    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1
    if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(fd);
        return 2;
    }
    if (listen(fd, 16) < 0) {
        ::close(fd);
        return 3;
    }
    for (;;) {
        sockaddr_in caddr{};
        socklen_t clen = sizeof(caddr);
        int cfd = accept(fd, reinterpret_cast<sockaddr*>(&caddr), &clen);
        if (cfd < 0) {
            if (errno == EINTR) continue;
            break;
        }
        handleClient(cfd);
    }
    ::close(fd);
    return 0;
}

// Simple testing utilities
static void assertTest(bool cond, const std::string& name, int& passed, int& failed) {
    if (cond) {
        std::cout << "[OK] " << name << "\n";
        ++passed;
    } else {
        std::cout << "[FAIL] " << name << "\n";
        ++failed;
    }
}

int main() {
    int passed = 0, failed = 0;

    // Test 1: Random token uniqueness
    std::string t1 = generateRandomToken(16);
    std::string t2 = generateRandomToken(16);
    assertTest(t1 != t2 && t1.size() == 32 && t2.size() == 32, "Token uniqueness and size", passed, failed);

    // Test 2: Constant time equals
    assertTest(constantTimeEquals("abc", "abc") && !constantTimeEquals("abc", "abd"), "Constant-time equals", passed, failed);

    // Test 3: URL decode
    std::string dec;
    bool ok = urlDecode("a%2Bb%20c", dec);
    assertTest(ok && dec == "a+b c", "URL decode", passed, failed);

    // Test 4: HTML escape
    std::string escaped = htmlEscape("<a & b>");
    assertTest(escaped == "&lt;a &amp; b&gt;", "HTML escape", passed, failed);

    // Test 5: Form includes CSRF hidden field
    Session s;
    s.id = "id";
    s.csrf = generateRandomToken(16);
    std::string page = buildFormPage(s, "");
    assertTest(page.find("name=\"csrf\"") != std::string::npos, "Form contains CSRF field", passed, failed);

    std::cout << "Passed: " << passed << " Failed: " << failed << "\n";

    const char* run = std::getenv("RUN_SERVER");
    if (run && std::string(run) == "1") {
        return run_server(8080);
    }
    return failed == 0 ? 0 : 1;
}