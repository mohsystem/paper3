
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <regex>
#include <algorithm>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

class Lse89 {
private:
    std::map<std::string, double> productPrices;
    
    std::string toUpper(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        return str;
    }
    
    bool isValidProductCode(const std::string& code) {
        std::regex pattern("^[A-Za-z0-9]+$");
        return std::regex_match(code, pattern);
    }
    
public:
    Lse89() {
        productPrices["P001"] = 10.99;
        productPrices["P002"] = 25.50;
        productPrices["P003"] = 15.75;
        productPrices["P004"] = 30.00;
        productPrices["P005"] = 5.99;
    }
    
    double getProductPrice(const std::string& productCode) {
        if (productCode.empty() || !isValidProductCode(productCode)) {
            return -1.0;
        }
        
        std::string upperCode = toUpper(productCode);
        auto it = productPrices.find(upperCode);
        if (it != productPrices.end()) {
            return it->second;
        }
        return -1.0;
    }
    
    std::string calculatePrice(const std::string& productCode, const std::string& quantityStr) {
        try {
            if (productCode.empty() || quantityStr.empty()) {
                return "Error: Missing parameters";
            }
            
            // Validate quantity
            int quantity;
            try {
                size_t pos;
                quantity = std::stoi(quantityStr, &pos);
                if (pos != quantityStr.length()) {
                    return "Error: Invalid quantity format";
                }
            } catch (...) {
                return "Error: Invalid quantity format";
            }
            
            if (quantity <= 0 || quantity > 10000) {
                return "Error: Invalid quantity. Must be between 1 and 10000";
            }
            
            double price = getProductPrice(productCode);
            if (price < 0) {
                return "Error: Product not found";
            }
            
            double totalPrice = price * quantity;
            std::ostringstream oss;
            oss.precision(2);
            oss << std::fixed;
            oss << "Product: " << toUpper(productCode) 
                << ", Quantity: " << quantity 
                << ", Total Price: $" << totalPrice;
            return oss.str();
            
        } catch (...) {
            return "Error: Processing request failed";
        }
    }
    
    void startServer(int port) {
        int server_fd, new_socket;
        struct sockaddr_in address;
        int opt = 1;
        int addrlen = sizeof(address);
        
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            std::cerr << "Socket creation failed" << std::endl;
            return;
        }
        
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
            std::cerr << "Setsockopt failed" << std::endl;
            return;
        }
        
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        
        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            std::cerr << "Bind failed" << std::endl;
            return;
        }
        
        if (listen(server_fd, 3) < 0) {
            std::cerr << "Listen failed" << std::endl;
            return;
        }
        
        std::cout << "Server running on port " << port << std::endl;
        
        // Note: This is a simplified server for demonstration
        // In production, use a proper HTTP server library
        close(server_fd);
    }
};

int main() {
    Lse89 app;
    
    // Test cases
    std::cout << "\\nTest Cases:" << std::endl;
    std::cout << "1. " << app.calculatePrice("P001", "5") << std::endl;
    std::cout << "2. " << app.calculatePrice("P003", "10") << std::endl;
    std::cout << "3. " << app.calculatePrice("P999", "5") << std::endl;
    std::cout << "4. " << app.calculatePrice("P002", "0") << std::endl;
    std::cout << "5. " << app.calculatePrice("P001", "abc") << std::endl;
    
    std::cout << "\\nNote: Full web server functionality requires external HTTP library" << std::endl;
    
    return 0;
}
