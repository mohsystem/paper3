
#include <iostream>
#include <string>
#include <map>
#include <algorithm>

class Lse131 {
private:
    // Environment class
    class Environment {
    private:
        std::map<std::string, std::string> templates;
    
    public:
        Environment() {
            templates["home.html"] = "Hello, {{name}}! Welcome to our secure application.";
        }
        
        std::string getTemplate(const std::string& templateName) {
            auto it = templates.find(templateName);
            if (it != templates.end()) {
                return it->second;
            }
            return "Template not found";
        }
    };
    
    // Request class
    class Request {
    private:
        std::map<std::string, std::string> parameters;
    
    public:
        Request(const std::string& name) {
            parameters["name"] = name;
        }
        
        std::string getParameter(const std::string& key) {
            auto it = parameters.find(key);
            if (it != parameters.end()) {
                return it->second;
            }
            return "";
        }
    };
    
    // Response class
    class Response {
    private:
        std::string content;
    
    public:
        Response(const std::string& content) : content(content) {}
        
        std::string getContent() const {
            return content;
        }
        
        friend std::ostream& operator<<(std::ostream& os, const Response& resp) {
            os << resp.content;
            return os;
        }
    };
    
    // HTML encode function to prevent XSS
    static std::string htmlEncode(const std::string& input) {
        std::string result;
        result.reserve(input.length() * 1.5);
        
        for (char c : input) {
            switch (c) {
                case '&':  result += "&amp;"; break;
                case '<':  result += "&lt;"; break;
                case '>':  result += "&gt;"; break;
                case '"':  result += "&quot;"; break;
                case '\\'': result += "&#x27;"; break;
                case '/':  result += "&#x2F;"; break;
                default:   result += c; break;
            }
        }
        return result;
    }
    
    // Render template with secure parameter substitution
    static std::string renderTemplate(const std::string& templateStr, 
                                     const std::map<std::string, std::string>& params) {
        std::string result = templateStr;
        for (const auto& pair : params) {
            std::string placeholder = "{{" + pair.first + "}}";
            std::string safeValue = htmlEncode(pair.second);
            
            size_t pos = 0;
            while ((pos = result.find(placeholder, pos)) != std::string::npos) {
                result.replace(pos, placeholder.length(), safeValue);
                pos += safeValue.length();
            }
        }
        return result;
    }

public:
    static Response renderResponseFromEnv(Environment& env, Request& request) {
        // Get name parameter from request with validation
        std::string name = request.getParameter("name");
        
        // Input validation - prevent empty values
        if (name.empty() || name.find_first_not_of(" \\t\\n\\r") == std::string::npos) {
            name = "Guest";
        }
        
        // Length validation to prevent DoS
        if (name.length() > 100) {
            name = name.substr(0, 100);
        }
        
        // Get template from environment
        std::string templateStr = env.getTemplate("home.html");
        
        // Prepare parameters for rendering
        std::map<std::string, std::string> params;
        params["name"] = name;
        
        // Render template with secure parameter substitution
        std::string renderedContent = renderTemplate(templateStr, params);
        
        // Return response with rendered template
        return Response(renderedContent);
    }
    
    static Response home(Environment& env, Request& request) {
        return renderResponseFromEnv(env, request);
    }
    
    // Public type definitions for external use
    using EnvironmentType = Environment;
    using RequestType = Request;
    using ResponseType = Response;
};

int main() {
    Lse131::EnvironmentType env;
    
    // Test case 1: Normal input
    Lse131::RequestType req1("Alice");
    Lse131::ResponseType resp1 = Lse131::home(env, req1);
    std::cout << "Test 1: " << resp1 << std::endl;
    
    // Test case 2: Input with special characters (XSS attempt)
    Lse131::RequestType req2("<script>alert('XSS')</script>");
    Lse131::ResponseType resp2 = Lse131::home(env, req2);
    std::cout << "Test 2: " << resp2 << std::endl;
    
    // Test case 3: Empty input
    Lse131::RequestType req3("");
    Lse131::ResponseType resp3 = Lse131::home(env, req3);
    std::cout << "Test 3: " << resp3 << std::endl;
    
    // Test case 4: Input with HTML tags
    Lse131::RequestType req4("<b>Bob</b>");
    Lse131::ResponseType resp4 = Lse131::home(env, req4);
    std::cout << "Test 4: " << resp4 << std::endl;
    
    // Test case 5: Input with quotes
    Lse131::RequestType req5("O'Reilly & Sons");
    Lse131::ResponseType resp5 = Lse131::home(env, req5);
    std::cout << "Test 5: " << resp5 << std::endl;
    
    return 0;
}
