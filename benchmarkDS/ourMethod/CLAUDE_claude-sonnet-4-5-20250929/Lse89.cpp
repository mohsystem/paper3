
#include <iostream>
#include <string>
#include <map>
#include <regex>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sstream>
#include <iomanip>
#include <limits>
#include <algorithm>

// Whitelist of valid product codes with their prices
static const std::map<std::string, double> PRODUCT_PRICES = {
    {"PROD001", 10.50},
    {"PROD002", 25.00},
    {"PROD003", 15.75},
    {"PROD004", 30.00},
    {"PROD005", 8.99}
};

// Input validation pattern for product codes (alphanumeric, max 20 chars)
static const std::regex PRODUCT_CODE_PATTERN("^[A-Za-z0-9]{1,20}$");

/**
 * Returns the price for a given product code.
 * @param productCode The product code to look up
 * @param price Output parameter for the price
 * @return true if found, false otherwise
 */
bool getPrice(const std::string& productCode, double& price) {
    // Input validation: check for empty
    if (productCode.empty()) {
        return false;
    }
    
    // Input validation: check length to prevent DOS
    if (productCode.length() > 20) {
        return false;
    }
    
    // Input validation: check format against whitelist pattern
    if (!std::regex_match(productCode, PRODUCT_CODE_PATTERN)) {
        return false;
    }
    
    // Look up price in secure map
    auto it = PRODUCT_PRICES.find(productCode);
    if (it != PRODUCT_PRICES.end()) {
        price = it->second;
        return true;
    }
    
    return false;
}

/**
 * Calculates total price for a product code and quantity.
 * @param productCode The product code
 * @param quantity The quantity (must be positive, max 10000)
 * @param totalPrice Output parameter for the total price
 * @return true if valid, false otherwise
 */
bool calculatePrice(const std::string& productCode, int quantity, double& totalPrice) {
    // Input validation: quantity must be positive and within reasonable limits
    if (quantity <= 0 || quantity > 10000) {
        return false;
    }
    
    double price = 0.0;
    if (!getPrice(productCode, price)) {
        return false;
    }
    
    // Check for overflow before multiplication
    if (price > std::numeric_limits<double>::max() / quantity) {
        return false;
    }
    
    totalPrice = price * quantity;
    return true;
}

/**
 * URL decode a string safely with bounds checking
 */
std::string urlDecode(const std::string& str) {
    std::string result;
    result.reserve(str.length());
    
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            // Input validation: check hex digits
            if (std::isxdigit(str[i+1]) && std::isxdigit(str[i+2])) {
                int value = 0;
                std::istringstream iss(str.substr(i+1, 2));
                iss >> std::hex >> value;
                result += static_cast<char>(value);
                i += 2;
            } else {
                result += str[i];
            }
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    
    return result;
}

/**
 * Parse query parameters from a query string
 */
std::map<std::string, std::string> parseQuery(const std::string& query) {
    std::map<std::string, std::string> params;
    
    // Input validation: limit query string length
    if (query.length() > 1000) {
        return params;
    }
    
    std::istringstream stream(query);
    std::string pair;
    int paramCount = 0;
    
    while (std::getline(stream, pair, '&')) {
        // Input validation: limit number of parameters to prevent DOS
        if (++paramCount > 10) {
            break;
        }
        
        size_t pos = pair.find('=');
        if (pos != std::string::npos) {
            std::string key = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);
            
            // Input validation: limit parameter name and value length
            if (key.length() <= 50 && value.length() <= 100) {
                params[key] = urlDecode(value);
            }
        }
    }
    
    return params;
}

/**
 * Handle HTTP request
 */
