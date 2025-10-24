
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <cstring>

// CORS configuration and validation class
class CORSConfig {
private:
    std::unordered_set<std::string> allowedOrigins;
    std::unordered_set<std::string> allowedMethods;
    std::unordered_set<std::string> allowedHeaders;
    std::unordered_set<std::string> exposedHeaders;
    bool allowCredentials;
    int maxAge;
    
    // Validate origin format - must be scheme://host[:port]
    bool isValidOrigin(const std::string& origin) const {
        if (origin.empty() || origin.length() > 2048) { // CWE-20: Input validation
            return false;
        }
        
        // Check for null bytes (CWE-20)
        if (origin.find('\\0') != std::string::npos) {
            return false;
        }
        
        // Must contain ://
        size_t schemeEnd = origin.find("://");
        if (schemeEnd == std::string::npos || schemeEnd == 0) {
            return false;
        }
        
        std::string scheme = origin.substr(0, schemeEnd);
        // Only allow http or https schemes (CWE-20)
        if (scheme != "http" && scheme != "https") {
            return false;
        }
        
        // Basic validation: no spaces or control characters (CWE-20)
        for (char c : origin) {
            if (std::iscntrl(static_cast<unsigned char>(c)) || std::isspace(static_cast<unsigned char>(c))) {
                return false;
            }
        }
        
        return true;
    }
    
    // Validate HTTP method
    bool isValidMethod(const std::string& method) const {
        if (method.empty() || method.length() > 16) { // CWE-20: Input validation
            return false;
        }
        
        // Check for null bytes and control characters (CWE-20)
        for (char c : method) {
            if (std::iscntrl(static_cast<unsigned char>(c)) || std::isspace(static_cast<unsigned char>(c))) {
                return false;
            }
        }
        
        return true;
    }
    
    // Validate header name
    bool isValidHeaderName(const std::string& header) const {
        if (header.empty() || header.length() > 256) { // CWE-20: Input validation
            return false;
        }
        
        // Check for null bytes and control characters (CWE-20)
        for (char c : header) {
            if (std::iscntrl(static_cast<unsigned char>(c))) {
                return false;
            }
        }
        
        return true;
    }

public:
    CORSConfig() : allowCredentials(false), maxAge(86400) {
        // Default safe methods
        allowedMethods.insert("GET");
        allowedMethods.insert("POST");
        allowedMethods.insert("HEAD");
    }
    
    bool addAllowedOrigin(const std::string& origin) {
        if (!isValidOrigin(origin)) { // CWE-20: Validate all inputs
            return false;
        }
        allowedOrigins.insert(origin);
        return true;
    }
    
    bool addAllowedMethod(const std::string& method) {
        if (!isValidMethod(method)) { // CWE-20: Validate all inputs
            return false;
        }
        allowedMethods.insert(method);
        return true;
    }
    
    bool addAllowedHeader(const std::string& header) {
        if (!isValidHeaderName(header)) { // CWE-20: Validate all inputs
            return false;
        }
        allowedHeaders.insert(header);
        return true;
    }
    
    bool addExposedHeader(const std::string& header) {
        if (!isValidHeaderName(header)) { // CWE-20: Validate all inputs
            return false;
        }
        exposedHeaders.insert(header);
        return true;
    }
    
    void setAllowCredentials(bool allow) {
        allowCredentials = allow;
    }
    
    void setMaxAge(int age) {
        if (age >= 0 && age <= 86400) { // CWE-20: Validate max age (max 24 hours)
            maxAge = age;
        }
    }
    
    // Check if origin is allowed
    bool isOriginAllowed(const std::string& origin) const {
        if (!isValidOrigin(origin)) { // CWE-20: Validate before processing
            return false;
        }
        
        // Check wildcard (only if credentials not allowed - security requirement)
        if (allowedOrigins.find("*") != allowedOrigins.end() && !allowCredentials) {
            return true;
        }
        
        return allowedOrigins.find(origin) != allowedOrigins.end();
    }
    
