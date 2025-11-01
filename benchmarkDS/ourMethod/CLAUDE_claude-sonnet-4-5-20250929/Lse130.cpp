
#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <algorithm>
#include <regex>
#include <stdexcept>

// HTML escape function to prevent XSS attacks
std::string escapeHtml(const std::string& input) {
    std::string escaped;
    escaped.reserve(input.length() * 2);
    
    for (char c : input) {
        switch (c) {
            case '<': escaped += "&lt;"; break;
            case '>': escaped += "&gt;"; break;
            case '&': escaped += "&amp;"; break;
            case '"': escaped += "&quot;"; break;
            case '\\'': escaped += "&#x27;"; break;
            case '/': escaped += "&#x2F;"; break;
            default: escaped += c;
        }
    }
    return escaped;
}

// Response class
class Response {
private:
    std::string content;
    int statusCode;
    
public:
    Response(const std::string& content, int statusCode) 
        : content(content), statusCode(statusCode) {}
    
    std::string getContent() const { return content; }
    int getStatusCode() const { return statusCode; }
};

// Request class
class Request {
private:
    std::map<std::string, std::string> params;
    
public:
    explicit Request(const std::map<std::string, std::string>& params) 
        : params(params) {}
    
    std::string getParameter(const std::string& key) const {
        auto it = params.find(key);
        if (it != params.end()) {
            std::string value = it->second;
            // Limit length to prevent DoS
            if (value.length() > 1000) {
                value = value.substr(0, 1000);
            }
            return value;
        }
        return "";
    }
};

// Environment class
class Environment {
private:
    std::shared_ptr<Request> request;
    std::string templateStr;
    
public:
    Environment(std::shared_ptr<Request> request, const std::string& templateStr) 
        : request(request), templateStr(templateStr) {
        // Validate template size
        if (templateStr.length() > 100000) {
            throw std::invalid_argument("Template size exceeds maximum limit");
        }
    }
    
    std::shared_ptr<Request> getRequest() const { return request; }
    std::string getTemplate() const { return templateStr; }
};

// Template engine with XSS protection
class TemplateEngine {
public:
    static std::string render(const std::string& templateStr, 
                             const std::map<std::string, std::string>& context) {
        if (templateStr.empty()) {
            return "";
        }
        
        // Get name from context and escape it
        std::string name;
        auto it = context.find("name");
        if (it != context.end()) {
            name = it->second;
        }
        
        std::string safeName = escapeHtml(name);
        
        // Replace template variable with escaped value
        std::regex pattern(R"(\\{\\{\\s*name\\s*\\}\\})");
        return std::regex_replace(templateStr, pattern, safeName);
    }
};

// Renders response from environment
// Security: Validates inputs, escapes output to prevent XSS
Response renderResponseFromEnv(std::shared_ptr<Environment> env) {
    if (!env) {
        return Response("Invalid environment", 400);
    }
    
    try {
        // Get request - treat all input as untrusted
        auto request = env->getRequest();
        if (!request) {
            return Response("Invalid request", 400);
        }
        
        // Get name parameter from request
        std::string name = request->getParameter("name");
        
        // Get template from environment
        std::string templateStr = env->getTemplate();
        if (templateStr.empty()) {
            return Response("Template not found", 404);
        }
        
        // Render template with sanitized context
        std::map<std::string, std::string> context;
        context["name"] = name;
        std::string rendered = TemplateEngine::render(templateStr, context);
        
        return Response(rendered, 200);
        
    } catch (const std::exception& e) {
        // Log error internally, return generic message
        std::cerr << "Error rendering template: " << e.what() << std::endl;
        return Response("Internal server error", 500);
    }
}

// Home function that calls renderResponseFromEnv
Response home(std::shared_ptr<Environment> env) {
    return renderResponseFromEnv(env);
}

// Test cases
int main() {
    // Test 1: Normal input
    std::map<std::string, std::string> params1 = {{"name", "John"}};
    auto req1 = std::make_shared<Request>(params1);
    auto env1 = std::make_shared<Environment>(req1, "Hello, {{ name }}!");
    Response resp1 = home(env1);
    std::cout << "Test 1: " << resp1.getContent() 
              << " (Status: " << resp1.getStatusCode() << ")" << std::endl;
    
    // Test 2: XSS attempt - should be escaped
    std::map<std::string, std::string> params2 = {{"name", "<script>alert('XSS')</script>"}};
    auto req2 = std::make_shared<Request>(params2);
    auto env2 = std::make_shared<Environment>(req2, "Hello, {{ name }}!");
    Response resp2 = home(env2);
    std::cout << "Test 2: " << resp2.getContent() 
              << " (Status: " << resp2.getStatusCode() << ")" << std::endl;
    
    // Test 3: Empty name parameter
    std::map<std::string, std::string> params3 = {{"name", ""}};
    auto req3 = std::make_shared<Request>(params3);
    auto env3 = std::make_shared<Environment>(req3, "Hello, {{ name }}!");
    Response resp3 = home(env3);
    std::cout << "Test 3: " << resp3.getContent() 
              << " (Status: " << resp3.getStatusCode() << ")" << std::endl;
    
    // Test 4: Missing name parameter
    std::map<std::string, std::string> params4;
    auto req4 = std::make_shared<Request>(params4);
    auto env4 = std::make_shared<Environment>(req4, "Hello, {{ name }}!");
    Response resp4 = home(env4);
    std::cout << "Test 4: " << resp4.getContent() 
              << " (Status: " << resp4.getStatusCode() << ")" << std::endl;
    
    // Test 5: Null environment
    Response resp5 = home(nullptr);
    std::cout << "Test 5: " << resp5.getContent() 
              << " (Status: " << resp5.getStatusCode() << ")" << std::endl;
    
    return 0;
}
