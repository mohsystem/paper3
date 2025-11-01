
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>

void http_hello_page(int client_socket, const std::string& request) {
    // Read the request body (username)
    std::string username = "";
    size_t body_pos = request.find("\\r\\n\\r\\n");
    if (body_pos != std::string::npos) {
        username = request.substr(body_pos + 4);
    }
    
    // Construct the response body (HTML page)
    std::ostringstream outputbuf;
    outputbuf << "<!DOCTYPE html>\\n"
              << "<html>\\n"
              << "<head><title>Hello Page</title></head>\\n"
              << "<body>\\n"
              << "<h1>Hello, " << username << "!</h1>\\n"
              << "<p>Welcome to our server.</p>\\n"
              << "</body>\\n"
              << "</html>";
    
    std::string response_body = outputbuf.str();
    
    // Construct HTTP response
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\\r\\n"
             << "Content-Type: text/html\\r\\n"
             << "Content-Length: " << response_body.length() << "\\r\\n"
             << "\\r\\n"
             << response_body;
    
    std::string response_str = response.str();
    
    // Write the response body to the response stream
    send(client_socket, response_str.c_str(), response_str.length(), 0);
    
    // Close the response stream
    close(client_socket);
}

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }
    
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        return 1;
    }
    
    if (listen(server_socket, 10) < 0) {
        std::cerr << "Listen failed" << std::endl;
        return 1;
    }
    
    std::cout << "Server started on port 8080" << std::endl;
    std::cout << "Test the server with:" << std::endl;
    std::cout << "curl -X POST -d 'Alice' http://localhost:8080/hello" << std::endl;
    std::cout << "curl -X POST -d 'Bob' http://localhost:8080/hello" << std::endl;
    std::cout << "curl -X POST -d 'Charlie' http://localhost:8080/hello" << std::endl;
    std::cout << "curl -X POST -d 'David' http://localhost:8080/hello" << std::endl;
    std::cout << "curl -X POST -d 'Eve' http://localhost:8080/hello" << std::endl;
    
    while (true) {
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0) {
            continue;
        }
        
        char buffer[4096] = {0};
        recv(client_socket, buffer, sizeof(buffer), 0);
        std::string request(buffer);
        
        if (request.find("POST /hello") != std::string::npos) {
            http_hello_page(client_socket, request);
        } else {
            close(client_socket);
        }
    }
    
    close(server_socket);
    return 0;
}
