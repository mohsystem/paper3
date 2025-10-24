
#include <iostream>
#include <string>
#include <map>
#include <set>
#include <sstream>

using namespace std;

class Task130 {
private:
    static set<string> getAllowedOrigins() {
        return {"http://localhost:3000", "http://example.com", "https://example.com"};
    }
    
    static set<string> getAllowedMethods() {
        return {"GET", "POST", "PUT", "DELETE", "OPTIONS"};
    }
    
    static set<string> getAllowedHeaders() {
        return {"Content-Type", "Authorization", "X-Requested-With"};
    }
    
    static string joinSet(const set<string>& s, const string& delimiter) {
        string result;
        for (auto it = s.begin(); it != s.end(); ++it) {
            if (it != s.begin()) result += delimiter;
            result += *it;
        }
        return result;
    }

public:
    static map<string, string> handleCORS(const string& origin, const string& method, const string& requestHeaders) {
        map<string, string> headers;
        set<string> allowedOrigins = getAllowedOrigins();
        set<string> allowedMethods = getAllowedMethods();
        set<string> allowedHeaders = getAllowedHeaders();
        
        if (!origin.empty() && allowedOrigins.find(origin) != allowedOrigins.end()) {
            headers["Access-Control-Allow-Origin"] = origin;
        }
        
        headers["Access-Control-Allow-Methods"] = joinSet(allowedMethods, ", ");
        headers["Access-Control-Allow-Headers"] = joinSet(allowedHeaders, ", ");
        headers["Access-Control-Max-Age"] = "3600";
        headers["Access-Control-Allow-Credentials"] = "true";
        
        return headers;
    }
    
    static string formatResponse(int statusCode, const string& statusText, const map<string, string>& headers, const string& body) {
        ostringstream response;
        response << "HTTP/1.1 " << statusCode << " " << statusText << "\\n";
        
        for (const auto& header : headers) {
            response << header.first << ": " << header.second << "\\n";
        }
        
        response << "Content-Type: application/json\\n";
        response << "Content-Length: " << body.length() << "\\n\\n";
        response << body;
        
        return response.str();
    }
    
    static string handleAPIRequest(const string& origin, const string& method, const string& requestHeaders, const string& path) {
        set<string> allowedOrigins = getAllowedOrigins();
        set<string> allowedMethods = getAllowedMethods();
        
        map<string, string> corsHeaders = handleCORS(origin, method, requestHeaders);
        
        if (method == "OPTIONS") {
            return formatResponse(204, "No Content", corsHeaders, "");
        }
        
        if (!origin.empty() && allowedOrigins.find(origin) == allowedOrigins.end()) {
            return formatResponse(403, "Forbidden", corsHeaders, "{\\"error\\": \\"Origin not allowed\\"}");
        }
        
        if (allowedMethods.find(method) == allowedMethods.end()) {
            return formatResponse(405, "Method Not Allowed", corsHeaders, "{\\"error\\": \\"Method not allowed\\"}");
        }
        
        string responseBody;
        if (path == "/api/users") {
            responseBody = "{\\"users\\": [{\\"id\\": 1, \\"name\\": \\"John\\"}, {\\"id\\": 2, \\"name\\": \\"Jane\\"}]}";
        } else if (path == "/api/data") {
            responseBody = "{\\"data\\": \\"Sample data from API\\"}";
        } else {
            responseBody = "{\\"message\\": \\"API endpoint not found\\"}";
        }
        
        return formatResponse(200, "OK", corsHeaders, responseBody);
    }
};

int main() {
    cout << "Test Case 1: Valid GET request from allowed origin" << endl;
    cout << Task130::handleAPIRequest("http://localhost:3000", "GET", "Content-Type", "/api/users") << endl;
    cout << "\\n" << string(80, '=') << "\\n" << endl;
    
    cout << "Test Case 2: OPTIONS preflight request" << endl;
    cout << Task130::handleAPIRequest("http://example.com", "OPTIONS", "Content-Type", "/api/data") << endl;
    cout << "\\n" << string(80, '=') << "\\n" << endl;
    
    cout << "Test Case 3: Request from disallowed origin" << endl;
    cout << Task130::handleAPIRequest("http://malicious.com", "GET", "Content-Type", "/api/users") << endl;
    cout << "\\n" << string(80, '=') << "\\n" << endl;
    
    cout << "Test Case 4: Invalid HTTP method" << endl;
    cout << Task130::handleAPIRequest("http://localhost:3000", "TRACE", "Content-Type", "/api/data") << endl;
    cout << "\\n" << string(80, '=') << "\\n" << endl;
    
    cout << "Test Case 5: POST request to valid endpoint" << endl;
    cout << Task130::handleAPIRequest("https://example.com", "POST", "Content-Type, Authorization", "/api/data") << endl;
    
    return 0;
}
