#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <random>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

static std::string sanitizeUser(const std::string& v) {
    if (v.empty()) return "unknown";
    std::string out;
    for (char c : v) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c=='-' || c=='_' || c=='.')
            out.push_back(c);
    }
    return out.empty() ? "unknown" : out;
}

static std::string sanitizeEmail(const std::string& v) {
    if (v.empty()) return "unknown@example.com";
    std::string out;
    for (char c : v) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c=='-' || c=='_' || c=='.' || c=='@')
            out.push_back(c);
    }
    if (out.find('@') == std::string::npos) return "unknown@example.com";
    return out;
}

static std::string maskSSN(const std::string& ssn) {
    if (ssn.empty()) return "*********";
    std::vector<char> digits;
    for (char c : ssn) if (std::isdigit(static_cast<unsigned char>(c))) digits.push_back(c);
    size_t keep = std::min<size_t>(4, digits.size());
    size_t keep_from = digits.size() - keep;
    std::string out;
    size_t di=0;
    for (char c : ssn) {
        if (std::isdigit(static_cast<unsigned char>(c))) {
            if (di < keep_from) out.push_back('*');
            else out.push_back(digits[di]);
            di++;
        } else {
            out.push_back(c);
        }
    }
    return out;
}

std::string info(const std::unordered_map<std::string, std::string>& session) {
    auto itU = session.find("username");
    auto itE = session.find("email");
    auto itS = session.find("ssn");
    std::string username = sanitizeUser(itU==session.end() ? "unknown" : itU->second);
    std::string email = sanitizeEmail(itE==session.end() ? "unknown@example.com" : itE->second);
    std::string ssnMasked = maskSSN(itS==session.end() ? "000-00-0000" : itS->second);
    return "username: " + username + "; email: " + email + "; ssn: " + ssnMasked;
}

static std::string genSID() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist;
    uint64_t a = dist(gen), b = dist(gen);
    char buf[33];
    std::snprintf(buf, sizeof(buf), "%016llx%016llx",
                  static_cast<unsigned long long>(a),
                  static_cast<unsigned long long>(b));
    return std::string(buf);
}

static std::string parseCookieSID(const std::string& headers) {
    auto pos = headers.find("Cookie:");
    if (pos == std::string::npos) return "";
    auto end = headers.find("\r\n", pos);
    std::string line = headers.substr(pos, end-pos);
    // Split by ;
    size_t start = line.find(':');
    if (start == std::string::npos) return "";
    std::string cookieStr = line.substr(start+1);
    // tokenize
    size_t i=0;
    while (i < cookieStr.size()) {
        while (i < cookieStr.size() && std::isspace(static_cast<unsigned char>(cookieStr[i]))) i++;
        size_t eq = cookieStr.find('=', i);
        if (eq == std::string::npos) break;
        std::string k = cookieStr.substr(i, eq - i);
        size_t sc = cookieStr.find(';', eq+1);
        std::string v = cookieStr.substr(eq+1, sc==(size_t)-1? std::string::npos : sc - (eq+1));
        if (k == "SID") {
            // trim spaces
            while (!v.empty() && std::isspace(static_cast<unsigned char>(v.front()))) v.erase(v.begin());
            while (!v.empty() && std::isspace(static_cast<unsigned char>(v.back()))) v.pop_back();
            return v;
        }
        if (sc == std::string::npos) break;
        i = sc + 1;
    }
    return "";
}

int main() {
    // 5 test cases for info()
    std::unordered_map<std::string, std::string> t1{{"username","alice"},{"email","alice@example.com"},{"ssn","123-45-6789"}};
    std::cout << info(t1) << "\n";
    std::unordered_map<std::string, std::string> t2{{"username","bob_123"},{"email","bob-123@domain.co.uk"},{"ssn","987654321"}};
    std::cout << info(t2) << "\n";
    std::unordered_map<std::string, std::string> t3{{"username","charlie!"},{"email","charlie_at_example.com"},{"ssn","111-22-3333"}};
    std::cout << info(t3) << "\n";
    std::unordered_map<std::string, std::string> t4{{"username","dora.d"},{"email","dora.d@example.com"},{"ssn","000000000"}};
    std::cout << info(t4) << "\n";
    std::unordered_map<std::string, std::string> t5{};
    std::cout << info(t5) << "\n";

    // Simple HTTP server on 127.0.0.1:8081
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) return 1;
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(8081);
    if (bind(srv, (sockaddr*)&addr, sizeof(addr)) < 0) { close(srv); return 1; }
    if (listen(srv, 8) < 0) { close(srv); return 1; }

    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> sessions;

    while (true) {
        int c = accept(srv, nullptr, nullptr);
        if (c < 0) continue;
        char buf[4096];
        ssize_t n = recv(c, buf, sizeof(buf)-1, 0);
        if (n <= 0) { close(c); continue; }
        buf[n] = '\0';
        std::string req(buf);
        // Only handle GET /info
        if (req.rfind("GET /info", 0) == 0) {
            std::string sid = parseCookieSID(req);
            bool setCookie = false;
            if (sid.empty() || sessions.find(sid) == sessions.end()) {
                sid = genSID();
                sessions[sid] = {{"username","demo_user"},{"email","demo_user@example.com"},{"ssn","123-45-6789"}};
                setCookie = true;
            }
            std::string body = info(sessions[sid]);
            std::string resp = "HTTP/1.1 200 OK\r\n"
                               "Content-Type: text/plain; charset=utf-8\r\n"
                               "X-Content-Type-Options: nosniff\r\n"
                               "Cache-Control: no-store\r\n";
            if (setCookie) {
                resp += "Set-Cookie: SID=" + sid + "; HttpOnly; SameSite=Lax; Path=/\r\n";
            }
            resp += "Content-Length: " + std::to_string(body.size()) + "\r\n"
                    "\r\n" + body;
            send(c, resp.data(), resp.size(), 0);
        } else {
            const char* notfound = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
            send(c, notfound, std::strlen(notfound), 0);
        }
        close(c);
    }
    close(srv);
    return 0;
}