void handleRequest(int clientSocket) {
    char buffer[4096];
    std::memset(buffer, 0, sizeof(buffer));
    
    // Read request with bounds checking
    ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
    if (bytesRead <= 0) {
        close(clientSocket);
        return;
    }
    
    // Null terminate for safety
    buffer[bytesRead] = '\\0';
    
    std::string request(buffer);
    std::string response;
    int statusCode = 400;
    
    try {
        // Parse request line
        size_t methodEnd = request.find(' ');
        size_t pathEnd = request.find(' ', methodEnd + 1);
        
        if (methodEnd != std::string::npos && pathEnd != std::string::npos) {
            std::string method = request.substr(0, methodEnd);
            std::string path = request.substr(methodEnd + 1, pathEnd - methodEnd - 1);
            
            // Only allow GET requests
            if (method == "GET") {
                size_t queryPos = path.find('?');
                if (queryPos != std::string::npos) {
                    std::string query = path.substr(queryPos + 1);
                    auto params = parseQuery(query);
                    
                    auto codeIt = params.find("code");
                    auto qtyIt = params.find("quantity");
                    
                    if (codeIt != params.end() && qtyIt != params.end()) {
                        try {
                            int quantity = std::stoi(qtyIt->second);
                            double totalPrice = 0.0;
                            
                            if (calculatePrice(codeIt->second, quantity, totalPrice)) {
                                std::ostringstream oss;
                                oss << std::fixed << std::setprecision(2) << totalPrice;
                                response = "Total price: " + oss.str();
                                statusCode = 200;
                            } else {
                                response = "Invalid product code or quantity";
                            }
                        } catch (...) {
                            response = "Invalid quantity format";
                        }
                    } else {
                        response = "Missing required parameters";
                    }
                } else {
                    response = "Missing parameters";
                }
            } else {
                response = "Method not allowed";
                statusCode = 405;
            }
        }
    } catch (...) {
        // Generic error message, no internal details leaked
        response = "Server error";
        statusCode = 500;
    }
    
    // Build HTTP response with security headers
    std::ostringstream httpResponse;
    httpResponse << "HTTP/1.1 " << statusCode << " OK\\r\\n";
    httpResponse << "Content-Type: text/plain; charset=utf-8\\r\\n";
    httpResponse << "X-Content-Type-Options: nosniff\\r\\n";
    httpResponse << "Content-Length: " << response.length() << "\\r\\n";
    httpResponse << "Connection: close\\r\\n\\r\\n";
    httpResponse << response;
    
    std::string responseStr = httpResponse.str();
    write(clientSocket, responseStr.c_str(), responseStr.length());
    close(clientSocket);
}

int main() {
    // Test cases
    double result = 0.0;
    std::cout << "Test Case 1: " << (calculatePrice("PROD001", 2, result) ? std::to_string(result) : "null") << std::endl;
    std::cout << "Test Case 2: " << (calculatePrice("PROD002", 3, result) ? std::to_string(result) : "null") << std::endl;
    std::cout << "Test Case 3: " << (calculatePrice("PROD003", 5, result) ? std::to_string(result) : "null") << std::endl;
    std::cout << "Test Case 4: " << (calculatePrice("INVALID", 1, result) ? std::to_string(result) : "null") << std::endl;
    std::cout << "Test Case 5: " << (calculatePrice("PROD001", -1, result) ? std::to_string(result) : "null") << std::endl;
    
    // Create socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }
    
    // Set socket options to reuse address
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // Bind to port 5000
    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(5000);
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Failed to bind socket" << std::endl;
        close(serverSocket);
        return 1;
    }
    
    // Listen for connections
    if (listen(serverSocket, 10) < 0) {
        std::cerr << "Failed to listen" << std::endl;
        close(serverSocket);
        return 1;
    }
    
    std::cout << "Server started on port 5000" << std::endl;
    std::cout << "Example: http://localhost:5000/?code=PROD001&quantity=2" << std::endl;
    
    // Accept and handle connections
    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
        
        if (clientSocket >= 0) {
            handleRequest(clientSocket);
        }
    }
    
    close(serverSocket);
    return 0;
}
