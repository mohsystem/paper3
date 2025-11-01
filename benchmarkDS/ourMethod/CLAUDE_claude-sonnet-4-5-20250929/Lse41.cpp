
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <regex>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/rand.h>

// Structure to hold user information
struct UserInfo {
    std::string username;
    std::string email;
    std::string ssn_last4;
};

// Global session store - in production use proper session management
std::map<std::string, UserInfo> session_store;

// Sanitize output to prevent XSS attacks
std::string sanitize_output(const std::string& input) {
    std::string output;
    output.reserve(input.length() * 2);
    for (char c : input) {
        switch (c) {
            case '&': output += "&amp;"; break;
            case '<': output += "&lt;"; break;
            case '>': output += "&gt;"; break;
            case '"': output += "&quot;"; break;
            case '\\'': output += "&#x27;"; break;
            case '/': output += "&#x2F;"; break;
            default: output += c; break;
        }
    }
    return output;
}

// Validate SSN last 4 digits
std::string validate_ssn_last4(const std::string& ssn) {
    std::regex pattern("^\\\\d{4}$");
    if (std::regex_match(ssn, pattern)) {
        return ssn;
    }
    return "****";
}

// Generate cryptographically secure session ID
std::string generate_session_id() {
    unsigned char buffer[32];
    if (RAND_bytes(buffer, sizeof(buffer)) != 1) {
        return "";
    }
    
    std::stringstream ss;
    for (size_t i = 0; i < sizeof(buffer); ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(buffer[i]);
    }
    return ss.str();
}

// Extract cookie value from HTTP headers
std::string get_cookie(const std::string& headers, const std::string& name) {
    size_t cookie_pos = headers.find("Cookie:");
    if (cookie_pos == std::string::npos) {
        return "";
    }
    
    size_t line_end = headers.find("\\r\\n", cookie_pos);
    std::string cookie_line = headers.substr(cookie_pos + 8, line_end - cookie_pos - 8);
    
    size_t pos = 0;
    while (pos < cookie_line.length()) {
        size_t eq_pos = cookie_line.find('=', pos);
        if (eq_pos == std::string::npos) break;
        
        size_t semi_pos = cookie_line.find(';', eq_pos);
        if (semi_pos == std::string::npos) semi_pos = cookie_line.length();
        
        std::string key = cookie_line.substr(pos, eq_pos - pos);
        std::string value = cookie_line.substr(eq_pos + 1, semi_pos - eq_pos - 1);
        
        // Trim whitespace
        key.erase(0, key.find_first_not_of(" \\t"));
        key.erase(key.find_last_not_of(" \\t") + 1);
        
        if (key == name) {
            return value;
        }
        
        pos = semi_pos + 1;
    }
    return "";
}

// Handle HTTP request
void handle_request(int client_socket) {
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    
    ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0) {
        close(client_socket);
        return;
    }
    
    std::string request(buffer);
    std::string response;
    
    try {
        // Check if GET /info
        if (request.find("GET /info") != 0) {
            response = "HTTP/1.1 404 Not Found\\r\\n"
                      "Content-Type: text/plain\\r\\n"
                      "Content-Length: 9\\r\\n\\r\\n"
                      "Not Found";
            write(client_socket, response.c_str(), response.length());
            close(client_socket);
            return;
        }
        
        // Extract session ID
        std::string session_id = get_cookie(request, "sessionId");
        
        if (session_id.empty() || session_store.find(session_id) == session_store.end()) {
            response = "HTTP/1.1 401 Unauthorized\\r\\n"
                      "Content-Type: text/plain\\r\\n"
                      "Content-Length: 12\\r\\n\\r\\n"
                      "Unauthorized";
            write(client_socket, response.c_str(), response.length());
            close(client_socket);
            return;
        }
        
        UserInfo& user = session_store[session_id];
        
        // Build HTML response with sanitized data
        std::stringstream html;
        html << "<!DOCTYPE html>\\n<html>\\n<head>\\n"
             << "<meta charset=\\"UTF-8\\">\\n"
             << "<meta http-equiv=\\"Content-Security-Policy\\" content=\\"default-src 'self'\\">\\n"
             << "<title>User Info</title>\\n</head>\\n<body>\\n"
             << "<h1>User Information</h1>\\n"
             << "<p>Username: " << sanitize_output(user.username) << "</p>\\n"
             << "<p>Email: " << sanitize_output(user.email) << "</p>\\n"
             << "<p>SSN (Last 4): " << sanitize_output(user.ssn_last4) << "</p>\\n"
             << "</body>\\n</html>";
        
        std::string body = html.str();
        
        std::stringstream resp;
        resp << "HTTP/1.1 200 OK\\r\\n"
             << "Content-Type: text/html; charset=UTF-8\\r\\n"
             << "X-Content-Type-Options: nosniff\\r\\n"
             << "X-Frame-Options: DENY\\r\\n"
             << "X-XSS-Protection: 1; mode=block\\r\\n"
             << "Content-Length: " << body.length() << "\\r\\n\\r\\n"
             << body;
        
        response = resp.str();
        write(client_socket, response.c_str(), response.length());
        
    } catch (...) {
        response = "HTTP/1.1 500 Internal Server Error\\r\\n"
                  "Content-Type: text/plain\\r\\n"
                  "Content-Length: 21\\r\\n\\r\\n"
                  "Internal Server Error";
        write(client_socket, response.c_str(), response.length());
    }
    
    close(client_socket);
}

int main() {
    // Initialize OpenSSL
    OPENSSL_init_crypto(0, NULL);
    
    // Create test sessions
    std::string session1 = generate_session_id();
    std::string session2 = generate_session_id();
    std::string session3 = generate_session_id();
    std::string session4 = generate_session_id();
    std::string session5 = generate_session_id();
    
    session_store[session1] = {"john_doe", "john@example.com", validate_ssn_last4("1234")};
    session_store[session2] = {"jane_smith", "jane@example.com", validate_ssn_last4("5678")};
    session_store[session3] = {"bob_jones", "bob@example.com", validate_ssn_last4("9012")};
    session_store[session4] = {"alice<script>", "alice@test.com", validate_ssn_last4("3456")};
    session_store[session5] = {"charlie", "charlie@example.com", validate_ssn_last4("7890")};
    
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }
    
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(server_socket);
        return 1;
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8000);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        close(server_socket);
        return 1;
    }
    
    if (listen(server_socket, 10) < 0) {
        std::cerr << "Listen failed" << std::endl;
        close(server_socket);
        return 1;
    }
    
    std::cout << "Server started on port 8000" << std::endl;
    std::cout << "Test sessions created:" << std::endl;
    std::cout << "Session 1: " << session1 << std::endl;
    std::cout << "Session 2: " << session2 << std::endl;
    std::cout << "Session 3: " << session3 << std::endl;
    std::cout << "Session 4: " << session4 << std::endl;
    std::cout << "Session 5: " << session5 << std::endl;
    std::cout << "\\nAccess /info with Cookie: sessionId=<session_id>" << std::endl;
    
    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket >= 0) {
            handle_request(client_socket);
        }
    }
    
    close(server_socket);
    return 0;
}
