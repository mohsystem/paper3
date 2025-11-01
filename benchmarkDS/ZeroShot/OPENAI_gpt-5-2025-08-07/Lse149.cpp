#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <ctime>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

static std::mutex db_mutex;
static std::vector<std::pair<std::string, std::string>> DB;
static const size_t MAX_USERNAME = 50;
static const size_t MAX_MESSAGE = 500;
static const size_t MAX_BODY = 4096;

bool insert_user_message_in_db(const std::string& username, const std::string& message) {
    std::string u = username;
    std::string m = message;
    auto trim = [](std::string& s){
        size_t a = s.find_first_not_of(" \t\r\n");
        size_t b = s.find_last_not_of(" \t\r\n");
        if (a == std::string::npos) { s.clear(); return; }
        s = s.substr(a, b - a + 1);
    };
    trim(u); trim(m);
    if (u.empty() || m.empty()) return false;
    if (u.size() > MAX_USERNAME || m.size() > MAX_MESSAGE) return false;
    std::lock_guard<std::mutex> lock(db_mutex);
    DB.emplace_back(u, m);
    return true;
}

std::string html_escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() * 2);
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#x27;"; break;
            case '/': out += "&#x2F;"; break;
            default: out.push_back(c);
        }
    }
    return out;
}

std::string url_decode(const std::string& s) {
    std::string r;
    r.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '+') {
            r.push_back(' ');
        } else if (s[i] == '%' && i + 2 < s.size()) {
            char hex[3] = {s[i+1], s[i+2], 0};
            char* end = nullptr;
            long val = strtol(hex, &end, 16);
            if (end != hex + 2) {
                r.push_back('%');
            } else {
                r.push_back(static_cast<char>(val));
                i += 2;
            }
        } else {
            r.push_back(s[i]);
        }
    }
    return r;
}

std::map<std::string, std::string> parse_form(const std::string& body) {
    std::map<std::string, std::string> m;
    size_t pos = 0;
    while (pos < body.size()) {
        size_t amp = body.find('&', pos);
        if (amp == std::string::npos) amp = body.size();
        std::string kv = body.substr(pos, amp - pos);
        size_t eq = kv.find('=');
        std::string k = url_decode(eq == std::string::npos ? kv : kv.substr(0, eq));
        std::string v = url_decode(eq == std::string::npos ? "" : kv.substr(eq + 1));
        m[k] = v;
        pos = amp + 1;
    }
    return m;
}

std::string render_messages_html() {
    std::vector<std::pair<std::string, std::string>> snapshot;
    {
        std::lock_guard<std::mutex> lock(db_mutex);
        snapshot = DB;
    }
    std::ostringstream oss;
    oss << "<!doctype html><html><head><meta charset='utf-8'>"
        << "<meta http-equiv='Content-Security-Policy' content=\"default-src 'self'; style-src 'self' 'unsafe-inline'\">"
        << "<title>Messages</title></head><body><h1>Messages</h1>"
        << "<form method='POST' action='/post'>"
        << "<label>Username: <input name='username' maxlength='" << MAX_USERNAME << "'></label><br>"
        << "<label>Message: <input name='message' maxlength='" << MAX_MESSAGE << "'></label><br>"
        << "<button type='submit'>Post</button></form><hr><ul>";
    for (auto& p : snapshot) {
        oss << "<li><strong>" << html_escape(p.first) << "</strong>: "
            << html_escape(p.second) << "</li>";
    }
    oss << "</ul></body></html>";
    return oss.str();
}

bool recv_until(int fd, std::string& buf, const std::string& delim, size_t max_len) {
    char tmp[1024];
    while (buf.find(delim) == std::string::npos) {
        ssize_t n = recv(fd, tmp, sizeof(tmp), 0);
        if (n <= 0) return false;
        buf.append(tmp, tmp + n);
        if (buf.size() > max_len) return false;
    }
    return true;
}