    // Generate CORS headers for a request
    std::unordered_map<std::string, std::string> generateCORSHeaders(
        const std::string& origin,
        const std::string& requestMethod,
        bool isPreflight) const {
        
        std::unordered_map<std::string, std::string> headers;
        
        // Validate inputs (CWE-20)
        if (!isValidOrigin(origin) || !isValidMethod(requestMethod)) {
            return headers; // Return empty headers for invalid input
        }
        
        // Check if origin is allowed
        if (!isOriginAllowed(origin)) {
            return headers; // No CORS headers if origin not allowed
        }
        
        // Set Access-Control-Allow-Origin (never use * with credentials - security rule)
        if (allowCredentials) {
            headers["Access-Control-Allow-Origin"] = origin; // Must be specific origin
        } else if (allowedOrigins.find("*") != allowedOrigins.end()) {
            headers["Access-Control-Allow-Origin"] = "*";
        } else {
            headers["Access-Control-Allow-Origin"] = origin;
        }
        
        // Set Access-Control-Allow-Credentials if enabled
        if (allowCredentials) {
            headers["Access-Control-Allow-Credentials"] = "true";
        }
        
        // For preflight requests, add additional headers
        if (isPreflight) {
            // Access-Control-Allow-Methods
            if (!allowedMethods.empty()) {
                std::string methods;
                for (const auto& method : allowedMethods) {
                    if (!methods.empty()) methods += ", ";
                    methods += method;
                }
                headers["Access-Control-Allow-Methods"] = methods;
            }
            
            // Access-Control-Allow-Headers
            if (!allowedHeaders.empty()) {
                std::string headersStr;
                for (const auto& header : allowedHeaders) {
                    if (!headersStr.empty()) headersStr += ", ";
                    headersStr += header;
                }
                headers["Access-Control-Allow-Headers"] = headersStr;
            }
            
            // Access-Control-Max-Age
            headers["Access-Control-Max-Age"] = std::to_string(maxAge);
        } else {
            // For actual requests, add exposed headers
            if (!exposedHeaders.empty()) {
                std::string headersStr;
                for (const auto& header : exposedHeaders) {
                    if (!headersStr.empty()) headersStr += ", ";
                    headersStr += header;
                }
                headers["Access-Control-Expose-Headers"] = headersStr;
            }
        }
        
        return headers;
    }
};

// Simulate an HTTP request
struct HTTPRequest {
    std::string method;
    std::string origin;
    std::string path;
    std::unordered_map<std::string, std::string> headers;
};

// Simulate an HTTP response
struct HTTPResponse {
    int statusCode;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

// API Handler with CORS support
class CORSEnabledAPI {
private:
    CORSConfig corsConfig;
    
    // Validate HTTP method (CWE-20)
    bool isValidHTTPMethod(const std::string& method) const {
        if (method.empty() || method.length() > 16) {
            return false;
        }
        
        static const std::unordered_set<std::string> validMethods = {
            "GET", "POST", "PUT", "DELETE", "PATCH", "HEAD", "OPTIONS"
        };
        
        return validMethods.find(method) != validMethods.end();
    }

public:
    CORSEnabledAPI(const CORSConfig& config) : corsConfig(config) {}
    
