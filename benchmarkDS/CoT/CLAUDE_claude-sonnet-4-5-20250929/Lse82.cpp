
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <regex>

const int MAX_PAYLOAD_SIZE = 10000;
const int PORT = 8080;

std::string urlDecode(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == '%' && i + 2 < str.length()) {
            int value;
            std::istringstream iss(str.substr(i + 1, 2));
            if (iss >> std::hex >> value) {
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

std::map<std::string, std::string> parseQuery(const std::string& query) {
    std::map<std::string, std::string> params;
    std::istringstream iss(query);
    std::string pair;
    
    while (std::getline(iss, pair, '&')) {
        size_t pos = pair.find('=');
        if (pos != std::string::npos) {
            std::string key = urlDecode(pair.substr(0, pos));
            std::string value = urlDecode(pair.substr(pos + 1));
            params[key] = value;
        }
    }
    return params;
}

std::string extractJsonValue(const std::string& json, const std::string& key) {
    std::regex pattern("\\"" + key + "\\"\\\\s*:\\\\s*\\"([^\\"]*)\\"");
    std::smatch match;
    
    if (std::regex_search(json, match, pattern)) {
        return match[1].str();
    }
    return "";
}

bool isValidJson(const std::string& str) {
    int braceCount = 0;
    bool inString = false;
    
    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == '"' && (i == 0 || str[i-1] != '\\\\')) {\n            inString = !inString;\n        }\n        if (!inString) {\n            if (str[i] == '{') braceCount++;\n            if (str[i] == '}') braceCount--;\n        }\n    }\n    return braceCount == 0;\n}\n\nstd::string handleRequest(const std::string& request) {\n    std::string response;\n    \n    size_t queryPos = request.find("GET /api?");\n    if (queryPos == std::string::npos) {\n        return "HTTP/1.1 400 Bad Request\\r\\nContent-Type: application/json\\r\\n\\r\\n{\\"error\\":\\"Invalid request\\"}";\n    }\n    \n    size_t queryStart = request.find('?') + 1;\n    size_t queryEnd = request.find(' ', queryStart);
    std::string query = request.substr(queryStart, queryEnd - queryStart);
    
    std::map<std::string, std::string> params = parseQuery(query);
    
    if (params.find("payload") == params.end()) {
        return "HTTP/1.1 400 Bad Request\\r\\nContent-Type: application/json\\r\\n\\r\\n{\\"error\\":\\"Missing payload parameter\\"}";
    }
    
    std::string payload = params["payload"];
    
    if (payload.length() > MAX_PAYLOAD_SIZE) {
        return "HTTP/1.1 413 Payload Too Large\\r\\nContent-Type: application/json\\r\\n\\r\\n{\\"error\\":\\"Payload too large\\"}";
    }
    
    if (!isValidJson(payload)) {
        return "HTTP/1.1 400 Bad Request\\r\\nContent-Type: application/json\\r\\n\\r\\n{\\"error\\":\\"Invalid JSON format\\"}";
    }
    
    std::string name = extractJsonValue(payload, "name");
    
    if (name.empty()) {
        return "HTTP/1.1 400 Bad Request\\r\\nContent-Type: application/json\\r\\n\\r\\n{\\"error\\":\\"Name field not found\\"}";
    }
    
    return "HTTP/1.1 200 OK\\r\\nContent-Type: application/json\\r\\n\\r\\n{\\"name\\":\\"" + name + "\\"}";
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    std::cout << "Test Cases:\\n";
    std::cout << "1. Valid: http://localhost:8080/api?payload={\\"name\\":\\"John\\"}\\n";
    std::cout << "2. Valid: http://localhost:8080/api?payload={\\"name\\":\\"Alice\\",\\"age\\":30}\\n";
    std::cout << "3. Missing name: http://localhost:8080/api?payload={\\"age\\":25}\\n";
    std::cout << "4. Invalid JSON: http://localhost:8080/api?payload={invalid}\\n";
    std::cout << "5. Missing payload: http://localhost:8080/api\\n\\n";
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    std::cout << "Server started on port " << PORT << std::endl;
    
    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }
        
        char buffer[30000] = {0};
        read(new_socket, buffer, 30000);
        
        std::string request(buffer);
        std::string response = handleRequest(request);
        
        send(new_socket, response.c_str(), response.length(), 0);
        close(new_socket);
    }
    
    return 0;
}
