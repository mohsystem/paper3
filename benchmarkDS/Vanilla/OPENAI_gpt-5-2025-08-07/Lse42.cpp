#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <thread>
#include <mutex>
#include <random>
#include <sstream>
#include <cstring>
#include <cstdlib>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

std::mutex g_mutex;
std::unordered_map<std::string, std::map<std::string, std::string>> SESSIONS_CPP;

std::string info(const std::map<std::string, std::string>& session) {
    auto get = [&](const std::string& k)->std::string {
        auto it = session.find(k);
        return it == session.end() ? "" : it->second;
    };
    std::ostringstream oss;
    oss << "username: " << get("username")
        << "; email: " << get("email")
        << "; ssn: " << get("ssn");
    return oss.str();
}

std::string generate_sid() {
    static const char* hex = "0123456789abcdef";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, 15);
    std::string s;
    s.reserve(32);
    for (int i = 0; i < 32; ++i) s.push_back(hex[dis(gen)]);
    return s;
}

std::string parse_cookie_sid(const std::string& cookie) {
    if (cookie.empty()) return "";
    std::string s = cookie;
    size_t start = 0;
    while (start < s.size()) {
        size_t end = s.find(';', start);
        std::string part = s.substr(start, end == std::string::npos ? std::string::npos : end - start);
        // trim
        size_t l = part.find_first_not_of(" \t\r\n");
        size_t r = part.find_last_not_of(" \t\r\n");
        if (l != std::string::npos) part = part.substr(l, r - l + 1); else part.clear();
        size_t eq = part.find('=');
        if (eq != std::string::npos) {
            std::string k = part.substr(0, eq);
            std::string v = part.substr(eq + 1);
            if (k == "SID") return v;
        }
        if (end == std::string::npos) break;
        start = end + 1;
    }
    return "";
}

void handle_client(int client_fd) {
    char buffer[4096];
    ssize_t n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0) { close(client_fd); return; }
    buffer[n] = '\0';
    std::string req(buffer);

    // Parse request line
    size_t line_end = req.find("\r\n");
    if (line_end == std::string::npos) { close(client_fd); return; }
    std::string request_line = req.substr(0, line_end);

    if (request_line.find("GET /info") != 0) {
        std::string resp = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
        send(client_fd, resp.c_str(), resp.size(), 0);
        close(client_fd);
        return;
    }

    // Parse headers
    std::string headers = req.substr(line_end + 2);
    std::string cookieHeader;
    size_t pos = 0;
    while (true) {
        size_t next = headers.find("\r\n", pos);
        if (next == std::string::npos || next == pos) break;
        std::string h = headers.substr(pos, next - pos);
        pos = next + 2;
        if (h.size() >= 7 && strncasecmp(h.c_str(), "Cookie:", 7) == 0) {
            cookieHeader = h.substr(7);
            // trim
            size_t l = cookieHeader.find_first_not_of(" \t");
            if (l != std::string::npos) cookieHeader = cookieHeader.substr(l);
        }
    }

    std::string sid = parse_cookie_sid(cookieHeader);
    bool isNew = false;
    {
        std::lock_guard<std::mutex> lk(g_mutex);
        if (sid.empty() || SESSIONS_CPP.find(sid) == SESSIONS_CPP.end()) {
            sid = generate_sid();
            SESSIONS_CPP[sid] = {
                {"username", "guest"},
                {"email", "guest@example.com"},
                {"ssn", "000-00-0000"}
            };
            isNew = true;
        }
    }

    std::map<std::string, std::string> session;
    {
        std::lock_guard<std::mutex> lk(g_mutex);
        session = SESSIONS_CPP[sid];
    }
    std::string body = info(session);

    std::ostringstream resp;
    resp << "HTTP/1.1 200 OK\r\n";
    if (isNew) {
        resp << "Set-Cookie: SID=" << sid << "; Path=/; HttpOnly\r\n";
    }
    resp << "Content-Type: text/plain; charset=utf-8\r\n";
    resp << "Content-Length: " << body.size() << "\r\n";
    resp << "Connection: close\r\n\r\n";
    resp << body;

    std::string respStr = resp.str();
    send(client_fd, respStr.c_str(), respStr.size(), 0);
    close(client_fd);
}

void run_server_cpp(uint16_t port) {
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) return;
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(srv, (sockaddr*)&addr, sizeof(addr)) < 0) { close(srv); return; }
    if (listen(srv, 16) < 0) { close(srv); return; }
    std::cout << "C++ server running on http://localhost:" << port << "/info" << std::endl;
    while (true) {
        int c = accept(srv, nullptr, nullptr);
        if (c < 0) continue;
        std::thread(handle_client, c).detach();
    }
}

int main() {
    // 5 test cases
    std::vector<std::map<std::string, std::string>> tests = {
        {{"username","alice"},{"email","alice@example.com"},{"ssn","111-22-3333"}},
        {{"username","bob"},{"email","bob@example.com"},{"ssn","222-33-4444"}},
        {{"username","carol"},{"email","carol@example.com"},{"ssn","333-44-5555"}},
        {{"username","dave"},{"email","dave@example.com"},{"ssn","444-55-6666"}},
        {{"username","eve"},{"email","eve@example.com"},{"ssn","555-66-7777"}},
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        std::cout << "Test " << (i + 1) << ": " << info(tests[i]) << std::endl;
    }

    std::thread srv(run_server_cpp, 8082);
    srv.join();
    return 0;
}