    HTTPResponse handleRequest(const HTTPRequest& request) {
        HTTPResponse response;
        
        // Validate request method (CWE-20)
        if (!isValidHTTPMethod(request.method)) {
            response.statusCode = 400;
            response.body = "Invalid HTTP method";
            return response;
        }
        
        // Check if this is a preflight request
        bool isPreflight = (request.method == "OPTIONS" && 
                           !request.origin.empty());
        
        if (isPreflight) {
            // Handle preflight request
            response.statusCode = 204; // No Content
            
            // Get CORS headers
            auto corsHeaders = corsConfig.generateCORSHeaders(
                request.origin,
                request.method,
                true
            );
            
            // Add CORS headers to response
            for (const auto& pair : corsHeaders) {
                response.headers[pair.first] = pair.second;
            }
            
            return response;
        }
        
        // Handle actual request
        if (request.method == "GET" && request.path == "/api/data") {
            response.statusCode = 200;
            response.body = "{\\"message\\":\\"Success\\",\\"data\\":[1,2,3]}";
            response.headers["Content-Type"] = "application/json";
        } else if (request.method == "POST" && request.path == "/api/data") {
            response.statusCode = 201;
            response.body = "{\\"message\\":\\"Created\\",\\"id\\":123}";
            response.headers["Content-Type"] = "application/json";
        } else {
            response.statusCode = 404;
            response.body = "{\\"error\\":\\"Not found\\"}";
            response.headers["Content-Type"] = "application/json";
        }
        
        // Add CORS headers if origin is present
        if (!request.origin.empty()) {
            auto corsHeaders = corsConfig.generateCORSHeaders(
                request.origin,
                request.method,
                false
            );
            
            // Add CORS headers to response
            for (const auto& pair : corsHeaders) {
                response.headers[pair.first] = pair.second;
            }
        }
        
        return response;
    }
};

int main() {
    std::cout << "CORS-Enabled API Test Cases\\n" << std::endl;
    
    // Test Case 1: Basic GET request from allowed origin
    {
        std::cout << "Test 1: GET request from allowed origin" << std::endl;
        CORSConfig config;
        config.addAllowedOrigin("https://example.com");
        config.addAllowedMethod("GET");
        config.addExposedHeader("X-Custom-Header");
        
        CORSEnabledAPI api(config);
        
        HTTPRequest req;
        req.method = "GET";
        req.origin = "https://example.com";
        req.path = "/api/data";
        
        HTTPResponse resp = api.handleRequest(req);
        
        std::cout << "Status: " << resp.statusCode << std::endl;
        std::cout << "CORS Headers:" << std::endl;
        for (const auto& pair : resp.headers) {
            std::cout << "  " << pair.first << ": " << pair.second << std::endl;
        }
        std::cout << std::endl;
    }
    
    // Test Case 2: Preflight OPTIONS request
    {
        std::cout << "Test 2: Preflight OPTIONS request" << std::endl;
        CORSConfig config;
        config.addAllowedOrigin("https://app.example.com");
        config.addAllowedMethod("POST");
        config.addAllowedMethod("PUT");
        config.addAllowedHeader("Content-Type");
        config.addAllowedHeader("Authorization");
        config.setMaxAge(3600);
        
        CORSEnabledAPI api(config);
        
        HTTPRequest req;
        req.method = "OPTIONS";
        req.origin = "https://app.example.com";
        req.path = "/api/data";
        
        HTTPResponse resp = api.handleRequest(req);
        
        std::cout << "Status: " << resp.statusCode << std::endl;
        std::cout << "CORS Headers:" << std::endl;
        for (const auto& pair : resp.headers) {
            std::cout << "  " << pair.first << ": " << pair.second << std::endl;
        }
        std::cout << std::endl;
    }
    
    // Test Case 3: Request from disallowed origin
    {
        std::cout << "Test 3: Request from disallowed origin" << std::endl;
        CORSConfig config;
        config.addAllowedOrigin("https://trusted.com");
        
        CORSEnabledAPI api(config);
        
        HTTPRequest req;
        req.method = "GET";
        req.origin = "https://untrusted.com";
        req.path = "/api/data";
        
        HTTPResponse resp = api.handleRequest(req);
        
        std::cout << "Status: " << resp.statusCode << std::endl;
        std::cout << "CORS Headers (should be empty):" << std::endl;
        for (const auto& pair : resp.headers) {
            if (pair.first.find("Access-Control") != std::string::npos) {
                std::cout << "  " << pair.first << ": " << pair.second << std::endl;
            }
        }
        std::cout << std::endl;
    }
    
    // Test Case 4: Wildcard origin without credentials
    {
        std::cout << "Test 4: Wildcard origin configuration" << std::endl;
        CORSConfig config;
        config.addAllowedOrigin("*");
        config.addAllowedMethod("GET");
        
        CORSEnabledAPI api(config);
        
        HTTPRequest req;
        req.method = "GET";
        req.origin = "https://any-origin.com";
        req.path = "/api/data";
        
        HTTPResponse resp = api.handleRequest(req);
        
        std::cout << "Status: " << resp.statusCode << std::endl;
        std::cout << "CORS Headers:" << std::endl;
        for (const auto& pair : resp.headers) {
            std::cout << "  " << pair.first << ": " << pair.second << std::endl;
        }
        std::cout << std::endl;
    }
    
    // Test Case 5: POST request with credentials enabled
    {
        std::cout << "Test 5: POST with credentials" << std::endl;
        CORSConfig config;
        config.addAllowedOrigin("https://secure.example.com");
        config.addAllowedMethod("POST");
        config.addAllowedHeader("Content-Type");
        config.setAllowCredentials(true);
        
        CORSEnabledAPI api(config);
        
        HTTPRequest req;
        req.method = "POST";
        req.origin = "https://secure.example.com";
        req.path = "/api/data";
        
        HTTPResponse resp = api.handleRequest(req);
        
        std::cout << "Status: " << resp.statusCode << std::endl;
        std::cout << "CORS Headers:" << std::endl;
        for (const auto& pair : resp.headers) {
            std::cout << "  " << pair.first << ": " << pair.second << std::endl;
        }
        std::cout << std::endl;
    }
    
    return 0;
}
