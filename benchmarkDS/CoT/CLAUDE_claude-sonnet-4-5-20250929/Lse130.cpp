
#include <iostream>
#include <string>
#include <map>
#include <regex>
#include <algorithm>

class Lse130 {
private:
    static const std::regex SAFE_NAME_PATTERN;
    
    static std::string sanitizeInput(const std::string& input) {
        if (input.empty()) {
            return "";
        }
        
        // Validate against allowed pattern
        if (!std::regex_match(input, SAFE_NAME_PATTERN)) {
            return "";
        }
        
        // HTML escape to prevent XSS
        std::string result = input;
        std::string temp;
        
        for (char c : result) {
            switch (c) {
                case '&': temp += "&amp;"; break;
                case '<': temp += "&lt;"; break;
                case '>': temp += "&gt;"; break;
                case '"': temp += "&quot;"; break;
                case '\\'': temp += "&#x27;"; break;
                default: temp += c;
            }
        }
        
        return temp;
    }

public:
    class Request {
    private:
        std::map<std::string, std::string> parameters;
    
    public:
        Request(const std::map<std::string, std::string>& params) : parameters(params) {}
        
        std::string getParameter(const std::string& key) const {
            auto it = parameters.find(key);
            return (it != parameters.end()) ? it->second : "";
        }
    };
    
    class Environment {
    private:
        std::string templateStr;
        Request request;
    
    public:
        Environment(const std::string& tmpl, const Request& req) 
            : templateStr(tmpl), request(req) {}
        
        std::string getTemplate() const { return templateStr; }
        Request getRequest() const { return request; }
    };
    
    class Response {
    private:
        std::string content;
    
    public:
        Response(const std::string& cnt) : content(cnt) {}
        std::string getContent() const { return content; }
    };
    
    static Response renderResponseFromEnv(const Environment& env) {
        // Security: Validate and sanitize name parameter
        std::string name = env.getRequest().getParameter("name");
        std::string safeName = sanitizeInput(name);
        
        if (safeName.empty()) {
            safeName = "Guest";
        }
        
        // Security: Use safe template rendering
        std::string templateStr = env.getTemplate();
        if (templateStr.empty()) {
            templateStr = "Hello, {name}!";
        }
        
        // Security: Safe template rendering with escaped content
        size_t pos = templateStr.find("{name}");
        if (pos != std::string::npos) {
            templateStr.replace(pos, 6, safeName);
        }
        
        return Response(templateStr);
    }
    
    static Response home(const Environment& env) {
        return renderResponseFromEnv(env);
    }
};

const std::regex Lse130::SAFE_NAME_PATTERN("^[a-zA-Z0-9_\\\\s-]{1,50}$");

int main() {
    // Test Case 1: Normal valid name
    std::map<std::string, std::string> params1 = {{"name", "John"}};
    Lse130::Environment env1("Hello, {name}!", Lse130::Request(params1));
    std::cout << "Test 1: " << Lse130::home(env1).getContent() << std::endl;
    
    // Test Case 2: Name with XSS attempt
    std::map<std::string, std::string> params2 = {{"name", "<script>alert('XSS')</script>"}};
    Lse130::Environment env2("Welcome, {name}!", Lse130::Request(params2));
    std::cout << "Test 2: " << Lse130::home(env2).getContent() << std::endl;
    
    // Test Case 3: Empty name parameter
    std::map<std::string, std::string> params3;
    Lse130::Environment env3("Hi, {name}!", Lse130::Request(params3));
    std::cout << "Test 3: " << Lse130::home(env3).getContent() << std::endl;
    
    // Test Case 4: Name with special characters
    std::map<std::string, std::string> params4 = {{"name", "O'Brien"}};
    Lse130::Environment env4("Greetings, {name}!", Lse130::Request(params4));
    std::cout << "Test 4: " << Lse130::home(env4).getContent() << std::endl;
    
    // Test Case 5: Name exceeding length limit
    std::map<std::string, std::string> params5 = {{"name", std::string(100, 'A')}};
    Lse130::Environment env5("Hello, {name}!", Lse130::Request(params5));
    std::cout << "Test 5: " << Lse130::home(env5).getContent() << std::endl;
    
    return 0;
}
