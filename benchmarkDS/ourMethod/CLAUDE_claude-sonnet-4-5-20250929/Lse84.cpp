
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <yaml-cpp/yaml.h>

// Maximum payload size to prevent DoS attacks
const size_t MAX_PAYLOAD_SIZE = 10240; // 10KB
const int PORT = 5000;
const int BUFFER_SIZE = 65536;

// URL decode function
std::string urlDecode(const std::string& str) {
    std::string result;
    result.reserve(str.length());
    
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            // Validate hex characters
            if (isxdigit(str[i + 1]) && isxdigit(str[i + 2])) {
                int value = 0;
                std::istringstream is(str.substr(i + 1, 2));
                is >> std::hex >> value;
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

// Parse query parameters safely
std::map<std::string, std::string> parseQuery(const std::string& query) {
    std::map<std::string, std::string> params;
    
    if (query.empty()) {
        return params;
    }
    
    std::istringstream iss(query);
    std::string pair;
    
    while (std::getline(iss, pair, '&')) {
        size_t pos = pair.find('=');
        if (pos != std::string::npos && pos > 0 && pos < pair.length() - 1) {
            std::string key = urlDecode(pair.substr(0, pos));
            std::string value = urlDecode(pair.substr(pos + 1));
            params[key] = value;
        }
    }
    
    return params;
}

// Process YAML payload securely
std::string processPayload(const std::string& payload) {
    try {
        // Validate payload size
        if (payload.length() > MAX_PAYLOAD_SIZE) {
            return "Error: Payload too large";
        }
        
        // Parse YAML - yaml-cpp is safe by default (no code execution)
        YAML::Node data = YAML::Load(payload);
        
        // Validate parsed data is a map
        if (!data.IsMap()) {
            return "Error: Invalid payload format";
        }
        
        // Check if type field exists and equals "Create"
        if (data["type"] && data["type"].IsScalar()) {
            std::string type = data["type"].as<std::string>();
            if (type == "Create") {
                return "Error: Create type not allowed";
            }
        }
        
        return "Success: Payload processed";
        
    } catch (const YAML::Exception& e) {
        // Don't leak internal details in error messages\n        return "Error: Invalid YAML format";\n    } catch (const std::exception& e) {\n        return "Error: Invalid YAML format";\n    }\n}\n\n// Handle HTTP request\nvoid handleRequest(int client_socket) {\n    char* buffer = nullptr;\n    \n    try {\n        // Allocate buffer\n        buffer = new char[BUFFER_SIZE];\n        if (buffer == nullptr) {\n            const char* error_msg = "HTTP/1.1 500 Internal Server Error\\r\\n\\r\\nError: Memory allocation failed";\n            send(client_socket, error_msg, strlen(error_msg), 0);\n            return;\n        }\n        \n        // Initialize buffer\n        memset(buffer, 0, BUFFER_SIZE);\n        \n        // Read request with bounds check\n        ssize_t bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);\n        if (bytes_read <= 0) {\n            delete[] buffer;\n            return;\n        }\n        \n        // Ensure null termination\n        buffer[bytes_read] = '\\0';\n        \n        std::string request(buffer);\n        std::string response;\n        \n        // Parse request line\n        size_t first_line_end = request.find("\\r\\n");\n        if (first_line_end == std::string::npos) {\n            response = "HTTP/1.1 400 Bad Request\\r\\n\\r\\nError: Invalid request";\n        } else {\n            std::string request_line = request.substr(0, first_line_end);\n            std::istringstream iss(request_line);\n            std::string method, path, version;\n            iss >> method >> path >> version;\n            \n            // Only accept POST requests\n            if (method != "POST") {\n                response = "HTTP/1.1 405 Method Not Allowed\\r\\n\\r\\nError: Method not allowed";\n            } else {\n                // Parse query string\n                size_t query_pos = path.find('?');
                std::string query = (query_pos != std::string::npos) ? path.substr(query_pos + 1) : "";
                
                std::map<std::string, std::string> params = parseQuery(query);
                
                // Check if payload parameter exists
                if (params.find("payload") == params.end()) {
                    response = "HTTP/1.1 400 Bad Request\\r\\n\\r\\nError: Missing payload parameter";
                } else {
                    std::string result = processPayload(params["payload"]);
                    response = "HTTP/1.1 200 OK\\r\\n\\r\\n" + result;
                }
            }
        }
        
        // Send response
        send(client_socket, response.c_str(), response.length(), 0);
        
        // Clean up
        delete[] buffer;
        buffer = nullptr;
        
    } catch (const std::exception& e) {
        if (buffer != nullptr) {
            delete[] buffer;
        }
        const char* error_msg = "HTTP/1.1 400 Bad Request\\r\\n\\r\\nError: Invalid request";
        send(client_socket, error_msg, strlen(error_msg), 0);
    }
}

void runTestCases() {
    std::cout << "\\n=== Running Test Cases ===" << std::endl;
    
    // Test 1: Valid payload with type=Create
    std::cout << "\\nTest 1: Valid YAML with type=Create" << std::endl;
    std::cout << processPayload("type: Create\\ndata: test") << std::endl;
    
    // Test 2: Valid payload with different type
    std::cout << "\\nTest 2: Valid YAML with type=Update" << std::endl;
    std::cout << processPayload("type: Update\\ndata: test") << std::endl;
    
    // Test 3: Valid payload without type field
    std::cout << "\\nTest 3: Valid YAML without type field" << std::endl;
    std::cout << processPayload("data: test\\nvalue: 123") << std::endl;
    
    // Test 4: Invalid YAML syntax
    std::cout << "\\nTest 4: Invalid YAML syntax" << std::endl;
    std::cout << processPayload("invalid: [yaml: syntax") << std::endl;
    
    // Test 5: Empty payload
    std::cout << "\\nTest 5: Empty payload" << std::endl;
    std::cout << processPayload("") << std::endl;
}

int main() {
    // Run test cases
    runTestCases();
    
    int server_fd = -1;
    int client_socket = -1;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Socket creation failed" << std::endl;
        return 1;
    }
    
    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        close(server_fd);
        std::cerr << "Setsockopt failed" << std::endl;
        return 1;
    }
    
    // Initialize address structure
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // Bind socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        close(server_fd);
        std::cerr << "Bind failed" << std::endl;
        return 1;
    }
    
    // Listen for connections
    if (listen(server_fd, 3) == -1) {
        close(server_fd);
        std::cerr << "Listen failed" << std::endl;
        return 1;
    }
    
    std::cout << "\\nServer started on port " << PORT << std::endl;
    
    // Accept and handle connections
    while (true) {
        client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (client_socket == -1) {
            continue;
        }
        
        handleRequest(client_socket);
        close(client_socket);
    }
    
    close(server_fd);
    return 0;
}
