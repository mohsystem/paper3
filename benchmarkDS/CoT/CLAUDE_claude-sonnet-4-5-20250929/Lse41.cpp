
#include <iostream>
#include <string>
#include <map>
#include <regex>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

class Lse41 {
private:
    struct User {
        std::string username;
        std::string email;
        std::string ssnLast4;
        std::string sessionToken;
        
        User(const std::string& user, const std::string& mail, const std::string& ssn, const std::string& token)
            : username(escapeHtml(user)), email(escapeHtml(mail)), sessionToken(token) {
            // Only store last 4 digits, validate format
            std::regex ssnPattern("^\\\\d{4}$");
            if (std::regex_match(ssn, ssnPattern)) {
                ssnLast4 = ssn;
            } else {
                ssnLast4 = "****";
            }
        }
    };
    
    static std::string escapeHtml(const std::string& input) {
        std::string result;
        for (char c : input) {
            switch (c) {
                case '&': result += "&amp;"; break;
                case '<': result += "&lt;"; break;
                case '>': result += "&gt;"; break;
                case '"': result += "&quot;"; break;
                case '\\'': result += "&#x27;"; break;
                default: result += c;
            }
        }
        return result;
    }
    
    static User* authenticateUser(const std::string& token) {
        // Simulated user database - in production, use secure database
        if (token == "valid_token_123") {
            return new User("john_doe", "john@example.com", "5678", token);
        }
        return nullptr;
    }
    
    static std::string extractHeader(const std::string& request, const std::string& headerName) {
        size_t pos = request.find(headerName);
        if (pos != std::string::npos) {
            size_t start = pos + headerName.length();
            size_t end = request.find("\\r\\n", start);
            return request.substr(start, end - start);
        }
        return "";
    }
    
public:
    static void handleRequest(int clientSocket) {
        char buffer[4096] = {0};
        read(clientSocket, buffer, sizeof(buffer));
        std::string request(buffer);
        
        std::string response;
        std::string authHeader = extractHeader(request, "Authorization: ");
        User* user = authenticateUser(authHeader);
        
        if (user == nullptr) {
            response = "HTTP/1.1 401 Unauthorized\\r\\n"
                      "Content-Type: text/html; charset=utf-8\\r\\n"
                      "X-Content-Type-Options: nosniff\\r\\n"
                      "X-Frame-Options: DENY\\r\\n\\r\\n"
                      "<html><body><h1>Unauthorized</h1></body></html>";
        } else {
            response = "HTTP/1.1 200 OK\\r\\n"
                      "Content-Type: text/html; charset=utf-8\\r\\n"
                      "X-Content-Type-Options: nosniff\\r\\n"
                      "X-Frame-Options: DENY\\r\\n\\r\\n"
                      "<html><head><meta charset='UTF-8'></head><body>"
                      "<h1>User Information</h1>"
                      "<p>Username: " + user->username + "</p>"
                      "<p>Email: " + user->email + "</p>"
                      "<p>SSN Last 4: ****-" + user->ssnLast4 + "</p>"
                      "</body></html>";
            delete user;
        }
        
        write(clientSocket, response.c_str(), response.length());
        close(clientSocket);
    }
    
    static void main() {
        std::cout << "Testing with 5 scenarios:\\n";
        std::cout << "1. Valid token: curl -H 'Authorization: valid_token_123' http://localhost:8080/info\\n";
        std::cout << "2. Invalid token: curl -H 'Authorization: invalid' http://localhost:8080/info\\n";
        std::cout << "3. No token: curl http://localhost:8080/info\\n";
        std::cout << "4. XSS attempt in username (blocked by escapeHtml)\\n";
        std::cout << "5. Valid user data display with masked SSN\\n";
        
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) {
            std::cerr << "Socket creation failed\\n";
            return;
        }
        
        int opt = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(8080);
        
        if (bind(serverSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
            std::cerr << "Bind failed\\n";
            close(serverSocket);
            return;
        }
        
        listen(serverSocket, 3);
        std::cout << "Server started on port 8080. Press Ctrl+C to stop.\\n";
        
        while (true) {
            int clientSocket = accept(serverSocket, nullptr, nullptr);
            if (clientSocket >= 0) {
                handleRequest(clientSocket);
            }
        }
        
        close(serverSocket);
    }
};

int main() {
    Lse41::main();
    return 0;
}
