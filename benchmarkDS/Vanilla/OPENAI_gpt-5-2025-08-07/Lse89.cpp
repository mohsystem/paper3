// Simple POSIX-only HTTP server and client in one file.
// Builds with: g++ -std=c++17 file.cpp -lpthread
#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <vector>
#include <cstring>
#include <sstream>
#include <chrono>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

double price_for_code(const std::string& code) {
    if (code == "A100") return 10.0;
    if (code == "B200") return 20.0;
    if (code == "C300") return 7.5;
    if (code == "D400") return 15.25;
    if (code == "E500") return 5.75;
    return 0.0;
}

double total_price(const std::string& code, int qty) {
    if (qty < 0) qty = 0;
    return price_for_code(code) * qty;
}

static std::string url_decode(const std::string& s) {
    std::string out;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '+') out.push_back(' ');
        else if (s[i] == '%' && i + 2 < s.size()) {
            int v = 0;
            std::istringstream iss(s.substr(i + 1, 2));
            iss >> std::hex >> v;
            out.push_back(static_cast<char>(v));
            i += 2;
        } else out.push_back(s[i]);
    }
    return out;
}

static void parse_query(const std::string& q, std::string& code, int& qty) {
    code.clear();
    qty = 0;
    std::istringstream iss(q);
    std::string kv;
    while (std::getline(iss, kv, '&')) {
        auto pos = kv.find('=');
        std::string k = (pos == std::string::npos) ? kv : kv.substr(0, pos);
        std::string v = (pos == std::string::npos) ? "" : kv.substr(pos + 1);
        k = url_decode(k);
        v = url_decode(v);
        if (k == "code") code = v;
        else if (k == "qty") {
            try { qty = std::stoi(v); } catch (...) { qty = 0; }
        }
    }
}

static void handle_client(int client_fd) {
    char buf[4096];
    ssize_t n = recv(client_fd, buf, sizeof(buf)-1, 0);
    if (n <= 0) { close(client_fd); return; }
    buf[n] = '\0';
    std::string req(buf);

    // Parse request line
    std::istringstream iss(req);
    std::string method, path, version;
    iss >> method >> path >> version;

    std::string body;
    int status = 200;
    std::string contentType = "application/json; charset=utf-8";

    if (method != "GET") {
        status = 405;
        body = "{\"error\":\"method not allowed\"}";
    } else {
        std::string route = path;
        std::string query;
        auto qpos = path.find('?');
        if (qpos != std::string::npos) {
            route = path.substr(0, qpos);
            query = path.substr(qpos + 1);
        }

        if (route != "/price") {
            status = 404;
            body = "{\"error\":\"not found\"}";
        } else {
            std::string code;
            int qty = 0;
            parse_query(query, code, qty);
            double unit = price_for_code(code);
            double total = (qty < 0 ? 0 : qty) * unit;
            std::ostringstream os;
            os.setf(std::ios::fixed); os.precision(2);
            os << "{\"code\":\"" << code << "\",\"unitPrice\":" << unit
               << ",\"qty\":" << qty << ",\"total\":" << total << "}";
            body = os.str();
        }
    }

    std::ostringstream resp;
    resp << "HTTP/1.1 " << status << " OK\r\n";
    resp << "Content-Type: " << contentType << "\r\n";
    resp << "Content-Length: " << body.size() << "\r\n";
    resp << "Connection: close\r\n\r\n";
    resp << body;
    std::string r = resp.str();
    send(client_fd, r.data(), r.size(), 0);
    close(client_fd);
}

static void server_loop(std::atomic<bool>& running, int port) {
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) return;

    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sfd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sfd); return;
    }
    if (listen(sfd, 16) < 0) {
        close(sfd); return;
    }

    while (running.load()) {
        sockaddr_in caddr{};
        socklen_t clen = sizeof(caddr);
        int cfd = accept(sfd, (sockaddr*)&caddr, &clen);
        if (cfd < 0) continue;
        std::thread(handle_client, cfd).detach();
    }
    close(sfd);
}

static std::string http_get(const std::string& host, int port, const std::string& path) {
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) return "";

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &addr.sin_addr);

    if (connect(sfd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sfd);
        return "";
    }

    std::ostringstream req;
    req << "GET " << path << " HTTP/1.1\r\n"
        << "Host: " << host << "\r\n"
        << "Connection: close\r\n\r\n";
    std::string r = req.str();
    send(sfd, r.data(), r.size(), 0);

    std::string out;
    char buf[4096];
    ssize_t n;
    while ((n = recv(sfd, buf, sizeof(buf), 0)) > 0) {
        out.append(buf, buf + n);
    }
    close(sfd);
    return out;
}

int main() {
    const int port = 5000;
    std::atomic<bool> running(true);
    std::thread srv([&] { server_loop(running, port); });

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // 5 test cases against the endpoint
    std::vector<std::string> tests = {
        "/price?code=A100&qty=3",
        "/price?code=B200&qty=2",
        "/price?code=C300&qty=5",
        "/price?code=D400&qty=1",
        "/price?code=Z999&qty=4"
    };

    for (auto& t : tests) {
        std::string resp = http_get("127.0.0.1", port, t);
        // Print only the body for brevity
        auto pos = resp.find("\r\n\r\n");
        std::string body = (pos != std::string::npos) ? resp.substr(pos + 4) : resp;
        std::cout << "GET " << t << " -> " << body << std::endl;
    }

    // Function tests
    std::cout << "price_for_code('A100') = " << price_for_code("A100") << std::endl;
    std::cout << "total_price('B200', 3) = " << total_price("B200", 3) << std::endl;

    running.store(false);
    // Connect to unblock accept if needed
    http_get("127.0.0.1", port, "/");
    srv.join();
    return 0;
}