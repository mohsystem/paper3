
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <algorithm>

class Task38 {
public:
    static std::string sanitizeInput(const std::string& input) {
        std::string result;
        for (char c : input) {
            switch (c) {
                case '&': result += "&amp;"; break;
                case '<': result += "&lt;"; break;
                case '>': result += "&gt;"; break;
                case '"': result += "&quot;"; break;
                case '\\'': result += "&#x27;"; break;
                case '/': result += "&#x2F;"; break;
                default: result += c;
            }
        }
        return result;
    }
    
    static std::string urlDecode(const std::string& str) {
        std::string result;
        for (size_t i = 0; i < str.length(); i++) {
            if (str[i] == '%' && i + 2 < str.length()) {
                int value;
                std::istringstream is(str.substr(i + 1, 2));
                if (is >> std::hex >> value) {
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
    
    static std::map<std::string, std::string> parseFormData(const std::string& data) {
        std::map<std::string, std::string> params;
        std::istringstream stream(data);
        std::string pair;
        
        while (std::getline(stream, pair, '&')) {
            size_t pos = pair.find('=');
            if (pos != std::string::npos) {
                std::string key = urlDecode(pair.substr(0, pos));
                std::string value = urlDecode(pair.substr(pos + 1));
                params[key] = value;
            }
        }
        return params;
    }
    
    static std::string getFormHTML() {
        return "HTTP/1.1 200 OK\\r\\nContent-Type: text/html; charset=utf-8\\r\\n\\r\\n"
               "<!DOCTYPE html><html><head><title>User Input Form</title>"
               "<style>body{font-family:Arial,sans-serif;max-width:600px;margin:50px auto;padding:20px;}"
               "input,textarea{width:100%;padding:10px;margin:10px 0;box-sizing:border-box;}"
               "button{background:#4CAF50;color:white;padding:10px 20px;border:none;cursor:pointer;}"
               "button:hover{background:#45a049;}</style></head><body>"
               "<h1>User Input Form</h1>"
               "<form method='POST' action='/submit'>"
               "<label>Name:</label><input type='text' name='name' required><br>"
               "<label>Message:</label><textarea name='message' rows='4' required></textarea><br>"
               "<button type='submit'>Submit</button></form></body></html>";
    }
    
    static std::string getResponseHTML(const std::string& name, const std::string& message) {
        std::string safeName = sanitizeInput(name);
        std::string safeMessage = sanitizeInput(message);
        return "HTTP/1.1 200 OK\\r\\nContent-Type: text/html; charset=utf-8\\r\\n\\r\\n"
               "<!DOCTYPE html><html><head><title>Your Response</title>"
               "<style>body{font-family:Arial,sans-serif;max-width:600px;margin:50px auto;padding:20px;}"
               ".result{background:#f0f0f0;padding:20px;border-radius:5px;margin:20px 0;}"
               "a{color:#4CAF50;text-decoration:none;}</style></head><body>"
               "<h1>Your Submission</h1><div class='result'>"
               "<p><strong>Name:</strong> " + safeName + "</p>"
               "<p><strong>Message:</strong> " + safeMessage + "</p></div>"
               "<a href='/'>← Back to Form</a></body></html>";
    }
    
    static void handleRequest(int client_socket) {
        char buffer[4096] = {0};
        read(client_socket, buffer, 4096);
        
        std::string request(buffer);
        std::string response;
        
        if (request.find("GET / ") == 0 || request.find("GET /index") == 0) {
            response = getFormHTML();
        } else if (request.find("POST /submit") == 0) {
            size_t bodyPos = request.find("\\r\\n\\r\\n");
            if (bodyPos != std::string::npos) {
                std::string body = request.substr(bodyPos + 4);
                auto params = parseFormData(body);
                response = getResponseHTML(params["name"], params["message"]);
            }
        } else {
            response = "HTTP/1.1 404 Not Found\\r\\n\\r\\n<html><body><h1>404 Not Found</h1></body></html>";
        }
        
        send(client_socket, response.c_str(), response.length(), 0);
        close(client_socket);
    }
};

int main() {
    std::cout << "Testing sanitizeInput function with 5 test cases:\\n\\n";
    
    std::string testCases[] = {
        "Hello World",
        "<script>alert('XSS')</script>",
        "Name with <b>HTML</b> tags",
        "Special chars: & < > \\" '",
        "Normal text without special characters"
    };
    
    for (int i = 0; i < 5; i++) {
        std::cout << "Test Case " << (i + 1) << ":\\n";
        std::cout << "Input: " << testCases[i] << "\\n";
        std::cout << "Sanitized: " << Task38::sanitizeInput(testCases[i]) << "\\n\\n";
    }
    
    std::cout << "\\nStarting web server on http://localhost:8000\\n";
    std::cout << "Press Ctrl+C to stop the server\\n\\n";
    
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        std::cerr << "Socket creation failed\\n";
        return -1;
    }
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8000);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed\\n";
        return -1;
    }
    
    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed\\n";
        return -1;
    }
    
    while (true) {
        int client_socket = accept(server_fd, NULL, NULL);
        if (client_socket < 0) continue;
        Task38::handleRequest(client_socket);
    }
    
    close(server_fd);
    return 0;
}
