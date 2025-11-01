#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cctype>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

std::string html_escape(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#39;"; break;
            default: out += c; break;
        }
    }
    return out;
}

std::string last4_digits(const std::string& ssn) {
    std::string digits;
    for (char c : ssn) if (std::isdigit(static_cast<unsigned char>(c))) digits += c;
    if (digits.size() <= 4) return digits;
    return digits.substr(digits.size() - 4);
}

std::string render_info(const std::string& username, const std::string& email, const std::string& ssn) {
    std::string u = html_escape(username);
    std::string e = html_escape(email);
    std::string l4 = html_escape(last4_digits(ssn));
    std::ostringstream oss;
    oss << "<!DOCTYPE html>"
        << "<html><head><meta charset=\"utf-8\"><title>Info</title></head><body>"
        << "<h1>User Info</h1><ul>"
        << "<li>Username: " << u << "</li>"
        << "<li>Email: " << e << "</li>"
        << "<li>SSN Last4: " << l4 << "</li>"
        << "</ul></body></html>";
    return oss.str();
}

static int hexval(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

std::string url_decode(const std::string& in) {
    std::string out;
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        char c = in[i];
        if (c == '+') {
            out.push_back(' ');
        } else if (c == '%' && i + 2 < in.size()) {
            int h1 = hexval(in[i + 1]);
            int h2 = hexval(in[i + 2]);
            if (h1 >= 0 && h2 >= 0) {
                out.push_back(static_cast<char>((h1 << 4) | h2));
                i += 2;
            } else {
                out.push_back(c);
            }
        } else {
            out.push_back(c);
        }
    }
    return out;
}

std::map<std::string, std::string> parse_query(const std::string& query) {
    std::map<std::string, std::string> params;
    size_t start = 0;
    while (start <= query.size()) {
        size_t amp = query.find('&', start);
        std::string pair = query.substr(start, amp == std::string::npos ? std::string::npos : amp - start);
        if (!pair.empty()) {
            size_t eq = pair.find('=');
            std::string key = url_decode(eq == std::string::npos ? pair : pair.substr(0, eq));
            std::string val = url_decode(eq == std::string::npos ? "" : pair.substr(eq + 1));
            params[key] = val;
        }
        if (amp == std::string::npos) break;
        start = amp + 1;
    }
    return params;
}

void run_server() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) return;

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8082);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        close(server_fd);
        return;
    }

    if (listen(server_fd, 16) < 0) {
        close(server_fd);
        return;
    }

    char buffer[8192];

    while (true) {
        int client = accept(server_fd, nullptr, nullptr);
        if (client < 0) continue;

        ssize_t n = recv(client, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            close(client);
            continue;
        }
        buffer[n] = '\0';
        std::string req(buffer);
        // Parse request line
        size_t line_end = req.find("\r\n");
        if (line_end == std::string::npos) {
            close(client);
            continue;
        }
        std::string reqline = req.substr(0, line_end);
        size_t sp1 = reqline.find(' ');
        size_t sp2 = reqline.find(' ', sp1 == std::string::npos ? 0 : sp1 + 1);
        if (sp1 == std::string::npos || sp2 == std::string::npos) {
            close(client);
            continue;
        }
        std::string method = reqline.substr(0, sp1);
        std::string target = reqline.substr(sp1 + 1, sp2 - sp1 - 1);

        if (method != "GET") {
            std::string resp = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
            send(client, resp.c_str(), resp.size(), 0);
            close(client);
            continue;
        }

        std::string path = target;
        std::string query;
        size_t qpos = target.find('?');
        if (qpos != std::string::npos) {
            path = target.substr(0, qpos);
            query = target.substr(qpos + 1);
        }

        if (path != "/info") {
            std::string body = "<h1>Not Found</h1>";
            std::ostringstream oss;
            oss << "HTTP/1.1 404 Not Found\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: "
                << body.size() << "\r\nConnection: close\r\n\r\n" << body;
            std::string resp = oss.str();
            send(client, resp.c_str(), resp.size(), 0);
            close(client);
            continue;
        }

        auto params = parse_query(query);
        std::string username = params.count("username") ? params["username"] : "";
        std::string email = params.count("email") ? params["email"] : "";
        std::string ssn = params.count("ssn") ? params["ssn"] : "";

        std::string body = render_info(username, email, ssn);
        std::ostringstream oss;
        oss << "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: "
            << body.size() << "\r\nConnection: close\r\n\r\n" << body;
        std::string resp = oss.str();
        send(client, resp.c_str(), resp.size(), 0);
        close(client);
    }

    close(server_fd);
}

int main() {
    // 5 test cases
    std::cout << render_info("alice", "alice@example.com", "123-45-6789") << std::endl;
    std::cout << render_info("bob", "bob@example.com", "000112223") << std::endl;
    std::cout << render_info("charlie", "charlie@company.org", "9999") << std::endl;
    std::cout << render_info("dora", "dora@mail.net", "111-22-3333") << std::endl;
    std::cout << render_info("eve", "eve@domain.com", "12-34-56-78-90") << std::endl;

    run_server();
    return 0;
}