// Chain-of-Through:
// 1) Problem: Web app on port 5000 with function returning unit price and route /price?code=&qty= to compute total.
// 2) Security: Validate inputs, bound checks, safe parsing, minimal surface.
// 3) Secure coding: careful socket handling, avoid buffer overflows, simple JSON, restricted characters.
// 4) Review: parse GET line safely, close sockets properly, handle errors.
// 5) Output: hardened code.

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cctype>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>

static const std::map<std::string, double> PRICES = {
    {"A100", 9.99},
    {"B200", 14.50},
    {"C300", 4.25},
    {"D400", 100.00},
    {"E500", 0.99},
};

bool is_valid_code(const std::string& code) {
    if (code.empty() || code.size() > 20) return false;
    for (char c : code) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-')) return false;
    }
    return true;
}

std::optional<double> get_price(const std::string& code) {
    if (!is_valid_code(code)) return std::nullopt;
    auto it = PRICES.find(code);
    if (it == PRICES.end()) return std::nullopt;
    return it->second;
}

static int parse_int_safely(const std::string& s) {
    if (s.empty() || s.size() > 10) return -1;
    for (char c : s) if (!std::isdigit(static_cast<unsigned char>(c))) return -1;
    try {
        long long v = std::stoll(s);
        if (v < 1 || v > 1000000) return -1;
        return static_cast<int>(v);
    } catch (...) {
        return -1;
    }
}

static std::string urldecode(const std::string& in) {
    std::string out;
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        if (in[i] == '%' && i + 2 < in.size() && std::isxdigit(in[i+1]) && std::isxdigit(in[i+2])) {
            std::string hex = in.substr(i+1, 2);
            char ch = static_cast<char>(std::strtoul(hex.c_str(), nullptr, 16));
            out.push_back(ch);
            i += 2;
        } else if (in[i] == '+') {
            out.push_back(' ');
        } else {
            out.push_back(in[i]);
        }
    }
    return out;
}

static void send_response(int client, int status, const std::string& body) {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << status << " "
        << (status == 200 ? "OK" : (status == 400 ? "Bad Request" : (status == 404 ? "Not Found" : "Error"))) << "\r\n"
        << "Content-Type: application/json; charset=utf-8\r\n"
        << "X-Content-Type-Options: nosniff\r\n"
        << "Cache-Control: no-store\r\n"
        << "Connection: close\r\n"
        << "Content-Length: " << body.size() << "\r\n\r\n"
        << body;
    std::string resp = oss.str();
    ssize_t sent = 0;
    const char* data = resp.c_str();
    size_t left = resp.size();
    while (left > 0) {
        ssize_t n = send(client, data + sent, left, 0);
        if (n <= 0) break;
        sent += n;
        left -= n;
    }
}

static void handle_client(int client) {
    char buf[2048];
    std::memset(buf, 0, sizeof(buf));
    ssize_t n = recv(client, buf, sizeof(buf) - 1, 0);
    if (n <= 0) {
        close(client);
        return;
    }
    std::string req(buf, n);
    // Parse request line
    size_t line_end = req.find("\r\n");
    if (line_end == std::string::npos) { send_response(client, 400, "{\"error\":\"bad_request\"}"); close(client); return; }
    std::string line = req.substr(0, line_end);
    if (line.rfind("GET ", 0) != 0) { send_response(client, 405, "{\"error\":\"method_not_allowed\"}"); close(client); return; }
    size_t sp2 = line.find(' ', 4);
    if (sp2 == std::string::npos) { send_response(client, 400, "{\"error\":\"bad_request\"}"); close(client); return; }
    std::string target = line.substr(4, sp2 - 4); // path?query
    // Extract path and query
    std::string path = target;
    std::string query;
    size_t qpos = target.find('?');
    if (qpos != std::string::npos) {
        path = target.substr(0, qpos);
        query = target.substr(qpos + 1);
    }
    if (path != "/price") {
        send_response(client, 404, "{\"error\":\"not_found\"}");
        close(client);
        return;
    }
    // Parse query params
    std::string code, qtys;
    size_t start = 0;
    while (start < query.size()) {
        size_t amp = query.find('&', start);
        if (amp == std::string::npos) amp = query.size();
        std::string pair = query.substr(start, amp - start);
        size_t eq = pair.find('=');
        if (eq != std::string::npos) {
            std::string k = urldecode(pair.substr(0, eq));
            std::string v = urldecode(pair.substr(eq + 1));
            if (k == "code") code = v;
            else if (k == "qty") qtys = v;
        }
        start = amp + 1;
    }
    if (code.empty() || qtys.empty()) { send_response(client, 400, "{\"error\":\"missing_parameters\"}"); close(client); return; }
    if (!is_valid_code(code)) { send_response(client, 400, "{\"error\":\"invalid_code\"}"); close(client); return; }
    int qty = parse_int_safely(qtys);
    if (qty <= 0) { send_response(client, 400, "{\"error\":\"invalid_quantity\"}"); close(client); return; }
    auto priceOpt = get_price(code);
    if (!priceOpt.has_value()) { send_response(client, 404, "{\"error\":\"unknown_code\"}"); close(client); return; }
    double unit = priceOpt.value();
    double total = unit * static_cast<double>(qty);
    std::ostringstream body;
    body.setf(std::ios::fixed); body.precision(2);
    body << "{\"code\":\"" << code << "\",\"quantity\":" << qty << ",\"unit_price\":" << unit << ",\"total\":" << total << "}";
    send_response(client, 200, body.str());
    close(client);
}

void start_server(int port) {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) { std::perror("socket"); return; }
    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (sockaddr*)&addr, sizeof(addr)) < 0) { std::perror("bind"); close(s); return; }
    if (listen(s, 16) < 0) { std::perror("listen"); close(s); return; }
    std::cout << "C++ server started on port " << port << std::endl;
    while (true) {
        sockaddr_in caddr{};
        socklen_t clen = sizeof(caddr);
        int c = accept(s, (sockaddr*)&caddr, &clen);
        if (c < 0) {
            if (errno == EINTR) continue;
            std::perror("accept"); break;
        }
        handle_client(c);
    }
    close(s);
}

int main() {
    // 5 test cases (function)
    auto t1 = get_price("A100"); std::cout << "Test1 A100 x2 => " << (t1 ? (*t1 * 2) : -1) << std::endl;
    auto t2 = get_price("B200"); std::cout << "Test2 B200 x1 => " << (t2 ? (*t2 * 1) : -1) << std::endl;
    auto t3 = get_price("C300"); std::cout << "Test3 C300 x5 => " << (t3 ? (*t3 * 5) : -1) << std::endl;
    auto t4 = get_price("D400"); std::cout << "Test4 D400 x10 => " << (t4 ? (*t4 * 10) : -1) << std::endl;
    auto t5 = get_price("Z999"); std::cout << "Test5 Z999 x3 => " << (t5 ? (*t5 * 3) : -1) << std::endl;

    start_server(5000);
    return 0;
}