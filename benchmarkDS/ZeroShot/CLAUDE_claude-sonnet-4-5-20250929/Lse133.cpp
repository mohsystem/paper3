
// C++ doesn't have direct WSGI equivalent, using simple HTTP server approach
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

class Lse133 {
private:
    int server_fd;
    int port;
    
public:
    Lse133(int p = 8000) : port(p), server_fd(-1) {}
    
    static std::string getMessage() {
        return "Hello, World!";
    }
    
    void setup() {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            std::cerr << "Socket creation failed" << std::endl;
            return;
        }
        
        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);
        
        bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    }
    
    void run() {
        listen(server_fd, 3);
        std::cout << "Server started on port " << port << std::endl;
    }
    
    ~Lse133() {
        if (server_fd >= 0) {
            close(server_fd);
        }
    }
};

int main() {
    // Test cases
    std::cout << "Test Case 1: " << Lse133::getMessage() << std::endl;
    std::cout << "Test Case 2: " << Lse133::getMessage() << std::endl;
    std::cout << "Test Case 3: " << Lse133::getMessage() << std::endl;
    std::cout << "Test Case 4: " << Lse133::getMessage() << std::endl;
    std::cout << "Test Case 5: " << Lse133::getMessage() << std::endl;
    
    // Uncomment to run the server
    // Lse133 app(8000);
    // app.setup();
    // app.run();
    
    return 0;
}
