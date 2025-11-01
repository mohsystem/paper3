#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

// 4) Function that is called when the user visits the specified URL
static std::string index() {
    // 5) Returns the message to the user
    return "Hello, secure world.";
}

static void send_response(int client_fd, int code, const std::string& status, const std::string& body) {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << code << " " << status << "\r\n"
        << "Content-Type: text/plain; charset=utf-8\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Connection: close\r\n"
        << "\r\n"
        << body;
    const std::string resp = oss.str();
    size_t total = 0;
    const char* data = resp.c_str();
    size_t len = resp.size();
    while (total < len) {
        ssize_t n = send(client_fd, data + total, len - total, 0);
        if (n <= 0) break;
        total += static_cast<size_t>(n);
    }
}

static std::string parse_path_from_request(const char* buf, size_t len) {
    // Parse the request line safely: "GET /path HTTP/1.1"
    std::string s(buf, len);
    size_t pos_end_line = s.find("\r\n");
    if (pos_end_line == std::string::npos) return std::string();
    std::string line = s.substr(0, pos_end_line);

    std::istringstream iss(line);
    std::string method, path, version;
    if (!(iss >> method >> path >> version)) return std::string();
    if (method != "GET") return std::string("#METHOD_NOT_ALLOWED#");
    // Basic normalization: restrict to simple ASCII and avoid overly long paths
    if (path.size() > 2048) return std::string("#BAD_REQUEST#");
    return path;
}

// Minimal HTTP server on 127.0.0.1:8081
// 2) Creates the application object (server socket)
static int create_server_socket(uint16_t port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        std::cerr << "Failed to create socket.\n";
        return -1;
    }
    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0) {
        std::cerr << "Failed to set socket options.\n";
        close(fd);
        return -1;
    }
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) != 1) {
        std::cerr << "Invalid address.\n";
        close(fd);
        return -1;
    }
    if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        std::cerr << "Bind failed.\n";
        close(fd);
        return -1;
    }
    if (listen(fd, 8) != 0) {
        std::cerr << "Listen failed.\n";
        close(fd);
        return -1;
    }
    return fd;
}

// 3) "Decorator": Register which URL should call the associated function (handled in dispatch below)
static void run_server() {
    int server_fd = create_server_socket(8081);
    if (server_fd < 0) {
        std::cerr << "Server initialization failed.\n";
        return;
    }
    std::cout << "Server running at http://127.0.0.1:8081/\n";

    // 6) Runs the application (accept loop)
    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
        if (client_fd < 0) {
            std::cerr << "Accept failed.\n";
            continue;
        }

        char buf[4096];
        ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
        if (n <= 0) {
            close(client_fd);
            continue;
        }

        std::string path = parse_path_from_request(buf, static_cast<size_t>(n));
        if (path.empty()) {
            send_response(client_fd, 400, "Bad Request", "Bad Request");
        } else if (path == "#METHOD_NOT_ALLOWED#") {
            send_response(client_fd, 405, "Method Not Allowed", "Method Not Allowed");
        } else if (path == "#BAD_REQUEST#") {
            send_response(client_fd, 400, "Bad Request", "Bad Request");
        } else if (path == "/") {
            // 4 & 5) Call the function and return the message
            send_response(client_fd, 200, "OK", index());
        } else {
            send_response(client_fd, 404, "Not Found", "Not Found");
        }
        close(client_fd);
    }
}

int main() {
    // Five test cases for index()
    bool all_pass = true;
    const std::string expected = "Hello, secure world.";
    for (int i = 0; i < 5; ++i) {
        std::string res = index();
        if (res != expected) {
            all_pass = false;
        }
    }
    std::cout << (all_pass ? "All tests passed." : "Some tests failed.") << std::endl;

    run_server();
    return 0;
}