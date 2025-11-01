
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <map>

std::string urlDecode(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == '%' && i + 2 < str.length()) {
            int value;
            std::sscanf(str.substr(i + 1, 2).c_str(), "%x", &value);
            result += static_cast<char>(value);
            i += 2;
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    return result;
}

std::string processRequest(const std::string& payload) {
    if (payload.empty()) {
        return "Error: Missing payload parameter";
    }
    
    if (payload.find("type: Create") != std::string::npos || 
        payload.find("type:Create") != std::string::npos) {
        return "Error: Create type is not allowed";
    }
    
    return "Success: Payload processed";
}

int main() {
    // Test cases
    std::cout << "Test Case 1: " << processRequest("") << std::endl;
    std::cout << "Test Case 2: " << processRequest("type: Create") << std::endl;
    std::cout << "Test Case 3: " << processRequest("type: Update") << std::endl;
    std::cout << "Test Case 4: " << processRequest("name: test\\nvalue: 123") << std::endl;
    std::cout << "Test Case 5: " << processRequest("type:Create") << std::endl;
    
    // Start web server
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(5000);
    
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 3);
    
    std::cout << "Server started on port 5000" << std::endl;
    
    while (true) {
        int client_fd = accept(server_fd, NULL, NULL);
        char buffer[4096] = {0};
        read(client_fd, buffer, 4096);
        
        std::string request(buffer);
        std::string payload;
        
        size_t pos = request.find("payload=");
        if (pos != std::string::npos) {
            size_t end = request.find(" ", pos);
            payload = request.substr(pos + 8, end - pos - 8);
            payload = urlDecode(payload);
        }
        
        std::string response = processRequest(payload);
        std::string http_response = "HTTP/1.1 200 OK\\r\\nContent-Type: text/plain\\r\\n\\r\\n" + response;
        
        send(client_fd, http_response.c_str(), http_response.length(), 0);
        close(client_fd);
    }
    
    close(server_fd);
    return 0;
}
