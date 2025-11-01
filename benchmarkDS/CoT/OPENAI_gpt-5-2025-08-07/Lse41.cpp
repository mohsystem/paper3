// 1) Problem understanding:
// Provide a minimal HTTP server with route /info that returns an HTML page with username, email,
// and last 4 digits of SSN. Core logic in renderInfo(username, email, ssn).

#include <iostream>
#include <string>
#include <regex>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cstring>

#ifdef _WIN32
// For simplicity, this example targets POSIX sockets; Windows would need Winsock initialization.
// If building on Windows, additional setup is required.
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

// 2) Security requirements:
// - Escape HTML.
// - Only last 4 digits of SSN.
// - Basic email validation.
// - Minimal and safe request handling (no parsing of sensitive info from URL).

static std::string escapeHtml(const std::string& in) {
    std::string out;
    out.reserve(in.size() + 16);
    for (char c : in) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#x27;"; break;
            case '/': out += "&#x2F;"; break;
            default: out += c; break;
        }
    }
    return out;
}

static std::string extractLast4(const std::string& ssn) {
    std::string digits;
    digits.reserve(ssn.size());
    for (char c : ssn) if (std::isdigit(static_cast<unsigned char>(c))) digits.push_back(c);
    if (digits.size() < 4) return "N/A";
    return digits.substr(digits.size() - 4);
}

static bool isLikelyValidEmail(const std::string& email) {
    if (email.empty() || email.size() > 254) return false;
    static const std::regex re("^[A-Za-z0-9._%+\\-]+@[A-Za-z0-9.\\-]+\\.[A-Za-z]{2,63}$");
    return std::regex_match(email, re);
}

// 3) Secure coding generation: function interface takes params and returns output (HTML)
std::string renderInfo(const std::string& username, const std::string& email, const std::string& ssn) {
    std::string safeUser = escapeHtml(username);
    std::string safeEmail = escapeHtml(email);
    std::string last4 = escapeHtml(extractLast4(ssn));
    std::string emailNote = isLikelyValidEmail(email) ? "" : " (unverified format)";
    std::string csp = "default-src 'none'; style-src 'unsafe-inline'; img-src 'none'; frame-ancestors 'none'; base-uri 'none'; form-action 'none'";
    std::ostringstream html;
    html << "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">"
         << "<meta http-equiv=\"Content-Security-Policy\" content=\"" << escapeHtml(csp) << "\">"
         << "<meta http-equiv=\"X-Content-Type-Options\" content=\"nosniff\">"
         << "<title>User Info</title>"
         << "<style>body{font-family:sans-serif;margin:2rem} .box{border:1px solid #ccc;padding:1rem;border-radius:8px;max-width:500px}</style>"
         << "</head><body><div class=\"box\">"
         << "<h1>User Info</h1>"
         << "<p><strong>Username:</strong> " << safeUser << "</p>"
         << "<p><strong>Email:</strong> " << safeEmail << emailNote << "</p>"
         << "<p><strong>SSN (last 4):</strong> " << last4 << "</p>"
         << "</div></body></html>";
    return html.str();
}

static void runServer(int port) {
    int server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Socket creation failed\n";
        return;
    }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(static_cast<uint16_t>(port));
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Bind failed\n";
        close(server_fd);
        return;
    }
    if (listen(server_fd, 10) < 0) {
        std::cerr << "Listen failed\n";
        close(server_fd);
        return;
    }
    std::cout << "C++ server started on http://localhost:" << port << "/info\n";
    while (true) {
        int client = accept(server_fd, nullptr, nullptr);
        if (client < 0) continue;

        char buffer[2048];
        ssize_t n = read(client, buffer, sizeof(buffer) - 1);
        if (n <= 0) { close(client); continue; }
        buffer[n] = '\0';
        std::string req(buffer);

        // Very minimal parsing: check if GET /info
        bool ok = false;
        {
            // parse first line
            auto pos = req.find("\r\n");
            std::string line = (pos == std::string::npos) ? req : req.substr(0, pos);
            if (line.rfind("GET ", 0) == 0) {
                auto sp = line.find(' ', 4);
                std::string path = (sp == std::string::npos) ? "/" : line.substr(4, sp - 4);
                if (path == "/info") ok = true;
            }
        }

        if (!ok) {
            const char* notFound = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: 9\r\n\r\nNot Found";
            write(client, notFound, std::strlen(notFound));
            close(client);
            continue;
        }

        std::string body = renderInfo("alice", "alice@example.com", "123-45-6789");
        std::ostringstream resp;
        resp << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: text/html; charset=utf-8\r\n"
             << "Content-Security-Policy: default-src 'none'; style-src 'unsafe-inline'; img-src 'none'; frame-ancestors 'none'; base-uri 'none'; form-action 'none'\r\n"
             << "X-Content-Type-Options: nosniff\r\n"
             << "Content-Length: " << body.size() << "\r\n"
             << "Connection: close\r\n\r\n"
             << body;
        std::string out = resp.str();
        write(client, out.data(), out.size());
        close(client);
    }
}

// 4) Code review summary in comments: outputs escaped; only last 4 digits; simple router; security headers.

int main() {
    // 5 Test cases
    std::vector<std::tuple<std::string,std::string,std::string>> tests = {
        {"alice", "alice@example.com", "123-45-6789"},
        {"bob", "bob.smith@corp.co.uk", "000123456"},
        {"<script>alert(1)</script>", "user@ex<ample>.com", "12-34"},
        {"charlie", "invalid-email@", "xxx-yy-7788"},
        {"Δημήτρης", "dimitris@example.gr", "9999"}
    };
    int idx = 1;
    for (auto& t : tests) {
        std::cout << "=== C++ Test Case " << idx++ << " ===\n";
        std::cout << renderInfo(std::get<0>(t), std::get<1>(t), std::get<2>(t)) << "\n\n";
    }
    runServer(8082);
    return 0;
}