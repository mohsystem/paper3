#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <thread>
#include <cstring>

#ifdef _WIN32
// Windows sockets (optional): For simplicity, we provide a note that POSIX sockets are used below.
// If running on Windows, compile with Winsock setup or use WSL.
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

std::string extractName(const std::string& payload) {
    std::string trimmed = payload;
    // JSON regex: "name" : "value"
    try {
        std::regex r("\"name\"\\s*:\\s*\"((?:[^\"\\\\]|\\\\.)*)\"");
        std::smatch m;
        if (std::regex_search(trimmed, m, r)) {
            std::string val = m[1].str();
            // Unescape basic sequences
            std::string out;
            out.reserve(val.size());
            for (size_t i = 0; i < val.size(); ++i) {
                if (val[i] == '\\' && i + 1 < val.size()) {
                    char c = val[i + 1];
                    if (c == '"' || c == '\\' || c == '/') { out.push_back(c); i++; }
                    else if (c == 'n') { out.push_back('\n'); i++; }
                    else if (c == 'r') { out.push_back('\r'); i++; }
                    else if (c == 't') { out.push_back('\t'); i++; }
                    else { out.push_back(val[i]); }
                } else {
                    out.push_back(val[i]);
                }
            }
            return out;
        }
    } catch (...) {}

    // Form-urlencoded: name=value
    auto pos = trimmed.find("name=");
    if (pos != std::string::npos) {
        size_t start = pos + 5;
        size_t end = trimmed.find('&', start);
        std::string val = (end != std::string::npos) ? trimmed.substr(start, end - start) : trimmed.substr(start);
        // Basic URL decode for + and %xx
        std::string out;
        for (size_t i = 0; i < val.size(); ++i) {
            if (val[i] == '+') {
                out.push_back(' ');
            } else if (val[i] == '%' && i + 2 < val.size()) {
                auto hex = val.substr(i + 1, 2);
                char ch = (char) strtol(hex.c_str(), nullptr, 16);
                out.push_back(ch);
                i += 2;
            } else {
                out.push_back(val[i]);
            }
        }
        return out;
    }
    return std::string();
}

#ifndef _WIN32
void run_server_cpp() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "C++ server socket creation failed\n";
        return;
    }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8082);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "C++ bind failed\n";
        close(server_fd);
        return;
    }
    if (listen(server_fd, 5) < 0) {
        std::cerr << "C++ listen failed\n";
        close(server_fd);
        return;
    }
    std::cout << "C++ server running at http://localhost:8082/\n";
    while (true) {
        int client = accept(server_fd, nullptr, nullptr);
        if (client < 0) break;
        std::string req;
        char buf[4096];
        ssize_t n;
        while ((n = recv(client, buf, sizeof(buf), 0)) > 0) {
            req.append(buf, buf + n);
            if (req.find("\r\n\r\n") != std::string::npos) {
                break;
            }
        }
        // Extract body if Content-Length present
        size_t header_end = req.find("\r\n\r\n");
        std::string body;
        if (header_end != std::string::npos) {
            std::string headers = req.substr(0, header_end);
            size_t cl_pos = headers.find("Content-Length:");
            if (cl_pos != std::string::npos) {
                size_t line_end = headers.find("\r\n", cl_pos);
                std::string cl_line = headers.substr(cl_pos, line_end - cl_pos);
                int content_length = 0;
                sscanf(cl_line.c_str(), "Content-Length: %d", &content_length);
                body = req.substr(header_end + 4);
                while ((int)body.size() < content_length) {
                    n = recv(client, buf, sizeof(buf), 0);
                    if (n <= 0) break;
                    body.append(buf, buf + n);
                }
                if ((int)body.size() > content_length) body.resize(content_length);
            } else {
                // GET: parse query string as body
                size_t first_line_end = headers.find("\r\n");
                std::string first_line = headers.substr(0, first_line_end);
                size_t qpos = first_line.find(' ');
                size_t qpos2 = first_line.find(' ', qpos + 1);
                std::string path = first_line.substr(qpos + 1, qpos2 - qpos - 1);
                size_t qmark = path.find('?');
                if (qmark != std::string::npos) {
                    body = path.substr(qmark + 1);
                }
            }
        }
        std::string name = extractName(body);
        std::string resp;
        if (!name.empty()) {
            resp = "HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: " + std::to_string(name.size()) + "\r\n\r\n" + name;
        } else {
            std::string msg = "Field 'name' not found";
            resp = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: " + std::to_string(msg.size()) + "\r\n\r\n" + msg;
        }
        send(client, resp.c_str(), resp.size(), 0);
        close(client);
    }
    close(server_fd);
}
#endif

int main() {
#ifndef _WIN32
    std::thread srv(run_server_cpp);
    srv.detach();
    std::cout << "Note: C++ server started in background thread.\n";
#else
    std::cout << "Note: C++ example expects POSIX sockets. Use WSL or adapt for Winsock.\n";
#endif

    // 5 test cases
    std::vector<std::string> tests = {
        "{\"name\":\"Alice\"}",
        "{\"age\":30}",
        "{\"name\":\"Bob\",\"x\":1}",
        "name=Carol&x=1",
        "{\"greet\":\"hi\",\"name\":\"A\\\"B C\"}"
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        std::string res = extractName(tests[i]);
        std::cout << "Test " << (i + 1) << ": " << (res.empty() ? std::string("null") : res) << "\n";
    }
    // Keep main alive for server demonstration
#ifndef _WIN32
    while (true) std::this_thread::sleep_for(std::chrono::seconds(60));
#endif
    return 0;
}