void handle_client(int cfd) {
    std::string req;
    if (!recv_until(cfd, req, "\r\n\r\n", 8192)) { close(cfd); return; }
    // Parse request line
    size_t line_end = req.find("\r\n");
    if (line_end == std::string::npos) { close(cfd); return; }
    std::string request_line = req.substr(0, line_end);
    std::istringstream rl(request_line);
    std::string method, path, version;
    rl >> method >> path >> version;
    // Headers
    std::map<std::string, std::string> headers;
    size_t pos = line_end + 2;
    while (true) {
        size_t next = req.find("\r\n", pos);
        if (next == std::string::npos || next == pos) break;
        std::string h = req.substr(pos, next - pos);
        size_t colon = h.find(':');
        if (colon != std::string::npos) {
            std::string key = h.substr(0, colon);
            std::string val = h.substr(colon + 1);
            // trim spaces
            auto ltrim = [](std::string& s){ s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch){return !std::isspace(ch);})); };
            auto rtrim = [](std::string& s){ s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch){return !std::isspace(ch);}).base(), s.end()); };
            ltrim(val); rtrim(val);
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            headers[key] = val;
        }
        pos = next + 2;
    }

    std::string body;
    auto it = headers.find("content-length");
    if (it != headers.end()) {
        size_t cl = std::min((size_t)std::stoul(it->second), MAX_BODY);
        size_t header_len = req.find("\r\n\r\n") + 4;
        body = req.substr(header_len);
        while (body.size() < cl) {
            char tmp[1024];
            ssize_t n = recv(cfd, tmp, sizeof(tmp), 0);
            if (n <= 0) break;
            body.append(tmp, tmp + n);
            if (body.size() > MAX_BODY) { break; }
        }
        if (body.size() > cl) body.resize(cl);
    }

    auto send_all = [&](const std::string& s){
        const char* p = s.data();
        size_t left = s.size();
        while (left > 0) {
            ssize_t n = send(cfd, p, left, 0);
            if (n <= 0) break;
            p += n;
            left -= n;
        }
    };

    if (method == "GET" && path == "/") {
        std::string content = render_messages_html();
        std::ostringstream resp;
        resp << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: text/html; charset=utf-8\r\n"
             << "Content-Length: " << content.size() << "\r\n"
             << "Connection: close\r\n\r\n";
        resp << content;
        send_all(resp.str());
    } else if (method == "POST" && path == "/post") {
        std::string ctype = headers.count("content-type") ? headers["content-type"] : "";
        std::string ctypelow = ctype;
        std::transform(ctypelow.begin(), ctypelow.end(), ctypelow.begin(), ::tolower);
        if (ctypelow.rfind("application/x-www-form-urlencoded", 0) != 0) {
            std::string resp = "HTTP/1.1 415 Unsupported Media Type\r\nConnection: close\r\n\r\n";
            send_all(resp);
            close(cfd);
            return;
        }
        auto form = parse_form(body);
        std::string username = form.count("username") ? form["username"] : "";
        std::string message  = form.count("message") ? form["message"] : "";
        insert_user_message_in_db(username, message);
        std::ostringstream resp;
        resp << "HTTP/1.1 303 See Other\r\n"
             << "Location: /\r\n"
             << "Connection: close\r\n\r\n";
        send_all(resp.str());
    } else {
        std::string resp = "HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n";
        send_all(resp);
    }
    close(cfd);
}

int main() {
    // 5 test cases: pre-populate
    insert_user_message_in_db("alice", "Hello from C++!");
    insert_user_message_in_db("bob", "Safety matters.");
    insert_user_message_in_db("charlie", "No XSS please <tag>");
    insert_user_message_in_db("dora", "Validate inputs.");
    insert_user_message_in_db("eve", "Keep it simple.");

    signal(SIGPIPE, SIG_IGN);
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) return 1;
    int yes = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sfd, (sockaddr*)&addr, sizeof(addr)) < 0) { close(sfd); return 1; }
    if (listen(sfd, 16) < 0) { close(sfd); return 1; }

    while (true) {
        int cfd = accept(sfd, nullptr, nullptr);
        if (cfd < 0) continue;
        handle_client(cfd);
    }
    close(sfd);
    return 0;
}