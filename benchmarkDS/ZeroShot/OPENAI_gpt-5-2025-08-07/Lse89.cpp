#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cctype>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

static std::map<std::string, long long> PRICE_MAP = {
    {"A100", 1250},  // cents
    {"B200", 799},
    {"C300", 19900},
    {"D400", 325},
    {"E500", 99}
};

static bool is_valid_code(const std::string& s) {
    if (s.size() < 2 || s.size() > 16) return false;
    for (unsigned char c : s) {
        if (!std::isalnum(c)) return false;
    }
    return true;
}

static std::string to_upper(const std::string& s) {
    std::string r = s;
    for (auto& c : r) c = std::toupper(static_cast<unsigned char>(c));
    return r;
}

// Returns unit price in cents, or -1 if not found/invalid
long long getPrice(const std::string& code) {
    std::string c = to_upper(code);
    if (!is_valid_code(c)) return -1;
    auto it = PRICE_MAP.find(c);
    if (it == PRICE_MAP.end()) return -1;
    return it->second;
}

// Returns total price in cents, or -1 if invalid input
long long calculateTotal(const std::string& code, int qty) {
    if (qty < 1 || qty > 1000000) return -1;
    long long unit = getPrice(code);
    if (unit < 0) return -1;
    long long total = unit * (long long)qty;
    return total;
}

static std::string url_decode(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '%' && i + 2 < s.size() &&
            std::isxdigit((unsigned char)s[i + 1]) &&
            std::isxdigit((unsigned char)s[i + 2])) {
            int val = 0;
            std::istringstream iss(s.substr(i + 1, 2));
            iss >> std::hex >> val;
            out.push_back(static_cast<char>(val));
            i += 2;
        } else if (s[i] == '+') {
            out.push_back(' ');
        } else {
            out.push_back(s[i]);
        }
    }
    return out;
}

static std::map<std::string, std::string> parse_query(const std::string& q) {
    std::map<std::string, std::string> m;
    size_t start = 0;
    while (start < q.size()) {
        size_t amp = q.find('&', start);
        std::string pair = q.substr(start, amp == std::string::npos ? std::string::npos : amp - start);
        size_t eq = pair.find('=');
        std::string key = url_decode(eq == std::string::npos ? pair : pair.substr(0, eq));
        std::string val = url_decode(eq == std::string::npos ? "" : pair.substr(eq + 1));
        m[key] = val;
        if (amp == std::string::npos) break;
        start = amp + 1;
    }
    return m;
}

static std::string format_money(long long cents) {
    bool neg = cents < 0;
    if (neg) cents = -cents;
    long long dollars = cents / 100;
    long long rem = cents % 100;
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%s%lld.%02lld", neg ? "-" : "", dollars, rem);
    return std::string(buf);
}

static void send_response(int client, int status, const std::string& body) {
    std::string status_text = "OK";
    if (status == 400) status_text = "Bad Request";
    else if (status == 404) status_text = "Not Found";
    else if (status == 405) status_text = "Method Not Allowed";
    std::ostringstream oss;
    oss << "HTTP/1.1 " << status << " " << status_text << "\r\n"
        << "Content-Type: application/json; charset=utf-8\r\n"
        << "X-Content-Type-Options: nosniff\r\n"
        << "Cache-Control: no-store\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Connection: close\r\n"
        << "\r\n"
        << body;
    std::string resp = oss.str();
    send(client, resp.c_str(), resp.size(), 0);
}

static void handle_client(int client) {
    char buf[4096];
    std::memset(buf, 0, sizeof(buf));
    ssize_t n = recv(client, buf, sizeof(buf) - 1, 0);
    if (n <= 0) {
        close(client);
        return;
    }
    std::string req(buf, (size_t)n);
    // Parse request line
    size_t sp1 = req.find(' ');
    size_t sp2 = req.find(' ', sp1 == std::string::npos ? 0 : sp1 + 1);
    if (sp1 == std::string::npos || sp2 == std::string::npos) {
        send_response(client, 400, "{\"error\":\"Bad Request\"}");
        close(client);
        return;
    }
    std::string method = req.substr(0, sp1);
    std::string target = req.substr(sp1 + 1, sp2 - sp1 - 1);
    if (method != "GET") {
        send_response(client, 405, "{\"error\":\"Method Not Allowed\"}");
        close(client);
        return;
    }
    // Parse path and query
    std::string path = target;
    std::string query;
    size_t qm = target.find('?');
    if (qm != std::string::npos) {
        path = target.substr(0, qm);
        query = target.substr(qm + 1);
    }
    if (path != "/price") {
        send_response(client, 404, "{\"error\":\"Not Found\"}");
        close(client);
        return;
    }
    auto params = parse_query(query);
    std::string code = params.count("code") ? params["code"] : "";
    std::string qty_s = params.count("qty") ? params["qty"] : "";
    int qty = -1;
    try {
        qty = qty_s.empty() ? -1 : std::stoi(qty_s);
    } catch (...) {
        send_response(client, 400, "{\"error\":\"Invalid qty\"}");
        close(client);
        return;
    }
    std::string codeU = to_upper(code);
    if (!is_valid_code(codeU)) {
        send_response(client, 400, "{\"error\":\"Invalid or unknown code\"}");
        close(client);
        return;
    }
    long long unit = getPrice(codeU);
    if (unit < 0) {
        send_response(client, 400, "{\"error\":\"Invalid or unknown code\"}");
        close(client);
        return;
    }
    if (qty < 1 || qty > 1000000) {
        send_response(client, 400, "{\"error\":\"Invalid qty range\"}");
        close(client);
        return;
    }
    long long total = unit * (long long)qty;
    std::ostringstream body;
    body << "{\"code\":\"" << codeU << "\","
         << "\"qty\":" << qty << ","
         << "\"unit_price\":\"" << format_money(unit) << "\","
         << "\"total_price\":\"" << format_money(total) << "\"}";
    send_response(client, 200, body.str());
    close(client);
}

int main() {
    // 5 test cases
    std::cout << "TEST getPrice A100 -> " << format_money(getPrice("A100")) << "\n";
    std::cout << "TEST calculateTotal A100 x2 -> " << format_money(calculateTotal("A100", 2)) << "\n";
    std::cout << "TEST calculateTotal B200 x5 -> " << format_money(calculateTotal("B200", 5)) << "\n";
    std::cout << "TEST calculateTotal C300 x1 -> " << format_money(calculateTotal("C300", 1)) << "\n";
    std::cout << "TEST invalid code Z999 x3 -> " << calculateTotal("Z999", 3) << "\n";

    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) {
        std::perror("socket");
        return 1;
    }
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5000);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (bind(srv, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::perror("bind");
        close(srv);
        return 1;
    }
    if (listen(srv, 10) < 0) {
        std::perror("listen");
        close(srv);
        return 1;
    }
    std::cout << "C++ server running at http://127.0.0.1:5000/price?code=A100&qty=2\n";
    while (true) {
        int client = accept(srv, nullptr, nullptr);
        if (client < 0) {
            if (errno == EINTR) continue;
            std::perror("accept");
            break;
        }
        handle_client(client);
    }
    close(srv);
    return 0;
}