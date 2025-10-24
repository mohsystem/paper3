
#include <iostream>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>

class Task130 {
private:
    static const std::unordered_set<std::string> ALLOWED_ORIGINS;
    static const std::unordered_set<std::string> ALLOWED_METHODS;
    static const std::unordered_set<std::string> ALLOWED_HEADERS;
    static const int MAX_AGE = 3600;

    static std::string sanitizeHeader(const std::string& header) {
        if (header.empty()) {
            return "";
        }
        
        std::string sanitized;
        for (char c : header) {
            // Remove control characters
            if (c >= 32 && c != 127) {
                sanitized += c;
            }
        }
        
        // Trim whitespace
        size_t start = sanitized.find_first_not_of(" \\t");
        size_t end = sanitized.find_last_not_of(" \\t");
        
        if (start == std::string::npos) {
            return "";
        }
        
        sanitized = sanitized.substr(start, end - start + 1);
        
        // Check for CRLF injection
        if (sanitized.find('\\r') != std::string::npos || 
            sanitized.find('\\n') != std::string::npos) {
            return "";
        }
        
        return sanitized;
    }

    static std::string toLower(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(),
                      [](unsigned char c){ return std::tolower(c); });
        return result;
    }

    static std::string toUpper(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(),
                      [](unsigned char c){ return std::toupper(c); });
        return result;
    }

    static std::string trim(const std::string& str) {
        size_t start = str.find_first_not_of(" \\t");
        size_t end = str.find_last_not_of(" \\t");
        
        if (start == std::string::npos) {
            return "";
        }
        
        return str.substr(start, end - start + 1);
    }

    static bool validateRequestedHeaders(const std::string& requestedHeaders) {
        std::stringstream ss(requestedHeaders);
        std::string header;
        
        while (std::getline(ss, header, ',')) {
            std::string trimmedHeader = toLower(trim(header));
            if (!trimmedHeader.empty()) {
                bool found = false;
                for (const auto& allowed : ALLOWED_HEADERS) {
                    if (toLower(allowed) == trimmedHeader) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    return false;
                }
            }
        }
        return true;
    }

public:
    struct CORSResponse {
        bool allowed;
        std::unordered_map<std::string, std::string> headers;
        std::string message;
        
        CORSResponse(bool a, const std::unordered_map<std::string, std::string>& h, 
                    const std::string& m) 
            : allowed(a), headers(h), message(m) {}
    };

    static CORSResponse handleCORS(const std::string& origin, 
                                   const std::string& method,
                                   const std::string& requestHeaders = "") {
        std::unordered_map<std::string, std::string> responseHeaders;
        
        // Validate and sanitize origin
        std::string sanitizedOrigin = sanitizeHeader(origin);
        if (sanitizedOrigin.empty()) {
            return CORSResponse(false, responseHeaders, "Invalid origin");
        }
        
        // Check if origin is allowed
        if (ALLOWED_ORIGINS.find(sanitizedOrigin) == ALLOWED_ORIGINS.end()) {
            return CORSResponse(false, responseHeaders, "Origin not allowed");
        }
        
        // Validate method
        std::string sanitizedMethod = sanitizeHeader(method);
        if (sanitizedMethod.empty() || 
            ALLOWED_METHODS.find(toUpper(sanitizedMethod)) == ALLOWED_METHODS.end()) {
            return CORSResponse(false, responseHeaders, "Method not allowed");
        }
        
        // Set CORS headers
        responseHeaders["Access-Control-Allow-Origin"] = sanitizedOrigin;
        
        std::string methods;
        for (const auto& m : ALLOWED_METHODS) {
            if (!methods.empty()) methods += ", ";
            methods += m;
        }
        responseHeaders["Access-Control-Allow-Methods"] = methods;
        
        std::string headers;
        for (const auto& h : ALLOWED_HEADERS) {
            if (!headers.empty()) headers += ", ";
            headers += h;
        }
        responseHeaders["Access-Control-Allow-Headers"] = headers;
        responseHeaders["Access-Control-Max-Age"] = std::to_string(MAX_AGE);
        responseHeaders["Access-Control-Allow-Credentials"] = "true";
        
        // Security headers
        responseHeaders["X-Content-Type-Options"] = "nosniff";
        responseHeaders["X-Frame-Options"] = "DENY";
        responseHeaders["Strict-Transport-Security"] = "max-age=31536000; includeSubDomains";
        
        // Handle preflight request
        if (toUpper(sanitizedMethod) == "OPTIONS") {
            if (!requestHeaders.empty()) {
                std::string sanitizedReqHeaders = sanitizeHeader(requestHeaders);
                if (!sanitizedReqHeaders.empty() && 
                    !validateRequestedHeaders(sanitizedReqHeaders)) {
                    return CORSResponse(false, responseHeaders, "Requested headers not allowed");
                }
            }
            return CORSResponse(true, responseHeaders, "Preflight request approved");
        }
        
        return CORSResponse(true, responseHeaders, "CORS request approved");
    }

    static void printResponse(const CORSResponse& response) {
        std::cout << "Allowed: " << (response.allowed ? "true" : "false") << std::endl;
        std::cout << "Message: " << response.message << std::endl;
        std::cout << "Headers:" << std::endl;
        for (const auto& header : response.headers) {
            std::cout << "  " << header.first << ": " << header.second << std::endl;
        }
        std::cout << std::endl;
    }
};

const std::unordered_set<std::string> Task130::ALLOWED_ORIGINS = {
    "https://example.com",
    "https://app.example.com",
    "http://localhost:3000"
};

const std::unordered_set<std::string> Task130::ALLOWED_METHODS = {
    "GET", "POST", "PUT", "DELETE", "OPTIONS"
};

const std::unordered_set<std::string> Task130::ALLOWED_HEADERS = {
    "Content-Type", "Authorization", "X-Requested-With"
};

int main() {
    std::cout << "=== CORS Implementation Test Cases ===" << std::endl << std::endl;
    
    // Test case 1: Valid GET request
    std::cout << "Test 1: Valid GET request from allowed origin" << std::endl;
    Task130::CORSResponse resp1 = Task130::handleCORS("https://example.com", "GET");
    Task130::printResponse(resp1);
    
    // Test case 2: Valid OPTIONS preflight request
    std::cout << "Test 2: Valid OPTIONS preflight request" << std::endl;
    Task130::CORSResponse resp2 = Task130::handleCORS(
        "https://app.example.com", "OPTIONS", "Content-Type, Authorization"
    );
    Task130::printResponse(resp2);
    
    // Test case 3: Invalid origin
    std::cout << "Test 3: Request from non-allowed origin" << std::endl;
    Task130::CORSResponse resp3 = Task130::handleCORS("https://malicious.com", "GET");
    Task130::printResponse(resp3);
    
    // Test case 4: Invalid method
    std::cout << "Test 4: Request with non-allowed method" << std::endl;
    Task130::CORSResponse resp4 = Task130::handleCORS("https://example.com", "TRACE");
    Task130::printResponse(resp4);
    
    // Test case 5: Valid POST request from localhost
    std::cout << "Test 5: Valid POST request from localhost" << std::endl;
    Task130::CORSResponse resp5 = Task130::handleCORS(
        "http://localhost:3000", "POST", "Content-Type"
    );
    Task130::printResponse(resp5);
    
    return 0;
}
