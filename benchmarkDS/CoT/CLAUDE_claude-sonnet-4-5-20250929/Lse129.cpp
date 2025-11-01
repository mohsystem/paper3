
#include <iostream>
#include <string>
#include <map>
#include <regex>
#include <algorithm>

class Lse129 {
public:
    class Environment {
    private:
        std::map<std::string, std::string> templates;
    
    public:
        Environment() {
            templates["greeting"] = "Hello, {{name}}! Welcome to our application.";
            templates["default"] = "Welcome, {{name}}!";
        }
        
        std::string getTemplate(const std::string& templateName) {
            auto it = templates.find(templateName);
            if (it != templates.end()) {
                return it->second;
            }
            return templates["default"];
        }
    };
    
    class Request {
    private:
        std::map<std::string, std::string> parameters;
    
    public:
        void setParameter(const std::string& key, const std::string& value) {
            parameters[key] = value;
        }
        
        std::string getParameter(const std::string& key) {
            auto it = parameters.find(key);
            if (it != parameters.end()) {
                return it->second;
            }
            return "";
        }
    };
    
    class Response {
    private:
        std::string content;
    
    public:
        Response(const std::string& content) : content(content) {}
        
        std::string getContent() const {
            return content;
        }
    };
    
    static std::string sanitizeInput(const std::string& input) {
        if (input.empty()) {
            return "Guest";
        }
        
        std::string trimmed = input;
        trimmed.erase(0, trimmed.find_first_not_of(" \\t\\n\\r\\f\\v"));
        trimmed.erase(trimmed.find_last_not_of(" \\t\\n\\r\\f\\v") + 1);
        
        std::regex safePattern("^[a-zA-Z0-9_\\\\s-]{1,50}$");
        if (!std::regex_match(trimmed, safePattern)) {
            return "Guest";
        }
        
        std::string result = trimmed;
        std::string dangerous = "<>\\"'&";
        for (char c : dangerous) {
            result.erase(std::remove(result.begin(), result.end(), c), result.end());
        }
        
        return result;
    }
    
    static std::string escapeHtml(const std::string& input) {
        std::string result = input;
        
        size_t pos = 0;
        while ((pos = result.find("&", pos)) != std::string::npos) {
            result.replace(pos, 1, "&amp;");
            pos += 5;
        }
        
        pos = 0;
        while ((pos = result.find("<", pos)) != std::string::npos) {
            result.replace(pos, 1, "&lt;");
            pos += 4;
        }
        
        pos = 0;
        while ((pos = result.find(">", pos)) != std::string::npos) {
            result.replace(pos, 1, "&gt;");
            pos += 4;
        }
        
        pos = 0;
        while ((pos = result.find("\\"", pos)) != std::string::npos) {
            result.replace(pos, 1, "&quot;");
            pos += 6;
        }
        
        pos = 0;
        while ((pos = result.find("'", pos)) != std::string::npos) {
            result.replace(pos, 1, "&#x27;");
            pos += 6;
        }
        
        return result;
    }
    
    static Response renderResponseFromEnv(Environment& env, Request& request) {
        try {
            std::string name = request.getParameter("name");
            name = sanitizeInput(name);
            
            std::string templateStr = env.getTemplate("greeting");
            
            size_t pos = templateStr.find("{{name}}");
            if (pos != std::string::npos) {
                templateStr.replace(pos, 8, escapeHtml(name));
            }
            
            return Response(templateStr);
        } catch (...) {
            return Response("Error processing request");
        }
    }
    
    static Response home(Environment& env, Request& request) {
        return renderResponseFromEnv(env, request);
    }
};

int main() {
    Lse129::Environment env;
    
    // Test case 1: Valid name
    Lse129::Request req1;
    req1.setParameter("name", "John");
    std::cout << "Test 1: " << Lse129::home(env, req1).getContent() << std::endl;
    
    // Test case 2: Name with spaces
    Lse129::Request req2;
    req2.setParameter("name", "Jane Doe");
    std::cout << "Test 2: " << Lse129::home(env, req2).getContent() << std::endl;
    
    // Test case 3: Empty name
    Lse129::Request req3;
    req3.setParameter("name", "");
    std::cout << "Test 3: " << Lse129::home(env, req3).getContent() << std::endl;
    
    // Test case 4: Name with special characters (should be sanitized)
    Lse129::Request req4;
    req4.setParameter("name", "<script>alert('xss')</script>");
    std::cout << "Test 4: " << Lse129::home(env, req4).getContent() << std::endl;
    
    // Test case 5: Name with invalid characters
    Lse129::Request req5;
    req5.setParameter("name", "User@123!");
    std::cout << "Test 5: " << Lse129::home(env, req5).getContent() << std::endl;
    
    return 0;
}
