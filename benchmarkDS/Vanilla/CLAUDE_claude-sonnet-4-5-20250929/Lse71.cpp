
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sstream>

#define PORT 8000
#define BUFFER_SIZE 8192
#define UPLOAD_DIR "uploads"

class Lse71 {
private:
    int server_fd;
    
    void createUploadDir() {
        mkdir(UPLOAD_DIR, 0777);
    }
    
    std::string getHomePage() {
        return "<html><body>"
               "<h2>File Upload Application</h2>"
               "<form action='/upload' method='post' enctype='multipart/form-data'>"
               "<input type='file' name='file' required><br><br>"
               "<input type='submit' value='Upload File'>"
               "</form></body></html>";
    }
    
    std::string getSuccessPage(const std::string& filename) {
        return "<html><body><h2>File uploaded successfully!</h2>"
               "<p>Filename: " + filename + "</p>"
               "<a href='/'>Upload another file</a></body></html>";
    }
    
    void sendResponse(int client_socket, const std::string& response) {
        std::ostringstream oss;
        oss << "HTTP/1.1 200 OK\\r\\n"
            << "Content-Type: text/html\\r\\n"
            << "Content-Length: " << response.length() << "\\r\\n"
            << "\\r\\n"
            << response;
        
        std::string http_response = oss.str();
        send(client_socket, http_response.c_str(), http_response.length(), 0);
    }
    
    void handleRequest(int client_socket) {
        char buffer[BUFFER_SIZE] = {0};
        int bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_read > 0) {
            std::string request(buffer);
            
            if (request.find("GET / ") == 0) {
                sendResponse(client_socket, getHomePage());
            }
            else if (request.find("POST /upload") == 0) {
                // Simplified file upload handling
                std::string filename = "uploaded_file.txt";
                std::string filepath = std::string(UPLOAD_DIR) + "/" + filename;
                
                // Find the file content in the request
                size_t content_start = request.find("\\r\\n\\r\\n");
                if (content_start != std::string::npos) {
                    content_start += 4;
                    std::string content = request.substr(content_start);
                    
                    std::ofstream file(filepath, std::ios::binary);
                    if (file.is_open()) {
                        file.write(content.c_str(), content.length());
                        file.close();
                    }
                }
                
                sendResponse(client_socket, getSuccessPage(filename));
            }
        }
    }
    
public:
    void start() {
        createUploadDir();
        
        struct sockaddr_in address;
        int addrlen = sizeof(address);
        
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == 0) {
            std::cerr << "Socket creation failed" << std::endl;
            return;
        }
        
        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(PORT);
        
        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            std::cerr << "Bind failed" << std::endl;
            return;
        }
        
        if (listen(server_fd, 3) < 0) {
            std::cerr << "Listen failed" << std::endl;
            return;
        }
        
        std::cout << "Server started on port " << PORT << std::endl;
        std::cout << "Open http://localhost:" << PORT << " in your browser" << std::endl;
        
        while (true) {
            int client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
            if (client_socket >= 0) {
                handleRequest(client_socket);
                close(client_socket);
            }
        }
    }
};

int main() {
    Lse71 server;
    server.start();
    return 0;
}
