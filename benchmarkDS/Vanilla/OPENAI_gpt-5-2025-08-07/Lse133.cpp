#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

// Function that accepts input and returns output
std::string greet(const std::string& name) {
    return "Hello, " + name + "!";
}

// Minimal HTTP server that listens on the given port; returns 0 on success
int start_server(int port) {
    int server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return -1;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        return -1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(static_cast<uint16_t>(port));

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        close(server_fd);
        return -1;
    }

    std::cout << "C++ HTTP server running on http://127.0.0.1:" << port << "/" << std::endl;

    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) {
            perror("accept");
            break;
        }

        char buffer[4096];
        ssize_t n = read(client_fd, buffer, sizeof(buffer) - 1);
        if (n <= 0) {
            close(client_fd);
            continue;
        }
        buffer[n] = '\0';
        std::string request(buffer);

        // Parse the request line
        std::istringstream iss(request);
        std::string method, path, version;
        iss >> method >> path >> version;

        std::string body;
        std::string status_line;
        if (method == "GET" && path == "/") {
            body = greet("C++ User");
            status_line = "HTTP/1.1 200 OK\r\n";
        } else {
            body = "Not Found";
            status_line = "HTTP/1.1 404 Not Found\r\n";
        }

        std::ostringstream resp;
        resp << status_line
             << "Content-Type: text/plain; charset=UTF-8\r\n"
             << "Content-Length: " << body.size() << "\r\n"
             << "Connection: close\r\n"
             << "\r\n"
             << body;

        std::string resp_str = resp.str();
        (void)write(client_fd, resp_str.c_str(), resp_str.size());
        close(client_fd);
    }

    close(server_fd);
    return 0;
}

int main() {
    // 5 test cases for greet
    std::cout << greet("Alice") << std::endl;
    std::cout << greet("Bob") << std::endl;
    std::cout << greet("Charlie") << std::endl;
    std::cout << greet("Dana") << std::endl;
    std::cout << greet("Eve") << std::endl;

    // Start server on port 8081
    start_server(8081);
    return 0;
}