#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstring>
#include <iostream>
#include <string>
#include <unordered_map>

// Product prices
static const std::unordered_map<std::string, double> PRICES = {
    {"P100", 10.00},
    {"P200", 25.50},
    {"P300", 99.99},
    {"P400", 5.25},
    {"P500", 1.50}
};

static bool isValidCode(const std::string& code) {
    if (code.empty() || code.size() > 20) return false;
    for (char c : code) {
        if (!((c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9'))) return false;
    }
    return true;
}

static bool parseQuantity(const std::string& s, int& out) {
    if (s.empty() || s.size() > 9) return false;
    for (char c : s) if (c < '0' || c > '9') return false;
    try {
        long v = std::stol(s);
        if (v < 1 || v > 100000) return false;
        out = static_cast<int>(v);
        return true;
    } catch (...) {
        return false;
    }
}

// Function: return price for a product code, or negative if invalid/unknown
static double getPriceForCode(const std::string& code) {
    if (!isValidCode(code)) return -1.0;
    auto it = PRICES.find(code);
    if (it == PRICES.end()) return -1.0;
    return it->second;
}

// Function: calculate total, or negative on invalid input
static double calculateTotal(const std::string& code, int qty) {
    if (!isValidCode(code)) return -1.0;
    if (qty < 1 || qty > 100000) return -1.0;
    auto it = PRICES.find(code);
    if (it == PRICES.end()) return -1.0;
    return it->second * static_cast<double>(qty);
}

static void send_response(int client_fd, int status, const std::string& json) {
    std::string status_text = "OK";
    if (status == 400) status_text = "Bad Request";
    else if (status == 404) status_text = "Not Found";
    else if (status == 405) status_text = "Method Not Allowed";
    else if (status == 500) status_text = "Internal Server Error";

    std::string hdr = "HTTP/1.1 " + std::to_string(status) + " " + status_text + "\r\n"
                      "Content-Type: application/json; charset=utf-8\r\n"
                      "Content-Length: " + std::to_string(json.size()) + "\r\n"
                      "Connection: close\r\n\r\n";
    // Best effort send
    ssize_t n = send(client_fd, hdr.c_str(), hdr.size(), 0);
    (void)n;
    n = send(client_fd, json.c_str(), json.size(), 0);
    (void)n;
}

static std::string url_decode(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '%' && i + 2 < s.size()) {
            char hi = s[i+1], lo = s[i+2];
            auto hex = [](char c)->int {
                if (c >= '0' && c <= '9') return c - '0';
                if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
                if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
                return -1;
            };
            int h = hex(hi), l = hex(lo);
            if (h >= 0 && l >= 0) {
                out.push_back(static_cast<char>((h<<4) | l));
                i += 2;
            } else {
                out.push_back(s[i]);
            }
        } else if (s[i] == '+') {
            out.push_back(' ');
        } else {
            out.push_back(s[i]);
        }
    }
    return out;
}

static void handle_client(int client_fd) {
    char buf[4096];
    memset(buf, 0, sizeof(buf));
    ssize_t r = recv(client_fd, buf, sizeof(buf)-1, 0);
    if (r <= 0) {
        close(client_fd);
        return;
    }
    std::string req(buf, static_cast<size_t>(r));

    // Expect GET
    if (req.rfind("GET ", 0) != 0) {
        send_response(client_fd, 405, "{\"error\":\"method not allowed\"}");
        close(client_fd);
        return;
    }
    size_t path_start = 4;
    size_t path_end = req.find(' ', path_start);
    if (path_end == std::string::npos) {
        send_response(client_fd, 400, "{\"error\":\"invalid input\"}");
        close(client_fd);
        return;
    }
    std::string fullpath = req.substr(path_start, path_end - path_start);
    // Split path and query
    std::string path = fullpath;
    std::string query;
    size_t qpos = fullpath.find('?');
    if (qpos != std::string::npos) {
        path = fullpath.substr(0, qpos);
        query = fullpath.substr(qpos + 1);
    }

    if (path != "/price") {
        send_response(client_fd, 404, "{\"error\":\"not found\"}");
        close(client_fd);
        return;
    }

    // Parse query
    std::string code, qtyStr;
    size_t start = 0;
    while (start < query.size()) {
        size_t amp = query.find('&', start);
        if (amp == std::string::npos) amp = query.size();
        std::string pair = query.substr(start, amp - start);
        size_t eq = pair.find('=');
        if (eq != std::string::npos) {
            std::string key = url_decode(pair.substr(0, eq));
            std::string val = url_decode(pair.substr(eq + 1));
            if (key == "code") code = val;
            else if (key == "qty") qtyStr = val;
        }
        start = amp + 1;
    }

    if (!isValidCode(code)) {
        send_response(client_fd, 400, "{\"error\":\"invalid input\"}");
        close(client_fd);
        return;
    }
    int qty = 0;
    if (!parseQuantity(qtyStr, qty)) {
        send_response(client_fd, 400, "{\"error\":\"invalid input\"}");
        close(client_fd);
        return;
    }
    auto it = PRICES.find(code);
    if (it == PRICES.end()) {
        send_response(client_fd, 404, "{\"error\":\"not found\"}");
        close(client_fd);
        return;
    }
    double unit = it->second;
    double total = unit * static_cast<double>(qty);

    // Build JSON (numbers without quotes)
    char json[256];
    int n = snprintf(json, sizeof(json),
                     "{\"code\":\"%s\",\"unitPrice\":%.2f,\"quantity\":%d,\"total\":%.2f}",
                     code.c_str(), unit, qty, total);
    if (n < 0 || n >= (int)sizeof(json)) {
        send_response(client_fd, 500, "{\"error\":\"server error\"}");
        close(client_fd);
        return;
    }
    send_response(client_fd, 200, std::string(json));
    close(client_fd);
}

int main() {
    // 5 tests
    std::cout << "Test1 total P100 x 3 = " << calculateTotal("P100", 3) << std::endl; // 30.00
    std::cout << "Test2 total P200 x 2 = " << calculateTotal("P200", 2) << std::endl; // 51.00
    std::cout << "Test3 price P300 = " << getPriceForCode("P300") << std::endl;       // 99.99
    std::cout << "Test4 invalid code -> " << getPriceForCode("BAD!") << std::endl;    // -1
    std::cout << "Test5 invalid qty -> " << calculateTotal("P100", 0) << std::endl;   // -1

    // Setup server on port 5000
    signal(SIGPIPE, SIG_IGN);
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(5000);

    if (bind(srv, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Bind failed: " << strerror(errno) << "\n";
        close(srv);
        return 1;
    }
    if (listen(srv, 16) < 0) {
        std::cerr << "Listen failed: " << strerror(errno) << "\n";
        close(srv);
        return 1;
    }
    std::cout << "Server running on http://0.0.0.0:5000\n";

    while (true) {
        sockaddr_in cli{};
        socklen_t clilen = sizeof(cli);
        int cfd = accept(srv, (sockaddr*)&cli, &clilen);
        if (cfd < 0) {
            continue;
        }
        handle_client(cfd);
    }

    close(srv);
    return 0;
}