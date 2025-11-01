
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

std::map<std::string, double> productPrices = {
    {"PROD001", 10.99},
    {"PROD002", 25.50},
    {"PROD003", 5.75},
    {"PROD004", 100.00},
    {"PROD005", 15.25}
};

double getPrice(const std::string& productCode) {
    auto it = productPrices.find(productCode);
    if (it != productPrices.end()) {
        return it->second;
    }
    return 0.0;
}

double calculatePrice(const std::string& productCode, int quantity) {
    if (quantity < 0) {
        return 0.0;
    }
    double price = getPrice(productCode);
    return price * quantity;
}

std::string parseQueryParam(const std::string& query, const std::string& param) {
    size_t pos = query.find(param + "=");
    if (pos != std::string::npos) {
        size_t start = pos + param.length() + 1;
        size_t end = query.find("&", start);
        if (end == std::string::npos) {
            return query.substr(start);
        }
        return query.substr(start, end - start);
    }
    return "";
}

void handleRequest(int clientSocket, const std::string& request) {
    std::string response;
    
    if (request.find("GET /calculate") != std::string::npos) {
        size_t queryStart = request.find("?");
        if (queryStart != std::string::npos) {
            size_t queryEnd = request.find(" ", queryStart);
            std::string query = request.substr(queryStart + 1, queryEnd - queryStart - 1);
            
            std::string productCode = parseQueryParam(query, "productCode");
            std::string quantityStr = parseQueryParam(query, "quantity");
            
            if (!productCode.empty() && !quantityStr.empty()) {
                int quantity = std::stoi(quantityStr);
                double totalPrice = calculatePrice(productCode, quantity);
                
                std::ostringstream json;
                json << "{\\"productCode\\": \\"" << productCode << "\\", "
                     << "\\"quantity\\": " << quantity << ", "
                     << "\\"totalPrice\\": " << totalPrice << "}";
                
                response = "HTTP/1.1 200 OK\\r\\nContent-Type: application/json\\r\\n\\r\\n" + json.str();
            } else {
                response = "HTTP/1.1 400 Bad Request\\r\\nContent-Type: application/json\\r\\n\\r\\n{\\"error\\": \\"Missing parameters\\"}";
            }
        } else {
            response = "HTTP/1.1 400 Bad Request\\r\\nContent-Type: application/json\\r\\n\\r\\n{\\"error\\": \\"No query parameters\\"}";
        }
    } else {
        response = "HTTP/1.1 404 Not Found\\r\\n\\r\\n";
    }
    
    send(clientSocket, response.c_str(), response.length(), 0);
}

int main() {
    std::cout << "\\nTest Cases:" << std::endl;
    std::cout << "1. Product PROD001, Quantity 5: " << calculatePrice("PROD001", 5) << std::endl;
    std::cout << "2. Product PROD002, Quantity 3: " << calculatePrice("PROD002", 3) << std::endl;
    std::cout << "3. Product PROD003, Quantity 10: " << calculatePrice("PROD003", 10) << std::endl;
    std::cout << "4. Product PROD999, Quantity 2: " << calculatePrice("PROD999", 2) << std::endl;
    std::cout << "5. Product PROD004, Quantity 0: " << calculatePrice("PROD004", 0) << std::endl;
    
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }
    
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(5000);
    
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        close(serverSocket);
        return 1;
    }
    
    listen(serverSocket, 10);
    std::cout << "\\nServer running on port 5000" << std::endl;
    
    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        
        if (clientSocket >= 0) {
            char buffer[4096] = {0};
            read(clientSocket, buffer, sizeof(buffer));
            handleRequest(clientSocket, std::string(buffer));
            close(clientSocket);
        }
    }
    
    close(serverSocket);
    return 0;
}
