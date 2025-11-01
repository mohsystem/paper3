
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace std;

class Lse89 {
private:
    map<string, double> prices;
    
public:
    Lse89() {
        prices["A001"] = 10.50;
        prices["A002"] = 15.75;
        prices["A003"] = 20.00;
        prices["A004"] = 25.50;
        prices["A005"] = 30.25;
    }
    
    double getPrice(const string& productCode) {
        if (prices.find(productCode) != prices.end()) {
            return prices[productCode];
        }
        return 0.0;
    }
    
    double calculateTotal(const string& productCode, int quantity) {
        return getPrice(productCode) * quantity;
    }
    
    void startServer() {
        int server_fd, new_socket;
        struct sockaddr_in address;
        int opt = 1;
        int addrlen = sizeof(address);
        
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(5000);
        
        bind(server_fd, (struct sockaddr *)&address, sizeof(address));
        listen(server_fd, 3);
        
        cout << "Server started on port 5000" << endl;
        
        while (true) {
            new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
            char buffer[1024] = {0};
            read(new_socket, buffer, 1024);
            
            string response = "HTTP/1.1 200 OK\\r\\nContent-Type: text/plain\\r\\n\\r\\nWeb App Running";
            send(new_socket, response.c_str(), response.length(), 0);
            close(new_socket);
        }
    }
};

int main() {
    Lse89 app;
    
    // Test cases
    cout << "Test Case 1: " << app.calculateTotal("A001", 2) << endl;
    cout << "Test Case 2: " << app.calculateTotal("A002", 3) << endl;
    cout << "Test Case 3: " << app.calculateTotal("A003", 1) << endl;
    cout << "Test Case 4: " << app.calculateTotal("A004", 5) << endl;
    cout << "Test Case 5: " << app.calculateTotal("A005", 4) << endl;
    
    // Uncomment to start server
    // app.startServer();
    
    return 0;
}
