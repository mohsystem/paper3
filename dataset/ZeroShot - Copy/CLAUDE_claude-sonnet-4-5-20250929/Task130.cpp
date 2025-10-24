
#include <iostream>
#include <string>
#include <map>
#include <set>
#include <sstream>

class Task130 {
private:
    static const std::set<std::string> ALLOWED_ORIGINS;
    static const std::set<std::string> ALLOWED_METHODS;
    static const std::set<std::string> ALLOWED_HEADERS;
    static const int MAX_AGE = 3600;
    
public:
    class APIResponse {
    private:
        int statusCode;
        std::map<std::string, std::string> headers;
        std::string body;
        
    public:
        APIResponse(int code) : statusCode(code), body("") {}
        
        void setHeader(const std::string& key, const std::string& value) {
            headers[key] = value;
        }
        
        void setBody(const std::string& b) {
            body = b;
        }
        
        int getStatusCode() const { return statusCode; }
        void setStatusCode(int code) { statusCode = code; }
        
        std::string toString() const {
            std::stringstream ss;
            ss << "Status Code: " << statusCode << "\\n";
            ss << "Headers:\\n";
            for (const auto& pair : headers) {
                ss << "  " << pair.first << ": " << pair.second << "\\n";
            }
            ss << "Body: " << body;
            return ss.str();
        }
    };
    
    static std::string joinSet(const std::set<std::string>& s, const std::string& delimiter) {
        std::string result;
        bool first = true;
        for (const auto& item : s) {
            if (!first) result += delimiter;
            result += item;
            first = false;
        }
        return result;
    }
    
    static APIResponse handleCORS(const std::string& origin, const std::string& method) {
        APIResponse response(200);
        
        if (!origin.empty() && ALLOWED_ORIGINS.find(origin) != ALLOWED_ORIGINS.end()) {
            response.setHeader("Access-Control-Allow-Origin", origin);
            response.setHeader("Access-Control-Allow-Credentials", "true");
            response.setHeader("Vary", "Origin");
        } else if (!origin.empty()) {
            response.setStatusCode(403);
            response.setBody("{\\"error\\": \\"Origin not allowed\\"}");
            return response;
        }
        
        if (method == "OPTIONS") {
            response.setHeader("Access-Control-Allow-Methods", joinSet(ALLOWED_METHODS, ", "));
            response.setHeader("Access-Control-Allow-Headers", joinSet(ALLOWED_HEADERS, ", "));
            response.setHeader("Access-Control-Max-Age", std::to_string(MAX_AGE));
            response.setStatusCode(204);
        } else {
            if (ALLOWED_METHODS.find(method) == ALLOWED_METHODS.end()) {
                response.setStatusCode(405);
                response.setBody("{\\"error\\": \\"Method not allowed\\"}");
                return response;
            }
            response.setHeader("Access-Control-Expose-Headers", "Content-Length, X-Request-ID");
        }
        
        return response;
    }
    
    static APIResponse handleAPIRequest(const std::string& origin, const std::string& method, const std::string& path) {
        APIResponse corsResponse = handleCORS(origin, method);
        
        if (corsResponse.getStatusCode() != 200 && corsResponse.getStatusCode() != 204) {
            return corsResponse;
        }
        
        if (method == "GET" && path == "/api/data") {
            corsResponse.setBody("{\\"message\\": \\"Data retrieved successfully\\", \\"data\\": [1, 2, 3]}");
        } else if (method == "POST" && path == "/api/data") {
            corsResponse.setBody("{\\"message\\": \\"Data created successfully\\", \\"id\\": 123}");
        } else {
            corsResponse.setStatusCode(404);
            corsResponse.setBody("{\\"error\\": \\"Endpoint not found\\"}");
        }
        
        corsResponse.setHeader("Content-Type", "application/json");
        return corsResponse;
    }
};

const std::set<std::string> Task130::ALLOWED_ORIGINS = {
    "https://example.com",
    "https://app.example.com",
    "http://localhost:3000"
};

const std::set<std::string> Task130::ALLOWED_METHODS = {
    "GET", "POST", "PUT", "DELETE", "OPTIONS"
};

const std::set<std::string> Task130::ALLOWED_HEADERS = {
    "Content-Type", "Authorization", "X-Requested-With"
};

int main() {
    std::cout << "=== CORS API Implementation Test Cases ===\\n\\n";
    
    std::cout << "Test Case 1: Preflight OPTIONS request from allowed origin\\n";
    Task130::APIResponse response1 = Task130::handleAPIRequest("https://example.com", "OPTIONS", "/api/data");
    std::cout << response1.toString() << "\\n\\n";
    std::cout << std::string(60, '=') << "\\n\\n";
    
    std::cout << "Test Case 2: GET request from allowed origin\\n";
    Task130::APIResponse response2 = Task130::handleAPIRequest("https://example.com", "GET", "/api/data");
    std::cout << response2.toString() << "\\n\\n";
    std::cout << std::string(60, '=') << "\\n\\n";
    
    std::cout << "Test Case 3: POST request from allowed origin\\n";
    Task130::APIResponse response3 = Task130::handleAPIRequest("http://localhost:3000", "POST", "/api/data");
    std::cout << response3.toString() << "\\n\\n";
    std::cout << std::string(60, '=') << "\\n\\n";
    
    std::cout << "Test Case 4: Request from disallowed origin\\n";
    Task130::APIResponse response4 = Task130::handleAPIRequest("https://malicious.com", "GET", "/api/data");
    std::cout << response4.toString() << "\\n\\n";
    std::cout << std::string(60, '=') << "\\n\\n";
    
    std::cout << "Test Case 5: Request with disallowed method\\n";
    Task130::APIResponse response5 = Task130::handleAPIRequest("https://example.com", "PATCH", "/api/data");
    std::cout << response5.toString() << "\\n\\n";
    std::cout << std::string(60, '=') << "\\n\\n";
    
    return 0;
}
